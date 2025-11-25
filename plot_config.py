import matplotlib.pyplot as plt

def apply_custom_mpl_settings(style_name='default_thesis'):
    """
    Applica una configurazione di stile Matplotlib coerente e professionale.

    Parameters:
    - style_name (str): Nome dello stile da applicare (per future estensioni).
    """

    # --- DIZIONARIO DI CONFIGURAZIONE GLOBALE ---
    
    # Raccogliamo tutte le impostazioni in un unico dizionario per chiarezza.
    config_dict = {
        # 1. FONT AND TEXT SETTINGS
        'font.family': 'sans-serif',
        'font.sans-serif': ['Arial', 'Helvetica', 'Lato', 'DejaVu Sans'],
        'font.size': 11,
        'axes.titlesize': 15,
        'axes.titleweight': 'bold',
        'axes.labelsize': 13,
        'legend.fontsize': 11,

        # 2. AXES, TICKS, AND GRID SETTINGS
        'axes.grid': True,
        'grid.linestyle': '--',
        'grid.color': 'lightgray',
        'grid.alpha': 0.7,
        'axes.spines.right': True,
        'axes.spines.top': True,
        'axes.edgecolor': 'black',
        'axes.linewidth': 1.2,
        'xtick.direction': 'in',
        'ytick.direction': 'in',
        'xtick.major.size': 6,
        'ytick.major.size': 6,
        'xtick.top': True,
        'ytick.right': True,
        'xtick.labeltop': True,
        'ytick.labelright': True,
        'xtick.minor.visible': True,
        'ytick.minor.visible': True,
        'xtick.minor.top': True,
        'ytick.minor.right': True,
        'xtick.minor.size': 4,
        'xtick.minor.width': 0.8,
        'ytick.minor.width': 0.8,

        # 3. LINES AND MARKERS SETTINGS
        'lines.linewidth': 2.0,
        'lines.markersize': 4,
        'lines.markeredgewidth': 1.0,

        # 4. ERRORBAR SETTINGS
        'errorbar.capsize': 0.0,

        # 5. LEGEND AND SAVING SETTINGS
        'figure.dpi': 300,
        'savefig.format': 'png',
        'savefig.bbox': 'tight',
        'savefig.pad_inches': 0.05,
        'legend.frameon': True,
        'legend.edgecolor': 'lightgray',
        'legend.fancybox': True,
    }

    # Applica il dizionario a rcParams
    plt.rcParams.update(config_dict)
    # print("Matplotlib configurato con stile '{}'.".format(style_name))
    
    # Restituisce il dizionario per eventuali debug o override esterni
    # return config_dict