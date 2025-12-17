# Exercise 11: Introduction to Machine Learning and Neural Networks (FFNNs)

This exercise focuses on the development, training, and testing of **Feed-Forward Neural Networks (FFNNs)** for regression problems using the Keras/TensorFlow library.

In the field of Deep Learning, there is currently no deterministic theoretical framework that provides *a priori* guidelines for designing the optimal network architecture for a specific problem. Consequently, this exercise adopts a **heuristic approach**: we systematically construct various models, tuning hyperparameters such as dataset size, noise levels, activation functions, optimizers, and network depth/width. By comparing their performance on synthetic datasets, we derive empirical insights into the behavior of neural networks.

The work is organized into three main sections, progressively increasing the complexity of the target function to be fitted.

## Prerequisites

This exercise is developed entirely in **Python** using **Jupyter Notebooks**.

To run the code, you must have a Python environment set up with the following core libraries:
* **Jupyter** (Lab or Notebook)
* **NumPy**
* **Matplotlib**
* **TensorFlow / Keras**

You can install the necessary deep learning library via `pip` or `conda`:

```bash
pip install tensorflow
```

```bash
conda install -c conda-forge tensorflow
```

---

## 1. The Exercises

The requirements for the exercise are outlined in the notebooks `LSN_Exercises_11.ipynb`. Each notebook (`LSN_Notebook_11_1.ipynb`, `LSN_Notebook_11_2.ipynb` and `LSN_Notebook_11_3.ipynb`) contains one exercise:

- **Exercise 11.1 in `LSN_Notebook_11_1.ipynb`: Linear Fit ()**
  The goal is to fit a simple linear function $f(x) = 2x + 1$ with $x \in [-1, 1]$. The exercise requires studying how the model performance depends on:
  - The size of the training dataset $N_{train}$.
  - The noise level $\sigma$ added to the data.
  - The number of training epochs $N_{epochs}$.

- **Exercise 11.2 in `LSN_Notebook_11_2.ipynb`: Polynomial Fit**
  The goal is to extend the neural network to fit a non-linear cubic polynomial:
  $$f(x) = 3x^3 - 2x^2 - 3x + 4, \quad x \in [-1, 1]$$
  This section requires exploring different network architectures (depth and width) and activation functions to capture the non-linearity. Furthermore, it asks to test the model's generalization capabilities by making predictions **outside** the training range (extrapolation).

- **Exercise 11.3 in `LSN_Notebook_11_3.ipynb`: 2D Trigonometric Fit**
  The goal is to fit a multi-variable trigonometric function:
  $$f(x, y) = \sin(x^2 + y^2), \quad x, y \in [-1.5, 1.5]$$
  This requires adapting the input layers to accept 2D data and finding an architecture capable of modelling the complex 3D surface.

---

## 3. Implementation and Analysis

The implementation and analysis are contained in `LSN_Notebook_11_*.ipynb`. The notebooks utilize a custom Python module, `ml_functions.py` (located in the parent directory), which wraps Keras functionalities to streamline dataset generation, model creation, training, and visualization.

The analysis follows a systematic "single-variable sensitivity" approach: varying one hyperparameter at a time while fixing the others to isolate its effect on the **Validation MSE** and the **Diagnostic MSE** (calculated against the true noise-free function).

#### 11.1 Linear Regression Analysis
- **Dataset Size:** We observed that performance saturates around $N=1000$. Smaller datasets ($N<100$) lead to high variance and unstable fits.
- **Noise:** The error scales quadratically with the noise level ($MSE \propto \sigma^2$).
- **Optimizer:** **SGD** (Stochastic Gradient Descent) proved to be the fastest and most stable optimizer for this simple convex problem, converging within $\sim 100$ epochs.
- **Architecture:** The optimal model was found to be a **single-layer network with linear activation**. Adding hidden layers or non-linear activations did not improve performance and only increased complexity.

#### 11.2 Polynomial Regression Analysis
- **Activation Functions:** The linear model failed to capture the cubic curve. Non-linear activations were tested; **Softsign** and **Tanh** provided the smoothest approximations, while ReLU introduced discontinuities.
- **Architecture:** A "Deep" architecture is required. An "hourglass" shaped network (wide input/output layers, narrow bottleneck) proved effective.
- **Generalization:** The model showed excellent interpolation within $x \in [-1, 1]$ but **failed completely at extrapolation** for $x \in [-2, 2]$, highlighting a fundamental limitation of standard neural networks.

#### 11.3 2D Surface Fitting
- **Activation Functions:** **GeLU** (Gaussian Error Linear Unit) and ReLU yielded the fastest convergence and best surface reconstruction compared to Sigmoid or Tanh.
- **Architecture:** A progressively widening architecture `[10, 20, 30, 40]` was selected.
- **Result:** The final model successfully reconstructed the concentric waves of the target function $\sin(x^2+y^2)$.

---

## 4. Directory Structure

The project files are organized as follows. Note that the helper script ml_functions.py is expected to be in the parent directory relative to the notebook folder.

| Directory/File | Description |
| :--- | :--- |
| `LSN_Exercises_11.ipynb `| **Jupyter Notebook** outlining the exercise requests. |
| `LSN_Notebook_11_1.ipynb` | **Jupyter Notebook** containing the implementation and analysis of exercise 11.1. |
| `LSN_Notebook_11_2.ipynb` | **Jupyter Notebook** containing the implementation and analysis of exercise 11.2. |
| `LSN_Notebook_11_3.ipynb` | **Jupyter Notebook** containing the implementation and analysis of exercise 11.3. |
| `images/` | Directory for supporting images and plots. |


```
.
├── ml_functions.py               # Custom library for Dataset generation and Plotting
└── LSN_11/
    ├── LSN_Exercises_11.ipynb
    ├── LSN_Notebook_11_1.ipynb   # Jupyter Notebook for data analysis.
    ├── LSN_Notebook_11_2.ipynb   # Jupyter Notebook for data analysis.
    ├── LSN_Notebook_11_3.ipynb   # Jupyter Notebook for data analysis.
    ├── ReadMe.md
    └── images/
```