//
// Created by yuval_cohen on 7/24/24.
//
#include "VirtualMemory.h"
#include "PhysicalMemory.h"
#include "VirtualMemoryHelpers.h"
#include <algorithm>  // For std::min
#include <cstdlib>    // For std::abs


// Extract 'count' bits starting from index 'start'
uint64_t extractBits (uint64_t value, int start, int count)
{
  return (value >> start) & ((1LL << count) - 1);
}

void VMinitialize ()
{
  // Clear the root page table (frame 0)
  for (int i = 0; i < PAGE_SIZE; ++i)
  {
    PMwrite (i, 0);
  }
}

void splitVirtualAddress (uint64_t virtualAddress, uint64_t resultArray[])
{
  int remainingBits = VIRTUAL_ADDRESS_WIDTH - OFFSET_WIDTH;

  // Extract the offset and store it in the last position of the array
  resultArray[TABLES_DEPTH] = extractBits (virtualAddress, 0, OFFSET_WIDTH);

  int i = 1;
  while (remainingBits >= OFFSET_WIDTH)
  {
    resultArray[TABLES_DEPTH - i] = extractBits (virtualAddress, i
                                                                 * OFFSET_WIDTH, OFFSET_WIDTH);
    remainingBits -= OFFSET_WIDTH;
    i++;
  }

  if (remainingBits > 0)
  {
    // Extract the remaining bits and store them in the first position of the array
    resultArray[0] = extractBits (virtualAddress,
                                  i * OFFSET_WIDTH, remainingBits);
  }
}

int VMwrite (uint64_t virtualAddress, word_t value)
{
  // Check if the virtual address is within the valid range
  if (virtualAddress >= VIRTUAL_MEMORY_SIZE)
  {
    return 0; // Invalid virtual address
  }

  uint64_t currentFrame = 0; // Start at the root frame
  word_t frameValue;
  uint64_t pageNum = virtualAddress >> OFFSET_WIDTH; // Extract page number

  //split the V address
  uint64_t splitAddress[TABLES_DEPTH + 1];
  splitVirtualAddress (virtualAddress, splitAddress);

  for (int i = 0; i < (TABLES_DEPTH); i++)
  {
    PMread (currentFrame * PAGE_SIZE + splitAddress[i], &frameValue);

    if (frameValue == 0)
    {

      // Page fault handling: select a new frame and update the table
      uint64_t newFrame = selectFrame (pageNum, currentFrame);
      if (newFrame == NUM_FRAMES)
      {
        return 0; // Failed to allocate a new frame
      }
      if (i + 1 < TABLES_DEPTH)
      {
        // Initialize the frame with zeros after eviction
        for (uint64_t j = 0; j < PAGE_SIZE; ++j)
        {
          PMwrite (newFrame * PAGE_SIZE + j, 0);
        }
      }
      else
      {
        PMrestore (newFrame, pageNum);
      }
      PMwrite (currentFrame * PAGE_SIZE + splitAddress[i], newFrame);
      currentFrame = newFrame;
    }
    else
    {
      currentFrame = frameValue;
    }

  }
  // Write the value to the correct offset in the final frame
  PMwrite (currentFrame * PAGE_SIZE + splitAddress[TABLES_DEPTH], value);
  return 1;
}

int VMread (uint64_t virtualAddress, word_t *value)
{
  // Check if the virtual address is within the valid range
  if (virtualAddress >= VIRTUAL_MEMORY_SIZE)
  {
    return 0; // Invalid virtual address
  }

  uint64_t currentFrame = 0; // Start at the root frame
  word_t frameValue;
  uint64_t pageNum = virtualAddress >> OFFSET_WIDTH; // Extract page number

  // Split the virtual address
  uint64_t splitAddress[TABLES_DEPTH + 1];
  splitVirtualAddress (virtualAddress, splitAddress);

  for (int i = 0; i < TABLES_DEPTH; i++)
  {
    PMread (currentFrame * PAGE_SIZE + splitAddress[i], &frameValue);

    if (frameValue == 0)
    {
      // Page fault handling: select a new frame and update the table
      uint64_t newFrame = selectFrame (pageNum, currentFrame);
      if (newFrame == NUM_FRAMES)
      {
        return 0; // Failed to allocate a new frame
      }

      if (i + 1 < TABLES_DEPTH)
      {
        // Initialize the frame with zeros if it's a page table
        for (uint64_t j = 0; j < PAGE_SIZE; ++j)
        {
          PMwrite (newFrame * PAGE_SIZE + j, 0);
        }
      }
      else
      {
        // Restore the page if it's the final level
        PMrestore (newFrame, pageNum);
      }

      // Update the table entry to point to the new frame
      PMwrite (currentFrame * PAGE_SIZE + splitAddress[i], newFrame);
      currentFrame = newFrame;
    }
    else
    {
      currentFrame = frameValue;
    }
  }

  // Read the value from the correct offset in the final frame
  PMread (currentFrame * PAGE_SIZE + splitAddress[TABLES_DEPTH], value);
  return 1;
}

uint64_t findFrame (uint64_t currentFrame, uint64_t depth, uint64_t &
maxFrameIndex, bool &foundEmptyTable, uint64_t excludeFrame, uint64_t
prevFrame, uint64_t &parentFrame)
{

  word_t value; // Variable to store the value read from the physical memory.
  bool isEmptyTable = true; // Flag to check if the current table is empty.

  // Iterate through each entry in the current table/frame.
  for (uint64_t i = 0; i < PAGE_SIZE; ++i)
  {
    // Read the value from the current frame's entry.
    PMread ((currentFrame) * PAGE_SIZE + i, &value);
    // If the entry is not empty, update the flags and maxFrameIndex.
    if (value != 0)
    {
      isEmptyTable = false; // The table is not empty.

      // Update the maximum frame index encountered so far.
      if (static_cast<uint64_t>(value) > maxFrameIndex)
      {
        maxFrameIndex = value;
      }

      // If not at the maximum depth, recursively check the next level.
      if (depth + 1 < TABLES_DEPTH)
      {
        uint64_t result = findFrame (value, depth + 1, maxFrameIndex,
                                     foundEmptyTable, excludeFrame,
                                     currentFrame, parentFrame);

        // If an empty table was found  a suitable frame was found in the
        // recursion, return the result.
        if (foundEmptyTable || (result != NUM_FRAMES && result !=
                                                        excludeFrame))
        {
          return result;
        }
      }
    }
  }
  // If the current table is empty, mark it and return its frame index.
  if (isEmptyTable && currentFrame != 0 && currentFrame != excludeFrame)
  {
    foundEmptyTable = true;
    parentFrame = prevFrame;
    return currentFrame;
  }
  // Return NUM_FRAMES to indicate no suitable frame was found in this branch.
  return NUM_FRAMES;
}

uint64_t selectFrame (uint64_t pageToSwapIn, uint64_t frameToExclude)
{
  uint64_t maxFrameIndex = 0; // Variable to keep track of the maximum frame index encountered.
  bool foundEmptyTable = false; // Flag to indicate if an empty table is found.
  uint64_t selectedFrame = NUM_FRAMES; // Initialize selected frame to an invalid value.
  uint64_t parentFrame = 0;

  // First priority: Look for a frame containing an empty table.
  selectedFrame = findFrame (0, 0, maxFrameIndex, foundEmptyTable,
                             frameToExclude, 0, parentFrame);
  if (foundEmptyTable)
  {
    // Update the parent frame to reflect the eviction
    for (uint64_t i = 0; i < PAGE_SIZE; ++i) {
      word_t value;
      PMread(parentFrame * PAGE_SIZE + i, &value);
      if (static_cast<uint64_t>(value) == selectedFrame) {
        PMwrite(parentFrame * PAGE_SIZE + i, 0);
        break;
      }
    }
    return selectedFrame; // Return the frame if an empty table is found.

  }

  // Second priority: Look for an unused frame.
  if (maxFrameIndex + 1 < NUM_FRAMES)
  {
    return maxFrameIndex + 1; // Return the next unused frame.
  }

  // Third priority: Evict a page based on cyclical distance.
  uint64_t maxDistance = 0;
  uint64_t frameToEvict = 0;
  parentFrame = 0;
  uint64_t evictedPageIndex = 0;
  uint64_t currentPageIndex = 0;
  findEvictionFrame(0, 0, pageToSwapIn, maxDistance, frameToEvict,
                    parentFrame,currentPageIndex, evictedPageIndex);
  // Update the parent frame to reflect the eviction
  for (uint64_t i = 0; i < PAGE_SIZE; ++i) {
    word_t value;
    PMread(parentFrame * PAGE_SIZE + i, &value);
    if (static_cast<uint64_t>(value) == frameToEvict) {
      PMwrite(parentFrame * PAGE_SIZE + i, 0);
      break;
    }
  }
  // Evict the selected page and return the frame
  PMevict (frameToEvict, evictedPageIndex);

  return frameToEvict;
}

// Helper function to calculate cyclical distance
uint64_t cyclicalDistance(uint64_t page1, uint64_t page2)
{
  int64_t diff = (int64_t)page1 - (int64_t)page2;
  uint64_t abs_diff = (diff < 0) ? -diff : diff;
  uint64_t cyclical_diff = NUM_PAGES - abs_diff;
  return (cyclical_diff < abs_diff) ? cyclical_diff : abs_diff;
}

void
findEvictionFrame (uint64_t currentFrame, uint64_t depth, uint64_t pageToSwapIn,
                   uint64_t &maxDistance, uint64_t &frameToEvict, uint64_t
                   &parentFrame, uint64_t currentPageIndex, uint64_t
                   &evictedPageIndex)
{
  word_t value;

  // Iterate through each entry in the current table/frame.
  for (uint64_t i = 0; i < PAGE_SIZE; ++i)
  {
    PMread (currentFrame * PAGE_SIZE + i, &value);
    if (value !=0){
      if (depth + 1 < TABLES_DEPTH)
      {
        // Recurse to the next level
        findEvictionFrame (value, depth
                                  + 1, pageToSwapIn, maxDistance,
                           frameToEvict, parentFrame,
                           (currentPageIndex << OFFSET_WIDTH) | i,
                            (evictedPageIndex));
      }
      else
      {
        // We are at the leaf level, calculate the cyclical distance
        uint64_t distance = cyclicalDistance (pageToSwapIn,   (currentPageIndex << OFFSET_WIDTH) | i);
        if (distance > maxDistance)
        {
          maxDistance = distance;
          frameToEvict = value;
          parentFrame = currentFrame;
          evictedPageIndex =   (currentPageIndex << OFFSET_WIDTH) | i;
        }
      }
    }
  }
}


