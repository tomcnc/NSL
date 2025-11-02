#!/bin/bash

# set -e: Interrompe lo script immediatamente se un comando fallisce.
# set -o pipefail: Assicura che una pipeline fallisca se fallisce un qualsiasi comando al suo interno.
set -e
set -o pipefail

# ==============================================================================
# SCRIPT PER L'ESECUZIONE DEGLI ESERCIZI 4.1, 4.2 e 4.3 (Molecular Dynamics)
#
# Questo script gestisce la compilazione, l'esecuzione, la modifica dei parametri
# di input e l'archiviazione dei risultati per le varie fasi della simulazione 
# di Dinamica Molecolare (MD) richiesta per gli esercizi.
# ==============================================================================

echo "========================================================"
echo "          STARTING MD SIMULATION WORKFLOW               "
echo "========================================================"
echo "--- INITIAL CONFIGURATION ---"

# --- VARIABILI GLOBALI DI CONFIGURAZIONE ---
# Definisce le directory principali per la simulazione
SIM_DIR="NSL_SIMULATOR"
SOURCE_DIR="$SIM_DIR/SOURCE" # Directory che contiene il codice sorgente (es. .cpp)
INPUT_DIR="$SIM_DIR/INPUT"   # Directory che contiene i file di input (es. input.dat)
OUTPUT_DIR="$SIM_DIR/OUTPUT" # Directory che riceve i risultati dell'output
INPUT_FILE="$INPUT_DIR/input.dat" # Percorso completo del file dei parametri di input
EXECUTABLE_PATH="$SOURCE_DIR/simulator.exe" # Percorso dell'eseguibile compilato
COMPILATION_LOG="$OUTPUT_DIR/compilation_log.txt"   # File per il log della compilazione

# Assicurarsi che la directory OUTPUT esista per il log
mkdir -p "$OUTPUT_DIR/CONFIG"

echo "--------------------------------------------------------"
echo "--- UTILITY FUNCTIONS DEFINITION ---"
# ------------------------------------------------------------------------------
# --- FUNZIONI UTILITY ---
# ------------------------------------------------------------------------------

# Funzione per modificare un parametro nel file input.dat
# Sintassi: modify_input_param <NOME_PARAMETRO> <NUOVO_VALORE>
modify_input_param() {
    # 'local' definisce variabili visibili solo all'interno della funzione
    local param_name="$1" # $1 è il primo argomento passato alla funzione (NOME_PARAMETRO)
    local new_value="$2"  # $2 è il secondo argomento (NUOVO_VALORE)
    local temp_file="${INPUT_FILE}.tmp" # File temporaneo per la modifica

    # echo "Modifica: Parametro '$param_name' impostato a '$new_value' in $INPUT_FILE"

    # Controllo se il file di input esiste
    if [ ! -f "$INPUT_FILE" ]; then
        echo "ERROR: Input file $INPUT_FILE not found."
        exit 1
    fi

    # Uso di 'awk' per la modifica del file:
    # -v P_NAME="..." -v P_VALUE="...": Passa le variabili Bash (param_name, new_value) ad awk.
    # '...': Inizia lo script AWK
    # ($1 == P_NAME): Se il primo campo ($1) della riga corrente è uguale al nome del parametro:
    #   { print $1, P_VALUE; next }: Stampa il nome del parametro ($1) e il NUOVO_VALORE (P_VALUE), poi passa alla riga successiva ('next').
    # { print }: Per tutte le altre righe, stampa la riga originale senza modifiche.
    # "$INPUT_FILE": Specifica il file su cui AWK deve operare.
    # > "$temp_file": Reindirizza l'output di AWK al file temporaneo.
    # && mv "$temp_file" "$INPUT_FILE": Se il comando AWK ha successo ('&&'), il file temporaneo sovrascrive il file di input originale ('mv').
    awk -v P_NAME="$param_name" -v P_VALUE="$new_value" '
        ($1 == P_NAME) { print $1, P_VALUE; next }
        { print }
    ' "$INPUT_FILE" > "$temp_file" && mv "$temp_file" "$INPUT_FILE"
}

# Funzione per estrarre la temperatura media dell'ultimo blocco
# Sintassi: extract_last_temp <FILE_TEMPERATURE>
extract_last_temp() {
    local temp_file="$1" # $1 è il percorso del file di output della temperatura

    # Controllo se il file di temperatura esiste
    if [ ! -f "$temp_file" ]; then
        echo "ERROR: Temperature file $temp_file not found. Returning 2.0 (default value)." >&2 
        echo "2.0" # Ritorna un valore di fallback
        return 1
    fi

    # Estrazione del dato:
    # tail -n 1 "$temp_file": Estrae l'ultima riga del file (che contiene il dato dell'ultimo blocco).
    # |: 'pipe' l'output di 'tail' al comando successivo.
    # awk '{print $2}': Stampa il secondo campo ($2) dell'ultima riga.
    # Assumendo che il formato del file sia: [BLOCCO] [TEMP_MEDIA] ...
    LAST_TEMP=$(tail -n 1 "$temp_file" | awk '{print $2}')
    
    # Controllo che il valore non sia vuoto
    if [ -z "$LAST_TEMP" ]; then
        echo "ATTENTION: Failing in reading last row temperature. Returning 2.0 (default value)." >&2
        echo "2.0"
        return 1
    fi

    # echo "Temperatura estratta: $LAST_TEMP"
    echo "$LAST_TEMP" # Stampa il valore estratto come output della funzione
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

# Pulizia di eventuali file di output prodotti da esecuzioni fallite
echo "--------------------------------------------------------"
echo "INITIAL CLEANUP: Removing output files remaining in $OUTPUT_DIR directory using 'make remove'."
make -C "$SOURCE_DIR" remove || { echo "ATTENTION: make remove failed, continuing."; }

# Reset file di input nel caso di esecuzioni fallite
echo "--------------------------------------------------------"
echo "Resetting input file..."
modify_input_param "SIMULATION_TYPE" "0"
modify_input_param "DISTRIBUTION_TYPE" "0" 
modify_input_param "RESTART" "0"          
modify_input_param "TEMP" "2.0" 
modify_input_param "NPART" "108"  
modify_input_param "RHO" "0.05"  
modify_input_param "DELTA" "0.001"       
modify_input_param "NBLOCKS" "20"
modify_input_param "NSTEPS" "2000"

echo "--------------------------------------------------------"
echo "Printing initial $INPUT_FILE:"
cat $INPUT_FILE
echo "--------------------------------------------------------"

# Elimina le configurazioni di restart e reimposta la configurazione iniziale (FCC)
echo "Removing temporary configuration file 'conf-1.xyz'."
rm -f $INPUT_DIR/CONFIG/conf-1.xyz # rm: rimuove il file conf-1.xyz
echo "Restoring initial configuration (config.fcc) to config.xyz."
cp $INPUT_DIR/CONFIG/config.fcc $INPUT_DIR/CONFIG/config.xyz 

echo "========================================================"
echo "                   STARTING EXERCISE 4.1                  "
echo "========================================================"
# ------------------------------------------------------------------------------
# ---- ES 4.1: Simulazione Iniziale (ad esempio, per test) ----
# ------------------------------------------------------------------------------

EX_DIR="es_4.1" # Directory di archiviazione

# 3. Rimuove e ricrea la struttura delle directory di destinazione
echo "CLEANUP: Removing previous archive ($EX_DIR)."
rm -rf "$EX_DIR" # rm -rf: Rimuove forzatamente (-f) e ricorsivamente (-r) lda directory e il suo contenuto.
echo "Creating archive directory structure in $EX_DIR..."
mkdir -p "$EX_DIR" # mkdir -p: Crea la directory, e le directory genitore se non esistono.     
mkdir -p "$EX_DIR/plot" # Crea una sottodirectory per i futuri plot

# --- ESECUZIONE DELLA SIMULAZIONE (USARE SUBSHELL) ---
echo "--------------------------------------------------------"
echo "Starting MD simulation for Exercise 4.1..."

# ( ... ) || { ...; exit 1; } Esegue i comandi in una subshell. Se fallisce un comando, esce.
(
    cd "$SOURCE_DIR" || { echo "ERROR: Not possible changing directory in $SOURCE_DIR."; exit 1; }
    ./simulator.exe || { echo "ERROR: simulator.exe execution failed."; exit 1; }
) || { echo "CRITICAL ERROR: Simulator execution failed. Exiting main script."; exit 1; }

echo "Simulation completed."

# --- COPIA E ORGANIZZAZIONE DEI DATI ---
# 5. Copia le directory INPUT e OUTPUT in es_4.1/
echo "--------------------------------------------------------"
if [ -d "$INPUT_DIR" ] && [ -d "$OUTPUT_DIR" ]; then
    echo "Copying $INPUT_DIR and $OUTPUT_DIR directories to $EX_DIR/..."
    cp -r "$INPUT_DIR" "$EX_DIR/" 
    cp -r "$OUTPUT_DIR" "$EX_DIR/"
    echo "INPUT and OUTPUT copied to $EX_DIR."
else
    echo "ATTENTION: INPUT or OUTPUT directory not found after simulation 4.1."
fi

# 6. Pulizia dei file di output
echo "--------------------------------------------------------"
echo "FINAL CLEANUP: Removing output files generated in $OUTPUT_DIR directory using 'make remove'."
make -C "$SOURCE_DIR" remove || { echo "ATTENTION: make remove failed, continuing."; }

echo "--------------------------------------------------------"
echo "Simulation 4.1 completed and archived in $EX_DIR."

echo "========================================================"
echo "                   STARTING EXERCISE 4.2                  "
echo "========================================================"
echo "--------------------------------------------------------"
echo "           Equilibration from Dirac Delta             "
echo "--------------------------------------------------------"

EX_DIR="es_4.2"      
EQ_DIR="$EX_DIR/equilibration" 
RUN_DIR="$EX_DIR/simulation"   
EQ_TEMP_FILE="$EQ_DIR/OUTPUT/temperature.dat" 

# 3. Rimuove e ricrea la struttura delle directory
echo "CLEANUP: Removing previous archive ($EX_DIR)."
rm -rf "$EX_DIR"

echo "Creating archive directory structure in $EX_DIR..." 
mkdir -p "$EQ_DIR/plot" 
mkdir -p "$RUN_DIR/plot" 

# --- PREPARAZIONE EQUILIBRAZIONE ---
echo "--------------------------------------------------------"
echo "PREPARATION: Modifying input file for EQUILIBRATION phase (4.2.1)"

# Modifica i parametri di input per l'equilibrazione:
echo "Modifying input file..."
modify_input_param "DISTRIBUTION_TYPE" "1"  # 1: Genera velocità distributite come Delta di Dirac.
modify_input_param "TEMP" "2.72"            # Temperatura desiderata per equilibrare a 2.0.
modify_input_param "NBLOCKS" "5"            # Blocchi ridotti.
modify_input_param "RESTART" "0"            # 0: Inizia da capo.
echo "Printing initial $INPUT_FILE:"
cat $INPUT_FILE

# --- ESECUZIONE EQUILIBRAZIONE ---
echo "--------------------------------------------------------"
echo "Starting MD simulation for Exercise 4.2 (EQUILIBRATION)..."

(
    cd "$SOURCE_DIR" || { echo "ERROR: Not possible changing directory in $SOURCE_DIR."; exit 1; }
    ./simulator.exe || { echo "ERROR: simulator.exe execution failed."; exit 1; }
) || { echo "CRITICAL ERROR: Simulator execution failed. Exiting main script."; exit 1; }

echo "Equilibration completed."

# --- ARCHIVIAZIONE DATI EQUILIBRAZIONE ---
echo "--------------------------------------------------------"
if [ -d "$INPUT_DIR" ] && [ -d "$OUTPUT_DIR" ]; then
    echo "Copying $INPUT_DIR and $OUTPUT_DIR directories for equilibration phase to $EQ_DIR..."
    cp -r "$INPUT_DIR" "$EQ_DIR/"
    cp -r "$OUTPUT_DIR" "$EQ_DIR/"
    echo "INPUT and OUTPUT copied to $RUN_DIR."
else
    echo "ATTENTION: INPUT or OUTPUT directory not found after equilibration 4.2."
fi

# --- PULIZIA FILE OUTPUT ---
echo "--------------------------------------------------------"
echo "FINAL CLEANUP: Removing output files generated in $OUTPUT_DIR directory using 'make remove'."
make -C "$SOURCE_DIR" remove || { echo "ATTENTION: make remove failed, continuing."; }

echo "--------------------------------------------------------"
echo "Equilibration 4.2.1 completed and archived in $EQ_DIR."

echo "--------------------------------------------------------"
echo "              Equilibrium simulation                   "
echo "--------------------------------------------------------"

echo "PREPARATION: Modifying input file for MAIN SIMULATION (4.2.2)"

# 1. Estrae la temperatura media dall'ultimo blocco di equilibrazione
echo "Extracting last block's average temperature from file $EQ_TEMP_FILE..."
LAST_TEMP=$(extract_last_temp "$EQ_TEMP_FILE") 
echo "Last block's average temperature extracted: T_start* = $LAST_TEMP. Used for main simulation."

# 2. Modifica i parametri per la simulazione di produzione (più lunga)
echo "Modifying input file..."
modify_input_param "DISTRIBUTION_TYPE" "0"  # 0: Mantiene le velocità correnti.
modify_input_param "TEMP" "$LAST_TEMP"      # Imposta la temperatura all'equilibrio.
modify_input_param "RESTART" "1"            # 1: Abilita il restart (usa conf-1.xyz e config.xyz).
modify_input_param "NBLOCKS" "20"           # Aumenta i blocchi.
echo "Printing initial $INPUT_FILE:"
cat $INPUT_FILE
echo "--------------------------------------------------------"

# 3. Copia le configurazioni per il RESTART dalla directory di equilibrazione
EQ_CONF_DIR="$EQ_DIR/OUTPUT/CONFIG"
SIM_INPUT_CONF="$INPUT_DIR/CONFIG"
echo "Copying configurations $EQ_CONF_DIR/conf-1.xyz and $EQ_CONF_DIR/config.xyz for RESTART."
cp "$EQ_CONF_DIR/conf-1.xyz" "$SIM_INPUT_CONF/conf-1.xyz"
cp "$EQ_CONF_DIR/config.xyz" "$SIM_INPUT_CONF/config.xyz"
echo "Final configurations copied to $SIM_INPUT_CONF."


# --- ESECUZIONE SIMULAZIONE PRINCIPALE 4.2 ---
echo "--------------------------------------------------------"
echo "Starting MD simulation for Exercise 4.2 (EQUILIBRIUM SIMULATION)..."

(
    cd "$SOURCE_DIR" || { echo "ERROR: Not possible changing directory in $SOURCE_DIR."; exit 1; }
    ./simulator.exe || { echo "ERROR: simulator.exe execution failed."; exit 1; }
) || { echo "CRITICAL ERROR: Simulator execution failed. Exiting main script."; exit 1; }

echo "Simulation 4.2.2 completed."

# --- ARCHIVIAZIONE SIMULAZIONE PRINCIPALE ---
echo "--------------------------------------------------------"
if [ -d "$INPUT_DIR" ] && [ -d "$OUTPUT_DIR" ]; then
    echo "Copying INPUT and OUTPUT for Main Simulation phase to $RUN_DIR..."
    cp -r "$INPUT_DIR" "$RUN_DIR/"
    cp -r "$OUTPUT_DIR" "$RUN_DIR/"
    echo "INPUT and OUTPUT copied to $RUN_DIR."
else
    echo "ATTENTION: INPUT or OUTPUT directory not found after simulation 4.2."
fi

# --- PULIZIA FILE OUTPUT ---
echo "--------------------------------------------------------"
echo "FINAL CLEANUP: Removing output files generated in $OUTPUT_DIR directory using 'make remove'."
make -C "$SOURCE_DIR" remove || { echo "ATTENTION: make remove failed, continuing."; }

echo "--------------------------------------------------------"
echo "Simulation 4.2 (Main) completed and archived in $RUN_DIR."

echo "========================================================"
echo "                   STARTING EXERCISE 4.3                  "
echo "========================================================"

echo "--------------------------------------------------------"
echo "      Time-Reversal (starting from equilibration)      "
echo "--------------------------------------------------------"


EX_DIR="es_4.3" 
B1_DIR="$EX_DIR/back_1" # Inversione corta
B2_DIR="$EX_DIR/back_2" # Inversione lunga

# 3. Rimuove e ricrea la struttura delle directory
echo "CLEANUP: Removing previous archive ($EX_DIR)."
rm -rf "$EX_DIR"
echo "Creating subdirectories for the two time-reversals."
mkdir -p "$B1_DIR/plot" 
mkdir -p "$B2_DIR/plot" 

# Directory dei dati sorgente per le due inversioni
EQ_OUT_CONF="$EQ_DIR/OUTPUT/CONFIG"  # Configurazioni dalla run di Equilibrazione (corta)
RUN_OUT_CONF="$RUN_DIR/OUTPUT/CONFIG" # Configurazioni dalla run di Produzione (lunga)
SIM_INPUT_CONF="$INPUT_DIR/CONFIG"
EQ_TEMP_FILE="$EQ_DIR/OUTPUT/temperature.dat"
RUN_TEMP_FILE="$RUN_DIR/OUTPUT/temperature.dat"

# ------------------------------------------------------------------------------
# --- INVERSIONE 1 (Corta: NBLOCKS=5) ---
echo "--------------------------------------------------------"
echo "PREPARATION: Reverse simulation 1 starting from the final configuration of equilibration 4.2.1"

# 1. Estrae la temperatura
echo "Extracting last block's average temperature from file $EQ_TEMP_FILE..."
LAST_TEMP=$(extract_last_temp "$EQ_TEMP_FILE")
echo "Last block's average temperature extracted: T_start* = $LAST_TEMP.."

# 2. Modifica i parametri di input
echo "Modifying input file..."
modify_input_param "TEMP" "$LAST_TEMP" 
modify_input_param "RESTART" "1"       
modify_input_param "NBLOCKS" "5"       
modify_input_param "DISTRIBUTION_TYPE" "0"
echo "--------------------------------------------------------"
echo "Printing initial $INPUT_FILE:"
cat $INPUT_FILE
echo "--------------------------------------------------------"

# 3. Copia le configurazioni per la TIME-REVERSAL (INVERSIONE)
echo "Copying inverted configurations from $EQ_OUT_CONF to simulate time reversal (swapping config.xyz with conf-1.xyz and vice versa)."
cp "$EQ_OUT_CONF/config.xyz" "$SIM_INPUT_CONF/conf-1.xyz" # r(finale) -> r(iniziale-dt)
cp "$EQ_OUT_CONF/conf-1.xyz" "$SIM_INPUT_CONF/config.xyz" # r(finale-dt) -> r(iniziale)
echo "Final configurations copied to $SIM_INPUT_CONF."

# --- ESECUZIONE INVERSIONE 1 ---
echo "--------------------------------------------------------"
echo "Starting MD simulation for Exercise 4.3 (REVERSE SIMULATION 1)..."

(
    cd "$SOURCE_DIR" || { echo "ERROR: Not possible changing directory in $SOURCE_DIR."; exit 1; }
    ./simulator.exe || { echo "ERROR: simulator.exe execution failed."; exit 1; }
) || { echo "CRITICAL ERROR: Simulator execution failed. Exiting main script."; exit 1; }

echo "Time-reverse simulation 1 completed."

# --- ARCHIVIAZIONE INVERSIONE 1 ---
echo "--------------------------------------------------------"
if [ -d "$INPUT_DIR" ] && [ -d "$OUTPUT_DIR" ]; then
    echo "Copying INPUT and OUTPUT to $B1_DIR..."
    cp -r "$INPUT_DIR" "$B1_DIR/"
    cp -r "$OUTPUT_DIR" "$B1_DIR/"
    echo "INPUT and OUTPUT copied to $RUN_DIR."
else
    echo "ATTENTION: INPUT or OUTPUT directory not found after backward simulation 4.3.1."
fi

# --- PULIZIA OUTPUT ---
echo "--------------------------------------------------------"
echo "FINAL CLEANUP: Removing output files generated in $OUTPUT_DIR directory using 'make remove'."
make -C "$SOURCE_DIR" remove || { echo "ATTENTION: make remove failed, continuing."; }

echo "--------------------------------------------------------"
echo "First reverse simulation 4.3.1 completed and archived in $B1_DIR."

echo "--------------------------------------------------------"
echo "  Time-Reversal (starting from equilibrium simulation)  "
echo "--------------------------------------------------------"

echo "--------------------------------------------------------"
echo "PREPARATION: Reverse simulation 2 starting from the final configuration of simulation 4.2.2"

# 1. Estrae la temperatura media 
echo "Extracting last block's average temperature from file $RUN_TEMP_FILE..."
LAST_TEMP_LONG=$(extract_last_temp "$RUN_TEMP_FILE")
echo "Last block's average temperature extracted: T_start* = $LAST_TEMP_LONG.."

# 2. Modifica i parametri di input
echo "Modifying input file..."
modify_input_param "TEMP" "$LAST_TEMP_LONG" 
modify_input_param "NBLOCKS" "25" # Aumenta i blocchi per il test di irreversibilità a lungo termine
modify_input_param "RESTART" "1"

echo "--------------------------------------------------------"
echo "Printing initial $INPUT_FILE:"
cat $INPUT_FILE
echo "--------------------------------------------------------"

# 3. Copia le configurazioni per la TIME-REVERSAL (INVERSIONE)
echo "Copying inverted configurations from $RUN_OUT_CONF to simulate time reversal (swapping config.xyz with conf-1.xyz and vice versa)."
cp "$RUN_OUT_CONF/config.xyz" "$SIM_INPUT_CONF/conf-1.xyz" # r(finale Lunga) -> r(iniziale-dt)
cp "$RUN_OUT_CONF/conf-1.xyz" "$SIM_INPUT_CONF/config.xyz" # r(finale-dt Lunga) -> r(iniziale)
echo "Final configurations copied to $SIM_INPUT_CONF."

# --- ESECUZIONE INVERSIONE 2 ---
echo "--------------------------------------------------------"
echo "Starting MD simulation for Exercise 4.3 (REVERSE SIMULATION 2)..."

(
    cd "$SOURCE_DIR" || { echo "ERROR: Not possible changing directory in $SOURCE_DIR."; exit 1; }
    ./simulator.exe || { echo "ERROR: simulator.exe execution failed."; exit 1; }
) || { echo "CRITICAL ERROR: Simulator execution failed. Exiting main script."; exit 1; }

echo "Time-reverse simulation 2 completed."
echo "--------------------------------------------------------"

# --- ARCHIVAZIONE INVERSIONE 2 ---
echo "--------------------------------------------------------"
if [ -d "$INPUT_DIR" ] && [ -d "$OUTPUT_DIR" ]; then
    echo "Copying INPUT and OUTPUT to $B2_DIR..."
    cp -r "$INPUT_DIR" "$B2_DIR/"
    cp -r "$OUTPUT_DIR" "$B2_DIR/"
    echo "INPUT and OUTPUT copied to $RUN_DIR."
else
    echo "ATTENTION: INPUT or OUTPUT directory not found after backward simulation 4.3.2."
fi

# --- PULIZIA OUTPUT ---
echo "--------------------------------------------------------"
echo "FINAL CLEANUP: Removing output files generated in $OUTPUT_DIR directory using 'make remove'."
make -C "$SOURCE_DIR" remove || { echo "ATTENTION: make remove failed, continuing."; }

echo "--------------------------------------------------------"
echo "Second reverse simulation 4.3.2 completed and archived in $B2_DIR."

echo "========================================================"
echo "FINAL RESET: Restoring input file and configurations."
# ------------------------------------------------------------------------------
# --- PULIZIA FINALE E RESET DEL FILE DI INPUT ---
# ------------------------------------------------------------------------------

# Imposta i valori di default/iniziali per pulizia
echo "Resetting input file..."
modify_input_param "DISTRIBUTION_TYPE" "0" 
modify_input_param "RESTART" "0"          
modify_input_param "TEMP" "2.0"            
modify_input_param "NBLOCKS" "20"

echo "--------------------------------------------------------"
echo "Printing initial $INPUT_FILE:"
cat $INPUT_FILE
echo "--------------------------------------------------------"

# Elimina le configurazioni di restart e reimposta la configurazione iniziale (FCC)
echo "Removing temporary configuration file 'conf-1.xyz'."
rm $INPUT_DIR/CONFIG/conf-1.xyz # rm: rimuove il file conf-1.xyz
echo "Restoring initial configuration (config.fcc) to config.xyz."
cp $INPUT_DIR/CONFIG/config.fcc $INPUT_DIR/CONFIG/config.xyz 


echo "--------------------------------------------------------"
echo "Ex 4 completed. Check results in directories es_4.1, es_4.2, es_4.3."
echo "========================================================"