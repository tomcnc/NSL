#include <iostream>
#include <fstream>
#include <cmath>

#include "../../generator/random.h"
#include "../../functions.h"

using namespace std;

int main(){

    double pi_2{M_PI/2.0};

    // function to be integrated
    auto func = [&] (double x){
        return pi_2 * cos(pi_2 * x) ;
    } ;

    // normalized distribution to be sampled
    auto d = [] (double x){
        return 2.0 * (1.0 - x) ;
    } ;

    // function to compute mean
    auto G = [&] (double x){
        return func(x)/d(x) ;
    } ;

    // cumulative inverse to sample p keeping a variable from outside (y is rnd in [0,1) and is a defined outside)
    auto inv = [] (double y){
        return 1.0 - sqrt(1.0 - y);
    } ;


    Random rnd ;
    rnd.StartRandom() ;
    
    ofstream out_unif("OUTPUT/mean_error_uniform_sampling.data") ;
    if(!out_unif.is_open()){
        cerr << "ERROR : Not possible to open \"mean_error_uniform_sampling.data\" file" << endl ;
    }
    out_unif << "# BLOCK:       CURRENT_MEAN:   PROGRESSIVE_MEAN:              ERROR:" << endl ;

    // samples number
    int M{1000000} ;
    // blocks number
    int N{100} ;
    // samples in each block
    int L{M/N} ;
    // variable to accumulate progressive mean
    double acc{} ; 
    double mean_block{} ;
    double mean_acc{} ;
    double mean2_acc{} ;

    // ex 2.1.1 sampling an unform distribution in [0;1)

    for(int i{}; i < N; i++){
        // for loop to accumulate f(xi)
        for(int j{}; j < L; j++){
            acc += func(rnd.Rannyu()) ;
        }
        mean_block = acc/double(L) ;
        // accumulating mean over blocks
        mean_acc += mean_block ;
        mean2_acc += mean_block * mean_block ;
        out_unif << setw(8) << i
                 << setw(20) << mean_block
                 << setw(20) << mean_acc/double(i+1)
                 << setw(20) << error(mean_acc, mean2_acc, i)
                 << endl ;
        // reset accumulating variable
        acc = 0.0 ;
    }
    // reset mean accumulating variables
    mean_acc = 0.0 ;
    mean2_acc = 0.0 ;

    out_unif.close() ;

    // ex 2.1.2 importance sampling
    ofstream out_import("OUTPUT/mean_error_importance_sampling.data") ;
    if(!out_import.is_open()){
        cerr << "ERROR : Not possible to open \"mean_error_importance_sampling.data\" file" << endl ;
    }
    out_import << "# BLOCK:       CURRENT_MEAN:   PROGRESSIVE_MEAN:              ERROR:" << endl ;

    for(int i{}; i < N; i++){
        // for loop to accumulate f(xi)
        for(int j{}; j < L; j++){
            acc += G(inv(rnd.Rannyu())) ;
        }
        mean_block = acc/double(L) ;
        // accumulating mean over blocks
        mean_acc += mean_block ;
        mean2_acc += mean_block * mean_block ;
        out_import << setw(8) << i
                   << setw(20) << mean_block
                   << setw(20) << mean_acc/double(i+1)
                   << setw(20) << error(mean_acc, mean2_acc, i)
                   << endl ;
        // reset accumulating variable
        acc = 0.0 ;
    }
    
    out_import.close() ;

    mean_acc = 0.0 ;
    mean2_acc = 0.0 ;

    // for fun
    // ex 2.1.2 importance sampling with exponential PDF

    auto d_1 = [&] (double x){
        return exp(x)/(M_E - 1);
    } ;

    // function to compute mean
    auto G_1 = [&] (double x){
        return func(x)/d_1(x) ;
    } ;

    auto inv_1 = [] (double y){
        return log((M_E -1)*y +1);
    } ;


    ofstream out_import2("OUTPUT/mean_error_exponential_sampling.data") ;
    if(!out_import2.is_open()){
        cerr << "ERROR : Not possible to open \"mean_error_exponential_sampling.data\" file" << endl ;
    }
    out_import2 << "# BLOCK:       CURRENT_MEAN:   PROGRESSIVE_MEAN:              ERROR:" << endl ;

    for(int i{}; i < N; i++){
        // for loop to accumulate f(xi)
        for(int j{}; j < L; j++){
            acc += G_1(inv_1(rnd.Rannyu())) ;
        }
        mean_block = acc/double(L) ;
        // accumulating mean over blocks
        mean_acc += mean_block ;
        mean2_acc += mean_block * mean_block ;
        out_import2 << setw(8) << i
                   << setw(20) << mean_block
                   << setw(20) << mean_acc/double(i+1)
                   << setw(20) << error(mean_acc, mean2_acc, i)
                   << endl ;
        // reset accumulating variable
        acc = 0.0 ;
    }
    
    out_import2.close() ;

    return 0 ;
}