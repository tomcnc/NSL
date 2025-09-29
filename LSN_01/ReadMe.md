## ⚙️ Compilation and Execution Instructions

This section provides the guidelines necessary to compile and run the C++ codes for this exercise set.

### Code Requirements

The C++ code relies on the **Armadillo** library (C++ Linear Algebra Library). Please ensure that this library is correctly installed and configured on your system and that your build environment is set up to link against it.

### Structure and Navigation

Each exercise sub-section is contained within its own directory (e.g., `es_1.1`, `es_1.2`, `es_1.3`). Additional you will find my analysis notebook `LSN_Notebook_01.ipynb`.

Inside each directory, you will find:
* `main.cpp`: The source code for the experiment or simulation.
* `OUTPUT/`: Contains all generated output files (`*.data`).
* `plots/`: Contains the final or intermediate plots generated during the analysis.

**To execute an exercise, navigate to the desired directory (e.g., `cd es./1.1`) and follow the commands below.**

### Execution Commands

| Command | Description |
| :--- | :--- |
| `make` | Compiles `main.cpp` and all source files, creating the executable `main.exe`. |
| `./main.exe` | Executes the program. Output files will be generated in the `OUTPUT` folder. |
| `make remove_o` | Removes all object files (`*.o`) created during compilation. |
| `make remove_exe` | Removes the executable file (`main.exe`). |
| `make remove_output` | **CAUTION:** Removes all data files (`*.data`) from the `OUTPUT` folder. |

Once the data (`*.data`) is generated, please proceed with the results analysis by opening the [notebook](LSN_Notebook_01.ipynb).
