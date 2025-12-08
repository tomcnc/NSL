/****************************************************************
*****************************************************************
    _/    _/  _/_/_/  _/       Numerical Simulation Laboratory
   _/_/  _/ _/       _/       Physics Department
  _/  _/_/    _/    _/       Universita' degli Studi di Milano
 _/    _/       _/ _/       Prof. D.E. Galli
_/    _/  _/_/_/  _/_/_/_/ email: Davide.Galli@unimi.it
*****************************************************************
*****************************************************************/

#pragma once

#include <mpi.h>
#include "chromosome.h"

class Population{

    private:

    Random _rnd;                        // Random generator
    int _primes_row;                    // Row index used to read prime numbers for RNG initialization

    string _city_distribution;          // Type of city distribution (e.g., "circle" or "square" or "italy")
    int _ncity;                         // Total number of cities (length of a single path/chromosome)
    int _npath;                         // Total size of the population (number of chromosomes analyzed)
    int _nelite;                        // Number of elite paths passed directly to the next generation
    int _ngenerations;                  // Total number of generations to be produced/simulated

    int _nexchange;                     // Number of paths exchanged between processes during migration
    int _buffer_size;                   // Size of the dynamic exchange buffer (equal to _ncity * _nexchange)
    int* _exchange_buffer;              // Persistent buffer for MPI communication (migrating chromosomes)
    int _nmessage;			// Total number of messages exchanged bertween propcesses

    field <Chromosome> _population;     // Current field/vector of chromosomes (paths)
    field <Chromosome> _new_population; // Field to store the chromosomes of the next generation
    field <Chromosome> _elite;          // Field storing the elite subset of chromosomes

    mat _distance;                      // Distance matrix (stores the pre-computed distance between every city pair)
    mat _cities;                        // City coordinates matrix (_ncity rows x 3 columns: index, x, y)

    double _p_swap;                     // Probability of the swap mutation operator
    double _p_crossover;                // Probability of the crossover operation
    double _p_shift;                    // Probability of the shift mutation operator
    double _p_inversion;                // Probability of the inversion mutation operator

    bool _measure_best_loss;            // Flag: enable logging of the population's best fitness (lowest loss)
    bool _measure_best_path;            // Flag: enable logging/printing of the best path's cities distribution
    bool _measure_mean_loss;            // Flag: enable logging of the mean fitness of the best half of the population

    public:

    int get_npath() { return _npath; };                                     // Returns the total number of chromosomes in the population
    int get_ngenerations() { return _ngenerations; };                       // Returns the total number of generations simulated
    int get_nmessage() { return _nmessage; } ;				    // Returns the total number of message passing between processes

    void initialize(int rank);                                              // Initializes essential population data members and RNG using process rank
    void initialize_properties(int rank);                                   // Reads properties.dat file, sets simulation flags, and prepares output headers
    void read_city_configuration();                                         // Initializes _cities matrix by reading the input city coordinates file

    // double get_distance(double x1, double y1, double x2, double y2);            // Computes distances between cities
    void compute_distances();                                               // Computes the symmetric distance matrix between all city pairs
    void compute_loss();                                                    // Computes the fitness (L^2 loss) function for every chromosome in the population

    void sort();                                                            // Sorts the population based on increasing loss function (best path first)

    void crossover();                                                       // Performs the crossover routine based on the selection operator
    void crossover1(Chromosome& c1, Chromosome& c2);                        // Implementation of the Ordered Crossover (OX1) method
    ivec create_new_tail(ivec right, Chromosome c);                         // Creates the segment of the child chromosome based on the second parent's order
    void mutation();                                                        // Applies mutation operators (swap, shift, inversion) to the chromosomes

    void measure(int gen, int rank);                                        // Calculates and records the best and mean fitness metrics for the current generation

    void finalize(int rank);                                                // Saves the last used RNG seed to seed.out file

    void print_last_best_path(int rank);                                    // Prints the final best path's city sequence and coordinates
    void print_start_population(int rank);                                  // Prints the initial random population's paths

    void process_path_exchange(int size, int rank);                         // Executes the chromosome migration routine between MPI processes (Ring Topology)

};

/****************************************************************
*****************************************************************
    _/    _/  _/_/_/  _/       Numerical Simulation Laboratory
   _/_/  _/ _/       _/       Physics Department
  _/  _/_/    _/    _/       Universita' degli Studi di Milano
 _/    _/       _/ _/       Prof. D.E. Galli
_/    _/  _/_/_/  _/_/_/_/ email: Davide.Galli@unimi.it
*****************************************************************
*****************************************************************/
