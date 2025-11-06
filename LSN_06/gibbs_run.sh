#!/bin/bash

# set -e: Interrupts the script immediately if a command fails. (Recommended)
# set -o pipefail: Ensures that a pipeline fails if any command within it fails. (Recommended)
set -e
set -o pipefail

# ==============================================================================
# SCRIPT FOR EXECUTING EXERCISE 6 (1D Ising model)
#
# This script manages compilation, execution, input parameter modification, 
# property setting, and archiving of results for the various phases of the 
# 1D Ising model simulation using the Gibbs algorithm.
# ==============================================================================

echo "========================================================"
echo "        STARTING ISING MODEL SIMULATION WORKFLOW        "
echo "========================================================"
echo "--- INITIAL CONFIGURATION ---"

# --- GLOBAL CONFIGURATION VARIABLES ---
# Defines the main directories for the simulation
EX_DIR="gibbs"                                     # Main directory for the Gibbs algorithm
SIM_DIR="NSL_SIMULATOR"
SOURCE_DIR="$SIM_DIR/SOURCE"                        # Directory containing the source code (e.g., .cpp)
INPUT_DIR="$SIM_DIR/INPUT"                          # Directory containing input files (e.g., input.dat)
OUTPUT_DIR="$SIM_DIR/OUTPUT"                        # Directory receiving output results
INPUT_FILE="$INPUT_DIR/input.dat"                   # Full path to the input parameters file
PROPERTIES_FILE="$INPUT_DIR/properties.dat"         # Full path to the measured properties file
EXECUTABLE_PATH="$SOURCE_DIR/simulator.exe"         # Path to the compiled executable
SEED_FILE="$OUTPUT_DIR/seed.out"                    # Full path to the last seed file used by the LCG
CONFIG_FILE="$OUTPUT_DIR/CONFIG/config.spin"        # Full path to the file of the last produced spin configuration
COMPILATION_LOG="$OUTPUT_DIR/compilation_log.txt"   # File for the compilation log

# Ensure the OUTPUT directory exists for the log
mkdir -p "$OUTPUT_DIR/CONFIG"

echo "--------------------------------------------------------"
echo "--- UTILITY FUNCTIONS DEFINITION ---"
# ------------------------------------------------------------------------------
# --- UTILITY FUNCTIONS ---
# ------------------------------------------------------------------------------

# Function to modify a parameter in the input.dat file
# Overwrites the value of a line corresponding to PARAMETER_NAME.
# Syntax: modify_input_param <PARAMETER_NAME> <NEW_VALUE>
modify_input_param() {
    # 'local' defines variables visible only within the function
    local param_name="$1"               # $1 is the first passed argument (PARAMETER_NAME)
    local new_value="$2"                # $2 is the second argument (NEW_VALUE)
    local temp_file="${INPUT_FILE}.tmp" # Path of the temporary file for modification

    # Check if the input file exists before attempting to read it
    if [ ! -f "$INPUT_FILE" ]; then
        echo "ERROR: Input file $INPUT_FILE not found."
        exit 1
    fi

    # Using 'awk' to modify the file safely:
    # -v P_NAME="$param_name" -v P_VALUE="$new_value": Passes Bash variables (name and value) to awk.
    # AWK Script:
    #   ($1 == P_NAME) { print $1, P_VALUE; next }: If the first field ($1) of the line matches
    #                                               the parameter, prints the name ($1) and the NEW_VALUE,
    #                                               then skips to the next line ('next').
    #   { print }: For all other lines (that do not match), prints the original line.
    # "$INPUT_FILE" > "$temp_file": Executes awk on the input file and writes the output to the temporary file.
    # && mv "$temp_file" "$INPUT_FILE": If awk is successful ('&&'), moves ('mv') the temporary file
    #                                   overwriting the original, completing the modification.
    awk -v P_NAME="$param_name" -v P_VALUE="$new_value" '
        ($1 == P_NAME) { print $1, P_VALUE; next }
        { print }
    ' "$INPUT_FILE" > "$temp_file" && mv "$temp_file" "$INPUT_FILE"
}

# ==============================================================================
# FUNCTION TO OVERWRITE THE PROPERTIES FILE
# ==============================================================================
# Function to OVERWRITE the properties.dat file with a list of properties.
# Writes each property provided as an argument on a new line.
# Syntax: set_properties <PROPERTY_1> [PROPERTY_2] ...
set_properties() {
    # 'local' defines variables visible only within the function
    local properties=("$@") # "$@" captures all passed arguments in a Bash array
    local output_file="$PROPERTIES_FILE"

    echo "Setting properties in $output_file to: ${properties[*]}"

    # Checks that at least one property has been passed
    if [ ${#properties[@]} -eq 0 ]; then
        echo "WARNING: No property specified. The file $output_file will be empty."
    fi

    # Step 1: Uses '>' to overwrite/create the file.
    # This command empties the file if it exists or creates it if it doesn't.
    > "$output_file"
    
    # Step 2: Cycles through each property in the array
    for prop in "${properties[@]}"; do
        # 'printf "%s\n"' writes the property (%s) followed by a "new line" character (\n).
        # Uses '>>' (append) to add each line to the file (now empty).
        printf "%s\n" "$prop" >> "$output_file"
    done
}


# ==============================================================================
# COMPILATION MANAGEMENT
# ==============================================================================
echo "--------------------------------------------------------"
echo "COMPILATION: Attempting 'make simulator.exe'..."
echo "             Output redirected to $COMPILATION_LOG (and console)."

# Executes make, redirecting output (stdout and stderr) to the log file, 
# and uses 'tee' to also print to console (/dev/tty)
if make -C "$SOURCE_DIR" simulator.exe 2>&1 | tee "$COMPILATION_LOG"; then
    echo "COMPILATION SUCCESSFUL: Executable is ready at $EXECUTABLE_PATH."
else
    # If the make command fails (exit code != 0)
    echo "========================================================"
    echo "ATTENTION: COMPILATION FAILED."
    echo "Check the C++ code for syntax or linking errors."
    exit 1
fi

# Initial cleanup of output directories from previous executions
echo "--------------------------------------------------------"
echo "INITIAL CLEANUP: Removing output files remaining in $OUTPUT_DIR directory using 'make remove'."
make -C "$SOURCE_DIR" remove || { echo "ATTENTION: make remove failed, continuing."; }

# --- CRITICAL CORRECTION: Use double quotes for paths to prevent syntax errors ---
echo "INITIAL CLEANUP: Removing Ising spin configuration (config.spin) from $INPUT_DIR."
rm -f "$INPUT_DIR/CONFIG/config.spin" # Added double quotes for safety
echo "INITIAL CLEANUP: Removing seed.out file from $INPUT_DIR."
rm -f "$INPUT_DIR/seed.out" # Added double quotes for safety
echo "INITIAL CLEANUP: Removing all plots from plot directory"
rm -rf plot
echo "Creating archive plot directory"
mkdir plot

# Restores the initial state of the system for the simulation
echo "Restoring initial Ising lattice configuration (config.ising) to config.xyz."
cp "$INPUT_DIR/CONFIG/config.ising" "$INPUT_DIR/CONFIG/config.xyz" # Added double quotes for safety


echo "========================================================"
echo "           Gibbs EQUILIBRATION (H field on)           "
echo "========================================================"
# ------------------------------------------------------------------------------
# ---- Phase 1: Equilibration with magnetic field on (H=0.02) ----
# ------------------------------------------------------------------------------

# Definition of directories to archive this specific simulation
FIELD_DIR="$EX_DIR/magnetic_field_on" # Subdirectory for H > 0
EQ_DIR="$FIELD_DIR/equilibration" # Subdirectory for equilibration only

echo "--------------------------------------------------------"
echo "Modifying input file for equilibration format (H on)..."
modify_input_param "SIMULATION_TYPE" "3 1.0 0.02" # (3 = Gibbs Ising, 1.0 = J, 0.02 = H)
modify_input_param "RESTART" "0"          # (0 = Starts from random spin configuration, not from config.spin)
modify_input_param "TEMP" "2.0" 
modify_input_param "NPART" "50"  
modify_input_param "RHO" "1.0"
modify_input_param "R_CUT" "0.0"  
modify_input_param "DELTA" "0.0"       
modify_input_param "NBLOCKS" "1"
modify_input_param "NSTEPS" "20000"

echo "--------------------------------------------------------"
echo "Printing initial $INPUT_FILE:"
cat "$INPUT_FILE"
echo "--------------------------------------------------------"

# Sets the 'properties.dat' file to measure only magnetization
echo "--------------------------------------------------------"
echo "Modifying properties file for equilibration (H on)..."
# Calls the function to overwrite the file
set_properties "MAGNETIZATION" "MAGNETIZATION_STEP" "ENDPROPERTIES"
# =====================
echo "--------------------------------------------------------"
echo "Printing $PROPERTIES_FILE:"
cat "$PROPERTIES_FILE"
echo "--------------------------------------------------------"

# Removes and recreates the archive directory structure
echo "CLEANUP: Removing previous archive ($EQ_DIR)."
rm -rf "$EQ_DIR" # rm -rf: Removes forcibly (-f) and recursively (-r)
echo "Creating archive directory structure in $EQ_DIR..."
mkdir -p "$EQ_DIR" # mkdir -p: Creates the directory and parents if they don't exist
mkdir -p "$EQ_DIR/plot" # Creates a subdirectory for future plots

# --- SIMULATION EXECUTION (USE SUBSHELL) ---
echo "--------------------------------------------------------"
echo "Starting ISING 1D equilibration (field on)..."

# Executes the simulation in a subshell to isolate the environment
# ( ... ) || { ...; exit 1; }: If a command in the subshell fails (e.g., cd or simulator.exe),
#                              the entire main script is interrupted.
(
    cd "$SOURCE_DIR" || { echo "ERROR: Not possible changing directory in $SOURCE_DIR."; exit 1; }
    ./simulator.exe || { echo "ERROR: simulator.exe execution failed."; exit 1; }
) || { echo "CRITICAL ERROR: Simulator execution failed. Exiting main script."; exit 1; }

echo "Equilibration completed."

# --- COPY AND DATA ORGANIZATION ---
# Copies input and output files to the archive and saves the state for restart
echo "--------------------------------------------------------"
# Checks if INPUT and OUTPUT directories exist before copying them
if [ -d "$INPUT_DIR" ] && [ -d "$OUTPUT_DIR" ]; then
    echo "Copying $INPUT_DIR and $OUTPUT_DIR directories to $EQ_DIR..."
    cp -r "$INPUT_DIR" "$EQ_DIR/." 
    cp -r "$OUTPUT_DIR" "$EQ_DIR/."
    echo "INPUT and OUTPUT copied to $EQ_DIR."
else
    echo "ATTENTION: INPUT or OUTPUT directory not found after Gibbs equilibration."; exit 1;
fi
# Checks if state files (seed and config.spin) exist before copying them
if [ -f "$SEED_FILE" ] && [ -f "$CONFIG_FILE" ]; then 
    echo "Copying $SEED_FILE and $CONFIG_FILE files to $INPUT_DIR..."
    cp "$SEED_FILE" "$INPUT_DIR/." 
    cp "$CONFIG_FILE" "$INPUT_DIR/CONFIG/."
    echo "$SEED_FILE and $CONFIG_FILE files copied to $INPUT_DIR."
else
    echo "ATTENTION: $SEED_FILE and $CONFIG_FILE files not found after Gibbs equilibration."; exit 1;
fi

# Cleanup of output files from the working directory
echo "--------------------------------------------------------"
echo "CLEANUP: Removing output files generated in $OUTPUT_DIR directory using 'make remove'."
make -C "$SOURCE_DIR" remove || { echo "ATTENTION: make remove failed, continuing."; }

echo "--------------------------------------------------------"
echo "Gibbs ISING 1D equilibration completed and archived in $EX_DIR."

echo "========================================================"
echo "              SIMULATION MAGNETIC FIELD ON              "
echo "========================================================"
# ------------------------------------------------------------------------------
# ---- Phase 2: Simulation at different T, with magnetic field on (H=0.02) ----
# ------------------------------------------------------------------------------

# Removes previous T_* and plot directories from this section
# echo "CLEANUP: Removing previous plot archive in $FIELD_DIR."
# # rm -rf "$FIELD_DIR/T_*"
# rm -rf "$FIELD_DIR/plot"

# # Recreates the directory for aggregated plots
# mkdir -p "$FIELD_DIR/plot"

# Sets 'properties.dat' to measure only magnetization (for block averaging)
echo "--------------------------------------------------------"
echo "Modifying properties file for magnetic field on simulations..."
# Calls the function to overwrite the file
set_properties "MAGNETIZATION" "ENDPROPERTIES"
# =====================
echo "--------------------------------------------------------"
echo "Printing $PROPERTIES_FILE:"
cat "$PROPERTIES_FILE"
echo "--------------------------------------------------------"
 
# Defines the list of temperatures to simulate
TEMPERATURE_SUFFIXES="2.0 1.8 1.6 1.4 1.2 1.0 0.8 0.6 0.5 0.4 0.3"

echo "Start creating archive temperature directory structure for 1D Ising simulation with Gibbs algorithm..."

# Cycles through each temperature defined in the list
for suffix in $TEMPERATURE_SUFFIXES; do
    # Defines the name of the archive directory for this T
    TEMPERATURE_DIR="$FIELD_DIR/T_$suffix"

    # Creates the archive directory
    rm -rf "$TEMPERATURE_DIR" # Added double quotes
    mkdir -p "$TEMPERATURE_DIR"
    echo "Directory $TEMPERATURE_DIR created."

    # --- SIMULATION PREPARATION ---
    echo "--------------------------------------------------------"
    echo "PREPARATION: Modifying input file for T = $suffix"

    # Modifies input parameters for the current simulation
    echo "Modifying input file..."
    modify_input_param "TEMP" "$suffix" # Sets the current temperature
    
    # Checks if this is the first temperature (T=2.0)
    if [ "$suffix" == "2.0" ]; then
        # If T=2.0, sets RESTART=1 (uses config.spin) and NBLOCKS=20
        modify_input_param "RESTART" "1"
        modify_input_param "NBLOCKS" "20"
    fi
    echo "Printing initial $INPUT_FILE:"
    cat "$INPUT_FILE"

    # --- SIMULATION EXECUTION ---
    echo "--------------------------------------------------------"
    echo "Starting 1D Ising simulation at T = $suffix..."

    # Executes the simulation in a subshell
    (
        cd "$SOURCE_DIR" || { echo "ERROR: Not possible changing directory in $SOURCE_DIR."; exit 1; }
        ./simulator.exe || { echo "ERROR: simulator.exe execution failed."; exit 1; }
    ) || { echo "CRITICAL ERROR: Simulator execution failed. Exiting main script."; exit 1; }

    echo "Simulation T = $suffix completed."

    # --- DATA ARCHIVING ---
    echo "--------------------------------------------------------"
    # Checks if INPUT and OUTPUT directories exist before copying them
    if [ -d "$INPUT_DIR" ] && [ -d "$OUTPUT_DIR" ]; then
        echo "Copying $INPUT_DIR and $OUTPUT_DIR directories to $TEMPERATURE_DIR..."
        cp -r "$INPUT_DIR" "$TEMPERATURE_DIR/."
        cp -r "$OUTPUT_DIR" "$TEMPERATURE_DIR/."
        echo "INPUT and OUTPUT copied to $TEMPERATURE_DIR."
    else
        echo "ATTENTION: INPUT or OUTPUT directory not found."; exit 1;
    fi
    # Checks if state files exist before copying them
    if [ -f "$SEED_FILE" ] && [ -f "$CONFIG_FILE" ]; then 
        echo "Copying $SEED_FILE and $CONFIG_FILE files to $INPUT_DIR..."
        cp "$SEED_FILE" "$INPUT_DIR/." 
        cp "$CONFIG_FILE" "$INPUT_DIR/CONFIG/."
        echo "$SEED_FILE and $CONFIG_FILE files copied to $INPUT_DIR."
    else
        echo "ATTENTION: $SEED_FILE and $CONFIG_FILE files not found."; exit 1;
    fi

    # --- OUTPUT FILE CLEANUP ---
    echo "--------------------------------------------------------"
    echo "CLEANUP: Removing output files generated in $OUTPUT_DIR directory using 'make remove'."
    make -C "$SOURCE_DIR" remove || { echo "ATTENTION: make remove failed."; }
done 

echo "--------------------------------------------------------"
echo "Gibbs simulations of 1D Ising model with magnetic field on are completed and archived in $FIELD_DIR."

# --- END OF SIMULATION PART WITH FIELD ON

echo "--------------------------------------------------------"
# Rimuove la configurazione di spin finale e il file dell'ultime seme di simulazioni precedenti dalla directory di input
echo "FINAL CLEANUP: Removing Ising spin configuration (config.spin) from $INPUT_DIR."
rm -f "$INPUT_DIR/CONFIG/config.spin" # Added double quotes for safety
echo "FINAL CLEANUP: Removing seed.out file from $INPUT_DIR."
rm -f "$INPUT_DIR/seed.out" # Added double quotes for safety

echo "========================================================"
echo "           Gibbs EQUILIBRATION (H field off)           "
echo "========================================================"
# ------------------------------------------------------------------------------
# ---- Phase 3: Equilibration with magnetic field off (H=0.0) ----
# ------------------------------------------------------------------------------

# Definition of directories to archive this specific simulation
FIELD_DIR="$EX_DIR/magnetic_field_off" # Subdirectory for H = 0
EQ_DIR="$FIELD_DIR/equilibration" # Subdirectory for equilibration

echo "--------------------------------------------------------"
echo "Modifying input file to equilibration format (H off)..."
modify_input_param "SIMULATION_TYPE" "3 1.0 0.0" # (3 = Gibbs Ising, 1.0 = J, 0.0 = H)
modify_input_param "RESTART" "0"          # (0 = Starts from random spin configuration)
modify_input_param "TEMP" "2.0" 
modify_input_param "NPART" "50"  
modify_input_param "RHO" "1.0"
modify_input_param "R_CUT" "0.0"  
modify_input_param "DELTA" "0.0"       
modify_input_param "NBLOCKS" "1"
modify_input_param "NSTEPS" "20000"
echo "--------------------------------------------------------"
echo "Printing initial $INPUT_FILE:"
cat "$INPUT_FILE"
echo "--------------------------------------------------------"

# Sets 'properties.dat' to measure energy
echo "--------------------------------------------------------"
echo "Modifying properties file for equilibration (H off)..."
# Calls the function to overwrite the file
set_properties "TOTAL_ENERGY" "TOTAL_ENERGY_STEP" "ENDPROPERTIES"
# =====================
echo "--------------------------------------------------------"
echo "Printing $PROPERTIES_FILE:"
cat "$PROPERTIES_FILE"
echo "--------------------------------------------------------"

# Removes and recreates the archive directory structure
echo "CLEANUP: Removing previous archive ($EQ_DIR)."
rm -rf "$EQ_DIR" 
echo "Creating archive directory structure in $EQ_DIR..."
mkdir -p "$EQ_DIR" 
mkdir -p "$EQ_DIR/plot" 

# --- SIMULATION EXECUTION (USE SUBSHELL) ---
echo "--------------------------------------------------------"
echo "Starting ISING 1D equilibration (field off)..."

# Executes the simulation in a subshell
(
    cd "$SOURCE_DIR" || { echo "ERROR: Not possible changing directory in $SOURCE_DIR."; exit 1; }
    ./simulator.exe || { echo "ERROR: simulator.exe execution failed."; exit 1; }
) || { echo "CRITICAL ERROR: Simulator execution failed. Exiting main script."; exit 1; }

echo "Equilibration completed."

# --- COPY AND DATA ORGANIZATION ---
echo "--------------------------------------------------------"
# Checks if INPUT and OUTPUT directories exist before copying them
if [ -d "$INPUT_DIR" ] && [ -d "$OUTPUT_DIR" ]; then
    echo "Copying $INPUT_DIR and $OUTPUT_DIR directories to $EQ_DIR..."
    cp -r "$INPUT_DIR" "$EQ_DIR/." 
    cp -r "$OUTPUT_DIR" "$EQ_DIR/."
    echo "INPUT and OUTPUT copied to $EQ_DIR."
else
    echo "ATTENTION: INPUT or OUTPUT directory not found after Gibbs equilibration."; exit 1;
fi
# Checks if state files exist before copying them
if [ -f "$SEED_FILE" ] && [ -f "$CONFIG_FILE" ]; then 
    echo "Copying $SEED_FILE and $CONFIG_FILE files to $INPUT_DIR..."
    cp "$SEED_FILE" "$INPUT_DIR/." 
    cp "$CONFIG_FILE" "$INPUT_DIR/CONFIG/."
    echo "$SEED_FILE and $CONFIG_FILE files copied to $INPUT_DIR."
else
    echo "ATTENTION: $SEED_FILE and $CONFIG_FILE files not found after Gibbs equilibration."; exit 1;
fi

# Cleanup of output files from the working directory
echo "--------------------------------------------------------"
echo "CLEANUP: Removing output files generated in $OUTPUT_DIR directory using 'make remove'."
make -C "$SOURCE_DIR" remove || { echo "ATTENTION: make remove failed, continuing."; }

echo "--------------------------------------------------------"
echo "Gibbs ISING 1D equilibration completed and archived in $FIELD_DIR."

echo "========================================================"
echo "             SIMULATION MAGNETIC FIELD OFF              "
echo "========================================================"
# ------------------------------------------------------------------------------
# ---- Phase 4: Simulation at different T, with magnetic field off (H=0.0) ----
# ------------------------------------------------------------------------------

# Removes previous T_* and plot directories from this section
# echo "CLEANUP: Removing previous plot archive in $FIELD_DIR."
# rm -rf "$FIELD_DIR/plot"

# # Recreates the directory for aggregated plots
# mkdir -p "$FIELD_DIR/plot"

# Sets 'properties.dat' to measure thermodynamic quantities
echo "--------------------------------------------------------"
echo "Modifying properties file for magnetic field off simulations..."
# Calls the function to overwrite the file
set_properties "TOTAL_ENERGY" "SPECIFIC_HEAT" "SUSCEPTIBILITY" "ENDPROPERTIES"
# =====================
echo "--------------------------------------------------------"
echo "Printing $PROPERTIES_FILE:"
cat "$PROPERTIES_FILE"
echo "--------------------------------------------------------"
 
# Defines the list of temperatures to simulate
TEMPERATURE_SUFFIXES="2.0 1.8 1.6 1.4 1.2 1.0 0.8 0.6 0.5 0.4 0.3"

echo "Start creating archive directory structure for 1D Ising simulation with Gibbs algorithm..."

# Cycles through each temperature defined in the list
for suffix in $TEMPERATURE_SUFFIXES; do
    # Defines the name of the archive directory for this T
    TEMPERATURE_DIR="$FIELD_DIR/T_$suffix"
    
    # Creates the archive directory
    mkdir -p "$TEMPERATURE_DIR"
    echo "Directory $TEMPERATURE_DIR created."

    # --- SIMULATION PREPARATION ---
    echo "--------------------------------------------------------"
    echo "PREPARATION: Modifying input file for T = $suffix"

    # Modifies input parameters for the current simulation
    echo "Modifying input file..."
    modify_input_param "TEMP" "$suffix" # Sets the current temperature
    
    # Checks if this is the first temperature (T=2.0)
    if [ "$suffix" == "2.0" ]; then
        # If T=2.0, sets RESTART=1 (uses config.spin) and NBLOCKS=20
        modify_input_param "RESTART" "1"
        modify_input_param "NBLOCKS" "20"
    else
        # For T < 2.0, RESTART=1 to continue from the previous T's config.spin, 
        # NBLOCKS=20 for measurement, NSTEPS=20000
        modify_input_param "RESTART" "1"
        modify_input_param "NBLOCKS" "20"
        modify_input_param "NSTEPS" "20000"
    fi
    echo "Printing initial $INPUT_FILE:"
    cat "$INPUT_FILE"

    # --- SIMULATION EXECUTION ---
    echo "--------------------------------------------------------"
    echo "Starting 1D Ising simulation at T = $suffix..."

    # Executes the simulation in a subshell
    (
        cd "$SOURCE_DIR" || { echo "ERROR: Not possible changing directory in $SOURCE_DIR."; exit 1; }
        ./simulator.exe || { echo "ERROR: simulator.exe execution failed."; exit 1; }
    ) || { echo "CRITICAL ERROR: Simulator execution failed. Exiting main script."; exit 1; }

    echo "Simulation T = $suffix completed."

    # --- DATA ARCHIVING ---
    echo "--------------------------------------------------------"
    # Checks if INPUT and OUTPUT directories exist before copying them
    if [ -d "$INPUT_DIR" ] && [ -d "$OUTPUT_DIR" ]; then
        echo "Copying $INPUT_DIR and $OUTPUT_DIR directories to $TEMPERATURE_DIR..."
        cp -r "$INPUT_DIR" "$TEMPERATURE_DIR/."
        cp -r "$OUTPUT_DIR" "$TEMPERATURE_DIR/."
        echo "INPUT and OUTPUT copied to $TEMPERATURE_DIR."
    else
        echo "ATTENTION: INPUT or OUTPUT directory not found."; exit 1;
    fi
    # Checks if state files exist before copying them
    if [ -f "$SEED_FILE" ] && [ -f "$CONFIG_FILE" ]; then 
        echo "Copying $SEED_FILE and $CONFIG_FILE files to $INPUT_DIR..."
        cp "$SEED_FILE" "$INPUT_DIR/." 
        cp "$CONFIG_FILE" "$INPUT_DIR/CONFIG/."
        echo "$SEED_FILE and $CONFIG_FILE files copied to $INPUT_DIR."
    else
        echo "ATTENTION: $SEED_FILE and $CONFIG_FILE files not found."; exit 1;
    fi

    # --- OUTPUT FILE CLEANUP ---
    echo "--------------------------------------------------------"
    echo "CLEANUP: Removing output files generated in $OUTPUT_DIR directory using 'make remove'."
    make -C "$SOURCE_DIR" remove || { echo "ATTENTION: make remove failed."; exit 1; }
done 

echo "--------------------------------------------------------"
echo "Gibbs simulations of 1D Ising model with magnetic field off are completed and archived in $FIELD_DIR."
echo "Gibbs simulations completed."

echo "--------------------------------------------------------"
# Rimuove la configurazione di spin finale e il file dell'ultime seme di simulazioni precedenti dalla directory di input
echo "FINAL CLEANUP: Removing Ising spin configuration (config.spin) from $INPUT_DIR."
rm -f "$INPUT_DIR/CONFIG/config.spin" # Added double quotes for safety
echo "FINAL CLEANUP: Removing seed.out file from $INPUT_DIR."
rm -f "$INPUT_DIR/seed.out" # Added double quotes for safety