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

void Population::initialize(){

    string property;

    ifstream in("../INPUT/input.dat");
    if(!in.is_open()){
        cerr << "Population::initialize() ERROR : Unable to open \"../INPUT/input.dat\"" << endl;
    }
    ofstream out("../OUTPUT/output.dat");
    if(!out.is_open()){
        cerr << "Population::initialize() ERROR : Unable to open \"../OUTPUT/output.dat\"" << endl;
    }

    while(!in.eof()){
        in >> property;
        if(property == "RND_ROW"){
            in >> _primes_row;
            _rnd.StartRandom(_primes_row);  // Initialize random number generator by reading row _primes_row from Primes file
            out << "Row read from \"Primes\" file : " << _primes_row << endl;
        }else if(property == "N_PATH"){
            in >> _npath;
            _population.set_size(_npath);    // Resize population container to hold the total number of chromosomes
            _new_population.set_size(_npath);
            out << "Total number of paths composing the population : " << _npath << endl;
            assert(_population.n_elem == _npath and "Population size does not correspond to the input parameter.");
        }else if(property == "N_ELITE"){
            in >> _nelite;
            _elite.set_size(_nelite);
            out << "Elitism dimension (number of elite individuals) : " << _nelite << endl;
        }else if(property == "N_CITY"){
            in >> _ncity;
            _distance.set_size(_ncity, _ncity);             // Initialize a square matrix (_ncity x _ncity) to store pairwise distances
            _distance.zeros();                              // Set to zero all distances matrix elements
            _cities.set_size(_ncity, 3);                    // Initialize a matrix (_ncity x 3) to store city indices and coordinates (x, y)
            _cities.zeros();                                // Set to zero all _cities matrix elements
            for(int i{}; i < _npath; i++){                  // Initialize city indices for each path in the population
                _population(i).set_path_size(_ncity);       // Resize each individual's path vector to _ncity
                _population(i).initialize_path(_rnd);       // Population's path initialization
                _population(i).check_path_properties();     // Population path's property check
                _new_population(i).set_path_size(_ncity);   // Set each population's path size to _ncity  and resize _path dimension
            }
            for(int i{}; i < _nelite; i++){
                _elite(i).set_path_size(_ncity);            // Set each population's path size to _ncity  and resize _path dimension
            }
            out << "Total number of cities visited by the traveling salesman : " << _ncity << endl;
        }else if(property == "CITY_DISTRIBUTION"){
            in >> _city_distribution;
            out << "City distribution geometry : " << _city_distribution << endl;
        }else if(property == "N_GENERATIONS"){
            in >> _ngenerations;
            out << "Number of generations to simulate : " << _ngenerations << endl;
        }else if(property == "P_SWAP"){
	        in >> _p_swap;
	        out << "Swap mutation probability : " << _p_swap << endl; 
	    }else if(property == "P_SHIFT"){
            in >> _p_shift;
            out << "Shift mutation probability : " << _p_shift << endl;
        }else if(property == "P_CROSSOVER"){
            in >> _p_crossover;
            out << "Crossover probability : " << _p_crossover << endl;
        }else if(property == "P_INVERSION"){
            in >> _p_inversion;
            out << "Inversion mutation probability : " << _p_inversion << endl;
        }else if(property == "ENDINPUT"){
            out << "Reading input completed!" << endl;
            break;
        }else{
            cerr << "Population::initialize() ERROR : unknown input read" << endl;
        }
    }
    in.close();
    this->read_city_configuration();
    out << "Cities distribution read from input file!" << endl;
    this->compute_distances();
    out << "Cities distances computed!" << endl;
    out.close();

    return;
}


void Population::read_city_configuration(){
    ifstream in;
    string comment;
    int idx{};
    double x,y;
    if(_city_distribution == "CIRCLE"){
        in.open("../INPUT/circle_map.dat");
        assert(in and "read_city_configuration() ERROR: Unable to open \"../INPUT/circle_map.dat\" .");
        getline(in, comment); // Skip file header (first row) by storing it in a temporary variable
        for(int i{}; i < _ncity; i++){
            in >> idx >> x >> y;
            _cities.row(i) = {static_cast<double>(idx), x, y};
        }   
    }else if(_city_distribution == "SQUARE"){
        in.open("../INPUT/squared_map.dat");
        assert(in and "read_city_configuration() ERROR: Unable to open \"../INPUT/squared_map.dat\" .");
        getline(in, comment); // Skip file header (first row) by storing it in a temporary variable
        for(int i{}; i < _ncity; i++){
            in >> idx >> x >> y;
            _cities.row(i) = {static_cast<double>(idx), x, y};
        }
    }

    return;
}


// double Population::get_distance(double x1, double y1, double x2, double y2){
//     return (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2);
// }


void Population::compute_distances(){
    double xi, yi, xj, yj;
    for(int i{}; i < _ncity - 1; i++){
        xi = _cities(i,1);                                      // X-coordinate of city i
        yi = _cities(i,2);                                      // Y-coordinate of city i
        for(int j{i+1}; j < _ncity; j++){                       // Diagonal elements are pre-initialized to zero; distance computation is skipped for i == j
            xj = _cities(j,1);                                  // X-coordinate of city j
            yj = _cities(j,2);                                  // Y-coordinate of city j
            _distance(i,j) = (xi-xj)*(xi-xj) + (yi-yj)*(yi-yj); // Calculate squared Euclidean distance between cities i and j
            _distance(j,i) = _distance(i,j);                    // _distance is symmetric
        }
    }

    return;
}


void Population::initialize_properties(){
    
    string property;

    _measure_best_loss = false;
    _measure_best_path = false;
    _measure_mean_loss = false;

    ifstream prop("../INPUT/properties.dat");
    if (prop.is_open()){
        while(!prop.eof()){
            prop >> property;
            if(property == "BEST_LOSS"){
                ofstream outbl("../OUTPUT/best_loss.dat");
                outbl << "# GENERATION:               LOSS:" << endl;
                outbl.close();
                _measure_best_loss = true;
            }else if(property == "BEST_PATH"){
                ofstream outbp("../OUTPUT/best_path.dat");
                outbp << "# GENERATION:    START CITY:";
                for(int i = 1; i < _ncity; i++){
                    if(i < 9) outbp << setw(13) << "CITY " << i+1 << ":";
                    else outbp << setw(12) << "CITY " << i+1 << ":";
                }
                outbp << "     LAST CITY:" << endl;
                outbp.close();
                _measure_best_path = true;
            }else if(property == "MEAN_LOSS"){
                ofstream outml("../OUTPUT/half_pop_mean_loss.dat");
                outml << "# GENERATION:               LOSS:" << endl;
                outml.close();
                _measure_mean_loss = true;
            }else if( property == "ENDPROPERTIES" ){
                ofstream coutf;
                coutf.open("../OUTPUT/output.dat",ios::app);
                coutf << "Reading properties completed!" << endl;
                coutf.close();
                break;
            }else cerr << "PROBLEM: unknown property" << endl;
        }
        prop.close();
    } else {
        cerr << "initialize_properties() ERROR : Unable to open \"../INPUT/properties.dat\" ." << endl;
        exit(EXIT_FAILURE);
    }

    return;
}


void Population::compute_loss(){
    
    for(int i = 0; i < _npath; i++){
        _population(i).compute_loss(_distance);
    }

    return;
}


void Population::sort(){    // Selection Sort
    int min_idx;
    for(int i = 0; i < _npath-1; i++){
        min_idx = i;
        for(int j = i+1; j < _npath; j++){
            if(_population(j) < _population(min_idx)){
                min_idx = j;
            }
        }
        if(min_idx != i){
            swap(_population(i), _population(min_idx)); 
        }
    }

    return;
}


void Population::crossover(){

    for(int i = 0; i < _nelite; i++){
        _elite(i) = _population(i);
    }

    int cross_couples = int(_npath/2);
    int sel_idx_1, sel_idx_2;
    for(int i = 0; i < cross_couples; i++){

        // Selection routine: preferentially selects lower indices (corresponding to lower loss values) with higher probability
        sel_idx_1 = int(_npath * pow(_rnd.Rannyu(0.0,1.0), 2));
        sel_idx_2 = int(_npath * pow(_rnd.Rannyu(0.0,1.0), 2));

        // Crossover happens with probability p_crossover
        if(_rnd.Rannyu(0.0,1.0) < _p_crossover){
            crossover1(_population(sel_idx_1), _population(sel_idx_2));

            // Add here a second type of crossover, maybe using this type of sintax
            // if(_rnd.Rannyu(0.0,1.0) < 0.5){
            //     crossover1(_population(sel_idx_1), _population(sel_idx_2));
            // }else{
            //     crossover1(_population(sel_idx_1), _population(sel_idx_2));
            // }
        }

        _new_population(i) = _population(sel_idx_1);
        _new_population(i).check_path_properties();

        _new_population(i+cross_couples) = _population(sel_idx_2);
        _new_population(i+cross_couples).check_path_properties();

    }

    return;
}


void Population::crossover1(Chromosome& c1, Chromosome& c2){

    int cut = int(_rnd.Rannyu(2.0, _ncity-1));  // Determine cut point for the left segment; valid range for cut is [2, ..., _ncity-2] 

    ivec c1_left, c1_right, c2_left, c2_right;
    c1_left.resize(cut);
    c1_right.resize(_ncity - cut);
    c2_left.resize(cut);
    c1_right.resize(_ncity - cut);

    c1_left = c1.get_left_segment(cut);
    c1_right = c1.get_right_segment(cut);
    c2_left = c2.get_left_segment(cut);
    c2_right = c2.get_right_segment(cut);

    ivec c1_right_new, c2_right_new;
    c1_right_new.resize(_ncity - cut);
    c2_right_new.resize(_ncity - cut);

    c1_right_new = create_new_tail(c1_right, c2);
    c2_right_new = create_new_tail(c2_right, c1);

    c1.merge(c1_left, c1_right_new);
    c2.merge(c2_left, c2_right_new);

    return;
}


ivec Population::create_new_tail(ivec right, Chromosome c){

    int idx = 0;
    int dim = right.n_elem;
    ivec n_right(dim);
    for(int i = 1; i < _ncity; i++){
        for(int j = 0; j < dim; j++){
            if(c.get_city(i) == right(j)){
                n_right(idx) = right(j);
                idx++; 
            }
        }
    }

    return n_right;
}


void Population::mutation(){

    int length_avaible = _ncity - 1;    // Maximum subset size of _path available for mutation
    int m;

    for(int i = 0; i < _npath; i++){

        if(_rnd.Rannyu(0.0,1.0) < _p_swap){                 // Swap mutation (permutation of a block of size m+1)
            m = int(_rnd.Rannyu(0.0, length_avaible/2));
            _new_population(i).swap_mutation(m, _rnd);
            _new_population(i).check_path_properties();
        }
        if(_rnd.Rannyu(0.0,1.0) < _p_shift){                // Shift mutation (block of size m+1)
            m = int(_rnd.Rannyu(0.0, length_avaible - 1));
            _new_population(i).shift_mutation(m, _rnd);
            _new_population(i).check_path_properties();
        }
        if(_rnd.Rannyu(0.0,1.0) < _p_inversion){            // Path inversion mutation
            _new_population(i).inversion_mutation(_rnd);
            _new_population(i).check_path_properties();
        }

        _new_population(i).compute_loss(_distance);
    }

    _population = _new_population;          // Replace the old generation with the new population produced via crossover and mutation

    sort();                                 // Sort population by loss

    int start_elite_idx = _npath - _nelite;

    for(int i = 0; i < _nelite; i++){
        _population(start_elite_idx + i) = _elite(i);   // Replace the worst individuals with the elite paths preserved from the previous generation
    }

    sort();                                 // Sort population by loss

    return;
}


void Population::measure(int gen){

    if(_measure_best_loss){
        ofstream outbl("../OUTPUT/best_loss.dat", ios::app);
        assert(outbl and "measure(int i) ERROR : Unable to open \"../OUTPUT/best_loss.dat\" .");
        outbl << setw(13) << gen
            << setw(20) << _population(0).get_loss() << endl;
        outbl.close();
    }

    if(_measure_mean_loss){
        double loss = 0.0;
        int half_pop = _npath/2;
        for(int i =0; i < half_pop; i++){
            loss += _population(i).get_loss()/half_pop;
        }
        ofstream outml("../OUTPUT/half_pop_mean_loss.dat", ios::app);
        assert(outml and "measure(int i) ERROR : Unable to open \"../OUTPUT/half_pop_mean_loss.dat\" .");
        outml << setw(13) << gen
              << setw(20) << loss << endl;
        outml.close();
    }

    if(_measure_best_path){
        ofstream outbp("../OUTPUT/best_path.dat", ios::app);
        assert(outbp and "measure(int i) ERROR : Unable to open \"../OUTPUT/best_path.dat\" .");
        outbp << setw(13) << gen;
        for(int i = 0; i < _ncity; i++){
            outbp << setw(15) << _population(0).get_city(i);
        }
        outbp << setw(15) << _population(0).get_city(0) << endl;    // Append the starting city index to the end of the path to denote a closed loop
        outbp.close();
    }

    return;
}


void Population::finalize(){

    _rnd.SaveSeed();
    ofstream coutf;
    coutf.open("../OUTPUT/output.dat",ios::app);
    coutf << "Evolution completed!" << endl;
    coutf.close();

    return;
}


void Population::print_last_best_path(){

    int city_idx, cities_row;
    ofstream out("../OUTPUT/best_city_distribution.dat");
    out << "# CITY INDEX:                  X:                  Y:" << endl;
    for(int i = 0; i < _ncity; i++){
        city_idx = _population(0).get_city(i);
        cities_row = city_idx - 1;
        out << setw(13) << city_idx
            << setw(20) << _cities(cities_row,1)
            << setw(20) << _cities(cities_row,2) << endl;
    }
    out << setw(13) << _population(0).get_city(0)   // Append the starting city index to the end of the path to denote a closed loop
        << setw(20) << _cities(0,1)
        << setw(20) << _cities(0,2) << endl;
    out.close();

    return;
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
