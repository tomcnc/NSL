# Exercise 3: European option price simulation

This document provides the necessary guidelines for compiling, executing, and analyzing the C++ codes for the third assignment of the Computational Physics Laboratory (LSN).

---

## 0. Introduction

This exercise is dedicated to the simulation of financial asset price, modeled by a **Geometric Brownian Motion (GBM)**, in order to compute the estimated value of **European call and put options**. These values are fondamental for structuring option contracts. The results produced through Monte Carlo simulations are **statistically compared** to the analytical values coming from the **Black-Scholes-Merton (BSM) model**.

---

## 1. Project Structure and Data Flow

The repository root contains the analysis environment and the exercise folder (`es_3`).

### Detailed Directory Tree:

```
LSN_03/
├── LSN_Exercises_03.ipynb          # Jupyter Notebook for consolidated analysis.
├── LSN_Notebook_03.ipynb
├── ReadMe.md
└── es_3                            # Exercise 3 directory
    ├── OUTPUT                      # Repository for raw statistical data (`*.data`).
    ├── main.cpp                    # Primary C++ source code.
    ├── main.exe                    # Executable binary.
    ├── main.o                      # Intermediate object file.
    ├── makefile                    # Build automation script.
    └── plot                        # Repository for graphical output
```


### Execution Workflow:

The analysis notebook (`LSN_Notebook_03.ipynb`) processes data from `es_3/OUTPUT/` directory. Therefore, **all simulations must be executed** before proceeding to the final analysis step.

---

## 2. Maintenance and Cleanup Commands

These commands are available inside (`es_3`) directory for granular file management.

| Command | Function |
| :--- | :--- |
| `make remove_exe` | Removes the executable file (`main.exe`). |
| `make remove_o` | Removes all object files (`*.o`). |
| `make remove_output` | **CAUTION**: Removes all data files (`*.data`) from the local `OUTPUT/` folder. |
| `make remove_plot` | Removes all plot produced during the notebook analysis from the `plot/` directory. |

---

## 3. Operational Guide

All commands in this guide assume you are starting from the **exercise directory** (`LSN_03/es_3`).

### Step 1: Cleanup, compilation and execution

The following commands ensure a **clean build** and **execution**. They first remove old build files (`main.o`, `main.exe`, `*.data`, `*.png` files) and then compile and run.

```bash
make remove_o remove_exe remove_output remove_plot
make
./main.exe
```

### Step 2: Launch the Analysis Notebook

After all data files are successfully generated in the `OUTPUT/`, launch the analysis notebook:

```bash
cd .. && jupyter-notebook LSN_Notebook_03.ipynb
```
