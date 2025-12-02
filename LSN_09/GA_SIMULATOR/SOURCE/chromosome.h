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

class Chromosome{

    private:

    int _ncity;             // Number of cities inside a path
    ivec _path;             // Vector of integer containing city index
    bool _path_goodness;    // Flag representing path goodness properties (first city index is 1 and no cities repeated)
    double _loss;           // Path loss function


    public:

    Chromosome& operator=(const Chromosome& c);         // Overload = operator to create a new path from one already existing
    bool operator<(const Chromosome& c) const;          // Overload < operator to order _path as growing loss function
    void set_path_size(int size);                       // Set number of cities inside a path
    int get_ncity(){return _ncity;};                    // Returns number of cities inside a path
    int get_city(int idx){return _path(idx);};          // Returns city label at position idx in _path
    void initialize_path(Random& rnd);                  // Path initialization
    void check_path_properties();                       // Check path properties (first element = 1 and no genes repetitions)
    bool get_path_properties(){return _path_goodness;}; // Returns true if _path satisfies TSP conditions 
    void compute_loss(mat D);                           // Set path loss to value computed
    double get_loss() const {return _loss;};            // Returns path loss function
    void swap_mutation(int m, Random& rnd);             // Swapping 2 not overlapping blocks of L = m+1 contiguous cities
    void shift_mutation(int m, Random& rnd);            // Block random shift L = m+1 contiguous cities
    void inversion_mutation(Random& rnd);               // Chooses two cities and inverts their order in path
    ivec get_left_segment(int cut);                     // Returns _path left part, above crossover cut
    ivec get_right_segment(int cut);                    // Returns _path right part, below crossover cut
    void merge(ivec left, ivec right);                  // Merges left and right segment to create a new _path
    void print_path();                                  // Prints cities sequence

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
