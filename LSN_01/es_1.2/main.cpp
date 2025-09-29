#include <fstream>
#include <iostream>


#include "../../generator/random.h"
#include "../../functions.h"


int main(){

    Random rnd ;
    rnd.StartRandom() ;

    int throws{10000} ;
    std::vector<int> nsum{1, 2, 10, 100} ;
    int size{static_cast<int>(nsum.size())} ;

    double lambda{1.} ;
    double gamma{1.} ;
    double centervalue{0.} ;

    std::ofstream out_unif{"OUTPUT/unif.data"} ;
    out_unif << "#             N = 1:              N = 2:             N = 10:            N = 100:" << endl;
    std::ofstream out_exp{"OUTPUT/exp.data"} ;
    out_exp << "#             N = 1:              N = 2:             N = 10:            N = 100:" << endl;
    std::ofstream out_lor{"OUTPUT/lor.data"} ;
    out_lor << "#             N = 1:              N = 2:             N = 10:            N = 100:" << endl;

    for(int i{}; i < throws; i++){
        for(int j{}; j < size; j++){
            double acc_unif{} ;
            double acc_exp{} ;
            double acc_lor{} ;
            for(int k{}; k < nsum.at(j); k++){
                acc_unif += rnd.Rannyu(0., 1.)/double(nsum.at(j))  ;
                acc_exp += rnd.Exponential(lambda)/double(nsum.at(j)) ;
                acc_lor += rnd.Lorentzian(gamma, centervalue)/double(nsum.at(j)) ;
            }
            out_unif << setw(20) << acc_unif;
            out_exp << setw(20) << acc_exp;
            out_lor << setw(20) << acc_lor;
        }
        out_unif << endl;
        out_exp << endl;
        out_lor << endl;
    }

    return 0 ;

}
