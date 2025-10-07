#include "random_walk.h"

using namespace std;

// Reads input.dat and set every RW class data member
void RandomWalk::initialize(){

    ifstream in("../INPUT/input.dat");
    ofstream out("../OUTPUT/output.dat");
    if(!out.is_open()) cerr << "RandomWalk::RandomWalk() ERROR : not possible to open \"../OUTPUT/output.dat\"" << endl;
    if(!in.is_open()) cerr << "RandomWalk::RandomWalk() ERROR : not possible to open \"../INPUT/input.dat\"" << endl;

    string property;
    double x;
    while(!in.eof()){
        in >> property;
        if(property == "RW_TYPE"){
            in >> _RWtype;
            out << "RW type : " << _RWtype << endl;
        }else if(property == "N_STEP"){
            in >> _nstep;   // Set RW total steps
            out << "RW total steps : " << _nstep << endl;
            _dist2acc.resize(_nstep);   // Resizes distance vector to _nstep dimension
            _meanblock.resize(_nstep);  // Resizes mean block vector to _nstep dimension
            _meanacc.resize(_nstep);    // Resizes mean accumulator over blocks vector to _nstep dimension
            _mean2acc.resize(_nstep);   // Resizes squared mean accumulator over blocks vector to _nstep dimension
            _dist2acc.zeros();          // Set to zero all components of _dist2acc
            _meanblock.zeros();         // Set to zero all components of _meanblock
            _meanacc.zeros();           // Set to zero all components of _meanacc
            _mean2acc.zeros();          // Set to zero all components of _mean2acc
        }else if(property == "N_DIM"){  // Set RW lattice dimension
            in >> _ndim;
            out << "RW lattice dimension : " << _ndim << endl;
            _xstart.resize(_ndim);   // Set position vector dimension
            _x.resize(_ndim);
            _xstart.zeros();
            _x.zeros();
            if(_ndim == 1 && _RWtype == "CONTINUUM"){
                _RWtype = "DISCRETE";    // Change to discrete type to perform 1D RW
                cerr << "No method implemented to perform continuous RW in 1 dimension. Changing to discrete type to perform 1D RW." << endl;
            }
            if(_ndim == 2 && _RWtype == "CONTINUUM"){
                _RWtype = "DISCRETE";    // Change to discrete type to perform 1D RW
                cerr << "No method implemented to perform continuous RW in 2 dimensions. Changing to discrete type to perform 2D RW." << endl;
            }
            if(_ndim > 3 && _RWtype == "CONTINUUM"){
                cerr << "No method implemented to perform continuous RW in more than 3 dimensions." << endl;
                exit(99);
            }
            // out << "_x dimension : " << _x.size() << endl;
        }else if(property == "RW_STEP"){
            in >> _step;    // Set RW step size
            out << "RW step size : " << _step << endl;
        }else if(property == "START_POS"){
            out << "RW starting position : " ;
            for(int i{}; i < _ndim; i++){   // Set all _x coordinates to starting configuration
                in >> x;
                _xstart.at(i) = x;
                _x.at(i) = x;
                out << _xstart.at(i) << " "; 
            }
            out << endl;
        }else if(property == "N_WALKS"){
            in >> _nwalks;
            out << "Total RW simulated : " << _nwalks << endl;
        }else if(property == "N_BLOCKS"){
            in >> _nblocks;
            out << "Blocks used : " << _nblocks << endl;
            _nwalksperblock = _nwalks/_nblocks;
            out << "RW simulated per block : " << _nwalksperblock << endl;
        }else if(property == "ROW"){  // Read Primes row and initialize random generator using that row
            in >> _row;
            out << "Row read from Primes : " << _row << endl;
            _rnd.StartRandom(_row); 
        }else if(property == "POSITION"){
            in >> _RWprint;
            out << "RW printed for each block : " << _RWprint << endl;
        }else if( property == "ENDINPUT" ){
            out << "Reading input completed!" << endl;
            break;
        }else cerr << "PROBLEM: unknown input" << endl;
    }

    in.close();
    out << "RW class initialized!" << endl;
    out.close();

    return;

}

// Reads properties.dat and prints outputfile description
void RandomWalk::initialize_properties(){

    _measure_dist2 = false;
    _measure_RW = false;
    _measure_last = false;

    ifstream prop("../INPUT/properties.dat");
    if(!prop.is_open()){
        cerr << "ERROR : not possible to open file \"../INPUT/properties.dat\"" << endl;
    }

    string property;
    while(!prop.eof()){
        prop >> property;
        if(property == "DISTANCE"){
            for(int i{}; i < _nstep; i++){
                ofstream out_dist("../OUTPUT/SQRT_MEAN_DISTANCE2/sqrt_mean_distance2_" + to_string(i) + ".dat");
                if(!out_dist.is_open()){
                cerr << "ERROR : not possible to open file \"../OUTPUT/SQRT_MEAN_DISTANCE2/sqrt_mean_distance2_" + to_string(i) + ".dat\"" << endl;
                }
                out_dist << "# RW_STEP = " + to_string(i) << endl << "# BLOCK:          PROG_MEAN:              ERROR:" << endl;
                out_dist.close();
            }
            _measure_dist2 = true;
        }else if(property == "POSITION"){
            for(int i{}; i < _nblocks; i++){
                ofstream xyz("../OUTPUT/RW/rw_" + to_string(_RWprint) + "_" + to_string(i) + ".dat");
                if(!xyz.is_open()){
                    cerr << "ERROR : not possible to open file \"../OUTPUT/RW/rw_" + to_string(_RWprint) + "_" + to_string(i) + ".dat\"" << endl;
                }
                xyz << "# BLOCK:       RW_SIMULATED:               STEP:                  X:                  Y:                  Z:" << endl;
                xyz.close();
            }
            _measure_RW = true;
        }else if(property == "LAST"){
            ofstream outend;
            outend.open("../OUTPUT/last_position.dat");
            outend << "#       RW:                  X:                  Y:                  Z:" << endl;
            outend.close();
            _measure_last = true;
        }else if(property == "ENDPROPERTIES"){
            ofstream out("../OUTPUT/output.dat",ios::app);
            out << "Reading properties completed!" << endl;
            out.close();
            break;
        }else cerr << "PROBLEM: unknown property" << endl;
    }

    return;

}

// Computes error for blocking method (mean and mean2 are accumulator over blocks, not mean over blocks)
double RandomWalk::error(double mean, double mean2, int n){
    
    return (n == 0 ? 0.0 : sqrt(fabs(mean2/double(n+1) - pow(mean/double(n+1), 2))/n));
    
}

// Computes squared distance between _x and start position (if called outside RW for loop _x = last position)
double RandomWalk::get_distance2(){

    double dist2{};
    for(int i{}; i < _ndim; i++){
        dist2 += pow(_x.at(i) - _xstart.at(i), 2);
    }

    return dist2;

}

// Performs RW step (to call inside RW for loop)
void RandomWalk::step(){

    if(_RWtype == "DISCRETE"){
        int direction{int(_ndim*_rnd.Rannyu())};       // doing int(direction) we choose move direction ([0,1] = x, [1,2] = y, [2,3] = z)
        int sign{(_rnd.Rannyu() < 0.5 ? -1 : 1)};      // if Rannyu < 0.5 backward move else forward move
        _x.at(direction) += sign * _step;
    }else if(_RWtype == "CONTINUOUS"){
        double phi{_rnd.Rannyu(0.0, 2.0*M_PI)};      // Uniform sampling phi in [0, 2pi]
        double theta{acos(1 - 2.0*_rnd.Rannyu())};   // sample sin(theta) in [0, pi]
        _x.at(0) += _step * sin(theta) * cos(phi);
        _x.at(1) += _step * sin(theta) * sin(phi);
        _x.at(2) += _step * cos(theta);
    }

    return;

}

// Accumulates RW position squared distance from start position at each step
void RandomWalk::measure(int step){

    if(_measure_dist2 == true){
        _dist2acc.at(step) += this->get_distance2();
    }
    return;

}

// Computes blocking method (to call outside block for loop)
void RandomWalk::averages(int blk){

    double meanacc{};
    double mean2acc{};

    _meanblock = _dist2acc / double(_nwalksperblock);   // Computes block mean for each step
    _meanacc += sqrt(_meanblock);                       // Accumulates distance RMS over blocks
    _mean2acc += _meanblock;                            // Accumulates distance RMS squared over blocks

    for(int i{}; i < _nstep; i++){
        ofstream out_dist("../OUTPUT/SQRT_MEAN_DISTANCE2/sqrt_mean_distance2_" + to_string(i) + ".dat", ios::app);
        if(!out_dist.is_open()) cerr << "ERROR : not possible to open file \"../OUTPUT/SQRT_MEAN_DISTANCE2/sqrt_mean_distance2_" + to_string(i) + ".dat\"" << endl;
        meanacc = _meanacc.at(i);
        mean2acc = _mean2acc.at(i);
        out_dist << setw(8) << blk 
                    << setw(20) << meanacc / double(blk + 1) 
                    << setw(20) << this->error(meanacc, mean2acc, blk) 
                    << endl;
        // This produces the output file to plot (mean distance and error after all blocks)
        if(i == _nstep - 1){
            ofstream fout("../OUTPUT/sqrt_mean_distance2_per_step.dat");
            if(!fout.is_open()) cerr << "ERROR : not possible to open file \"../OUTPUT/sqrt_mean_distance2_per_step.dat\"" << endl;
            fout << "# LAST BLOCK (" + to_string(blk) + ") SQRT MEAN DISTANCE FROM START POSITION FOR EACH STEP IN RW" << endl;
            fout << "# STEP:          PROG_MEAN:              ERROR:" << endl;
            for(int k{}; k < _nstep; k++){
                fout << setw(7) << k 
                     << setw(20) << _meanacc.at(k) / double(blk + 1) 
                     << setw(20) << this->error(_meanacc.at(k), _mean2acc.at(k), blk) 
                     << endl;
            } 
        }
    }
    
    return;

}

// Reset block accumulators to zero (to call every time ends a RW simulation)
void RandomWalk :: reset_block(int blk){

    ofstream out;
    out.open("../OUTPUT/output.dat",ios::app);
    out << "Block completed: " << blk + 1 << endl;
    out.close();

    _meanblock.zeros();     // Not entirely necessary because is not an accumulator
    _dist2acc.zeros();

    return;

}

// Reset _x and set it to starting position
void RandomWalk::reset_position(){
    _x = _xstart;
    return;
}

// Saves last random generated (to call outside outer for loop)
void RandomWalk :: finalize(){

    _rnd.SaveSeed();

    ofstream out;
    out.open("../OUTPUT/output.dat",ios::app);
    out << "Simulation completed!" << endl;
    out.close();

    return;

}

// Prints position evolution during single RW (to call inside RW for loop)
void RandomWalk::write_RW(int step, int RW, int blk){

    if(_measure_RW == true){
        ofstream xyz("../OUTPUT/RW/rw_" + to_string(RW) + "_" + to_string(blk) + ".dat", ios::app);
        if(!xyz.is_open()){
            cerr << "ERROR : not possible to open file \"../OUTPUT/RW/rw_" + to_string(RW) + "_" + to_string(blk) + ".dat\"" << endl;
        }
        xyz << setw(8) << blk // block 
            << setw(20) << RW // block's RW simulated 
            << setw(20) << step // RW step
            << setprecision(12) << setw(20) << _x.at(0) // x
            << setprecision(12) << setw(20) << _x.at(1) // y
            << setprecision(12) << setw(20) << _x.at(2) // z
            << endl;
    }

    return;

}

// Write RW final position (to call after RW for loop)
void RandomWalk :: write_last_position(int RW){
    if(_measure_last == true){
        ofstream outend;
        outend.open("../OUTPUT/last_position.dat", ios::app);
        if(!outend.is_open()) cerr << "PROBLEM: Unable to open \"../OUTPUT/last_position.dat\"" << endl;

        outend << setw(11) << RW
            << setw(20) << setprecision(12) << _x.at(0) // x
            << setw(20) << setprecision(12) << _x.at(1) // y
            << setw(20) << setprecision(12) << _x.at(2) // z
            << endl;
        
        outend.close();
    }

    return;

}