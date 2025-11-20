/****************************************************************
*****************************************************************
    _/    _/  _/_/_/  _/       Numerical Simulation Laboratory
   _/_/  _/ _/       _/       Physics Department
  _/  _/_/    _/    _/       Universita' degli Studi di Milano
 _/    _/       _/ _/       Prof. D.E. Galli
_/    _/  _/_/_/  _/_/_/_/ email: Davide.Galli@unimi.it
*****************************************************************
*****************************************************************/

#ifndef __System__
#define __System__

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <armadillo>
#include <stdlib.h> //exit
#include "particle.h"
#include "random.h"

using namespace std;
using namespace arma;

class System {

private:
  const int _ndim = 3;        // Dimensionality of the system
  bool _restart;              // Flag indicating if the simulation is restarted
  int _sim_type;              // Type of simulation (e.g., Lennard-Jones, Ising)
  int _dist_type;             // Type of starting velocity distribution (0=Maxwell-Boltzmann or 1=Dirac) added by me
  int _npart;                 // Number of particles
  int _nblocks;               // Number of blocks for block averaging
  int _nsteps;                // Number of simulation steps in each block
  int _nattempts;             // Number of attempted moves
  int _naccepted;             // Number of accepted moves
  double _temp, _beta;        // Temperature and inverse temperature
  double _rho, _volume;       // Density and volume of the system
  double _r_cut;              // Cutoff radius for pair interactions
  double _delta;              // Displacement step for particle moves
  double _J, _H;              // Parameters for the Ising Hamiltonian
  vec _side;                  // Box dimensions
  vec _halfside;              // Half of box dimensions
  Random _rnd;                // Random number generator
  field <Particle> _particle; // Field of particle objects representing the system
  vec _fx, _fy, _fz;          // Forces on particles along x, y, and z directions
  int _nparam;                // Added by me --> Number of parameters (VMC)
  vec _parameters;            // Added by me --> Parameter vectors (VMC)
  vec _parameters_current;    // Added by me --> Parameter vectors (VMC) old configuration
  double _delta_temp;         // Added by me --> SA temperature step
  vec _delta_parameters;      // Added by me --> Metropolis move size for GS parameters
  int _nsteps_SA;             // Added by me --> Number of Metropolis moves done on parameters for each temperature
  int _n_temp;                // Added by me --> Number of simulated temperature by SA
  int _eq_steps;              // Added by me --> Number of step for VMC equilibration
  double _final_h;            // Added by me --> Final Hamiltonian average value produced by VMC
  double _final_err;          // Added by me --> Final Hamiltonian uncertainty produced by VMC
  double _final_x;            // Added by me --> Final walker position sampled by VMC
  double _h_best;             // Added by me --> Hamiltonian value accepted after parameters move (becomes old values for the next VMC)
  double _err_best;           // Added by me --> Hamiltonian uncertainty accepted after parameters move (becomes old values for the next VMC)
  double _x_best;             // Added by me --> Final walker position accepted (becomes old values for the next VMC)
  int _naccepted_SA;          // Added by me --> Accepted parameters moves

  // Properties
  int _nprop;                                                               // Number of properties being measured
  bool _measure_penergy, _measure_kenergy, _measure_tenergy;                // Flags for measuring different energies
  bool _measure_penergy_step, _measure_kenergy_step, _measure_tenergy_step; // Added by me --> Flags for measuring different energies at each step
  bool _measure_temp, _measure_pressure, _measure_gofr;                     // Flags for measuring temperature, pressure, and radial dist. function
  bool _measure_temp_step, _measure_pressure_step;                          // Added by me --> Flags for measuring temperature and pressure at each step
  bool _measure_magnet, _measure_cv, _measure_chi;                          // Flags for measuring magnetization, heat capacity, and susceptibility
  bool _measure_magnet_step;                                                // Added by me --> Flag for measuring magnetization at each step
  bool _measure_pofv;                                                       // Added by me --> Flag for measuring the velocity modulus distribution
  bool _tails;                                                              // Added by me --> Flag for using potential and pressure tail corrections
  bool _measure_loc_energy, _measure_loc_energy_step;                       // Added by me --> Flag for measuring Hamiltonian and Hamiltonian at each step (VMC)
  bool _measure_walker;                                                     // Added by me --> Flag for recording walker position sampling GS wave function distribution
  bool _print_vmc = true;                                                   // Added by me --> Disable print VMC blocking method (default==true)
  bool _print_acceptance = true;                                            // Added by me --> Disable print VMC acceptance (default==true) 
  int _index_penergy, _index_kenergy, _index_tenergy;                       // Indices for accessing energy-related properties in vec _measurement
  int _index_temp, _index_pressure, _index_gofr;                            // Indices for accessing temperature, pressure, and radial dist. function
  int _index_magnet, _index_cv, _index_chi;                                 // Indices for accessing magnetization, heat capacity, and susceptibility
  int _index_H2;                                                            // Added by me --> Index for accessing hamiltonian squared to compute specific heat
  int _index_pofv;                                                          // Added by me --> Index for accessing velocity modulus distribution
  int _index_loc_energy;                                                    // Added by me --> Index for accessing Hamiltonian (VMC)
  int _n_bins;                                                              // Number of bins for radial distribution function
  int _n_bins_v;                                                            // Number of bins for velocity modulus distribution
  double _bin_size;                                                         // Size of bins for radial distribution function
  double _bin_size_v;                                                       // Size of bins for velocity modulus distribution
  double _pofv_normalization, _pofv_increment;                              // Added by me --> Normalization and increment constant for velocity modulus distribution
  vec _gofr_normalization, _gofr_increment;                                 // Added by me --> Normalization and increment vector for radial distribution function (vec since each bin has its normalization constant)
  double _vtail, _ptail;                                                    // Added by me --> Tail corrections for energy and pressure
  vec _block_av;                                                            // Block averages of properties
  vec _global_av;                                                           // Global averages of properties
  vec _global_av2;                                                          // Squared global averages of properties
  vec _average;                                                             // Average values of properties
  vec _measurement;                                                         // Measured values of properties

public: // Function declarations
  int get_nbl();                                  // Get the number of blocks
  int get_nsteps();                               // Get the number of steps in each block
  void print_velocity_distribution();             // Added by me --> Print the initial velocity distribution
  void initialize();                              // Initialize system properties
  void initialize_properties();                   // Initialize properties for measurement
  void finalize();                                // Finalize system and clean up
  void write_configuration();                     // Write final system configuration to XYZ file
  void write_XYZ(int nconf);                      // Write system configuration in XYZ format on the fly
  void read_configuration();                      // Read system configuration from file
  void initialize_velocities();                   // Initialize particle velocities
  void step();                                    // Perform a simulation step
  void block_reset(int blk);                      // Reset block averages
  void measure();                                 // Measure properties of the system
  void averages(int blk);                         // Compute averages of properties
  double error(double acc, double acc2, int blk); // Compute error
  void move(int part);                            // Move a particle
  bool metro(int part);                           // Perform Metropolis acceptance-rejection step
  double pbc(double position, int i);             // Apply periodic boundary conditions for coordinates
  int pbc(int i);                                 // Apply periodic boundary conditions for spins
  void Verlet();                                  // Perform Verlet integration step
  double Force(int i, int dim);                   // Calculate force on a particle along a dimension
  double Boltzmann(int i, bool xnew);             // Calculate Boltzmann factor for Metropolis acceptance
  double Psi2(int i, bool xnew);                  // Added by me --> Calculate GS wave functions ration for Metropolis acceptance
  void vmc();                                     // Added by me --> Performs equilibration and VMC run (blocking method)
  void SA_fixed_temp_step();                      // Added by me --> Performs for loop over parameters M(RT)^2 total moves at fixed temperature
  void SA();                                      // Added by me --> Performs Simulated Annealing algorithm
  void print_data_step(int step);                 // Added by me --> Prints all data for each M(RT)^2 step at fixed temperature
  void header_data();                             // Added by me --> Header SA output file routine
  void print_data();                              // Added by me --> Prints all data for each SA temperature simulated 

};

#endif // __System__

/****************************************************************
*****************************************************************
    _/    _/  _/_/_/  _/       Numerical Simulation Laboratory
   _/_/  _/ _/       _/       Physics Department
  _/  _/_/    _/    _/       Universita' degli Studi di Milano
 _/    _/       _/ _/       Prof. D.E. Galli
_/    _/  _/_/_/  _/_/_/_/ email: Davide.Galli@unimi.it
*****************************************************************
*****************************************************************/
