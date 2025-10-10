#include <cmath>
#include <fstream>
#include <iostream>
#include <iomanip>

#include "../../generator/random.h"
#include "../../functions.h"

using namespace std;

int main(){

    // generator setting and start
    Random rnd ;
    rnd.StartRandom();
    // random generator to print asset price time evolution
    Random rnd_print ;
    rnd_print.StartRandom(1);
    
    // lambda function used
    auto asset_evolution = [&] (double S0, double drift, double volatility, double t, double z){
        return S0 * exp((drift - 0.5 * volatility * volatility) * t + volatility * z * sqrt(t));
    };

    auto call_prize = [] (double S, double K){
        return (S - K > 0 ? S - K : 0);
    };

    auto put_prize = [] (double S, double K){
        return (K - S > 0 ? K - S : 0);
    };

    // throws number
    int M{1000000};
    // blocks numbers
    int N{100};
    // throws in each block
    int L{M/N};
    // start asset price
    double S0{100};
    // expiry time
    double T{1};
    // strike price
    double K{100};
    // risk-free interest rate
    double drift{0.1};
    // price volatility
    double volatility{0.25};
    // discount factor
    double discount_factor{exp(-drift * T)};

    // accumulating variables to perform data-blocking
    double acc_call{};
    double call_block_mean{};
    double call_block_mean2{};
    double call_mean_acc{};
    double call_mean2_acc{};

    double acc_put{};
    double put_block_mean{};
    double put_block_mean2{};
    double put_mean_acc{};
    double put_mean2_acc{};
    double S{};
    
    ofstream call_out("OUTPUT/call.data");
    ofstream put_out("OUTPUT/put.data");

    // ex 3.1.1

    call_out << "# BLOCK:          CURR_MEAN:          PROG_MEAN:             ERROR:" << endl;
    put_out << "# BLOCK:          CURR_MEAN:          PROG_MEAN:             ERROR:" << endl;

    for(int i{}; i < N; i++){

        acc_call = 0;
        acc_put = 0;

        for(int j{}; j < L; j++){
            S = asset_evolution(S0, drift, volatility, T, rnd.Gauss(0.0, 1.0));
            acc_call += call_prize(S, K);
            acc_put += put_prize(S, K);
            // exponential de-evolution to discont asset price at t = 0
            // acc_call += discount_factor * call_prize(S, K);
            // acc_put += discount_factor * put_prize(S, K);
        }

        // call_block_mean = acc_call / L;
        // put_block_mean = acc_put / L;
        // exponential de-evolution to discont asset price at t = 0
        call_block_mean = discount_factor * acc_call / L;
        put_block_mean = discount_factor * acc_put / L;
        call_block_mean2 = call_block_mean * call_block_mean;
        put_block_mean2 = put_block_mean * put_block_mean;
        
        call_mean_acc += call_block_mean;
        call_mean2_acc += call_block_mean2;
        put_mean_acc += put_block_mean;
        put_mean2_acc += put_block_mean2;

        call_out << setw(8) << i
                 << setw(20) << call_block_mean
                 << setw(20) << call_mean_acc/(i+1) 
                 << setw(20) << error(call_mean_acc, call_mean2_acc, i) 
                 << endl;
                  
        put_out << setw(8) << i
                << setw(20) << put_block_mean
                << setw(20) << put_mean_acc/(i+1) 
                << setw(20) << error(put_mean_acc, put_mean2_acc, i) 
                << endl;

    }

    call_out.close();
    put_out.close();
    
    // ex 3.1.2
    int nstep{100};
    double time_step{double(T)/double(nstep)}; 
    acc_call = 0;
    call_block_mean = 0;
    call_block_mean2 = 0;
    call_mean_acc = 0;
    call_mean2_acc = 0;

    acc_put = 0;
    put_block_mean = 0;
    put_block_mean2 = 0;
    put_mean_acc = 0;
    put_mean2_acc = 0;
    S = 0;
    double old_S{};

    ofstream call_out2("OUTPUT/call2.data");
    ofstream put_out2("OUTPUT/put2.data");
    ofstream asset_out2("OUTPUT/asset2.data");

    call_out2 << "# BLOCK:          CURR_MEAN:          PROG_MEAN:             ERROR:" << endl;
    put_out2 << "# BLOCK:          CURR_MEAN:          PROG_MEAN:             ERROR:" << endl;
    asset_out2 << "# TIME:         SPOT_PRICE:" << endl;
    
    int print_1{int(rnd_print.Rannyu(0, L))};
    int print_2{int(rnd_print.Rannyu(0, N))};

    for(int i{}; i < N; i++){
        
        acc_call = 0;
        acc_put = 0;

        for(int j{}; j < L; j++){
            old_S = S0;
            for(int k{}; k < 100; k++){
                // discrete time evolution
                S = asset_evolution(old_S, drift, volatility, time_step, rnd.Gauss(0.0, 1.0));
                // setting old price to current price 
                old_S = S;
                // printing price evolution of 1 random simulation
                if(j == print_1 && i == print_2){
                    asset_out2 << setw(7) << k
                               << setw(20) << S 
                               << endl;
                }
            }
            acc_call += call_prize(S, K);
            acc_put += put_prize(S, K);
            // exponential de-evolution to discont asset price at t = 0
            // acc_call += discount_factor * call_prize(S, K);
            // acc_put += discount_factor * put_prize(S, K);
        }
        // call_block_mean = acc_call / L;
        // put_block_mean = acc_put / L;
        // exponential de-evolution to discont asset price at t = 0
        call_block_mean = discount_factor * acc_call / L;
        put_block_mean = discount_factor * acc_put / L;
        call_block_mean2 = call_block_mean * call_block_mean;
        put_block_mean2 = put_block_mean * put_block_mean;
        
        call_mean_acc += call_block_mean;
        call_mean2_acc += call_block_mean2;
        put_mean_acc += put_block_mean;
        put_mean2_acc += put_block_mean2;

        call_out2 << setw(8) << i
                  << setw(20) << call_block_mean
                  << setw(20) << call_mean_acc/(i+1) 
                  << setw(20) << error(call_mean_acc, call_mean2_acc, i) 
                  << endl;

        put_out2 << setw(8) << i
                  << setw(20) << put_block_mean
                  << setw(20) << put_mean_acc/(i+1) 
                  << setw(20) << error(put_mean_acc, put_mean2_acc, i) 
                  << endl;

    }

    call_out2.close();
    put_out2.close();
    asset_out2.close();

    return 0;

}
