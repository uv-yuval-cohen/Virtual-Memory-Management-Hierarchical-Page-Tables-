//
// Created by Yuval Cohen on 26/07/2024.
//

#ifndef _VIRTUALMEMORYHELPERS_H_
#define _VIRTUALMEMORYHELPERS_H_

#include "PhysicalMemory.h"



uint64_t extractBits(uint64_t value, int start, int count);
void splitVirtualAddress(uint64_t virtualAddress, uint64_t resultArray[]);
uint64_t selectFrame(uint64_t pageToSwapIn, uint64_t frameToExclude);
uint64_t cyclicalDistance(uint64_t page1, uint64_t page2);
void
findEvictionFrame (uint64_t currentFrame, uint64_t depth, uint64_t pageToSwapIn,
                   uint64_t &maxDistance, uint64_t &frameToEvict, uint64_t
                   &parentFrame, uint64_t currentPageIndex, uint64_t&
                   evictedPageIndex);

#endif //_VIRTUALMEMORYHELPERS_H_
