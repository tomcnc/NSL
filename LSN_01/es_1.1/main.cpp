#include <armadillo>
#include <cmath>
#include <fstream>
#include <fmt/format.h>
#include <iostream>

#include "../../generator/random.h"
#include "../../functions.h"

using namespace arma;
using namespace std;

int main(){

    Random rnd;
    rnd.StartRandom();

    // number of throws
    int M[2] = {100000, 10000000};
    // number of blocks
    int N{100};

    // ex 1.1.1
    // simulation with M = 100000
    double shift = 0.0;
    int dist_momentum_order = 1;
    string ofilename1 = "OUTPUT/mean_error1.1.data";
    Simulation(rnd, M[0], N, shift, dist_momentum_order, ofilename1);

    // simulation with M = 10000000
    string ofilename2 = "OUTPUT/mean_error1.2.data";
    Simulation(rnd, M[1], N, shift, dist_momentum_order, ofilename2);

    // ex 1.1.2
    // simulation with M = 100000
    shift = -0.5;
    dist_momentum_order = 2;
    string ofilename3 = "OUTPUT/mean_error2.1.data";
    Simulation(rnd, M[0], N, shift, dist_momentum_order, ofilename3);

    // simulation with M = 10000000
    string ofilename4 = "OUTPUT/mean_error2.2.data";
    Simulation(rnd, M[1], N, shift, dist_momentum_order, ofilename4);

    // ex 1.1.3

    int throws{100000};
    // define also ntest with the idea to make ntest ≠ nbin
    int ntest{100};
    int nbin{100};
    string ofilename5{"OUTPUT/ChiSquared.data"};
    // chi squared test simulation
    ChiSquared_simulation(rnd, ntest, throws, nbin, ofilename5);
    
    return 0;

}
