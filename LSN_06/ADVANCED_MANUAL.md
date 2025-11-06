# Advanced Manual Execution and Troubleshooting Guide

This document provides step-by-step instructions for manually executing the entire simulation batch. This procedure should only be followed if the automated scripts (`gibbs_run.sh` and `metropoli_run.sh`) fail or if advanced debugging is required.

**Warning**: This manual process is complex and error-prone due to the necessary sequential file movements and parameter changes. Proceed with caution.

The primary working directory for compilation and execution is `LSN_06/NSL_SIMULATOR/SOURCE`.

***

## 1. Initial Setup and Compilation

Start from the `LSN_06` directory.

### 1.1 Global Cleanup and Configuration Reset

- Enter the source directory:
```bash
cd NSL_SIMULATOR/SOURCE
```
- Remove compiled files and old output from the simulator directory
```bash
make remove
rm -f ../INPUT/CONFIG/config.spin
rm -f ../INPUT/seed.out
```

- Navigate back to the main directory:
```bash
cd ../..
```

- Delete all archived data and plots (Use with caution!):
```bash
rm -rf plot
mkdir plot
rm -rf gibbs/magnetic_field_off/equilibration/INPUT gibbs/magnetic_field_off/equilibration/OUTPUT gibbs/magnetic_field_off/equilibration/plot/*.* gibbs/magnetic_field_off/T_2.0/* gibbs/magnetic_field_off/T_1.8/* gibbs/magnetic_field_off/T_1.6/* gibbs/magnetic_field_off/T_1.4/* gibbs/magnetic_field_off/T_1.2/* gibbs/magnetic_field_off/T_1.0/* gibbs/magnetic_field_off/T_0.8/* gibbs/magnetic_field_off/T_0.6/* gibbs/magnetic_field_off/T_0.5/* gibbs/magnetic_field_off/T_0.4/* gibbs/magnetic_field_off/T_0.3/*
rm -rf gibbs/magnetic_field_on/equilibration/INPUT gibbs/magnetic_field_on/equilibration/OUTPUT gibbs/magnetic_field_on/equilibration/plot/*.* gibbs/magnetic_field_on/T_2.0/* gibbs/magnetic_field_on/T_1.8/* gibbs/magnetic_field_on/T_1.6/* gibbs/magnetic_field_on/T_1.4/* gibbs/magnetic_field_on/T_1.2/* gibbs/magnetic_field_on/T_1.0/* gibbs/magnetic_field_on/T_0.8/* gibbs/magnetic_field_on/T_0.6/* gibbs/magnetic_field_on/T_0.5/* gibbs/magnetic_field_on/T_0.4/* gibbs/magnetic_field_on/T_0.3/*
rm -rf metropolis/magnetic_field_off/equilibration/INPUT metropolis/magnetic_field_off/equilibration/OUTPUT metropolis/magnetic_field_off/equilibration/plot/*.* metropolis/magnetic_field_off/T_2.0/* metropolis/magnetic_field_off/T_1.8/* metropolis/magnetic_field_off/T_1.6/* metropolis/magnetic_field_off/T_1.4/* metropolis/magnetic_field_off/T_1.2/* metropolis/magnetic_field_off/T_1.0/* metropolis/magnetic_field_off/T_0.8/* metropolis/magnetic_field_off/T_0.6/* metropolis/magnetic_field_off/T_0.5/* metropolis/magnetic_field_off/T_0.4/* metropolis/magnetic_field_off/T_0.3/*
rm -rf metropolis/magnetic_field_on/equilibration/INPUT metropolis/magnetic_field_on/equilibration/OUTPUT metropolis/magnetic_field_on/equilibration/plot/*.* metropolis/magnetic_field_on/T_2.0/* metropolis/magnetic_field_on/T_1.8/* metropolis/magnetic_field_on/T_1.6/* metropolis/magnetic_field_on/T_1.4/* metropolis/magnetic_field_on/T_1.2/* metropolis/magnetic_field_on/T_1.0/* metropolis/magnetic_field_on/T_0.8/* metropolis/magnetic_field_on/T_0.6/* metropolis/magnetic_field_on/T_0.5/* metropolis/magnetic_field_on/T_0.4/* metropolis/magnetic_field_on/T_0.3/*
```

- Reset initial configuration file (using ising structure):
```bash
cp NSL_SIMULATOR/INPUT/CONFIG/config.ising NSL_SIMULATOR/INPUT/CONFIG/config.xyz
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

## 2. METROPOLIS

### 2.1. Metropolis Equilibration ($T = 2.0, field off)

- Configure Input Files: Manually edit `../INPUT/input.dat` and `../INPUT/properties.dat` copying the following configurations:

`input.dat`:
```
SIMULATION_TYPE 2 1.0 0.0 
RESTART 0 
TEMP 2.0 
NPART 50 
RHO 1.0 
R_CUT 0.0 
DELTA 0.0
NBLOCKS 1 
NSTEPS 20000  

ENDINPUT
```

`properties.dat`
```
TOTAL_ENERGY
TOTAL_ENERGY_STEP
 
ENDPROPERTIES
```

- Execute Simulation:
```bash
./simulator.exe
```

- Archive Data and Cleanup:
```bash
cp -r ../OUTPUT ../../metropolis/magnetic_field_off/equilibration/.
cp -r ../INPUT ../../metropolis/magnetic_field_off/equilibration/.
cp ../OUTPUT/CONFIG/config.spin ../INPUT/CONFIG/.
cp ../OUTPUT/seed.out ../INPUT/.
make remove
```

### 2.2. Simulation (sequentially change TEMP in this order: 2.0 -> 1.8 -> 1.6 -> 1.4 -> 1.2 -> 1.0 -> 0.8 -> 0.6 -> 0.5 -> 0.4 -> 0.3)

- Configure Input Files: Manually edit `../INPUT/input.dat` and `../INPUT/properties.dat` copying this configuration:

`input.dat`
```
SIMULATION_TYPE 2 1.0 0.0 
RESTART 1 
TEMP *.* 
NPART 50 
RHO 1.0
R_CUT 0.0 
DELTA 0.0 
NBLOCKS 20 
NSTEPS 20000  

ENDINPUT
```

`properties.dat`
```
TOTAL_ENERGY
SPECIFIC_HEAT
SUSCEPTIBILITY
 
ENDPROPERTIES
```

- Execute Simulation:
```bash
./simulator.exe
```

- Archive Data and Cleanup:
```bash
cp -r ../OUTPUT ../../metropolis/magnetic_field_off/T_*.*/.
cp -r ../INPUT ../../metropolis/magnetic_field_off/T_*.*/.
cp ../OUTPUT/CONFIG/config.spin ../INPUT/CONFIG/.
cp ../OUTPUT/seed.out ../INPUT/.
make remove
```

### 2.3. Metropolis Equilibration ($T = 2.0, field on)

- Restore File Configuration:
```bash
rm -f ../INPUT/CONFIG/config.spin
rm -f ../INPUT/seed.out
cp ../INPUT/CONFIG/config.ising ../INPUT/CONFIG/config.xyz
```

- Configure Input Files: Manually edit `../INPUT/input.dat` and `../INPUT/properties.dat` copying the following configurations:

`input.dat`:
```
SIMULATION_TYPE 2 1.0 0.02
RESTART 0 
TEMP 2.0 
NPART 50 
RHO 1.0
R_CUT 0.0 
DELTA 0.0
NBLOCKS 1 
NSTEPS 20000  

ENDINPUT
```

`properties.dat`
```
MAGNETIZATION
MAGNETIZATION_STEP
 
ENDPROPERTIES
```

- Execute Simulation:
```bash
./simulator.exe
```

- Archive Data and Cleanup:
```bash
cp -r ../OUTPUT ../../metropolis/magnetic_field_on/equilibration/.
cp -r ../INPUT ../../metropolis/magnetic_field_on/equilibration/.
cp ../OUTPUT/CONFIG/config.spin ../INPUT/CONFIG/.
cp ../OUTPUT/seed.out ../INPUT/.
make remove
```

### 2.4. Simulation (sequentially change TEMP in this order: 2.0 -> 1.8 -> 1.6 -> 1.4 -> 1.2 -> 1.0 -> 0.8 -> 0.6 -> 0.5 -> 0.4 -> 0.3)

- Configure Input Files: Manually edit `../INPUT/input.dat` and `../INPUT/properties.dat` copying this configuration:

`input.dat`
```
SIMULATION_TYPE 2 1.0 0.02 
RESTART 1 
TEMP *.* 
NPART 50 
RHO 1.0
R_CUT 0.0 
DELTA 0.0 
NBLOCKS 20 
NSTEPS 20000  

ENDINPUT
```

`properties.dat`
```
MAGNETIZATION
 
ENDPROPERTIES
```

- Execute Simulation:
```bash
./simulator.exe
```

- Archive Data and Cleanup:
```bash
cp -r ../OUTPUT ../../metropolis/magnetic_field_on/T_*.*/.
cp -r ../INPUT ../../metropolis/magnetic_field_on/T_*.*/.
cp ../OUTPUT/CONFIG/config.spin ../INPUT/CONFIG/.
cp ../OUTPUT/seed.out ../INPUT/.
make remove
```

## 3. GIBBS

### 3.1. Gibbs Equilibration ($T = 2.0, field off)

- Configure Input Files: Manually edit `../INPUT/input.dat` and `../INPUT/properties.dat` copying the following configurations:

`input.dat`:
```
SIMULATION_TYPE 3 1.0 0.0 
RESTART 0 
TEMP 2.0 
NPART 50 
RHO 1.0 
R_CUT 0.0 
DELTA 0.0
NBLOCKS 1 
NSTEPS 20000  

ENDINPUT
```

`properties.dat`:
```
TOTAL_ENERGY
TOTAL_ENERGY_STEP
 
ENDPROPERTIES
```

- Execute Simulation:
```bash
./simulator.exe
```

- Archive Data and Cleanup:
```bash
cp -r ../OUTPUT ../../gibbs/magnetic_field_off/equilibration/.
cp -r ../INPUT ../../gibbs/magnetic_field_off/equilibration/.
cp ../OUTPUT/CONFIG/config.spin ../INPUT/CONFIG/.
cp ../OUTPUT/seed.out ../INPUT/.
make remove
```

### 3.2. Simulation (sequentially change TEMP in this order: 2.0 -> 1.8 -> 1.6 -> 1.4 -> 1.2 -> 1.0 -> 0.8 -> 0.6 -> 0.5 -> 0.4 -> 0.3)

- Configure Input Files: Manually edit `../INPUT/input.dat` and `../INPUT/properties.dat` copying this configuration:

`input.dat`
```
SIMULATION_TYPE 3 1.0 0.0 
RESTART 1 
TEMP *.* 
NPART 50 
RHO 1.0
R_CUT 0.0 
DELTA 0.0 
NBLOCKS 20 
NSTEPS 20000  

ENDINPUT
```

`properties.dat`
```
TOTAL_ENERGY
SPECIFIC_HEAT
SUSCEPTIBILITY
 
ENDPROPERTIES
```

- Execute Simulation:
```bash
./simulator.exe
```

- Archive Data and Cleanup:
```bash
cp -r ../OUTPUT ../../gibbs/magnetic_field_off/T_*.*/.
cp -r ../INPUT ../../gibbs/magnetic_field_off/T_*.*/.
cp ../OUTPUT/CONFIG/config.spin ../INPUT/CONFIG/.
cp ../OUTPUT/seed.out ../INPUT/.
make remove
```

### 3.3. Gibbs Equilibration ($T = 2.0, field on)

- Restore File Configuration:
```bash
rm -f ../INPUT/CONFIG/config.spin
rm -f ../INPUT/seed.out
cp ../INPUT/CONFIG/config.ising ../INPUT/CONFIG/config.xyz
```

- Configure Input Files: Manually edit `../INPUT/input.dat` and `../INPUT/properties.dat` copying the following configurations:

`input.dat`:
```
SIMULATION_TYPE 3 1.0 0.02
RESTART 0 
TEMP 2.0 
NPART 50 
RHO 1.0
R_CUT 0.0 
DELTA 0.0
NBLOCKS 1 
NSTEPS 20000  

ENDINPUT
```

`properties.dat`:
```
MAGNETIZATION
MAGNETIZATION_STEP
 
ENDPROPERTIES
```

- Execute Simulation:
```bash
./simulator.exe
```

- Archive Data and Cleanup:
```bash
cp -r ../OUTPUT ../../gibbs/magnetic_field_on/equilibration/.
cp -r ../INPUT ../../gibbs/magnetic_field_on/equilibration/.
cp ../OUTPUT/CONFIG/config.spin ../INPUT/CONFIG/.
cp ../OUTPUT/seed.out ../INPUT/.
make remove
```

### 3.4. Simulation (sequentially change TEMP in this order: 2.0 -> 1.8 -> 1.6 -> 1.4 -> 1.2 -> 1.0 -> 0.8 -> 0.6 -> 0.5 -> 0.4 -> 0.3)

- Configure Input Files: Manually edit `../INPUT/input.dat` and `../INPUT/properties.dat` copying this configuration:

`input.dat`
```
SIMULATION_TYPE 3 1.0 0.02 
RESTART 1 
TEMP *.* 
NPART 50 
RHO 1.0
R_CUT 0.0 
DELTA 0.0 
NBLOCKS 20 
NSTEPS 20000  

ENDINPUT
```

`properties.dat`
```
MAGNETIZATION
 
ENDPROPERTIES
```

- Execute Simulation:
```bash
./simulator.exe
```

- Archive Data and Cleanup:
```bash
cp -r ../OUTPUT ../../metropolis/magnetic_field_on/T_*.*/.
cp -r ../INPUT ../../metropolis/magnetic_field_on/T_*.*/.
cp ../OUTPUT/CONFIG/config.spin ../INPUT/CONFIG/.
cp ../OUTPUT/seed.out ../INPUT/.
make remove
```

## 4. Restoring Initial Configurations

- Clean up `NSL_SIMULATOR` directory:
```bash
rm -f ../INPUT/CONFIG/config.spin
rm -f ../INPUT/seed.out
```