/****************************************************************
*****************************************************************
    _/    _/  _/_/_/  _/       Numerical Simulation Laboratory
   _/_/  _/ _/       _/       Physics Department
  _/  _/_/    _/    _/       Universita' degli Studi di Milano
 _/    _/       _/ _/       Prof. D.E. Galli
_/    _/  _/_/_/  _/_/_/_/ email: Davide.Galli@unimi.it
*****************************************************************
*****************************************************************/

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "random.h"

using namespace std;

int main(){

    string property;
    int n_cities{};

    ifstream input("../INPUT/input.dat");
    if(!input){
        cerr << "ERROR: Not possible to open \"../INPUT/input.dat\" ." << endl;
    }else{
        input >> property;
        while(property != "N_CITY"){
            input >> n_cities >> property; 
        }
        input >> n_cities;
    }

    Random rnd;
    rnd.StartRandom();
    double x,y,theta;

    ofstream outc("../INPUT/circle_map.dat");
    if(!outc){
        cerr << "ERROR: Not possible to open \"../INPUT/circle_map.dat\" ." << endl;
    }else{
        outc << "#        CITY_INDEX:                  X:                  Y:" << endl;
        // cout << "#        CITY_INDEX:                  X:                  Y:" << endl;
        for(int idx{}; idx < n_cities; idx++){
            theta = rnd.RanAngle();
            x = 5.0*cos(theta);
            y = 5.0*sin(theta);
            outc << setw(20) << idx+1
                << setw(20) << x
                << setw(20) << y << endl;
            // cout << setw(20) << idx+1
            //     << setw(20) << x
            //     << setw(20) << y << endl;
        }
    }
    outc.close();

    ofstream outs("../INPUT/squared_map.dat");
    if(!outs){
        cerr << "ERROR: Not possible to open \"../INPUT/squared_map.dat\" ." << endl;
    }else{
        outs << "#        CITY_INDEX:                  X:                  Y:" << endl;
        // cout << "#        CITY_INDEX:                  X:                  Y:" << endl;
        for(int idx{}; idx < n_cities; idx++){
            x = rnd.Rannyu(-5.0, 5.0);
            y = rnd.Rannyu(-5.0, 5.0);
            outs << setw(20) << idx+1
                << setw(20) << x
                << setw(20) << y << endl;
            // cout << setw(20) << idx+1
            //     << setw(20) << x
            //     << setw(20) << y << endl;
        }
    }
    outs.close();

    return 0;
}

/****************************************************************
*****************************************************************
    _/    _/  _/_/_/  _/       Numerical Simulation Laboratory
   _/_/  _/ _/       _/       Physics Department
  _/  _/_/    _/    _/       Universita' degli Studi di Milano
 _/    _/       _/ _/       Prof. D.E. Galli
_/    _/  _/_/_/  _/_/_/_/ email: Davide.Galli@unimi.it
*****************************************************************
*****************************************************************/