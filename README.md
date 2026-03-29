#  📂  Virtual File System (Ext2 Simulator)

A robust, interactive **C++ simulator** for an Ext2-like file system.  
This project demonstrates core **Operating System concepts** by implementing a custom file system from scratch.

It includes its own:
- Virtual Disk
- Superblocks
- Inode & Block Managers
- Directory Structures  

Also features  **custom logging**.

---

## 🚀 Features

### 💽 Core System
- **Virtual Disk Operations**  
  Initializes, formats, and loads file systems onto a virtual disk file.

- **Hierarchical Directory Structure**  
  Full support for directories and subdirectories, along with absolute and relative path navigation.

- **File Management**  
  Create, read, write, copy, and move files seamlessly across directories.

- **Resource Management**  
  Custom bitmapped allocations for Inodes and Data Blocks.

---

### ⚙️ Advanced

- **🖥️ Built-in Shell**  
  Interactive CLI for managing the file system, with logging and statistics.

---

## 📁 Project Structure

```text
virtual-file-system/
├── code/
│   ├── include/       # C++ header files (architecture & managers)
│   ├── src/           # C++ source code files (implementation)
├── build/             # Output directory for compiled executables
├── doc/               # Presentations and project documentation PDFs
├── resources/         # Static resources or assets needed for the project
└── utils/             # Helper utilities and text generators
```

---

## 🧾 Available Commands

### 📌 System & Navigation

| Command | Description |
|--------|------------|
| `format` | Formats the virtual disk and resets the file system |
| `load` | Loads an existing file system |
| `info` | Displays disk statistics |
| `ls` | Lists directory contents |
| `cd <path>` | Changes directory (supports `..` and relative paths) |
| `pwd` | Shows current directory |
| `help` | Lists all commands |

---

### 📂 File & Directory Management

| Command | Description |
|--------|------------|
| `mkdir <name>` | Creates a directory |
| `touch <file>` | Creates an empty file |
| `read <file>` | Reads file contents |
| `write <file> <text>` | Writes text to file |
| `cp <src> <dest>` | Copies a file |
| `mv <src> <dest>` | Moves or renames |
| `rm <path>` | Deletes file or empty directory |
| `find <name> [-f /-d]` | Searches files or directories |

---

### ⚡ Advanced Features

| Command | Description |
|--------|------------|
| `log [clear]` | Shows or clears logs |

---

## 🛠️ Getting Started

### 📦 Prerequisites
- C++11 compatible compiler (GCC / MinGW / Clang)
- Make or CMake (optional)

---

### Building the Project

You can manually build the project using `g++`. From the root `code` directory, run:

```bash
cd code
g++ -std=c++11 -I include src/*.cpp -o ../build/ext2sim.exe
```

*Note: Ensure the `build` directory exists prior to running the compiler.*

### Running the Simulator

Execute the output binary to start the interactive file system shell:

```bash
cd build
./ext2sim.exe
```
*(On Windows, you can simply run `ext2sim.exe`)*

