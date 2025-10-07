#!/bin/bash

# ==============================================================================
# SCRIPT PER LA GESTIONE DEL RANDOM WALK (DISCRETO/CONTINUO)
# ==============================================================================

# Funzione per visualizzare l'uso corretto dello script
# Questa funzione viene chiamata in caso di errore di input. 
# Stampa il formato di utilizzo corretto e termina lo script con codice di errore (exit 1).
usage() {
    echo "Uso: $0 [discrete | continuous]"
    echo "Esempio: $0 discrete"
    exit 1
}

# Verifica che sia stato passato esattamente un argomento
# Il parametro speciale $# contiene il numero di argomenti passati allo script.
if [ "$#" -ne 1 ]; then
    usage # Chiama la funzione di aiuto in caso di input non corretto
fi

# Converte l'input in minuscolo
# `tr '[:upper:]' '[:lower:]'` converte l'argomento ricevuto in minuscolo, 
# rendendo lo script case-insensitive (accetta "Discrete", "DISCRETE", "discrete", ecc.).
MODE=$(echo "$1" | tr '[:upper:]' '[:lower:]')

# Definisce le directory e i file
# Definizione di variabili per la navigazione e la leggibilità del codice.

BASE_DIR=$(pwd) # Salva il percorso della directory corrente (dove viene eseguito lo script, es. es_2.2)
RANDOMWALK_DIR="$BASE_DIR/RANDOMWALK" # Directory principale dei sorgenti e degli I/O
INPUT_DIR="$RANDOMWALK_DIR/INPUT"     # Directory dei file di input (es. input.dat)
OUTPUT_DIR="$RANDOMWALK_DIR/OUTPUT"   # Directory in cui il codice compila salva i risultati temporanei
SOURCE_DIR="$RANDOMWALK_DIR/SOURCE"   # Directory dei file sorgenti (.cpp, .h) e del Makefile
INPUT_FILE="$INPUT_DIR/input.dat"     # Percorso completo del file di input da modificare
EXECUTABLE_NAME="main.exe"            # Nome atteso dell'eseguibile generato
EXECUTABLE_PATH="$SOURCE_DIR/$EXECUTABLE_NAME" # Percorso completo dell'eseguibile

# Variabili specifiche per la modalità e destinazione
# Struttura 'case' che gestisce la logica basata sull'argomento ($MODE) fornito.
case "$MODE" in
    "discrete")
        # Variabile per il valore da scrivere nel file di input
        RW_TYPE_VALUE="DISCRETE" 
        # Directory finale dove archiviare i risultati (snapshot dell'esecuzione)
        DEST_DIR="$BASE_DIR/RW_DISCRETE" 
        ;;
    "continuous")
        RW_TYPE_VALUE="CONTINUOUS"
        DEST_DIR="$BASE_DIR/RW_CONTINUOUS"
        ;;
    *)
        # Gestisce il caso in cui l'argomento non sia né "discrete" né "continuous".
        echo "Errore: la modalità specificata deve essere 'discrete' o 'continuous'."
        usage
        ;;
esac

echo "--- Inizio simulazione per Random Walk: $RW_TYPE_VALUE ---"
echo ""

# --- 1. Modifica del file di input (con AWK e limite ENDINPUT) ---
TEMP_FILE="${INPUT_FILE}.tmp"
echo "1. Modifica di $INPUT_FILE: impostazione di RW_TYPE = $RW_TYPE_VALUE (Ricerca limitata a 'ENDINPUT')"

# *************** INIZIO BLOCCO CORRETTO AWK ***************
# Usa AWK (Aho, Weinberger, Kernighan) per una potente manipolazione testuale del file.
# L'uso di '-v' definisce la variabile `rw_type_val` accessibile all'interno dello script AWK.
awk -v rw_type_val="$RW_TYPE_VALUE" '{
    # 1. Se non abbiamo raggiunto ENDINPUT e troviamo RW_TYPE:
    if (limit_reached != 1 && $1 == "RW_TYPE") {
        # Stampa la riga modificata.
        print "RW_TYPE " rw_type_val; 
        # Passa subito alla riga successiva per evitare la doppia stampa.
        next 
    }
    
    # 2. Se incontriamo ENDINPUT, impostiamo il flag 'limit_reached'
    #    (La stampa è gestita dal blocco finale)
    if ($1 == "ENDINPUT") {
        limit_reached = 1; 
    }
    
    # 3. Stampa la riga corrente. Questo stampa tutte le righe 
    #    NON modificate (e per le quali non è stato eseguito 'next').
    print $0

}' "$INPUT_FILE" > "$TEMP_FILE" # L'output di AWK viene reindirizzato a un file temporaneo

# *************** FINE BLOCCO CORRETTO AWK ***************

# Verifica se il file temporaneo è stato creato e sposta il risultato
# Il test '-s' verifica che il file esista e non sia vuoto (dimensione > 0).
if [ -s "$TEMP_FILE" ]; then
    mv "$TEMP_FILE" "$INPUT_FILE" # Sovrascrive il file originale con la versione modificata.
    echo "   Modifica completata: RW_TYPE impostato a $RW_TYPE_VALUE."
else
    # Gestione critica dell'errore di AWK.
    echo "Errore critico: Impossibile modificare il file di input con 'awk'. Interruzione."
    rm -f "$TEMP_FILE" # Rimuove eventuali file temporanei parzialmente scritti/danneggiati.
    exit 1
fi
echo ""

# --- 2. Compilazione (se l'eseguibile non esiste) ---
# Il test '! -f' verifica che l'eseguibile non esista.
if [ ! -f "$EXECUTABLE_PATH" ]; then
    echo "2. L'eseguibile $EXECUTABLE_NAME non è presente. Avvio compilazione."
    # Le parentesi tonde '( )' creano una subshell: le modifiche di directory (cd) 
    # fatte all'interno non influenzano l'ambiente principale dello script.
    (
        cd "$SOURCE_DIR" || exit 1 # Entra in SOURCE, termina lo script se fallisce (|| exit 1)
        
        # Uso dei target specifici del tuo Makefile per la pulizia (prevenzione conflitti)
        echo "   -> Pulizia file oggetto e eseguibile..."
        make remove_o > /dev/null   # Rimuove i file oggetto (.o), output soppresso (> /dev/null)
        make remove_exe > /dev/null # Rimuove il vecchio eseguibile, output soppresso
        
        echo "   -> Avvio make $EXECUTABLE_NAME..."
        make "$EXECUTABLE_NAME"     # Compila l'eseguibile specifico
        
        # $? contiene il codice di uscita dell'ultimo comando. 0 indica successo.
        if [ $? -ne 0 ]; then
            echo "Errore durante la compilazione. Interruzione."
            exit 1
        fi
        echo "   Compilazione completata."
    )
else
    echo "2. L'eseguibile $EXECUTABLE_NAME è già presente. Salto la compilazione."
fi
echo ""

# --- 3. Esecuzione della simulazione ---
echo "3. Esecuzione della simulazione..."
(
    # Entra nella directory SOURCE per eseguire l'eseguibile
    cd "$SOURCE_DIR" || exit 1 
    ./"$EXECUTABLE_NAME" # Esegue il programma
    if [ $? -ne 0 ]; then
        echo "Errore durante l'esecuzione della simulazione. Interruzione."
        exit 1
    fi
)
echo "   Simulazione completata."
echo ""

# --- 4. Archiviazione dei risultati (OUTPUT e INPUT) ---
echo "4. Archiviazione dei risultati nella directory: $DEST_DIR"

# Assicurati che la directory di destinazione esista
# `mkdir -p` crea la directory solo se non esiste.
mkdir -p "$DEST_DIR"

# PASSO DI PULIZIA AGGIUNTO: Pulizia mirata dei file nell'archivio di destinazione
echo "   -> Pulizia dei file nell'archivio di destinazione per garantire un snapshot pulito."

# Pulizia mirata della precedente cartella OUTPUT (rimuove solo i contenuti, non la cartella)
# Usa 'rm' per rimuovere tutti i file (*.*) nei sottodirectory critici.
if [ -d "$DEST_DIR/OUTPUT" ]; then
    # Correzione: `rm "$DEST_DIR/OUTPUT"/*.* rm` era un comando errato, rimosso il secondo `rm`
    rm "$DEST_DIR/OUTPUT"/*.* "$DEST_DIR/OUTPUT/RW"/*.*
    rm "$DEST_DIR/OUTPUT/SQRT_MEAN_DISTANCE2"/*.*
fi

# Rimuovi la vecchia cartella INPUT in modo ricorsivo (e il suo contenuto)
rm -r "$DEST_DIR/INPUT"

# Copia la directory OUTPUT in modo ricorsivo (sovrascriverà i file appena rimossi)
cp -r "$OUTPUT_DIR" "$DEST_DIR/"
# Copia la directory INPUT in modo ricorsivo (il file modificato è così archiviato)
cp -r "$INPUT_DIR" "$DEST_DIR/"

if [ $? -ne 0 ]; then
    echo "Errore durante la copia delle directory. Interruzione."
    exit 1
fi
echo "   Archiviazione completata."
echo ""

# Fase di pulizia dei plot (.png)
# Questa sezione è stata aggiunta per rimuovere i plot generati (es. dal codice Python).
echo "Pulizia dei plot in es_2.2/path/..."

# Rimuove tutti i file (*.*) dalla directory dei plot. 
# Si noti che questo rimuove tutti i file, non solo i .png (come richiesto in precedenza).
# SUGGERIMENTO: Per rimuovere solo PNG e PDF: rm -f es_2.2/path/*.png es_2.2/path/*.pdf
rm -f es_2.2/path/*.* echo "Pulizia completata."

# --- 5. Pulizia della directory OUTPUT in RANDOMWALK ---
echo "5. Pulizia (svuotamento) dei file in $OUTPUT_DIR tramite 'make remove_output'"

(
    # Entra nella directory SOURCE per eseguire il target del Makefile
    cd "$SOURCE_DIR" || exit 1 
    make remove_output # Esegue il comando 'make' per pulire i risultati temporanei
    if [ $? -ne 0 ]; then
        echo "Attenzione: Errore durante la pulizia dei file di output, procedere comunque."
    fi
)

echo "   Pulizia completata: la directory OUTPUT è ora vuota."
echo ""

echo "--- Simulazione $RW_TYPE_VALUE terminata con successo! ---"