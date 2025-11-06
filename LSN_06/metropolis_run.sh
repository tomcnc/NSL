#!/bin/bash

# set -e: Interrompe lo script immediatamente se un comando fallisce.
# set -o pipefail: Assicura che una pipeline fallisca se fallisce un qualsiasi comando al suo interno.
set -e
set -o pipefail

# ==============================================================================
# SCRIPT PER L'ESECUZIONE DELL'ESERCIZIO 6 (1D Ising model)
#
# Questo script gestisce la compilazione, l'esecuzione, la modifica dei parametri
# di input e di proprietà, e l'archiviazione dei risultati per le varie fasi 
# della simulazione del modello di Ising 1D.
# ==============================================================================

echo "========================================================"
echo "        STARTING ISING MODEL SIMULATION WORKFLOW        "
echo "========================================================"
echo "--- INITIAL CONFIGURATION ---"

# --- VARIABILI GLOBALI DI CONFIGURAZIONE ---
# Definisce le directory principali per la simulazione
EX_DIR="metropolis"                                 # Directory principale per l'algoritmo Metropolis
SIM_DIR="NSL_SIMULATOR"
SOURCE_DIR="$SIM_DIR/SOURCE"                        # Directory che contiene il codice sorgente (es. .cpp)
INPUT_DIR="$SIM_DIR/INPUT"                          # Directory che contiene i file di input (es. input.dat)
OUTPUT_DIR="$SIM_DIR/OUTPUT"                        # Directory che riceve i risultati dell'output
INPUT_FILE="$INPUT_DIR/input.dat"                   # Percorso completo del file dei parametri di input
PROPERTIES_FILE="$INPUT_DIR/properties.dat"         # Percorso completo del file delle proprietà misurate
EXECUTABLE_PATH="$SOURCE_DIR/simulator.exe"         # Percorso dell'eseguibile compilato
SEED_FILE="$OUTPUT_DIR/seed.out"                    # Percorso completo del file dell'ultimo seme usato dal LCG
CONFIG_FILE="$OUTPUT_DIR/CONFIG/config.spin"        # Percorso completo del file dell'ultima configurazione di spin prodotta
COMPILATION_LOG="$OUTPUT_DIR/compilation_log.txt"   # File per il log della compilazione

# Assicurarsi che la directory OUTPUT esista per il log
mkdir -p "$OUTPUT_DIR/CONFIG"

echo "--------------------------------------------------------"
echo "--- UTILITY FUNCTIONS DEFINITION ---"
# ------------------------------------------------------------------------------
# --- FUNZIONI UTILITY ---
# ------------------------------------------------------------------------------

# Funzione per modificare un parametro nel file input.dat
# Sovrascrive il valore di una riga che corrisponde a NOME_PARAMETRO.
# Sintassi: modify_input_param <NOME_PARAMETRO> <NUOVO_VALORE>
modify_input_param() {
    # 'local' definisce variabili visibili solo all'interno della funzione
    local param_name="$1"               # $1 è il primo argomento passato (NOME_PARAMETRO)
    local new_value="$2"                # $2 è il secondo argomento (NUOVO_VALORE)
    local temp_file="${INPUT_FILE}.tmp" # Percorso del file temporaneo per la modifica

    # Controllo se il file di input esiste prima di tentare di leggerlo
    if [ ! -f "$INPUT_FILE" ]; then
        echo "ERROR: Input file $INPUT_FILE not found."
        exit 1
    fi

    # Uso di 'awk' per modificare il file in modo sicuro:
    # -v P_NAME="$param_name" -v P_VALUE="$new_value": Passa le variabili Bash (nome e valore) ad awk.
    # Script AWK:
    #   ($1 == P_NAME) { print $1, P_VALUE; next }: Se il primo campo ($1) della riga corrisponde
    #                                               al parametro, stampa il nome ($1) e il NUOVO_VALORE,
    #                                               poi salta alla riga successiva ('next').
    #   { print }: Per tutte le altre righe (che non corrispondono), stampa la riga originale.
    # "$INPUT_FILE" > "$temp_file": Esegue awk sul file di input e scrive l'output nel file temporaneo.
    # && mv "$temp_file" "$INPUT_FILE": Se awk ha successo ('&&'), sposta ('mv') il file temporaneo
    #                                   sovrascrivendo l'originale, completando la modifica.
    awk -v P_NAME="$param_name" -v P_VALUE="$new_value" '
        ($1 == P_NAME) { print $1, P_VALUE; next }
        { print }
    ' "$INPUT_FILE" > "$temp_file" && mv "$temp_file" "$INPUT_FILE"
}

# ==============================================================================
# FUNZIONE PER SOVRASCRIVERE IL FILE DELLE PROPRIETÀ
# ==============================================================================
# Funzione per SOVRASCRIVERE il file properties.dat con un elenco di proprietà.
# Scrive ogni proprietà fornita come argomento su una nuova riga.
# Sintassi: set_properties <PROPRIETÀ_1> [PROPRIETÀ_2] ...
set_properties() {
    # 'local' definisce variabili visibili solo all'interno della funzione
    local properties=("$@") # "$@" cattura tutti gli argomenti passati in un array Bash
    local output_file="$PROPERTIES_FILE"

    echo "Setting properties in $output_file to: ${properties[*]}"

    # Controlla che siano state passate almeno una proprietà
    if [ ${#properties[@]} -eq 0 ]; then
        echo "ATTENZIONE: Nessuna proprietà specificata. Il file $output_file sarà vuoto."
    fi

    # Passo 1: Usa '>' per sovrascrivere/creare il file.
    # Questo comando svuota il file se esiste o lo crea se non esiste.
    > "$output_file"
    
    # Passo 2: Cicla su ogni proprietà nell'array
    for prop in "${properties[@]}"; do
        # 'printf "%s\n"' scrive la proprietà (%s) seguita da un carattere di "nuova riga" (\n).
        # Usa '>>' (append) per aggiungere ogni riga al file (ora vuoto).
        printf "%s\n" "$prop" >> "$output_file"
    done
}


# ==============================================================================
# GESTIONE COMPILAZIONE
# ==============================================================================
echo "--------------------------------------------------------"
echo "COMPILATION: Attempting 'make simulator.exe'..."
echo "             Output redirected to $COMPILATION_LOG (and console)."

# Esegue make, reindirizzando l'output (stdout e stderr) al log file, 
# e usa 'tee' per stampare anche a console (/dev/tty)
if make -C "$SOURCE_DIR" simulator.exe 2>&1 | tee "$COMPILATION_LOG"; then
    echo "COMPILATION SUCCESSFUL: Executable is ready at $EXECUTABLE_PATH."
else
    # Se il comando make fallisce (codice di uscita != 0)
    echo "========================================================"
    echo "ATTENTION: COMPILATION FAILED."
    echo "Check the C++ code for syntax or linking errors."
    exit 1
fi

# Pulizia iniziale delle directory di output da esecuzioni precedenti
echo "--------------------------------------------------------"
echo "INITIAL CLEANUP: Removing output files remaining in $OUTPUT_DIR directory using 'make remove'."
make -C "$SOURCE_DIR" remove || { echo "ATTENTION: make remove failed"; }
# Rimuove la configurazione di spin finale e il file dell'ultime seme di simulazioni precedenti dalla directory di input
echo "INITIAL CLEANUP: Removing Ising spin configuration (config.spin) from $INPUT_DIR."
rm -f $INPUT_DIR/CONFIG/config.spin # Aggiunto -f per evitare errori se non esiste
echo "INITIAL CLEANUP: Removing seed.out file from $INPUT_DIR."
rm -f $INPUT_DIR/seed.out # Aggiunto -f per evitare errori se non esiste
echo "INITIAL CLEANUP: Removing all plots from plot directory"
rm -rf plot
echo "Creating archive plot directory"
mkdir plot

# Reimposta lo stato iniziale del sistema per la simulazione
echo "Restoring initial Ising lattice configuration (config.ising) to config.xyz."
cp $INPUT_DIR/CONFIG/config.ising $INPUT_DIR/CONFIG/config.xyz 


echo "========================================================"
echo "           M(RT)^2 EQUILIBRATION (H field on)           "
echo "========================================================"
# ------------------------------------------------------------------------------
# ---- Fase 1: Equilibrazione con campo magnetico acceso (H=0.02) ----
# ------------------------------------------------------------------------------

# Definizione delle directory per archiviare questa specifica simulazione
FIELD_DIR="$EX_DIR/magnetic_field_on" # Sottodirectory per H > 0
EQ_DIR="$FIELD_DIR/equilibration" # Sottodirectory per la sola equilibrazione

echo "--------------------------------------------------------"
echo "Modifying input file for equilibration format (H on)..."
modify_input_param "SIMULATION_TYPE" "2 1.0 0.02" # (2 = M(RT)^2 Ising, 1.0 = J, 0.02 = H)
modify_input_param "RESTART" "0"          # (0 = Inizia da configurazione randomica di spin, non da config.spin)
modify_input_param "TEMP" "2.0" 
modify_input_param "NPART" "50"  
modify_input_param "RHO" "1.0"
modify_input_param "R_CUT" "0.0"  
modify_input_param "DELTA" "0.0"       
modify_input_param "NBLOCKS" "1"
modify_input_param "NSTEPS" "20000"

echo "--------------------------------------------------------"
echo "Printing initial $INPUT_FILE:"
cat $INPUT_FILE
echo "--------------------------------------------------------"

# Imposta il file 'properties.dat' per misurare solo la magnetizzazione
echo "--------------------------------------------------------"
echo "Modifying properties file for equilibration (H on)..."
# Chiama la funzione per sovrascrivere il file
set_properties "MAGNETIZATION" "MAGNETIZATION_STEP" "ENDPROPERTIES"
# =====================
echo "--------------------------------------------------------"
echo "Printing $PROPERTIES_FILE:"
cat $PROPERTIES_FILE
echo "--------------------------------------------------------"

# Rimuove e ricrea la struttura delle directory di archiviazione
echo "CLEANUP: Removing previous archive ($EQ_DIR)."
rm -rf "$EQ_DIR" # rm -rf: Rimuove forzatamente (-f) e ricorsivamente (-r)
echo "Creating archive directory structure in $EQ_DIR..."
mkdir -p "$EQ_DIR" # mkdir -p: Crea la directory e i genitori se non esistono
mkdir -p "$EQ_DIR/plot" # Crea una sottodirectory per i futuri plot

# --- ESECUZIONE DELLA SIMULAZIONE (USARE SUBSHELL) ---
echo "--------------------------------------------------------"
echo "Starting ISING 1D equilibration (field on)..."

# Esegue la simulazione in una subshell per isolare l'ambiente
# ( ... ) || { ...; exit 1; }: Se un comando nella subshell fallisce (es. cd o simulator.exe),
#                              l'intero script principale viene interrotto.
(
    cd "$SOURCE_DIR" || { echo "ERROR: Not possible changing directory in $SOURCE_DIR."; exit 1; }
    ./simulator.exe || { echo "ERROR: simulator.exe execution failed."; exit 1; }
) || { echo "CRITICAL ERROR: Simulator execution failed. Exiting main script."; exit 1; }

echo "Equilibration completed."

# --- COPIA E ORGANIZZAZIONE DEI DATI ---
# Copia i file di input e output nell'archivio e salva lo stato per il restart
echo "--------------------------------------------------------"
# Controlla se le directory di INPUT e OUTPUT esistono prima di copiarle
if [ -d "$INPUT_DIR" ] && [ -d "$OUTPUT_DIR" ]; then
    echo "Copying $INPUT_DIR and $OUTPUT_DIR directories to $EQ_DIR..."
    cp -r "$INPUT_DIR" "$EQ_DIR/." 
    cp -r "$OUTPUT_DIR" "$EQ_DIR/."
    echo "INPUT and OUTPUT copied to $EQ_DIR."
else
    echo "ATTENTION: INPUT or OUTPUT directory not found after M(RT)^2 equilibration."; exit 1;
fi
# Controlla se i file di stato (seed e config.spin) esistono prima di copiarli
if [ -f "$SEED_FILE" ] && [ -f "$CONFIG_FILE" ]; then 
    echo "Copying $SEED_FILE and $CONFIG_FILE files to $INPUT_DIR..."
    cp "$SEED_FILE" "$INPUT_DIR/." 
    cp "$CONFIG_FILE" "$INPUT_DIR/CONFIG/."
    echo "$SEED_FILE and $CONFIG_FILE files copied to $INPUT_DIR."
else
    echo "ATTENTION: $SEED_FILE and $CONFIG_FILE files not found after M(RT)^2 equilibration."; exit 1;
fi

# Pulizia dei file di output dalla directory di lavoro
echo "--------------------------------------------------------"
echo "CLEANUP: Removing output files generated in $OUTPUT_DIR directory using 'make remove'."
make -C "$SOURCE_DIR" remove || { echo "ATTENTION: make remove failed, continuing."; }

echo "--------------------------------------------------------"
echo "M(RT)^2 ISING 1D equilibration completed and archived in $EX_DIR."

echo "========================================================"
echo "              SIMULATION MAGNETIC FIELD ON              "
echo "========================================================"
# ------------------------------------------------------------------------------
# ---- Fase 2: Simulazione a diverse T, con campo magnetico acceso (H=0.02) ----
# ------------------------------------------------------------------------------

# Rimuove le directory T_* e plot precedenti da questa sezione
# echo "CLEANUP: Removing previous plot archive in $FIELD_DIR."
# # rm -rf "$FIELD_DIR/T_*"
# rm -rf "$FIELD_DIR/plot"

# # Ricrea la directory per i plot aggregati
# mkdir -p "$FIELD_DIR/plot"

# Imposta 'properties.dat' per misurare solo la magnetizzazione (per la media a blocchi)
echo "--------------------------------------------------------"
echo "Modifying properties file for magnetic field on simulations..."
# Chiama la funzione per sovrascrivere il file
set_properties "MAGNETIZATION" "ENDPROPERTIES"
# =====================
echo "--------------------------------------------------------"
echo "Printing $PROPERTIES_FILE:"
cat $PROPERTIES_FILE
echo "--------------------------------------------------------"
 
# Definisce l'elenco delle temperature da simulare
TEMPERATURE_SUFFIXES="2.0 1.8 1.6 1.4 1.2 1.0 0.8 0.6 0.5 0.4 0.3"

echo "Start creating archive temperature directory structure for 1D Ising simulation with M(RT)^2 algorithm..."

# Cicla su ogni temperatura definita nell'elenco
for suffix in $TEMPERATURE_SUFFIXES; do
    # Definisce il nome della directory di archiviazione per questa T
    TEMPERATURE_DIR="$FIELD_DIR/T_$suffix"

    # Crea la directory di archiviazione
    rm -rf $TEMPERATURE_DIR
    mkdir -p "$TEMPERATURE_DIR"
    echo "Directory $TEMPERATURE_DIR created."

    # --- PREPARAZIONE SIMULAZIONE ---
    echo "--------------------------------------------------------"
    echo "PREPARATION: Modifying input file for T = $suffix"

    # Modifica i parametri di input per la simulazione corrente
    echo "Modifying input file..."
    modify_input_param "TEMP" "$suffix" # Imposta la temperatura corrente
    
    # Controlla se questa è la prima temperatura (T=2.0)
    if [ "$suffix" == "2.0" ]; then
        # Se T=2.0, imposta RESTART=1 (usa config.spin) e NBLOCKS=20
        modify_input_param "RESTART" "1"
        modify_input_param "NBLOCKS" "20"
    fi
    echo "Printing initial $INPUT_FILE:"
    cat $INPUT_FILE

    # --- ESECUZIONE SIMULAZIONE ---
    echo "--------------------------------------------------------"
    echo "Starting 1D Ising simulation at T = $suffix..."

    # Esegue la simulazione in una subshell
    (
        cd "$SOURCE_DIR" || { echo "ERROR: Not possible changing directory in $SOURCE_DIR."; exit 1; }
        ./simulator.exe || { echo "ERROR: simulator.exe execution failed."; exit 1; }
    ) || { echo "CRITICAL ERROR: Simulator execution failed. Exiting main script."; exit 1; }

    echo "Simulation T = $suffix completed."

    # --- ARCHIVIAZIONE DATI ---
    echo "--------------------------------------------------------"
    # Controlla se le directory di INPUT e OUTPUT esistono prima di copiarle
    if [ -d "$INPUT_DIR" ] && [ -d "$OUTPUT_DIR" ]; then
        echo "Copying $INPUT_DIR and $OUTPUT_DIR directories to $TEMPERATURE_DIR..."
        cp -r "$INPUT_DIR" "$TEMPERATURE_DIR/."
        cp -r "$OUTPUT_DIR" "$TEMPERATURE_DIR/."
        echo "INPUT and OUTPUT copied to $TEMPERATURE_DIR."
    else
        echo "ATTENTION: INPUT or OUTPUT directory not found."; exit 1;
    fi
    # Controlla se i file di stato esistono prima di copiarli
    if [ -f "$SEED_FILE" ] && [ -f "$CONFIG_FILE" ]; then 
        echo "Copying $SEED_FILE and $CONFIG_FILE files to $INPUT_DIR..."
        cp "$SEED_FILE" "$INPUT_DIR/." 
        cp "$CONFIG_FILE" "$INPUT_DIR/CONFIG/."
        echo "$SEED_FILE and $CONFIG_FILE files copied to $INPUT_DIR."
    else
        echo "ATTENTION: $SEED_FILE and $CONFIG_FILE files not found."; exit 1;
    fi

    # --- PULIZIA FILE OUTPUT ---
    echo "--------------------------------------------------------"
    echo "CLEANUP: Removing output files generated in $OUTPUT_DIR directory using 'make remove'."
    make -C "$SOURCE_DIR" remove || { echo "ATTENTION: make remove failed."; }
done 

echo "--------------------------------------------------------"
echo "M(RT)^2 simulations of 1D Ising model with magnetic field on are completed and archived in $FIELD_DIR."

# --- FINE DELLA PARTE DI SIMULAZIONE CON CAMPO ON

echo "--------------------------------------------------------"
# Rimuove la configurazione di spin finale e il file dell'ultime seme di simulazioni precedenti dalla directory di input
echo "FINAL CLEANUP: Removing Ising spin configuration (config.spin) from $INPUT_DIR."
rm -f $INPUT_DIR/CONFIG/config.spin # Aggiunto -f per evitare errori se non esiste
echo "FINAL CLEANUP: Removing seed.out file from $INPUT_DIR."
rm -f $INPUT_DIR/seed.out # Aggiunto -f per evitare errori se non esiste

echo "========================================================"
echo "           M(RT)^2 EQUILIBRATION (H field off)           "
echo "========================================================"
# ------------------------------------------------------------------------------
# ---- Fase 3: Equilibrazione con campo magnetico spento (H=0.0) ----
# ------------------------------------------------------------------------------

# Definizione delle directory per archiviare questa specifica simulazione
FIELD_DIR="$EX_DIR/magnetic_field_off" # Sottodirectory per H = 0
EQ_DIR="$FIELD_DIR/equilibration" # Sottodirectory per l'equilibrazione

echo "--------------------------------------------------------"
echo "Modifying input file to equilibration format (H off)..."
modify_input_param "SIMULATION_TYPE" "2 1.0 0.0" # (2 = M(RT)^2 Ising, 1.0 = J, 0.0 = H)
modify_input_param "RESTART" "0"          # (0 = Inizia da configurazione randomica di spin)
modify_input_param "TEMP" "2.0" 
modify_input_param "NPART" "50"  
modify_input_param "RHO" "1.0"
modify_input_param "R_CUT" "0.0"  
modify_input_param "DELTA" "0.0"       
modify_input_param "NBLOCKS" "1"
modify_input_param "NSTEPS" "20000"
echo "--------------------------------------------------------"
echo "Printing initial $INPUT_FILE:"
cat $INPUT_FILE
echo "--------------------------------------------------------"

# Imposta 'properties.dat' per misurare l'energia
echo "--------------------------------------------------------"
echo "Modifying properties file for equilibration (H off)..."
# Chiama la funzione per sovrascrivere il file
set_properties "TOTAL_ENERGY" "TOTAL_ENERGY_STEP" "ENDPROPERTIES"
# =====================
echo "--------------------------------------------------------"
echo "Printing $PROPERTIES_FILE:"
cat $PROPERTIES_FILE
echo "--------------------------------------------------------"

# Rimuove e ricrea la struttura delle directory di archiviazione
echo "CLEANUP: Removing previous archive ($EQ_DIR)."
rm -rf "$EQ_DIR" 
echo "Creating archive directory structure in $EQ_DIR..."
mkdir -p "$EQ_DIR" 
mkdir -p "$EQ_DIR/plot" 

# --- ESECUZIONE DELLA SIMULAZIONE (USARE SUBSHELL) ---
echo "--------------------------------------------------------"
echo "Starting ISING 1D equilibration (field off)..."

# Esegue la simulazione in una subshell
(
    cd "$SOURCE_DIR" || { echo "ERROR: Not possible changing directory in $SOURCE_DIR."; exit 1; }
    ./simulator.exe || { echo "ERROR: simulator.exe execution failed."; exit 1; }
) || { echo "CRITICAL ERROR: Simulator execution failed. Exiting main script."; exit 1; }

echo "Equilibration completed."

# --- COPIA E ORGANIZZAZIONE DEI DATI ---
echo "--------------------------------------------------------"
# Controlla se le directory di INPUT e OUTPUT esistono prima di copiarle
if [ -d "$INPUT_DIR" ] && [ -d "$OUTPUT_DIR" ]; then
    echo "Copying $INPUT_DIR and $OUTPUT_DIR directories to $EQ_DIR..."
    cp -r "$INPUT_DIR" "$EQ_DIR/." 
    cp -r "$OUTPUT_DIR" "$EQ_DIR/."
    echo "INPUT and OUTPUT copied to $EQ_DIR."
else
    echo "ATTENTION: INPUT or OUTPUT directory not found after M(RT)^2 equilibration."; exit 1;
fi
# Controlla se i file di stato esistono prima di copiarli
if [ -f "$SEED_FILE" ] && [ -f "$CONFIG_FILE" ]; then 
    echo "Copying $SEED_FILE and $CONFIG_FILE files to $INPUT_DIR..."
    cp "$SEED_FILE" "$INPUT_DIR/." 
    cp "$CONFIG_FILE" "$INPUT_DIR/CONFIG/."
    echo "$SEED_FILE and $CONFIG_FILE files copied to $INPUT_DIR."
else
    echo "ATTENTION: $SEED_FILE and $CONFIG_FILE files not found after M(RT)^2 equilibration."; exit 1;
fi

# Pulizia dei file di output dalla directory di lavoro
echo "--------------------------------------------------------"
echo "CLEANUP: Removing output files generated in $OUTPUT_DIR directory using 'make remove'."
make -C "$SOURCE_DIR" remove || { echo "ATTENTION: make remove failed, continuing."; }

echo "--------------------------------------------------------"
echo "M(RT)^2 ISING 1D equilibration completed and archived in $FIELD_DIR."

echo "========================================================"
echo "             SIMULATION MAGNETIC FIELD OFF              "
echo "========================================================"
# ------------------------------------------------------------------------------
# ---- Fase 4: Simulazione a diverse T, con campo magnetico spento (H=0.0) ----
# ------------------------------------------------------------------------------

# # Rimuove le directory T_* e plot precedenti da questa sezione
# echo "CLEANUP: Removing previous plot archive in $FIELD_DIR."
# rm -rf "$FIELD_DIR/plot"

# # Ricrea la directory per i plot aggregati
# mkdir -p "$FIELD_DIR/plot"

# Imposta 'properties.dat' per misurare le grandezze termodinamiche
echo "--------------------------------------------------------"
echo "Modifying properties file for magnetic field off simulations..."
# Chiama la funzione per sovrascrivere il file
set_properties "TOTAL_ENERGY" "SPECIFIC_HEAT" "SUSCEPTIBILITY" "ENDPROPERTIES"
# =====================
echo "--------------------------------------------------------"
echo "Printing $PROPERTIES_FILE:"
cat $PROPERTIES_FILE
echo "--------------------------------------------------------"
 
# Definisce l'elenco delle temperature da simulare
TEMPERATURE_SUFFIXES="2.0 1.8 1.6 1.4 1.2 1.0 0.8 0.6 0.5 0.4 0.3"

echo "Start creating archive directory structure for 1D Ising simulation with M(RT)^2 algorithm..."

# Cicla su ogni temperatura definita nell'elenco
for suffix in $TEMPERATURE_SUFFIXES; do
    # Definisce il nome della directory di archiviazione per questa T
    TEMPERATURE_DIR="$FIELD_DIR/T_$suffix"
    
    # Crea la directory di archiviazione
    mkdir -p "$TEMPERATURE_DIR"
    echo "Directory $TEMPERATURE_DIR created."

    # --- PREPARAZIONE SIMULAZIONE ---
    echo "--------------------------------------------------------"
    echo "PREPARATION: Modifying input file for T = $suffix"

    # Modifica i parametri di input per la simulazione corrente
    echo "Modifying input file..."
    modify_input_param "TEMP" "$suffix" # Imposta la temperatura corrente
    
    # Controlla se questa è la prima temperatura (T=2.0)
    if [ "$suffix" == "2.0" ]; then
        # Se T=2.0, imposta RESTART=1 (usa config.spin) e NBLOCKS=20
        modify_input_param "RESTART" "1"
        modify_input_param "NBLOCKS" "20"
    fi
    echo "Printing initial $INPUT_FILE:"
    cat $INPUT_FILE

    # --- ESECUZIONE SIMULAZIONE ---
    echo "--------------------------------------------------------"
    echo "Starting 1D Ising simulation at T = $suffix..."

    # Esegue la simulazione in una subshell
    (
        cd "$SOURCE_DIR" || { echo "ERROR: Not possible changing directory in $SOURCE_DIR."; exit 1; }
        ./simulator.exe || { echo "ERROR: simulator.exe execution failed."; exit 1; }
    ) || { echo "CRITICAL ERROR: Simulator execution failed. Exiting main script."; exit 1; }

    echo "Simulation T = $suffix completed."

    # --- ARCHIVIAZIONE DATI ---
    echo "--------------------------------------------------------"
    # Controlla se le directory di INPUT e OUTPUT esistono prima di copiarle
    if [ -d "$INPUT_DIR" ] && [ -d "$OUTPUT_DIR" ]; then
        echo "Copying $INPUT_DIR and $OUTPUT_DIR directories to $TEMPERATURE_DIR..."
        cp -r "$INPUT_DIR" "$TEMPERATURE_DIR/."
        cp -r "$OUTPUT_DIR" "$TEMPERATURE_DIR/."
        echo "INPUT and OUTPUT copied to $TEMPERATURE_DIR."
    else
        echo "ATTENTION: INPUT or OUTPUT directory not found."; exit 1;
    fi
    # Controlla se i file di stato esistono prima di copiarli
    if [ -f "$SEED_FILE" ] && [ -f "$CONFIG_FILE" ]; then 
        echo "Copying $SEED_FILE and $CONFIG_FILE files to $INPUT_DIR..."
        cp "$SEED_FILE" "$INPUT_DIR/." 
        cp "$CONFIG_FILE" "$INPUT_DIR/CONFIG/."
        echo "$SEED_FILE and $CONFIG_FILE files copied to $INPUT_DIR."
    else
        echo "ATTENTION: $SEED_FILE and $CONFIG_FILE files not found."; exit 1;
    fi

    # --- PULIZIA FILE OUTPUT ---
    echo "--------------------------------------------------------"
    echo "CLEANUP: Removing output files generated in $OUTPUT_DIR directory using 'make remove'."
    make -C "$SOURCE_DIR" remove || { echo "ATTENTION: make remove failed."; exit 1; }
done 

echo "--------------------------------------------------------"
echo "M(RT)^2 simulations of 1D Ising model with magnetic field off are completed and archived in $FIELD_DIR."
echo "M(RT)^2 simulations completed."

echo "--------------------------------------------------------"
# Rimuove la configurazione di spin finale e il file dell'ultime seme di simulazioni precedenti dalla directory di input
echo "FINAL CLEANUP: Removing Ising spin configuration (config.spin) from $INPUT_DIR."
rm -f $INPUT_DIR/CONFIG/config.spin # Aggiunto -f per evitare errori se non esiste
echo "FINAL CLEANUP: Removing seed.out file from $INPUT_DIR."
rm -f $INPUT_DIR/seed.out # Aggiunto -f per evitare errori se non esiste