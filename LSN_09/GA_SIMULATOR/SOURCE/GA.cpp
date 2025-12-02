/****************************************************************
*****************************************************************
    _/    _/  _/_/_/  _/       Numerical Simulation Laboratory
   _/_/  _/ _/       _/       Physics Department
  _/  _/_/    _/    _/       Universita' degli Studi di Milano
 _/    _/       _/ _/       Prof. D.E. Galli
_/    _/  _/_/_/  _/_/_/_/ email: Davide.Galli@unimi.it
*****************************************************************
*****************************************************************/

#include "population.h"

int main(){

    Population pop;

    pop.initialize();
    pop.initialize_properties();
    
    pop.compute_loss();
    pop.sort();

    int generations = pop.get_ngenerations();
    ofstream out("../OUTPUT/output.dat", ios::app);
    for(int i = 0; i < generations; i++){
        pop.crossover();
        pop.mutation();
        pop.measure(i+1);
        out << "Generation " << i+1 << " completed." << endl;
    }
    out.close();

    pop.print_last_best_path();
    pop.finalize();

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
