import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense, Input
from tensorflow.keras.callbacks import EarlyStopping, ReduceLROnPlateau

def generate_dataset_regression(input_dim, input_min, input_max, output_dim, n_data, func, sigma=0.0, seed=0, verbose=False):
    """
    Generates a synthetic dataset for a regression problem, including training, 
    test data, and noise-free test labels (Y_true_test).

    The function f(X) is defined by the 'func' argument, and Gaussian noise
    is added to the labels (Y) to simulate real-world measurement error.

    Args:
        input_dim (int): Dimension of the input space (P in R^P).
        input_min (float or np.array): Minimum value(s) for the uniform sampling of X.
        input_max (float or np.array): Maximum value(s) for the uniform sampling of X.
        output_dim (int): Dimension of the output space (Q in R^Q).
        n_data (int): Total number of samples to generate (N).
        func (callable): The analytical function to be fitted. Must accept input of shape (N, input_dim).
        sigma (float, optional): Standard deviation of the Gaussian noise applied to labels Y. Defaults to 0.0.
        seed (int, optional): Random seed for reproducibility. Defaults to 0.
        verbose (bool, optional): If True, prints dataset statistics. Defaults to False.

    Returns:
        tuple: (x_train, y_train, x_test, y_test, y_true_test)
    """
    
    # Set seed for reproducibility
    np.random.seed(seed)

    # Handle input ranges (support for both scalar and array limits)
    input_min = np.atleast_1d(input_min)
    input_max = np.atleast_1d(input_max)

    if input_min.size not in [1, input_dim] or input_max.size not in [1, input_dim]:
        raise ValueError(f"input_min/max must be a single float or an array of dimension {input_dim}.")
    
    # Split data: 80% Training, 20% Testing (Standard split)
    n_train = int(0.80 * n_data)
    n_test = n_data - n_train
    
    # 1. Feature Generation (X)
    # Uniform sampling in the specified range
    X_all = np.random.uniform(low=input_min, high=input_max, size=(n_data, input_dim))
    
    # Split features
    x_train = X_all[:n_train, :]
    x_test = X_all[n_train:, :]
    
    # 2. True Label Calculation (Y_true)
    # Apply the analytical function (vectorized operation recommended)
    y_true_all = func(X_all)
    
    # Reshape if output is 1D to match Keras expectation (N, 1)
    if y_true_all.ndim == 1:
        y_true_all = y_true_all.reshape(-1, 1) 
    
    # Check output consistency
    if y_true_all.shape[1] != output_dim:
        raise ValueError(f"The function 'func' generates an output of dimension {y_true_all.shape[1]}, but output_dim is set to {output_dim}")

    # Extract clean test labels for diagnostics
    y_true_test = y_true_all[n_train:, :]

    # 3. Add Gaussian Noise
    if sigma > 0.0:
        noise = np.random.normal(loc=0.0, scale=sigma, size=y_true_all.shape)
        y_all = y_true_all + noise
    else:
        y_all = y_true_all

    # Split labels
    y_train = y_all[:n_train, :]
    y_test = y_all[n_train:, :]
    
    # Optional Summary Print
    if verbose:
        print("\n--- Dataset Generation Summary ---")
        print(f"Total size: {n_data} samples (Train: {n_train}, Test: {n_test})")
        print(f"X range: [{input_min}, {input_max}]")
        print(f"Applied Gaussian Noise (Sigma): {sigma}")
        print(f"Shape x_train: {x_train.shape}, y_train: {y_train.shape}")
        print(f"Shape x_test: {x_test.shape}, y_test: {y_test.shape}")
        print(f"Shape y_true_test (noise-free): {y_true_test.shape}")
        print("--------------------------------------")
    
    return x_train, y_train, x_test, y_test, y_true_test


def generate_network_regression(input_dim, output_dim, hidden_neurons, act_func, optimizer, loss, metrics, verbose=False):
    """
    Builds and compiles a Keras Sequential model for regression tasks.
    
    Args:
        input_dim (int): Dimension of the input space.
        output_dim (int): Dimension of the output space.
        hidden_neurons (list of int): List of neurons for each hidden layer (e.g., [30, 20]).
        act_func (str): Activation function for hidden layers.
        optimizer (str/obj): Optimization algorithm (e.g., 'adam').
        loss (str/obj): Loss function (e.g., 'mse').
        metrics (list): List of metrics to monitor.
        verbose (bool, optional): If True, prints the model summary. Defaults to False.

    Returns:
        model: Compiled Keras model.
    """
    
    model = Sequential()
    
    # Deep Neural Network case
    if len(hidden_neurons) > 0:
        # First Hidden Layer (Needs input_shape)
        model.add(Dense(hidden_neurons[0], input_shape=(input_dim,), activation=act_func))
        
        # Subsequent Hidden Layers
        for n_units in hidden_neurons[1:]:
            model.add(Dense(n_units, activation=act_func))
            
        # Output Layer (Linear for regression)
        model.add(Dense(output_dim, activation='linear'))
        
    else:
        # Simple Linear Regression case (No hidden layers)
        model.add(Dense(output_dim, input_shape=(input_dim,), activation='linear'))

    model.compile(optimizer=optimizer, loss=loss, metrics=metrics)

    if verbose:
        print("\n--- Network Architecture ---")
        print(f"Structure: Input(R^{input_dim}) -> Hidden: {hidden_neurons} ({act_func}) -> Output(R^{output_dim})")
        model.summary()
    
    return model


def model_optimization_regression(model, x_train, y_train, batch_size, epochs, x_test, y_test, 
                                  patience_es=10, patience_lr=5, verbose=0):
    """
    Trains the Keras model using EarlyStopping and ReduceLROnPlateau.
    
    Args:
        model: Compiled Keras model.
        x_train, y_train: Training data.
        batch_size (int): Size of mini-batches.
        epochs (int): Max epochs.
        x_test, y_test: Validation data.
        patience_es (int): Patience for Early Stopping.
        patience_lr (int): Patience for Learning Rate Reduction.
        verbose (int, optional): 0 = silent, 1 = progress bar, 2 = one line per epoch. Defaults to 0.

    Returns:
        history: Keras History object.
    """
    
    # Callbacks configuration
    early_stopper = EarlyStopping(
        monitor='val_loss', 
        patience=patience_es, 
        restore_best_weights=True
    )
    
    lr_reducer = ReduceLROnPlateau(
        monitor='val_loss', 
        factor=0.5, 
        patience=patience_lr, 
        min_lr=1e-6
    )
    
    callbacks_list = [early_stopper, lr_reducer]
    
    if verbose > 0:
        print(f"Starting training (max {epochs} epochs)...")

    # Training
    history = model.fit(
        x=x_train, 
        y=y_train, 
        batch_size=batch_size,
        shuffle=True, 
        epochs=epochs, 
        validation_data=(x_test, y_test), 
        # callbacks=callbacks_list,
        verbose=verbose
    )
    
    if verbose > 0:
        print(f"\nTraining completed in {len(history.history['loss'])} epochs.")
        
    return history


def evaluate_model(model, x_test, y_test, y_true_test, batch_size=32, verbose=False):
    """
    Evaluates the model and compares predictions against the True Function.

    Args:
        model: Trained Keras model.
        x_test, y_test: Noisy test data.
        y_true_test: Noise-free true labels (for diagnostic MSE).
        batch_size (int): Batch size for evaluation.
        verbose (bool, optional): If True, prints evaluation metrics. Defaults to False.

    Returns:
        tuple: (results_dict, y_pred_array)
    """
    
    # Evaluate on noisy data (Generalization metric)
    # Force verbose=0 here to control output manually
    scores = model.evaluate(x_test, y_test, batch_size=batch_size, verbose=0)
    
    # Map scores to metric names
    results = {name: score for name, score in zip(model.metrics_names, scores)}
    
    # Diagnostic: Error vs True Function
    y_pred = model.predict(x_test, batch_size=batch_size, verbose=0)
    mse_on_true = np.mean(np.square(y_pred - y_true_test))
    results['mse_on_true'] = mse_on_true
    
    if verbose:
        print("\n--- Model Evaluation ---")
        print(f"Test Loss (MSE vs Y_noisy): {results['loss']:.4f}")
        if 'mae' in results:
            print(f"Test MAE (vs Y_noisy): {results['mae']:.4f}")
        print(f"MSE on True Function (vs Y_true): {results['mse_on_true']:.4f}")
    
    return results, y_pred


# --- PLOTTING FUNCTIONS ---

def plot_comparison_history(history_list, titles, metric_name='loss'):
    """
    Plots the training and validation metric side-by-side for multiple models.

    Args:
        history_list (list): List of Keras History objects.
        titles (list): List of titles for each subplot.
        metric_name (str): Metric to plot (e.g., 'loss', 'mae').
    """
    
    n_models = len(history_list)
    ncols = 3
    nrows = int(np.ceil(n_models / ncols))
    
    fig, axes = plt.subplots(nrows, ncols, figsize=(5 * ncols, 4 * nrows))
    axes = axes.flatten()
    fig.suptitle(f'Comparative Analysis of Model Training History ({metric_name.upper()})', fontsize=18, fontweight='bold')

    for i, history in enumerate(history_list):
        if i >= len(axes): break
            
        epochs = range(1, len(history.history['loss']) + 1)
        ax = axes[i]
        
        ax.plot(epochs, history.history[metric_name], label=f'Training {metric_name.upper()}', color='blue')
        
        val_metric_name = f'val_{metric_name}'
        if val_metric_name in history.history:
            ax.plot(epochs, history.history[val_metric_name], label=f'Validation {metric_name.upper()}', color='red', linestyle='--')
            
        ax.set_title(titles[i])
        ax.set_xlabel('Epochs')
        ax.set_ylabel(metric_name.upper())
        ax.legend(loc='upper right')
        ax.grid(True, linestyle='--', alpha=0.5)

    # Hide empty subplots
    for j in range(n_models, len(axes)):
        fig.delaxes(axes[j])
        
    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    plt.show()


def plot_comparison_fit_1d(model_list, x_test_list, y_test_list, y_true_test_list, y_pred_list, titles):
    """
    Plots the model fit (Y_pred) vs Noisy Data and True Function for 1D inputs.
    Requires input_dim = 1.
    """
    
    n_models = len(model_list)
    
    # Helper to standardize inputs into lists of length N
    def standardize_input(data):
        if isinstance(data, list):
            if len(data) != n_models:
                raise ValueError(f"Input list length does not match number of models ({n_models}).")
            return data
        else:
            return [data] * n_models

    x_test_list = standardize_input(x_test_list)
    y_test_list = standardize_input(y_test_list)
    y_true_test_list = standardize_input(y_true_test_list)
    
    # Check dimensions
    if x_test_list[0].shape[1] != 1:
        print("WARNING: Comparison fit plot skipped. This plot requires input_dim=1.")
        return

    ncols = 3
    nrows = int(np.ceil(n_models / ncols))
    
    fig, axes = plt.subplots(nrows, ncols, figsize=(6 * ncols, 5 * nrows))
    axes = axes.flatten()
    fig.suptitle('Comparative Analysis of Model Fit (1D)', fontsize=18, fontweight='bold')
    
    for i in range(n_models):
        if i >= len(axes): break
            
        ax = axes[i]
        
        x_test_i = x_test_list[i]
        y_test_i = y_test_list[i]
        y_true_test_i = y_true_test_list[i]
        y_pred_i = y_pred_list[i]
        
        # Sort for clean line plotting
        sorted_indices = x_test_i.flatten().argsort()
        x_test_sorted = x_test_i[sorted_indices]
        y_true_test_sorted = y_true_test_i[sorted_indices]
        y_pred_sorted = y_pred_i[sorted_indices]
        
        # Plotting
        ax.scatter(x_test_i, y_test_i, color='gray', s=8, alpha=0.4, label='Noisy Test Data')
        ax.plot(x_test_sorted, y_true_test_sorted, color='blue', linestyle='--', label='True Function')
        ax.plot(x_test_sorted, y_pred_sorted, color='red', label='Model Prediction')
        
        ax.set_title(titles[i])
        ax.set_xlabel('Input X')
        ax.set_ylabel('Output Y')
        ax.legend()
        ax.grid(True, linestyle='--', alpha=0.5)
        
    for j in range(n_models, len(axes)):
        fig.delaxes(axes[j])
        
    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    plt.show()


def plot_compare(x, mse_test, mse_true, title, variable_type):
    """
    Plots the final MSE values (Validation vs True) against a changing hyperparameter.
    """
    
    fig, axs = plt.subplots(1, 2, figsize=(12, 5))
    plt.suptitle(title, fontsize=18, fontweight='bold')

    # Handle Categorical variables (strings)
    if variable_type in ['optimizer', 'layers', 'activation function']:
        x_pos = np.arange(len(x))
        
        for ax, y_data, y_label in zip(axs, [mse_test, mse_true], ['Validation MSE', 'MSE on True Function']):
            ax.plot(x_pos, y_data, marker='o', linestyle='--')
            ax.set_ylabel(y_label)
            ax.set_xticks(x_pos)
            ax.set_xticklabels(x, rotation=45, ha='right')
            ax.set_title(y_label)
            ax.grid(True, linestyle='--', alpha=0.5)
            
            if variable_type == 'optimizer': ax.set_xlabel('Optimizer')
            elif variable_type == 'layers': ax.set_xlabel('Network Architecture')
            elif variable_type == 'activation function': ax.set_xlabel('Activation Function')
             
    # Handle Numerical variables
    else:
        for ax, y_data, y_label in zip(axs, [mse_test, mse_true], ['Validation MSE', 'MSE on True Function']):
            ax.plot(x, y_data, marker='o', linestyle='--')
            ax.set_ylabel(y_label)
            ax.set_title(y_label)
            ax.grid(True, linestyle='--', alpha=0.5)

            if variable_type == 'n_data': ax.set_xlabel('Dataset Size (N)')
            elif variable_type == 'sigma': ax.set_xlabel(r'Noise Level ($\sigma$)')
            elif variable_type == 'epochs': ax.set_xlabel('Epochs')
            else: ax.set_xlabel(variable_type)

    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    plt.show()


def plot_history_and_fit_1d(model_list, x_test_list, y_test_list, y_true_test_list, y_pred_list, history_list, titles_history, titles_fit, suptitle, metric_name='loss'):
    """
    Combined plot: Left column = Training History, Right column = 1D Fit.
    """
    n_models = len(history_list)
    if n_models != len(model_list):
        raise ValueError(f'Mismatch: {len(model_list)} models vs {n_models} histories.')

    ncols = 2
    nrows = int(np.ceil(n_models)) # One row per model usually better for side-by-side
    
    # Helper for standardization
    def standardize_input(data):
        if isinstance(data, list):
            if len(data) != n_models: raise ValueError("Input list length mismatch.")
            return data
        else: return [data] * n_models

    x_test_list = standardize_input(x_test_list)
    y_test_list = standardize_input(y_test_list)
    y_true_test_list = standardize_input(y_true_test_list)
    
    if x_test_list[0].shape[1] != 1:
        print("WARNING: plot_history_and_fit_1d skipped (requires input_dim=1).")
        return

    # Handle single model case
    if n_models == 1:
        fig, axs = plt.subplots(1, 2, figsize=(12, 5))
        axs = [axs] # Wrap in list to make iterable
    else:
        fig, axs = plt.subplots(n_models, 2, figsize=(12, 5 * n_models))

    fig.suptitle(suptitle, fontsize=18, fontweight='bold')

    for i in range(n_models):
        # Extract data
        history = history_list[i]
        epochs = range(1, len(history.history['loss']) + 1)
        x_test_i = x_test_list[i]
        y_test_i = y_test_list[i]
        y_true_test_i = y_true_test_list[i]
        y_pred_i = y_pred_list[i]
        
        # Sort for fit plot
        sorted_indices = x_test_i.flatten().argsort()
        x_test_sorted = x_test_i[sorted_indices]
        y_true_test_sorted = y_true_test_i[sorted_indices]
        y_pred_sorted = y_pred_i[sorted_indices]

        # Select axes (handle 1 row vs multiple rows)
        ax_hist = axs[i][0] if n_models > 1 else axs[0][0]
        ax_fit = axs[i][1] if n_models > 1 else axs[0][1]

        # 1. History Plot
        ax_hist.set_title(titles_history[i])
        ax_hist.plot(epochs, history.history[metric_name], label=f'Train {metric_name.upper()}', color='blue')
        if f'val_{metric_name}' in history.history:
            ax_hist.plot(epochs, history.history[f'val_{metric_name}'], label=f'Val {metric_name.upper()}', color='red', linestyle='--')
        ax_hist.set_xlabel('Epochs')
        ax_hist.set_ylabel(metric_name.upper())
        ax_hist.legend()
        ax_hist.grid(True, linestyle='--', alpha=0.5)

        # 2. Fit Plot
        ax_fit.set_title(titles_fit[i])
        ax_fit.scatter(x_test_i, y_test_i, color='gray', s=8, alpha=0.4, label='Data')
        ax_fit.plot(x_test_sorted, y_true_test_sorted, color='black', linestyle='--', label='True Function')
        ax_fit.plot(x_test_sorted, y_pred_sorted, color='red', label='Prediction')
        ax_fit.set_xlabel('Input X')
        ax_fit.set_ylabel('Output Y')
        ax_fit.legend()
        ax_fit.grid(True, linestyle='--', alpha=0.5)

    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    plt.show()


def plot_optimizer_analysis(model_list, x_test, y_test, y_true_test, y_pred_list, history_list, titles_list, loss_name='loss', metric_name='mae'):
    """
    Plots for each model (optimizer):
    1. Training and validation loss.
    2. Training and validation secondary metric.
    3. Prediction curve fit vs. test data and true curve.
    
    Args:
        model_list (list): List of Keras models.
        x_test (np.array): Test input data.
        y_test (np.array): Test output data (with noise).
        y_true_test (np.array): Theoretical output data (without noise).
        y_pred_list (list): List of model predictions.
        history_list (list): List of Keras History objects.
        titles_list (list): List of titles (e.g., "Optimizer = sgd").
        loss_name (str): Name of the loss (e.g., 'loss', 'mse').
        metric_name (str): Name of the secondary metric (e.g., 'mae').
    """
    
    num_models = len(model_list)
    
    # 3 columns: Loss, Metric, Fit Curve
    fig, axs = plt.subplots(nrows=num_models,ncols=3,figsize=(15, 5 * num_models))
    
    # Add a general title
    fig.suptitle('Model Analysis (Training history and model predictions)', fontsize=18, fontweight='bold', y=1.02)

    # Ensure axs is always a 2D array, even for a single model
    if num_models == 1:
        axs = np.array([axs])

    # Ensure 'val_loss' and 'val_<metric_name>' are in history
    val_loss_name = f'val_{loss_name}'
    val_metric_name = f'val_{metric_name}'

    for i in range(num_models):
        history = history_list[i]
        y_pred = y_pred_list[i]
        epochs = range(1, len(history.history[loss_name]) + 1)
        title = titles_list[i]
        
        # --- Prepare data for Fit Plot (ordering) ---
        # Sort test data based on x_test input to draw smooth curves
        sorted_indices = np.argsort(x_test.flatten())
        x_test_sorted = x_test[sorted_indices]
        y_test_sorted = y_test[sorted_indices] # Validation data with noise
        y_true_test_sorted = y_true_test[sorted_indices] # Theoretical curve (target)
        y_pred_sorted = y_pred[sorted_indices] # Predicted curve

        # --- COLUMN 1: Learning Process (Loss) ---
        ax1 = axs[i, 0]
        ax1.set_title(f"Loss - {title}")
        ax1.plot(epochs, history.history[loss_name], label=f'Training {loss_name.upper()}', color='blue')
        if val_loss_name in history.history:
            ax1.plot(epochs, history.history[val_loss_name], label=f'Validation {loss_name.upper()}', color='orange', linestyle='--')
        ax1.set_xlabel('Epochs')
        ax1.set_ylabel(loss_name.upper())
        ax1.legend()
        ax1.grid(True)

        # --- COLUMN 2: Learning Process (Secondary Metric) ---
        ax2 = axs[i, 1]
        ax2.set_title(f"Metric - {title}")
        if metric_name in history.history:
            ax2.plot(epochs, history.history[metric_name], label=f'Training {metric_name.upper()}', color='green')
            if val_metric_name in history.history:
                ax2.plot(epochs, history.history[val_metric_name], label=f'Validation {metric_name.upper()}', color='red', linestyle='--')
            ax2.set_xlabel('Epochs')
            ax2.set_ylabel(metric_name.upper())
            ax2.legend()
        else:
             ax2.text(0.5, 0.5, f"Metric '{metric_name.upper()}' not found\nfor {title}", 
                     horizontalalignment='center', verticalalignment='center', transform=ax2.transAxes)
        ax2.grid(True)
        
        # --- COLUMN 3: Data Fit (Prediction Curve vs True Curve vs Test Data) ---
        ax3 = axs[i, 2]
        ax3.set_title(f"Curve Fit - {title}")
        ax3.scatter(x_test, y_test, label='Test Data (with noise)', s=10, alpha=0.3, color='gray')
        ax3.plot(x_test_sorted, y_true_test_sorted, label='Theoretical Curve (Target)', color='black', linestyle='--')
        ax3.plot(x_test_sorted, y_pred_sorted, label='Predicted Curve (Model)', color='red')
        ax3.set_xlabel('x')
        ax3.set_ylabel('y')
        ax3.legend()

    plt.tight_layout(rect=[0, 0, 1, 1.0]) # Adjust subplots, leaving space for suptitle
    plt.show()


def plot_history_and_3d_fit(model_list, history_list, titles, f_true, x_min, x_max, metric_name='loss'):
    """
    Plots the learning curve (2D) and the fitted surface (3D) for each model.
    Includes fixes for Matplotlib 3D axis tick issues.
    """
    
    n_models = len(model_list)
    ncols = 2 
    nrows = n_models 

    fig = plt.figure(figsize=(18, 7 * nrows)) 
    fig.suptitle('Neural Network Complexity and Activation Analysis (2D Input)', 
                 fontsize=18, fontweight='bold', y=1.0)
    
    axes = []
    
    # Explicit Axis Creation
    for i in range(nrows):
        ax_hist = fig.add_subplot(nrows, ncols, i * ncols + 1)
        ax_3d = fig.add_subplot(nrows, ncols, i * ncols + 2, projection='3d')
        axes.append((ax_hist, ax_3d))
    
    for i in range(n_models):
        history = history_list[i]
        model = model_list[i]
        ax_hist, ax_3d = axes[i]
        title = titles[i]
        epochs = range(1, len(history.history[metric_name]) + 1)
        val_metric_name = f'val_{metric_name}'

        # --- LEFT: Learning History ---
        ax_hist.set_title(f"Loss - {title}")
        ax_hist.plot(epochs, history.history[metric_name], label=f'Training {metric_name.upper()}', color='blue')
        if val_metric_name in history.history:
            ax_hist.plot(epochs, history.history[val_metric_name], label=f'Validation {metric_name.upper()}', color='red', linestyle='--')
        
        ax_hist.set_xlabel('Epochs')
        ax_hist.set_ylabel(metric_name.upper())
        ax_hist.legend(loc='upper right')
        ax_hist.grid(True, linestyle='--', alpha=0.6)

        # --- RIGHT: Surface Fit (3D) ---
        ax_3d.set_title(f"Predicted Surface - {title}")

        # Fix for 3D Ticks (removing global 2D formatting influence)
        ax_3d.tick_params(axis='x', which='both', top=False, labeltop=False, bottom=True)
        ax_3d.tick_params(axis='y', which='both', right=False, labelright=False, left=True)
        ax_3d.tick_params(axis='z', which='both', right=True, labelright=True, left=False)
        
        # Grid generation
        n_plot = 50
        x_range = np.linspace(x_min, x_max, n_plot)
        y_range = np.linspace(x_min, x_max, n_plot)
        X_plot, Y_plot = np.meshgrid(x_range, y_range)
        X_flat = np.column_stack((X_plot.flatten(), Y_plot.flatten()))
        
        # Predictions
        Z_true = f_true(X_flat).reshape(n_plot, n_plot)
        Z_pred = model.predict(X_flat, verbose=0).reshape(n_plot, n_plot)

        # Plot Surface (True) and Wireframe (Pred)
        ax_3d.plot_surface(X_plot, Y_plot, Z_true, cmap=plt.cm.viridis, alpha=0.6, rstride=2, cstride=2)
        ax_3d.plot_wireframe(X_plot, Y_plot, Z_pred, color='red', alpha=0.8, linewidth=0.8)

        ax_3d.set_xlabel('X')
        ax_3d.set_ylabel('Y')
        ax_3d.set_zlabel('f(X,Y)')
        
        custom_lines = [Line2D([0], [0], color=plt.cm.viridis(0.5), lw=4),
                        Line2D([0], [0], color='red', lw=1.5)]
        ax_3d.legend(custom_lines, ['True Function', 'Predicted Surface'], loc='upper right', frameon=False)

    plt.tight_layout(rect=[0, 0, 1, 0.98])
    plt.show()