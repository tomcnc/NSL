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

#include "chromosome.h"

class Population{

    private:

    Random _rnd;                        // Random generator
    int _primes_row;                    // Primes row read
    string _city_distribution;          // Genes type distributions (circle or square)
    int _ncity;                         // Total number of cities visited by the travellers
    int _npath;                         // Total number of path analized
    int _nelite;                        // Total number of path passed to the next generation without mutation
    int _ngenerations;                  // Total number of generations produced
    field <Chromosome> _population;     // Field of path (is a vector of path)
    field <Chromosome> _new_population; // New generation of path produced after crossover
    field <Chromosome> _elite;          // Elite field of path
    mat _distance;                      // Distances matrix
    mat _cities;                        // Cities matrix (_ncity x 3) each row is a city (index, x, y)
    double _p_swap;              	// Probability of swap mutation
    double _p_crossover;         	// Probability of crossover
    double _p_shift;             	// Probability of shift mutation
    double _p_inversion;         	// Probability of inversion mutation
    bool _measure_best_loss;            // Flag for printing population lowest loss function
    bool _measure_best_path;            // Flag for printing best path with lowest loss function
    bool _measure_mean_loss;            // Flag for measuring and printing half population mean loss function

    public:

    int get_npath(){return _npath;};                                    // Returns population paths' number
    int get_ngenerations(){return _ngenerations;};                      // Returns total total number of generations produced
    void initialize();                                                  // Initializes population data members
    void initialize_properties();                                       // Reads properties.dat file, sets to true measure flags and sers output header 
    void read_city_configuration();                                     // Initializes _cities matrix reading input cities distribution file
    // double get_distance(double x1, double y1, double x2, double y2);    // Computes distances between cities
    void compute_distances();                                           // Produces a matrix of distances between cities
    void compute_loss();                                                // Computes loss function (L^2) for each population path
    void sort();                                                        // Population sorting as incrementing loss function
    void crossover();                                                   // Performs the crossover routine between two path choosen using selection operator
    void crossover1(Chromosome& c1, Chromosome& c2);                    // Ordered Crossover (OX1) method
    ivec create_new_tail(ivec right, Chromosome c);                     // Creates a new path tail following cities order of other parents during crossover
    void mutation();                                                    // Performs mutation on path (single or block cities swap, shift and inversion)
    void measure(int gen);                                              // Compute best loss and mean best loss over generations
    void finalize();                                                    // Saves last seed used into seed.out
    void print_last_best_path();                                        // Prints final best path cities distribution    

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
