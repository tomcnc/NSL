/****************************************************************
*****************************************************************
    _/    _/  _/_/_/  _/       Numerical Simulation Laboratory
   _/_/  _/ _/       _/       Physics Department
  _/  _/_/    _/    _/       Universita' degli Studi di Milano
 _/    _/       _/ _/       Prof. D.E. Galli
_/    _/  _/_/_/  _/_/_/_/ email: Davide.Galli@unimi.it
*****************************************************************
*****************************************************************/

#include <chrono>

#include "population.h"

int main(int argc, char** argv){
 
    MPI_Init(&argc,&argv);
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    auto start = chrono::high_resolution_clock::now();    // Execution start time

    Population pop;

    pop.initialize(rank);
    pop.initialize_properties(rank);
    
    pop.compute_loss();
    pop.sort();

    int messages = pop.get_nmessage();
    int generations = pop.get_ngenerations();
    messages = int(generations)/messages;

    ofstream out("../OUTPUT/output.dat", ios::app);
    for(int i = 0; i < generations; i++){
        pop.crossover();
        pop.mutation();
        if(i % messages == 0){
            pop.process_path_exchange(size,rank);
        }
        pop.measure(i+1, rank);
        //out << "Process " << rank << " --> Generation " << i+1 << " completed." << endl;
    }
    out.close();

    pop.print_last_best_path(rank);
    pop.finalize(rank);

    auto end = chrono::high_resolution_clock::now();  // Execution end time
    
    if(rank == 0){
	    
        ofstream time("../OUTPUT/exe_time.dat");
    
        chrono::duration<double> duration = end - start;    // Sets execution time to seconds
	
	    time << "# POPULATION_SIZE:         EXECUTION_TIME (sec):" << endl;

        time << setw(18) << pop.get_npath()*size
	         << setw(30) << duration.count() << endl;

    }

    MPI_Finalize();

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
