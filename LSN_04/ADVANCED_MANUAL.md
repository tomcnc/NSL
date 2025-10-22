# ADVANCED_MANUAL.md: Advanced Manual Execution and Troubleshooting Guide

This document provides step-by-step instructions for manually executing the entire simulation batch. This procedure should only be followed if the automated script (`run_es4.sh`) fails or if advanced debugging is required.

**Warning**: This manual process is complex and error-prone due to the necessary sequential file movements and parameter changes. Proceed with caution.

The primary working directory for compilation and execution is `LSN_04/NSL_SIMULATOR/SOURCE`.

***

## 1. Initial Setup and Compilation

Start from the `LSN_04` directory.

### 1.1 Global Cleanup and Configuration Reset

- Enter the source directory:
```bash
cd NSL_SIMULATOR/SOURCE
```
- Remove compiled files and old output from the simulator directory
```bash
make remove
```

- Navigate back to the main directory:
```bash
cd ../..
```

- Delete all archived data and plots (Use with caution!):
```bash
rm -rf es_4.1/OUTPUT es_4.1/INPUT es_4.1/plot/*
rm -rf es_4.2/equilibration/OUTPUT es_4.2/equilibration/INPUT es_4.2/equilibration/plot/*
rm -rf es_4.2/simulation/OUTPUT es_4.2/simulation/INPUT es_4.2/simulation/plot/*
rm -rf es_4.3/back_1/OUTPUT es_4.3/back_1/INPUT es_4.3/back_1/plot/*
rm -rf es_4.3/back_2/OUTPUT es_4.3/back_2/INPUT es_4.3/back_2/plot/*
```

- Reset initial configuration file (using FCC structure):
```bash
rm NSL_SIMULATOR/INPUT/CONFIG/conf-1.xyz
cp NSL_SIMULATOR/INPUT/CONFIG/config.fcc NSL_SIMULATOR/INPUT/CONFIG/config.xyz
```

- Return to source directory for compilation/execution:
```bash
cd NSL_SIMULATOR/SOURCE
```

### 1.2. Compilation

Compile and remove `*.o` files:

```bash
make
make clean_o
```

## 2. Exercise 4.1

### 2.1. First Run ($T^* = 2.0, Maxwell-Boltzmann)

- Configure Input Files: Manually edit `../INPUT/input.dat` and `../INPUT/properties.dat` copying the following configurations:

`input.dat`:
```
SIMULATION_TYPE 0 
DISTRIBUTION_TYPE 0 
RESTART 0 
TEMP 2.0 
NPART 108 
RHO 0.05 
R_CUT 5.0 
DELTA 0.001 
NBLOCKS 20 
NSTEPS 2000  

ENDINPUT
```

`properties.dat`:
```
TOTAL_ENERGY
TOTAL_ENERGY_STEP
POTENTIAL_ENERGY
POTENTIAL_ENERGY_STEP
KINETIC_ENERGY
KINETIC_ENERGY_STEP
TEMPERATURE
TEMPERATURE_STEP
PRESSURE
PRESSURE_STEP
POFV 30
 
ENDPROPERTIES
```

- Execute Simulation:
```bash
./simulator.exe
```

- Archive Data and Cleanup:
```bash
cp -r ../OUTPUT ../../es_4.1/.
cp -r ../INPUT ../../es_4.1/.
make remove
```

### 2.2. Equilibration

- Configure Input Files: Manually edit `../INPUT/input.dat`copying this configuration:

```
SIMULATION_TYPE 0 
DISTRIBUTION_TYPE 1 
RESTART 0 
TEMP 2.72 
NPART 108 
RHO 0.05 
R_CUT 5.0 
DELTA 0.001 
NBLOCKS 5 
NSTEPS 2000  

ENDINPUT
```

- Execute Simulation:
```bash
./simulator.exe
```

- Archive Data and Cleanup:
```bash
cp -r ../OUTPUT ../../es_4.2/equilibration/.
cp -r ../INPUT ../../es_4.2/equilibration/.
make remove
```

### 2.3. Equilibrated Simulation

- Configure Input File: Manually edit `../INPUT/input.dat`copying this configuration:

```
SIMULATION_TYPE 0 
DISTRIBUTION_TYPE 0 
RESTART 1 
TEMP **COPY HERE LAST ROW, SECOND COLUMN OF FILE ../../es_4.2/equilibration/OUTPUT/temperature.dat** 
NPART 108 
RHO 0.05 
R_CUT 5.0 
DELTA 0.001 
NBLOCKS 20 
NSTEPS 2000  

ENDINPUT
```
- Copy last two equilibration simulation configuration in `../INPUT/CONFIG/.`:

```bash
cp ../../es_4.2/equilibration/OUTPUT/CONFIG/conf-1.xyz ../INPUT/CONFIG/.
cp ../../es_4.2/equilibration/OUTPUT/CONFIG/config.xyz ../INPUT/CONFIG/config.xyz
```

- Execute Simulation:
```bash
./simulator.exe
```

- Archive Data and Cleanup:
```bash
cp -r ../OUTPUT ../../es_4.2/simulation/.
cp -r ../INPUT ../../es_4.2/simulation/.
make remove
```

### 2.4. Time-Reversal Run starting from Equilibration Simulation

- Configure Input File: Manually edit `../INPUT/input.dat`copying this configuration:

```
SIMULATION_TYPE 0 
DISTRIBUTION_TYPE 0 
RESTART 1 
TEMP **COPY HERE LAST ROW, SECOND COLUMN OF FILE ../../es_4.2/equilibration/OUTPUT/temperature.dat** 
NPART 108 
RHO 0.05 
R_CUT 5.0 
DELTA 0.001 
NBLOCKS 5 
NSTEPS 2000  

ENDINPUT
```
- Copy last two equilibration simulation configuration in `../INPUT/CONFIG/.`:

```bash
cp ../../es_4.2/equilibration/OUTPUT/CONFIG/conf-1.xyz ../INPUT/CONFIG/config.xyz
cp ../../es_4.2/equilibration/OUTPUT/CONFIG/config.xyz ../INPUT/CONFIG/conf-1.xyz
```

**NB**: make sure that `conf-1.xyz` is overwritten with `config.xyz` and viceversa since we want to perform a time-reversal simulation.

- Execute Simulation:
```bash
./simulator.exe
```

- Archive Data and Cleanup:
```bash
cp -r ../OUTPUT ../../es_4.3/back_1/.
cp -r ../INPUT ../../es_4.3/back_1/.
make remove
```

### 2.5. Time-Reversal Run starting from Equilibrated Simulation

- Configure Input File: Manually edit `../INPUT/input.dat`copying this configuration:

```
SIMULATION_TYPE 0 
DISTRIBUTION_TYPE 0 
RESTART 1 
TEMP **COPY HERE LAST ROW, SECOND COLUMN OF FILE ../../es_4.2/simulation/OUTPUT/temperature.dat** 
NPART 108 
RHO 0.05 
R_CUT 5.0 
DELTA 0.001 
NBLOCKS 25 
NSTEPS 2000  

ENDINPUT
```
- Copy last two equilibration simulation configuration in `../INPUT/CONFIG/.`:

```bash
cp ../../es_4.2/simulation/OUTPUT/CONFIG/conf-1.xyz ../INPUT/CONFIG/config.xyz
cp ../../es_4.2/simulation/OUTPUT/CONFIG/config.xyz ../INPUT/CONFIG/conf-1.xyz
```

**NB**: make sure that `conf-1.xyz` is overwritten with `config.xyz` and viceversa since we want to perform a time-reversal simulation.

- Execute Simulation:
```bash
./simulator.exe
```

- Archive Data and Cleanup:
```bash
cp -r ../OUTPUT ../../es_4.3/back_2/.
cp -r ../INPUT ../../es_4.3/back_2/.
make remove
```
