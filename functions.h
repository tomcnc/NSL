#pragma once

#include <algorithm>
#include <armadillo>
#include <cfloat>
#include <cmath>
//#include <fmt/format.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>

using namespace std;
using namespace arma;

template <typename T> T CalcolaMedia(const vector<T>& v){

    T accumulo{} ;
    int size = v.size() ;
    if(size == 0){
        return accumulo ;
    }else{ 
        for(int k{}; k < size; k++){
            accumulo = static_cast<double>(k) / static_cast<double>(k + 1)* accumulo + 1./static_cast<double>(k + 1) * v.at(k) ;
        }
    }
    return accumulo ;

}


template <typename T> T CalcolaVarianza(const vector<T>& v){

    T accumulo{} ;
    T mean{CalcolaMedia<T>(v)} ;
    int size = v.size() ;
    if(size == 0 || size == 1){
        return accumulo ;
    }else{
        for(int k{}; k < size; k++){
            accumulo += (v.at(k) - mean) * (v.at(k) - mean) ;
        }
    }
    return accumulo/static_cast<T>(size - 1) ;

}

template <typename T> T CalcolaDevStd(const vector<T>& v){

    T accumulo{} ;
    T mean{CalcolaMedia<T>(v)} ;
    int size = v.size() ;
    if(size == 0 || size == 1){
        return accumulo ;
    }else{
        for(int k{}; k < size; k++){
            accumulo += (v.at(k) - mean) * (v.at(k) - mean) ;
        }
    }
    return sqrt(accumulo/static_cast<T>(size - 1)) ;

}

template <typename T> T CalcolaMediana(vector<T>& v){

    sort(v.begin(), v.end()) ;
    int size{static_cast<int>(v.size())} ;
    if(size == 0){
        cerr << "Vector is empty. Returning 0 as central value." << endl ;
        return 0 ;
    }
    int hsize{size/2} ;
    if(size % 2 == 0){
        return (v[hsize - 1] + v[hsize]) / 2. ;
    }else{
        return v[hsize] ;
    }

}

template <typename T> T CalcolaCorrelazione(const vector<T>& X, const vector<double>& Y, const vector<double>& XY){

    return (CalcolaMedia(XY) - CalcolaMedia(X) * CalcolaMedia(Y)) / (CalcolaDevStd(X) * CalcolaDevStd(Y)) ;
}

/*********************************************************************************************************************************/
// for NSL 

double error(double a, double a2, int n){
    if(n == 0){
        return 0.0 ;
    } else {
        return sqrt(fabs(a2/double(n+1) - pow(a/double(n+1), 2))/n) ;
    }
}

// function that evaluates equality between floating point to avoid rounding error
bool is_equal(double obs, double exp){
    return fabs(obs - exp) <= 10.0 * sqrt((obs * DBL_EPSILON) * (obs * DBL_EPSILON) + (exp * DBL_EPSILON) * (exp * DBL_EPSILON));
}

// b gives pseudo-random number shift and n gives central momentum order  
void Simulation(Random rnd, int M, int N, double b, int n, string ofilename){
    // output stream creation and opening file check
    ofstream out(ofilename.c_str()) ;
    if(!out.is_open()){
        cerr << "ERROR : Not possible to open " << ofilename << endl;
    }
    out << "#             BLOCK:         BLOCK_MEAN:               MEAN:              ERROR:" << endl;
    // throws in each block
    int L{M/N} ;
    // variable to accumulate blocks mean and blocks mean squared
    double mean_acc{} ;
    double mean2_acc{} ;
    // accumulate variable of random number created
    double acc{} ;   
    // for cicle on N blocks
    for(int i{}; i < N; i++){
        // for loop over throws in a single block
        for(int j{}; j < L; j++){
            acc += pow(rnd.Rannyu() + b, n) ;
        }
        // accumulating mean_block 
        mean_acc += acc/L;
        mean2_acc += (acc/L)*(acc/L);
        // output current block, current block mean, progressive mean over blocks and progressive std dev over blocks in ofile
        out << setw(20) << i 
            << setw(20) << acc/L
            << setw(20) << mean_acc/(i+1)
            << setw(20) << error(mean_acc, mean2_acc, i)
            << endl;
        // reset variable
        acc = 0.0;
    }
    out.close() ;
    return ;
}

double ChiSquared(int obs, int exp){
    return ((obs - exp) * (obs - exp)) / double(exp) ;
}

void ChiSquared_simulation(Random rnd, int ntest, int throws, int nbin, string ofilename){
    ofstream out(ofilename.c_str());
    if(!out.is_open()){
        cerr << "ERROR : Not possible to open " << ofilename << endl;
    }
    out << "# CHI_TEST_NUMBER:  CHI_SQUARED_VALUE:" << endl;
    
    // vec of counters for bins set to 0
    Col<int> counts(nbin);
    Col<int> total_counts(nbin);
    counts.zeros();
    total_counts.zeros();
    // variable to accumulate chisquared of each bin
    double ChiS{};
    // expected counts per bin
    int exp{int(throws/nbin)};
    // bin size
    double bin_size{1.0/nbin};
    // random number generated
    double rand;

    for(int i{}; i < ntest; i++){
        for(int j{}; j < throws; j++){
            rand = rnd.Rannyu();
            //using the pseudo-random number to check which intervals counter has to be incremented
            counts.at(int(rand * nbin))++;
            total_counts.at(int(rand * nbin))++;
        }
        // print of 4 different histograms of chisquared test (facultative)
        if(i % 30 == 0){
            ofstream out_counts("OUTPUT/ChiSquared_histo_after_" + to_string(i) + "_tests.data");
            if(!out_counts.is_open()){
                cerr << "ERROR : Not possible to open OUTPUT/ChiSquared_histo_after_" + to_string(i) + "_tests.txt" << endl;
            }
            out_counts << "#     BIN_CENTER:         BIN_COUNTS:" << endl;
            for(int k{}; k < nbin; k++){
                out_counts << setw(16) <<  (k + 0.5)*bin_size
                           << setw(20) << total_counts.at(k)
                           << endl;
            }
            out_counts.close();
        }
        // for loop over bins to compute chisquared
        for(int k{}; k < nbin; k++){
            ChiS += ChiSquared(counts.at(k), exp); 
        }
        out << setw(18) << i
            << setw(20) << ChiS
            << endl;
        // reset variables
        counts.zeros();
        ChiS = 0.0;
    }
    out.close();
    return;
}

// less efficient way using more variables 
// void Simulation(Random rnd, int M, int N, double b, int n, string ofilename){
//     ofstream out(ofilename.c_str()) ;
//     if(!out.is_open()){
//         cerr << "ERROR : Not possible to open " << ofilename << endl ;
//     }
//     out << "#             BLOCK:         BLOCK_MEAN:               MEAN:              ERROR:" << endl;
//     // throws in each block
//     int L{M/N} ;

//     // variable to accumulate blocks mean and blocks mean squared
//     double mean_acc{} ;
//     double mean2_acc{} ;

//     // accumulate variable of random number created
//     double acc{} ;

//     // mean value computed on one block
//     double mean_block{} ;
//     double mean2_block{} ;
//     double mean{} ;
//     double mean2{} ;
//     double err{} ;
    
//     // for cicle on N blocks
//     for(int i{}; i < N; i++){
//         // accumulating L pseudo-random values in acc
//         for(int j{}; j < L; j++){
//             acc += pow(rnd.Rannyu() + b, n) ;
//         }

//         // computing block mean and squared mean
//         mean_block = acc/L ;
//         mean2_block = mean_block * mean_block ;

//         // accumulating block mean and block mean squared
//         mean_acc += mean_block ;
//         mean2_acc += mean2_block ;

        
//         // computing mean and squared mean of i blocks
//         mean = mean_acc/(i+1) ;
//         mean2 = mean2_acc/(i+1) ;
//         err = error(mean_acc, mean2_acc, i) ;
        
//         // output current mean and std dev in ofile
//         out << setw(20) << i 
//             << setw(20) << mean_block
//             << setw(20) << mean 
//             << setw(20) << err 
//             << endl;

//         // reset variable
//         acc = 0.0;
//         mean_block = 0.0;
//         mean2_block = 0.0;
//         mean = 0.0;
//         mean2 = 0.0;
//         err = 0.0;
//     }
//     out.close() ;
//     return ;
// }

// other method less efficient using if to check which bin has to be incremented

// void ChiSquared_simulation(Random rnd, int ntest, int throws, int nbin, string ofilename){
//     ofstream out(ofilename.c_str());
//     if(!out.is_open()){
//         cerr << "ERROR : Not possible to open " << ofilename << endl;
//     }
//     out << "# CHI_TEST_NUMBER:  CHI_SQUARED_VALUE:" << endl;
    
//     // vec of counters for bins set to 0
//     Col<int> counts(nbin);
//     counts.zeros();
//     // variable to accumulate chisquared of each bin
//     double ChiS{};
//     // expected counts per bin
//     int exp{int(throws/nbin)};
//     // bin size
//     double bin_size{1.0/nbin};

//     for(int i{}; i < ntest; i++){
//         for(int j{}; j < throws; j++){
//             double val{rnd.Rannyu()} ;
//             for(int t{}; t < nbin; t++){
//                 if(val >= double(t)/nbin && val < double(t+1)/nbin){
//                     counts[t]++ ;
//                 }
//             }
//         }
//         // print of 4 different histograms of chisquared test (facultative)
//         if(i % 30 == 0){
//             ofstream out_counts("OUTPUT/ChiSquared_test_" + to_string(i) + ".data");
//             if(!out_counts.is_open()){
//                 cerr << "ERROR : Not possible to open OUTPUT/ChiSquared_test_" + to_string(i) + ".txt" << endl;
//             }
//             out_counts << "#     BIN_CENTER:         BIN_COUNTS:" << endl;
//             for(int k{}; k < nbin; k++){
//                 out_counts << setw(16) <<  (k + 0.5)*bin_size
//                            << setw(20) << counts.at(k)
//                            << endl;
//             }
//             out_counts.close();
//         }
//         // for loop over bins to compute chisquared
//         for(int k{}; k < nbin; k++){
//             ChiS += ChiSquared(counts.at(k), exp); 
//         }
//         out << setw(18) << i
//             << setw(20) << ChiS
//             << endl;
//         // reset variables
//         counts.zeros();
//         ChiS = 0.0;
//     }
//     out.close();
//     return;
// }