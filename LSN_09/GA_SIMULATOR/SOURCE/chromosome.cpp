/****************************************************************
*****************************************************************
    _/    _/  _/_/_/  _/       Numerical Simulation Laboratory
   _/_/  _/ _/       _/       Physics Department
  _/  _/_/    _/    _/       Universita' degli Studi di Milano
 _/    _/       _/ _/       Prof. D.E. Galli
_/    _/  _/_/_/  _/_/_/_/ email: Davide.Galli@unimi.it
*****************************************************************
*****************************************************************/

#include "chromosome.h"

Chromosome& Chromosome::operator=(const Chromosome& c){
    
    if(this != &c){
        _ncity = c._ncity;
        _path = c._path;
        _path_goodness = c._path_goodness;
        _loss = c._loss;
    }

    return *this;
}


bool Chromosome::operator<(const Chromosome& c) const {
    
    return this->_loss < c._loss;
    //return this->get_loss() < c.get_loss();
}


void Chromosome::set_path_size(int size){
    
    _ncity = size;
    _path.resize(_ncity);
    
    return;
}

 
void Chromosome::initialize_path(Random& rnd){

    _path.at(0) = 1;  // set first gene to 1

    int idx;
    int city_idx_size{_ncity-1};     // Since the path's starting city is fixed at index 1, the vector containing all other potential cities has size _ncity - 1
    ivec city_idx;                   // Create a vector of alleles
    city_idx.resize(city_idx_size);   // Possible city index are 2, ..., 34
    for(int i{}; i < city_idx_size; i++){
        city_idx(i) = i+2; // alleles will be {2, 3, ..., _ncity}
    }
	
    for(int i{1}; i < _ncity; i++){
        idx = int(rnd.Rannyu(0, city_idx_size - i + 1));    // Choose a random element of city_idx (its size decreases each iteration since the corresponding element is removed from the list once it is used)
        _path(i) = city_idx(idx);                           // Set _path element to city_idx element at index idx
        city_idx.shed_row(idx);                             // Remove city index placed into _path 
    }

    //=========================//
    //    using permutation    //
    //=========================//

    // _path.resize(_ncity);
    // for(int i{}; i < _ncity; i++){
    //     _path(i) = i+1;
    // }

    // for(int i{}; i < _ncity; i++){
    //     swap_mutation(0, rnd);
    // }

    return;
}


void Chromosome::check_path_properties(){

    if(_path(0) != 1){
        cerr << "Path condition ERROR : first city is " << _path(0) << " but has to be 1." << endl;
        _path_goodness = false;
    }else{
        for(int i{}; i < _ncity - 1; i++){
            for(int j{i+1}; j < _ncity; j++){
                if(_path(i) == _path(j)){
                    cerr << "Path condition ERROR : city at " << i << " is equal to city at " << j << endl;
                    _path_goodness = false;
                }
            }
        }
    }
    _path_goodness = true;

}


void Chromosome::compute_loss(mat D){

    double loss = 0.0;
    int row,col;
    for(int i=0; i < _ncity; i++){
        row = _path(i) - 1;         // _path() contains cities index [1, ..., _ncity], D matrix elements Dij have index i,j = 0, ..., _ncity-1
        if(i == _ncity-1){           
            col = _path(0) - 1;     // Path is closed so we need to add also distance between last city and first city using PBCs
        }else{
            col = _path(i+1) - 1;   // _path() contains cities index [1, ..., _ncity], D matrix elements Dij have index i,j = 0, ..., _ncity-1
        }
        loss += D(row,col);
    }
    _loss = loss;

    return;
}


void Chromosome::swap_mutation(int m, Random& rnd){

    int L = m + 1;              // Effective subset length (m = 0 -> L = 1 representing a simple swap of two cities)
    int ncity_eff = _ncity - 1; // Number of cities effectively available, given that _path(0) = 1 is the fixed starting city 

    int avaible_space = ncity_eff - 2*L + 1;    // Total number of possible placements for two blocks of size L

    int p1 = int(rnd.Rannyu(0.0, avaible_space));
    int p2 = int(rnd.Rannyu(0.0, avaible_space));

    if(p1 >= avaible_space) p1 = avaible_space - 1;
    if(p2 >= avaible_space) p2 = avaible_space - 1;

    int min_p = min(p1,p2); // Number of empty slots preceding the first block
    int max_p = max(p1,p2); // Number of empty slots preceding the second block

    int i = 1 + min_p;                  // Index of the first element of the first block 
    int j = i + L + (max_p - min_p);    // Index of the first element of the second block 

    bool overlap = (std::max(i, j) < std::min(i + L, j + L));
    assert(!overlap and "block_swap(m, _rnd) ERROR : blocks overlapping.");
    assert(j >= 1 and j + L <= _ncity and "block_swap(m, _rnd) ERROR : j does not check _path properties.");
    assert(i >= 1 and i + L <= _ncity and "block_swap(m, _rnd) ERROR : i does not check _path properties.");

    // Block swapping
    for(int k = 0; k < L; k++){
        std::swap(_path(i + k), _path(j + k));
    }
    
    return;
}


void Chromosome::shift_mutation(int m, Random& rnd){

    int L = m + 1;          // Effective subset length (m = 0 -> L = 1 so is the simple permutation of two cities)
    int i_max = _ncity - L; // Block range is [i, i + L - 1]; the last available index is _ncity - 1. Thus, i + L - 1 <= _ncity - 1 implies i <= _ncity - L = max_i

    int i = 1 + int(rnd.Rannyu(0.0, i_max));    // The starting index of the block can be any integer in the range [1, ..., max_i]
    
    int n_left = i - 1;         // Total number of possible left shift
    int n_right = i_max - i;    // Total number of possible right shift

    int choice = int(rnd.Rannyu(0.0, n_left + n_right));
    
    int j = 0;              // Final shift index
    if (choice < n_left) {  // Mapping [0, n_left-1] -> [1, i-1] 
        j = 1 + choice;
    } else {                // Remove left offset and mapping [0, n_right-1] -> [i+1, max_i]
        j = (i + 1) + (choice - n_left);
    }

    auto first = _path.begin(); // Iterator pointing to the first element of _path
    
    // rotate(first, middle, last) -> moves the subset [middle, last) to the beginning, shifting [first, middle) to the right
    if (j < i) {    // Left shift: the block [i, i+L) is moved to the left
        rotate(first + j, first + i, first + i + L);
    } 
    else {          // Right shift: the segment [i+L, j) is moved to the left, effectively shifting the block [i, i+L) to the right
        rotate(first + i, first + i + L, first + j + L);
    }

    return;
}


void Chromosome::inversion_mutation(Random& rnd){

    int i = int(rnd.Rannyu(1.0, _ncity-1));
    int j = int(rnd.Rannyu(1.0, _ncity-1));
    while(i==j){
        j = int(rnd.Rannyu(1.0, _ncity-1));
    }

    if(i > j) swap(i,j);    // This step is crucial to prevent out-of-bounds access, ensuring i is the minimum index (as it is incremented in the loop)

    int pairs = abs(j-i+1)/2;
    
    for(int k = 0; k < pairs; k++){
        swap(_path(i+k), _path(j-k));
    }

    return;
}


ivec Chromosome::get_left_segment(int cut){
    
    ivec left_c(cut);
    for(int i = 0; i < cut; i++){
        left_c(i) = _path(i);
    }

    return left_c;
}


ivec Chromosome::get_right_segment(int cut){

    int dim = _ncity - cut;
    ivec right_c(dim);
    for(int i = 0; i < dim; i++){
        right_c(i) = _path(i + cut);
    }

    return right_c;
}


void Chromosome::merge(ivec left, ivec right){

    // arma::join_cols(...) sequentially concatenates two column vectors
    _path = join_cols(left,right);

    return;
}


void Chromosome::print_path(){

    for(int i = 0; i < _ncity; i++){
        cout << _path(i) << " "; 
    }
    cout << endl;

    return ;
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