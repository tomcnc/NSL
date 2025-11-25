import numpy as np
import re

def leggi_nparam_da_input(nome_file):
    """
    Legge un file, cerca la riga che inizia con 'NPARAM' e salva 
    il secondo e il terzo dato (valore2 e valore3) in variabili separate.

    Args:
        nome_file (str): Il percorso del file da leggere.

    Returns:
        tuple: (valore2, valore3) se la riga viene trovata, altrimenti (None, None).
    """
    # 1. Pattern per trovare la riga NPARAM e catturare i tre valori successivi
    # ^\s*NPARAM\s+  -> Inizia la riga (^), con spazi opzionali (\s*) seguiti da NPARAM e uno o più spazi (\s+)
    # (\S+)          -> Gruppo 1: Cattura il primo valore non-spazio (valore1)
    # \s+            -> Uno o più spazi
    # (\S+)          -> Gruppo 2: Cattura il secondo valore non-spazio (valore2)
    # \s+            -> Uno o più spazi
    # (\S+)          -> Gruppo 3: Cattura il terzo valore non-spazio (valore3)
    pattern = re.compile(r"^\s*NPARAM\s+\S+\s+(\S+)\s+(\S+)") 

    try:
        with open(nome_file, 'r') as file:
            for riga in file:
                # Cerca la riga corrispondente al pattern
                match = pattern.search(riga)
                
                if match:
                    # Trovata la riga!
                    # match.group(1) è il secondo valore catturato (valore2)
                    # match.group(2) è il terzo valore catturato (valore3)
                    
                    # Tentiamo di convertire i valori in float o intero
                    try:
                        dato_due = float(match.group(1))
                        dato_tre = float(match.group(2))
                        
                        # Restituisce i due valori trovati
                        return dato_due, dato_tre
                    except ValueError:
                        print(f"Errore: I dati trovati ('{match.group(1)}', '{match.group(2)}') non sono numerici.")
                        return None, None
        
        # Se si esce dal ciclo senza trovare la riga
        print(f"Avviso: La riga 'NPARAM' non è stata trovata nel file {nome_file}.")
        return None, None

    except FileNotFoundError:
        print(f"Errore: File '{nome_file}' non trovato.")
        return None, None


def barlett_window(tau, tau_max):
    if(tau > tau_max):
        return 0.0
    return 1 - (tau/tau_max)

def unbiased_acf(quantity, var):
    
    t_max = len(quantity)
    acorr = np.zeros(t_max)

    for t in range(t_max):

        delta_t = t_max - t
        norm_num = 1.0 / delta_t
        x = 0.0
        y = 0.0
        xy = 0.0

        for t_prime in range(delta_t):

            x += quantity[t_prime]
            y += quantity[t_prime + t]
            xy += quantity[t_prime] * quantity[t_prime + t]

        
        num = norm_num * xy - (norm_num * x) * (norm_num * y)
        acorr[t] = num / var

    return acorr

def biased_acf(quantity, var):
    
    t_max = len(quantity)
    acorr = np.zeros(t_max)

    for t in range(t_max):

        delta_t = t_max - t
        norm = 1.0 / t_max
        xy = 0.0

        for t_prime in range(delta_t):
            xy += quantity[t_prime] * quantity[t_prime + t]
        
        num = norm * xy
        acorr[t] = (num / var) 

    return acorr

def integrated_ac_time(ac): 
    try:
        zero_crossing = np.where(ac < 0)[0][0]
    except IndexError:
        zero_crossing = len(ac)
    return 1 + 2 * np.sum(ac[1:zero_crossing])


def error(A_sum: float, A_sq_sum: float, i: int) -> float:
    
    # N_samp is the number of samples (blocks) calculated so far
    N_samp = i + 1 
    
    # If we have less than 2 samples, we cannot calculate the variance (ddof=1)
    if N_samp < 2:
        return 0.0
    
    # Calculate the cumulative mean of the squared block means (S2 / N_samp)
    A_sq_mean = A_sq_sum / N_samp
    # Calculate the cumulative mean of the block means (S1 / N_samp)
    A_mean = A_sum / N_samp

    # Calculate the variance numerator: E[A^2] - (E[A])^2
    num = A_sq_mean - A_mean**2

    # Calculate the Standard Deviation of the block means
    err = np.sqrt(num/(N_samp - 1))
    
    return err


def data_blocking(obs: np.ndarray, L: int) -> float:    # Calculates the final progressive error for a specific block length L
    
    M = len(obs)    # Total dataset size
    if L <= 0:
        return np.nan

    # Calculate N (number of blocks) using integer division (//)
    N = M // L
    
    if N < 2:
        # Cannot calculate variance with less than 2 blocks.
        return np.nan 
    
    # Determine the effective length of the data to be used (exact multiple of L)
    L_eff = N * L
    
    # Vectorized Calculation of Block Means (A_i)
    # 1. Truncation: obs[:L_eff] selects the data divisible by L
    # 2. Reshape: .reshape(N, L) creates a matrix where each row is a block 
    # 3. Mean: .mean(axis=1) calculates the mean for each row (single block mean A_i)
    block_means = obs[:L_eff].reshape(N, L).mean(axis=1)
    
    A_sum = 0.0      # Accumulator variable: Cumulative sum of block means (S1)
    A_sq_sum = 0.0   # Accumulator variable: Cumulative sum of squared block means (S2)
    
    last_error = 0.0 # Variable to store the error calculated on the last block

    # Loop over the N block means to calculate the progressive error
    for i in range(N):
        A_k = block_means[i]  # Current block mean (A_i)
        
        # Update the cumulative sum of means
        A_sum += A_k
        # Update the cumulative sum of squared means
        A_sq_sum += A_k * A_k
        
        # The final error (err[-1]) is calculated and saved only on the last iteration (i = N-1)
        if(i==N-1):
            last_error = error(A_sum, A_sq_sum, i)

    return last_error


def get_idx_min(arr):
    """
    Finds the minimum value and its index in a 1D array.
    Automatically handles NaNs by ignoring them.
    
    Args:
        arr (np.ndarray): 1D array of numeric data.
        
    Returns:
        tuple: (min_value, associated_index) or (None, None) on failure.
    """
    # Defensive dimensional check (optional but recommended)
    if arr.ndim != 1:
        raise ValueError(f"This function only accepts 1D arrays. Found dimensions: {arr.ndim}")
        
    try:
        # np.nanargmin is the safe choice for scientific data (ignores NaNs)
        idx = np.nanargmin(arr)
        val = arr[idx]
        return val, idx
        
    except ValueError:
        # Occurs if the array is empty or contains ONLY NaNs
        print("Warning: Empty array or composed entirely of NaNs.")
        return None, None