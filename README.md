# Virtual Memory Management

## 📌 Overview
This project implements a **virtual memory management system** using hierarchical page tables and a simulated physical memory. The system allows reading and writing to a virtual address space, handling **page faults**, and managing **page eviction** based on a cyclical distance algorithm.

---

## ✨ Features
✔️ Implements a **hierarchical page table structure**  
✔️ Handles **page faults** and allocates new frames dynamically  
✔️ Evicts pages using a **cyclical distance-based algorithm**  
✔️ Uses **simulated physical memory** for page storage and retrieval  

---

## 📂 File Structure
📌 `VirtualMemory.cpp` - Implements the virtual memory functions including `VMread` and `VMwrite`.  
📌 `VirtualMemory.h` - Declares the virtual memory functions.  
📌 `VirtualMemoryHelpers.h` - Contains helper functions for virtual memory operations.  
📌 `PhysicalMemory.cpp` - Implements functions for managing the simulated physical memory.  
📌 `PhysicalMemory.h` - Declares functions for accessing physical memory.  
📌 `MemoryConstants.h` - Defines constants for memory sizes, page sizes, and table depths.  
📌 `SimpleTest.cpp` - A basic test program to verify memory operations.  
📌 `Makefile` - Builds the project and compiles it into a library.  

---

## ⚙️ Installation and Compilation

1️⃣ Clone the repository:
   ```bash
   git clone <repository-url>
   cd <repository-name>
   ```

2️⃣ Compile the project using the provided Makefile:
   ```bash
   make
   ```
   This will generate a `libVirtualMemory.a` static library.

---

## 🚀 Usage

To use the virtual memory API, include `VirtualMemory.h` in your program and call the following functions:

### 🔹 Initialize the Virtual Memory System
```cpp
VMinitialize();
```

### 🔹 Read a Value from Virtual Memory
```cpp
word_t value;
if (VMread(virtualAddress, &value)) {
    // Successfully read value
}
```

### 🔹 Write a Value to Virtual Memory
```cpp
if (VMwrite(virtualAddress, value)) {
    // Successfully wrote value
}
```

---

## 🧪 Example Test Run
To test the implementation, run `SimpleTest.cpp`:
```bash
./simple_test
```
📌 **Expected Output:**
```
writing to 0
writing to 1
...
reading from 0 0
reading from 1 1
...
success
```

---

## 🏗️ Design Details

🔹 **Hierarchical Page Tables:**
   - Each virtual address is split into multiple levels, where each level points to the next.
   - The root page table is always located in **frame 0**.

🔹 **Page Fault Handling:**
   - If a required page is not in memory, it is loaded into an **available frame**.
   - If no frames are free, an existing page is evicted using a **cyclical distance strategy**.

🔹 **Eviction Strategy:**
   - If a frame must be freed, the page with the **maximum cyclical distance** is selected for eviction.
   - The evicted page is saved to disk before reusing its frame.

---

## 🤝 Contribution
Contributions are welcome! Feel free to **submit issues** or **create pull requests** to improve this project. 😊

---

## 📜 License
This project is licensed under the **MIT License**.
