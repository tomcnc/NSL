# Exercise 1: Computational Physics Simulations 💻

This document provides the necessary guidelines for compiling, executing, and analyzing the C++ codes for this multi-part exercise set. The workflow involves independent execution within separate sub-directories, followed by centralized data analysis.

---

## 0. Introduction

This exercise set, **LSN_01**, is dedicated to the fundamental concepts of computational physics, focusing primarily on **random number generators** and their applications, including testing their statistical properties (e.g., Chi-Squared test) and utilizing them to demonstrate principles like the Central Limit Theorem and solve problems such as Buffon's experiment for estimating Pi.

---

## 1. Prerequisites and Dependencies

The C++ source code is dependent on the **Armadillo** C++ Linear Algebra Library.

**Action Required:** Ensure that the Armadillo library is correctly installed on your system and that your build environment (via the provided `makefile`) is configured to link against it.

---

## 2. Project Structure and Data Flow

The repository root contains the analysis environment and the three exercise folders (`es_1.1`, `es_1.2`, `es_1.3`).

### Detailed Directory Tree:

```
LSN_01/
├── LSN_Exercise_01.ipynb
├── LSN_Notebook_01.ipynb       # Primary Jupyter Notebook for consolidated analysis.
├── ReadMe.md
├── es_1.1                      # Exercise 1.1: Random Number Generator Tests
│   ├── OUTPUT                  # Repository for raw statistical data.
│   ├── main.cpp                # Primary C++ source code.
│   ├── main.exe                # Executable binary (build target).
│   ├── main.o                  # Intermediate object file.
│   ├── makefile                # Build automation script.
│   └── plots                   # Repository for graphical output
├── es_1.2                      # Exercise 1.2: Central Limit Theorem (CLT)
│   ├── OUTPUT                  # Repository for raw statistical data.
│   ├── main.cpp                # Primary C++ source code.
│   ├── main.exe                # Executable binary (build target).
│   ├── main.o                  # Intermediate object file.
│   ├── makefile                # Build automation script.
│   └── plots                   # Repository for graphical output
└── es_1.3                      # Exercise 1.3: Buffon's Experiment (Pi estimation)
    ├── OUTPUT                  # Repository for raw statistical data.
    ├── main.cpp                # Primary C++ source code.
    ├── main.exe                # Executable binary (build target).
    ├── main.o                  # Intermediate object file.
    ├── makefile                # Build automation script.
    └── plots                   # Repository for graphical output
```

### Execution Workflow:

The analysis notebook (`LSN_Notebook_01.ipynb`) processes data from all three sub-directories. Therefore, **all simulations must be executed sequentially** before proceeding to the final analysis step.

---

## 3. Operational Guide

All commands in this guide assume you are starting from the **parent directory** (`LSN_EXERCISES/LSN_01`).

### Step 1: Run All Simulations Sequentially

The following commands ensure a **clean build** and **execution** for each exercise. They first remove old build files (`main.o`, `main.exe`, `*.data` files) and then compile and run.

```bash
# Clean, Compile, and Run es_1.1 (Random Number Tests)
cd es_1.1 && make remove_o remove_exe remove_output && make && ./main.exe
cd ..

# Clean, Compile, and Run es_1.2 (Central Limit Theorem)
cd es_1.2 && make remove_o remove_exe remove_output && make && ./main.exe
cd ..

# Clean, Compile, and Run es_1.3 (Buffon's Experiment)
cd es_1.3 && make remove_o remove_exe remove_output && make && ./main.exe
cd ..
```

### Step 2: Launch the Analysis Notebook

After all data files are successfully generated, launch the primary analysis notebook directly from the parent directory:

```bash
# Launches the LSN_Notebook_01.ipynb for data processing
jupyter-notebook LSN_Notebook_01.ipynb
```

## 4. Maintenance and Cleanup Commands

These commands are available inside any specific sub-directory (`es_1.x`) for granular file management.

| Command (Must run inside `es_1.x`) | Function |
| :--- | :--- |
| `make remove_exe` | Removes the executable file (`main.exe`). |
| `make remove_o` | Removes all object files (`*.o`). |
| `make remove_output` | **CAUTION**: Removes all data files (`*.data`) from the local `OUTPUT/` folder. |
