#pragma once

#include <algorithm>
#include <armadillo>
#include <cfloat>
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

class RandomWalk{

    private:
    unsigned int _nwalks;           // Total RW simulated
    unsigned int _nblocks;          // Blocks number used to do blocking method
    unsigned int _nwalksperblock;   // RW simulated per block
    unsigned int _nstep;            // RW total steps
    double _step;                   // RW step
    int _ndim;                      // RW lattice dimension
    int _row;                       // Row read from Primes
    Random _rnd;                    // Random number generator
    vec _x;                         // Current position vector
    vec _xstart;                    // Start position vector
    vec _dist2acc;                  // Distance accumulator vector (accumulates square distance for each RW step made, so has size = _nstep)
    vec _meanblock;                 // Block mean distance vector
    vec _meanacc;                   // Mean accumulator vector (accumulates distance RMS progressive for each RW step, so has size = _nstep)
    vec _mean2acc;                  // Mean squared accumulator vector
    string _RWtype;                 // RW type (discrete or continuous) to choose which type of step has to be used to simulate RW
    int _RWprint;                   // Block's RW index whose is going to be fully plotted
    
    // properties
    bool _measure_RW;
    bool _measure_last;
    bool _measure_dist2;

    
    public:

    void initialize();                                               // Initialization RW class reading file input
    void initialize_properties();                                    // Initialization properties reading file properties and preparing outputfile
    unsigned int get_nwalks() {return _nwalks;};                     // Returns total RW simulated
    unsigned int get_nblocks() {return _nblocks;};                   // Returns total blocks
    unsigned int get_nwalksperblock() {return _nwalksperblock;};     // Returns RW per block
    unsigned int get_nstep() {return _nstep;};                       // Returns RW total steps
    double get_step() {return _step;};                               // Returns RW step
    int get_RWprint() {return _RWprint;};                            // Returns block's RW index whose is going to be fully plotted
    double get_distance2();                                          // Returns squared distance between start and _x
    double error(double mean, double mean2, int blk);                // Computes error for mean blocking method
    void step();                                                     // Performs a RW step
    void measure(int step);                                          // Computes distance measurement and accumulates in _distacc for each RW step
    void averages(int blk);                                          // Performs blocking method
    void reset_block(int blk);                                       // Reset block accumulators to zero
    void reset_position();                                           // Set system position in starting configuration _x = (0,0,0)
    void finalize();                                                 // Saves last random generated
    void write_last_position(int RW);                                // Write RW final position
    void write_RW(int step, int RW, int blk);                        // Prints current position evolution of RW RW on outputfile

};
