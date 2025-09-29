#include <algorithm>
#include <armadillo>
#include <cmath>
#include <cstdlib>
#include <fmt/format.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include "../../generator/random.h"
#include "../../functions.h"

using namespace std;
using namespace arma;

int main(){

    Random rnd;
    rnd.StartRandom();

    // Number of needle throws
    int thr{1000000};

    // Number of blocks
    int blk{100};

    // Throws in each block
    int tpb{thr/blk};

    // needle length
    double L{1.0};

    // distance between horizontal plane lines
    double d{1.5};
    double a{(2.0 * L) / d};

    // needle y coordinates
    double start{};
    double end{};

    // 
    double theta{};

    // hit counter
    int hit{0};

    // pi mean block value
    double pi{};

    // mean accumulators
    double pi_acc{};
    double pi2_acc{};

    ofstream out("OUTPUT/pi.data");
    out << "# BLOCK:            PI_CURR:             PI_PROG:              ERROR:                HIT:             THROWS:" << endl;
    ofstream out_angle("OUTPUT/angle.data");
    out_angle << "# THETA:" << endl;

    for(int i{}; i < blk; i++){
        for(int j{}; j < tpb; j++){
            start = rnd.Rannyu(0.0, d);
            theta = rnd.RanAngle();
            // out_angle << setw(8) << theta << endl;
            end = start + sin(theta);
            if(end <= 0.0 || end >= d || is_equal(start, 0.0)){
                hit++;
            }
        }
        // pi block mean value 
        pi = a * double(tpb)/hit;
        // accumulating pi mean value
        pi_acc += pi;
        pi2_acc += pi * pi;
        out << setw(8) << i
            << setprecision(10) << setw(20) << pi
            << setprecision(10) << setw(20) << pi_acc/(i+1)
            << setprecision(10) << setw(20) << error(pi_acc, pi2_acc, i)
            << setw(20) << hit
            << setw(20) << tpb 
            << endl;
        // reset value
        hit = 0;
    }
    return 0;
}
