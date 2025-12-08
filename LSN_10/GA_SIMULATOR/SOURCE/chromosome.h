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

#include <algorithm>
#include <armadillo>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "random.h"

using namespace std;
using namespace arma;

/**
Represents a single solution (path/chromosome) for the Traveling Salesperson Problem (TSP).
In the context of Genetic Algorithms, this is a chromosome containing an ordered sequence of city indices (genes).
**/

class Chromosome{

    private:

    int _ncity;                 // The number of cities (genes) contained within this path.
    ivec _path;                 // Vector storing the sequence of city indices (the chromosome/path itself).
    bool _path_goodness;        // Flag indicating if the path satisfies TSP constraints (starts at city 1, no repeated cities).
    double _loss;               // The fitness value (L^2 loss function) associated with this path.


    public:

    Chromosome& operator=(const Chromosome& c);             // Overloads the assignment operator (=) for deep copying of paths.
    bool operator<(const Chromosome& c) const;              // Overloads the less-than operator (<) to enable sorting by increasing loss (best fitness).
    
    void set_path_size(int size);                           // Sets the number of cities (_ncity) for the path structure.
    int get_ncity() { return _ncity; };                     // Returns the total number of cities in the path.
    int get_city(int idx) { return _path(idx); };           // Returns the index/label of the city at a specific position (gene index) in the path.
    void set_city(int idx, int city);                       // Sets the city index/label at a specific position (gene index) in the path.
    
    void initialize_path(Random& rnd);                      // Initializes the path with a random, valid permutation of cities.
    void check_path_properties();                           // Verifies if the path adheres to all TSP constraints (e.g., city 1 fixed, no duplicates).
    bool get_path_properties() { return _path_goodness; };  // Returns true if the path is a valid TSP tour.
    
    void compute_loss(mat D);                               // Computes the path length (L^2 loss) based on the distance matrix D.
    double get_loss() const { return _loss; };              // Returns the path's fitness value (the computed L^2 loss).
    
    void swap_mutation(int m, Random& rnd);                 // Swaps two non-overlapping blocks of contiguous cities of length L = m+1.
    void shift_mutation(int m, Random& rnd);                // Shifts a contiguous block of L = m+1 cities to a random position.
    void inversion_mutation(Random& rnd);                   // Inverts the order of cities within a randomly chosen segment of the path.
    
    ivec get_left_segment(int cut);                         // Returns the path segment from the start up to the crossover cut point (Parent 1 segment).
    ivec get_right_segment(int cut);                        // Returns the path segment from the crossover cut point to the end.
    void merge(ivec left, ivec right);                      // Merges the head (left segment) and tail (right segment) to form the new path.
    
    void print_path();                                      // Prints the sequence of city indices that constitute the path.

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
