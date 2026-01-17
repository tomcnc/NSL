
# Exercise 4: Molecular Dynamics for a Lennard-Jones NVE Gas Model

This document provides the necessary guidelines for executing and analyzing the code developed for the fourth assignment of the **Computational Physics Laboratory (LSN)** course.

## Prerequisites

The **`NSL_SIMULATOR`** code is dependent on the **Armadillo C++** linear algebra library. Ensure this library is correctly **installed and linked** on your system before proceeding with compilation.

---

## 0. Introduction

This exercise is dedicated to the **Molecular Dynamics (MD)** simulation of a gas phase using a **Lennard-Jones (L-J)** potential model under **NVE (Microcanonical)** ensemble conditions. This system approximates the behavior of a real gas at high temperatures and employs **Periodic Boundary Conditions (PBCs)**, **Minimum Image Convention (MIC)** and **Potential Cut-Off**.

The system is simulated at a reduced density $\rho^* = 0.05$ and a reduced temperature $T^* = 2.0$.

The **primary objectives** of this exercise are:
1.  To **implement the measurement** of the **particle velocity distribution**.
2.  To **demonstrate** how the system, initially prepared in a low-entropic spatial configuration, spontaneously evolves towards a high-entropic configuration (consistent with the **Second Law of Thermodynamics**).
3.  To **analyze** the system's irreversible macroscopic evolution, despite the time-reversible nature of the underlying integration algorithm (e.g., the Verlet algorithm).

---

## 1. Project Structure and Data Flow

The project is organized within the **`LSN_04`** working directory. It contains subdirectories (`es_4.*`) for all input/output data and plots, two **Jupyter Notebooks** (one for the exercise requirements, one for data analysis), the simulator code (`NSL_SIMULATOR`), and a **Bash script** for automation.

| Directory/File | Description |
| :--- | :--- |
| **`LSN_Exercises_04.ipynb`** | **Jupyter Notebook** containing the exercise requests. |
| **`LSN_Notebook_4.ipynb`** | **Jupyter Notebook** dedicated to **data analysis and plotting**. |
| **`NSL_SIMULATOR/`** | The core **MD simulator** (source code and binaries). |
| **`es_4.*`** directories | Subdirectories containing the **input files, output data, and plots** for each part of the exercise. |
| **`run_es4.sh`** | **Bash script** for compilation, execution, cleanup, and file management. |

```
LSN_04
├── LSN_Exercises_04.ipynb
├── LSN_Notebook_4.ipynb                # Jupyter Notebook for data analysis.
├── NSL_SIMULATOR                       # MD simulator code
│   ├── INPUT                           # Input files for the simulator
│   ├── OUTPUT                          # Output files from the simulator
│   └── SOURCE                          # Source code files
├── ReadMe.md
├── ADVANCED_MANUAL.md
├── es_4.1                              # Data and plots for Exercise 4.1
├── es_4.2                              # Data and plots for Exercise 4.2 (Equilibration & Simulation)
├── es_4.3                              # Data and plots for Exercise 4.3 (Time Reversal)
└── run_es4.sh
```

**Note**: Sub-directories and files within INPUT/OUTPUT are omitted for brevity.

### Execution Workflow

The analysis notebook (`LSN_Notebook_4.ipynb`) processes data from the `es_4.*/OUTPUT/` directories. Therefore, **all simulations must be successfully executed** before proceeding to the final analysis step.

---

## 2. Configuration File Structure

### `input.dat`: Simulation Parameters

This file contains configuration parameters necessary to set up the simulation.

| Parameter | Possible Value | Description/Notes |
| :--- | :--- | :--- |
| `SIMULATION_TYPE` | $0/1/2/3$ | $0$: LJ MD (NVE); $1$: LJ MC (NVT); $2$: Ising 1D MC (M(RT)^2); $3$: Ising 1D MC (Gibbs). |
| `DISTRIBUTION_TYPE` | $0/1$ | $0$: **Maxwell-Boltzmann distribution** (initial velocities); $1$: Dirac Delta distribution. |
| `RESTART` | $0/1$ | $0$: Start from initial configuration/velocities; $1$: Restart from previous configuration files. |
| `TEMP` | $\in \mathbb{R}^+$ | The reduced temperature $T^*$. |
| `NPART` | $108$ | **Do not change** (critical for initialization). |
| `RHO` | $\in \mathbb{R}^+$ | The reduced density $\rho^*$. |
| `R_CUT` | $\in \mathbb{R}^+$ | The cut-off radius (depends on the system and box dimension). |
| `DELTA` | $0.001$ | The time step size for the **Verlet algorithm**. |
| `NBLOCKS` | $\in \mathbb{N}$ | Number of blocks for the **blocking method**. |
| `NSTEPS` | $\in \mathbb{N}$ | Number of steps per simulation block. |
| `ENDINPUT` | N/A | Marker indicating the end of the input file. |

### `properties.dat`: Measurable Properties

This file lists the properties the simulator measures and outputs.

| Parameter | Description | Applicable Simulation Type(s) | Implemented |
| :--- | :--- | :--- | :--- |
| `TOTAL_ENERGY` | Average total energy per particle (using blocking) | $0/1/2/3$ | ✅ |
| `TOTAL_ENERGY_STEP` | Istantaneous total energy per particle per step (monitoring thermalization) | $0/1/2/3$ | ✅ |
| `POTENTIAL_ENERGY` | Average potential energy per particle | $0/1$ | ✅ |
| `POTENTIAL_ENERGY_STEP` | Istantaneous potential energy per particle | $0/1$ | ✅ |
| `KINETIC_ENERGY` | Average kinetic energy per particle | $0/1$ | ✅ |
| `KINETIC_ENERGY_STEP` | Istantaneous kinetic energy per particle | $0/1$ | ✅ |
| `TEMPERATURE` | Average temperature | $0/1$ | ✅ |
| `TEMPERATURE_STEP` | Istantaneous temperature | $0/1$ | ✅ |
| `PRESSURE` | Average pressure | $0/1$ | ✅ |
| `PRESSURE_STEP` | Istantaneous pressure | $0/1$ | ✅ |
| `GOFR` | Radial distribution function *g(r)* | $0/1$ | ❌ |
| **`POFV`** | **Velocity probability distribution** | ***0*** | ✅ |
| `MAGNETIZATION` | Magnetization | $2/3$ | ❌ |
| `MAGNETIZATION_STEP` | Istantaneous magnetization | $2/3$ | ❌ |
| `SPECIFIC_HEAT` | Specific Heat | $2/3$ | ❌ |
| `SUSCEPTIBILITY` | Susceptibility | $2/3$ | ❌ |
| `ENDPROPERTIES` | N/A | Marker indicating the end of the properties file. |

### `config.xyz`

This file contains the **Cartesian coordinates** ($x, y, z$) for each particle, expressed in units of the box side (non-reduced units).

---

## 3. Execution Script: `run_es4.sh`

The **`run_es4.sh`** Bash script automates the entire exercise workflow, including compilation, sequential execution of all required parts, and output management.

### 3.1. Usage

Before running the simulation, make sure the script is executable:
```bash
chmod +x run_es4.sh
```

#### Run the simulation
```bash
./run_es4.sh
```

**Script Behavior**: During execution, the script will output status messages indicating which part of the exercise is currently being processed.

### Troubleshooting

⚠️ **IF THE AUTOMATED EXECUTION FAILS:** Detailed instructions for manual compilation, file management, and debugging are provided in the **`ADVANCED_MANUAL.md`** file.

---

## 4. Analysis and Results

After all data files are successfully generated into `es_4.*/OUTPUT/` directories, launch the analysis notebook:

```bash
jupyter-notebook LSN_Notebook_04.ipynb
```