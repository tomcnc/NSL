import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import sys
import os
import argparse
import functions
import plot_config as pc

def main():
    parser = argparse.ArgumentParser(
        description="Genera un'animazione MP4 di un percorso ottimizzato.",
        formatter_class=argparse.RawTextHelpFormatter # Per formattare meglio l'help
    )

    # Argomenti posizionali
    parser.add_argument('CITY_DIST', type=str, help='Percorso del file con le distanze tra le città.')
    parser.add_argument('BEST_PATH', type=str, help='Percorso del file con la sequenza del percorso ottimale.')
    parser.add_argument('OUTPUT_FILE', type=str, help='Nome del file MP4 di output.')
    
    # Argomento numerico, gestisce automaticamente la conversione a int e l'errore se non è un numero
    parser.add_argument('INTERVAL_MS', type=int, help='Intervallo (in millisecondi) tra i frame dell\'animazione.')

    args = parser.parse_args()
    
    # Chiamata alla funzione (i nomi sono gli stessi)
    try:
        # Nota: Non è necessario fare str(args.CITY_DIST) perché argparse ha già gestito il tipo str.
        functions.mp4_animation_creation(
            args.CITY_DIST, 
            args.BEST_PATH, 
            args.OUTPUT_FILE, 
            args.INTERVAL_MS
        )
    except Exception as e:
        print(f'Errore durante la creazione dell\'animazione: {e}')
        sys.exit(1) # Esci con codice di errore se la funzione fallisce

if __name__ == "__main__":

    pc.apply_custom_mpl_settings()

    main()