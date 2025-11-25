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
#include <cstdlib>
#include <string>
#include "system.h"

using namespace std;
using namespace arma;

void System :: step(){  // Perform a simulation step
  if(_sim_type == 0) this->Verlet();  // Perform a MD step
  else {
    if(_sim_type != 4){
      for(int i=0; i<_npart; i++){
        this->move(int(_rnd.Rannyu()*_npart)); // Perform a MC step on a randomly choosen particle (NB:a single step does 50 M(RT)^2 moves, so at each step the magnetization is the one after 50 Metropolis moves, not only after one Metropolis move)
      }
    }
    else{
      for(int i=0; i<_npart; i++){
      this->move(i); // Perform M(RT)^2 step in VMC
      }
    }
  }
  _nattempts += _npart; // Update number of attempts performed on the system
  return;
}

void System :: Verlet(){
  double xnew, ynew, znew;
  for(int i=0; i<_npart; i++){  // Force acting on particle i
    _fx(i) = this->Force(i,0);
    _fy(i) = this->Force(i,1);
    _fz(i) = this->Force(i,2);
  }
  for(int i=0; i<_npart; i++){  // Verlet integration scheme
    xnew = this->pbc( 2.0 * _particle(i).getposition(0,true) - _particle(i).getposition(0,false) + _fx(i) * pow(_delta,2), 0);
    ynew = this->pbc( 2.0 * _particle(i).getposition(1,true) - _particle(i).getposition(1,false) + _fy(i) * pow(_delta,2), 1);
    znew = this->pbc( 2.0 * _particle(i).getposition(2,true) - _particle(i).getposition(2,false) + _fz(i) * pow(_delta,2), 2);
    _particle(i).setvelocity(0, this->pbc(xnew - _particle(i).getposition(0,false), 0)/(2.0 * _delta));
    _particle(i).setvelocity(1, this->pbc(ynew - _particle(i).getposition(1,false), 1)/(2.0 * _delta));
    _particle(i).setvelocity(2, this->pbc(znew - _particle(i).getposition(2,false), 2)/(2.0 * _delta));
    _particle(i).acceptmove(); // xold = xnew
    _particle(i).setposition(0, xnew);
    _particle(i).setposition(1, ynew);
    _particle(i).setposition(2, znew);
  }
  _naccepted += _npart;
  return;
}

// Compute force as LJ potential gradient produced by other particles on method input i particle
// Evaluating the potential cut-off

double System :: Force(int i, int dim){
  double f=0.0, dr;
  vec distance;
  distance.resize(_ndim);
  for (int j=0; j<_npart; j++){
    if(i != j){
      distance(0) = this->pbc( _particle(i).getposition(0,true) - _particle(j).getposition(0,true), 0);
      distance(1) = this->pbc( _particle(i).getposition(1,true) - _particle(j).getposition(1,true), 1);
      distance(2) = this->pbc( _particle(i).getposition(2,true) - _particle(j).getposition(2,true), 2);
      dr = sqrt( dot(distance,distance) );
      if(dr < _r_cut){
        f += distance(dim) * (48.0/pow(dr,14) - 24.0/pow(dr,8));
      }
    }
  }
  return f;
}

void System :: move(int i){ // Propose a MC move for particle i
  if(_sim_type == 1){       // M(RT)^2 LJ NVT system
    vec shift(_ndim);       // Will store the proposed translation
    for(int j=0; j<_ndim; j++){
      shift(j) = _rnd.Rannyu(-1.0,1.0) * _delta; // Uniform distribution in [-_delta;_delta)
    }
    _particle(i).translate(shift, _side);  // Call the function Particle::translate
    if(this->metro(i)){ // Metropolis acceptance evaluation
      _particle(i).acceptmove();
      _naccepted++;
    } else _particle(i).moveback(); // If translation is rejected, restore the old configuration
  } else if(_sim_type == 2){  // Ising 1D M(RT)^2
      if(this->metro(i)){     // Metropolis acceptance evaluation for a spin flip involving spin i
        _particle(i).flip();  // If accepted, the spin i is flipped
        _naccepted++;
      }
  } else if(_sim_type == 3){ // Gibbs sampler for Ising (Added by me)
    int s_left = _particle(this->pbc(i-1)).getspin();
    int s_right = _particle(this->pbc(i+1)).getspin();
    double delta_E = 2.0 * (_H + _J * (s_left + s_right)); 
    double p_up = 1.0/(1.0 + exp(-_beta * delta_E));
    if(_rnd.Rannyu() < p_up){
      _particle(i).setspin(1);
    } else{
      _particle(i).setspin(-1);
    }
    _naccepted++;
  } else if(_sim_type == 4){
    vec shift(_ndim);       // Will store the proposed translation (only one dimension)
    shift(0) = _rnd.Rannyu(-1.0,1.0) * _delta; // Uniform distribution in [-_delta;_delta)
    shift(1) = 0.0;
    shift(2) = 0.0;
    _particle(i).translate(shift);
    if(this->metro(i)){ // Metropolis acceptance evaluation
      _particle(i).acceptmove();
      _naccepted++;
    } else _particle(i).moveback(); // If translation is rejected, restore the old configuration
  }
  return;
}

bool System :: metro(int i){ // Metropolis algorithm
  bool decision = false;
  double delta_E, acceptance;
  if(_sim_type == 1){
    delta_E = this->Boltzmann(i,true) - this->Boltzmann(i,false);
  }
  else if(_sim_type == 2){
    delta_E = 2.0 * _particle(i).getspin() * ( _J * (_particle(this->pbc(i-1)).getspin() + _particle(this->pbc(i+1)).getspin() ) + _H );
  }
  if(_sim_type == 1 or _sim_type == 2){
    acceptance = exp(-_beta*delta_E);
    if(_rnd.Rannyu() < acceptance ) decision = true; // Metropolis acceptance step
  }else if (_sim_type == 4){
    acceptance = this->Psi2(i,true)/this->Psi2(i, false);
    if(acceptance >= 1){
      decision = true;
    }else if(_rnd.Rannyu(0.0,1.0) < acceptance){
      decision = true;
    }
  }
  return decision;
}

double System :: Boltzmann(int i, bool xnew){
  double energy_i=0.0;
  double dx, dy, dz, dr;
  for (int j=0; j<_npart; j++){
    if(j != i){
      dx = this->pbc(_particle(i).getposition(0,xnew) - _particle(j).getposition(0,1), 0);
      dy = this->pbc(_particle(i).getposition(1,xnew) - _particle(j).getposition(1,1), 1);
      dz = this->pbc(_particle(i).getposition(2,xnew) - _particle(j).getposition(2,1), 2);
      dr = dx*dx + dy*dy + dz*dz;
      dr = sqrt(dr);
      if(dr < _r_cut){
        energy_i += 1.0/pow(dr,12) - 1.0/pow(dr,6);
      }
    }
  }
  return 4.0 * energy_i;
}

// Ritorna il valore del modulo quadro della funzione d'onda
double System :: Psi2(int i, bool xnew){
  double a{_particle(i).getposition(0, xnew) - _parameters(0)};
  double b{_particle(i).getposition(0, xnew) + _parameters(0)};
  double c{0.5/(_parameters(1) * _parameters(1))};
  double d{(-1.0)*a*a*c};
  double f{(-1.0)*b*b*c};
  double e_1{exp(d)};
  double e_2{exp(f)};
  double psi{e_1 + e_2};
  return psi*psi;
}

double System :: pbc(double position, int i){ // Enforce periodic boundary conditions
  return position - _side(i) * rint(position / _side(i));
}

int System :: pbc(int i){ // Enforce periodic boundary conditions for spins
  if(i >= _npart) i = i - _npart;
  else if(i < 0)  i = i + _npart;
  return i;
} 


void System :: initialize(){ // Initialize the System object according to the content of the input files in the ../INPUT/ directory

  // int p1, p2; // Read from ../INPUT/Primes a pair of numbers to be used to initialize the RNG
  // ifstream Primes("../INPUT/Primes");
  // Primes >> p1 >> p2 ;
  // Primes.close();
  // int seed[4]; // Read the seed of the RNG
  // ifstream Seed("../INPUT/seed.in");
  // Seed >> seed[0] >> seed[1] >> seed[2] >> seed[3];
  // _rnd.SetRandom(seed,p1,p2);

  // if(_print_acceptance){
  //   ofstream couta("../OUTPUT/acceptance.dat"); // Set the heading line in file ../OUTPUT/acceptance.dat
  //   couta << "#   N_BLOCK:         ACCEPTANCE:" << endl;
  //   couta.close();
  // }

  ifstream input("../INPUT/input.dat"); // Start reading ../INPUT/input.dat
  ofstream coutf;
  coutf.open("../OUTPUT/output.dat");
  string property;
  double delta;
  if( input.is_open() ){
    while ( !input.eof() ){
      input >> property;
      if( property == "SIMULATION_TYPE" ){
        input >> _sim_type;
        if(_sim_type > 1 and _sim_type < 4){
          input >> _J;
          input >> _H;
        }
        if(_sim_type > 4){
          cerr << "PROBLEM: unknown simulation type" << endl;
          exit(EXIT_FAILURE);
        }
        else if(_sim_type == 1) coutf << "LJ MONTE CARLO (NVT) SIMULATION"         << endl;
        else if(_sim_type == 2) coutf << "ISING 1D MONTE CARLO (MRT^2) SIMULATION" << endl;
        else if(_sim_type == 3) coutf << "ISING 1D MONTE CARLO (GIBBS) SIMULATION" << endl;
        else if(_sim_type == 4) coutf << "VARIATIONAL MONTE CARLO (VMC) SIMULATION" << endl;
      } else if( property == "DISTRIBUTION_TYPE" && _sim_type == 0){
        input >> _dist_type;
          if(_dist_type == 0)       coutf << "LJ MOLECULAR DYNAMICS (NVE) SIMULATION starting from MAXBOLTZ velocity distribution"  << endl;
          else if(_dist_type == 1)  coutf << "LJ MOLECULAR DYNAMICS (NVE) SIMULATION starting from DIRAC velocity distribution"  << endl;
          else{
            cerr << "PROBLEM: unknown initial simulation velocity distribution type" << endl;
            exit(EXIT_FAILURE);
          }
      } else if( property == "RESTART" ){
        input >> _restart;
        // Added here the LCG routine (more useful since allow to change seed and restart reading seed.out)
        if(_restart){
          _rnd.RestartRandom();  // LCG restart routine using Primes file first row
        } else{
          _rnd.StartRandom();  // LCG start routine using Primes file first row
        }
      } else if( property == "TEMP" ){
        input >> _temp;
        _beta = 1.0/_temp;
        coutf << "TEMPERATURE= " << _temp << endl;
      } else if( property == "NPART" ){
        input >> _npart;
        if(_sim_type < 4){  // VMC does not need force vectors
          _fx.resize(_npart);
          _fy.resize(_npart);
          _fz.resize(_npart);
        }
        _particle.set_size(_npart);
        for(int i=0; i<_npart; i++){ 
          _particle(i).initialize(); // initialize() sets _spin = 1 (ground state configuration at T = 0)
          if(_rnd.Rannyu() > 0.5) _particle(i).flip(); // to randomize the spin configuration
        }
        coutf << "NPART= " << _npart << endl;
      } else if( property == "RHO" ){
        input >> _rho;
        _volume = _npart/_rho;
        _side.resize(_ndim);
        _halfside.resize(_ndim);
        double side = pow(_volume, 1.0/3.0);
        for(int i=0; i<_ndim; i++) _side(i) = side;
        _halfside=0.5*_side;
        coutf << "SIDE= ";
        for(int i=0; i<_ndim; i++){
          coutf << setw(12) << _side[i];
        }
        coutf << endl;
      } else if( property == "R_CUT" ){
        input >> _r_cut;
        coutf << "R_CUT= " << _r_cut << endl;
      } else if( property == "DELTA" ){
        input >> delta;
        coutf << "DELTA= " << delta << endl;
        _delta = delta;
      } else if( property == "NBLOCKS" ){
        input >> _nblocks;
        coutf << "NBLOCKS= " << _nblocks << endl;
      } else if( property == "NSTEPS" ){
        input >> _nsteps;
        coutf << "NSTEPS= " << _nsteps << endl;
      } else if( property == "TAILS"){
        input >> _tails;
        coutf << "TAILS= " << _tails << endl;
        if(_tails){
          double a{M_PI * _rho};
          double b{3.0 * _r_cut * _r_cut * _r_cut};
          double c{1.0/b}; // --> 1.0/[3.0*(r_cut)^3]
          double d{3.0*c*c*c}; // --> 3.0/[27.0*(r_cut)^9] = 1.0/[9.0*(r_cut)^9]
          _vtail = 8.0 * a * (d - c); // Fixed in exercise 7 --> _vtail = 8 * pi * rho * {1/[9*(r_c)^9] - 1/[3*(r_c)^3]}
          _ptail = 32.0 * a * (d - 0.5*c); // Fixed in exercise 7 --> _ptail = 32 * pi * rho * {1/[9*(r_c)^9] - 1/[6*(r_c)^3]}
        } else {
          _vtail = 0.0;
          _ptail= 0.0;
        }
        coutf << "VTAIL=" << _vtail << endl;
        coutf << "PTAIL=" << _ptail << endl;
      } else if( property == "NPARAM"){
        input >> _nparam;
        coutf << "NPARAM= " << _nparam << "       ";
        _parameters.set_size(_nparam);
        _parameters.zeros();
        for(int i{}; i < _nparam; i++){
          input >> _parameters(i);
          coutf << "PARAM(" << i <<")=" << _parameters(i) << "       ";
        }
        coutf << endl;
        _parameters_current.set_size(_nparam);
        _parameters_current.zeros();
      } else if( property == "DELTA_PARAM"){
        _delta_parameters.set_size(_nparam);
        _delta_parameters.zeros();
        for(int i{}; i < _nparam; i++){
          input >> _delta_parameters(i);
          coutf << "DELTA_PARAM(" << i <<")=" << _delta_parameters(i) << "       ";
        }
        coutf << endl;
      } else if( property == "DELTA_TEMP"){
        input >> _delta_temp;
        coutf << "DELTA_TEMP= " << _delta_temp << endl;
      } else if( property == "NTEMP"){
        input >> _n_temp;
        coutf << "NTEMP= " << _n_temp << endl;
      } else if( property == "SA_TEMP_STEPS"){
        input >> _nsteps_SA;
        coutf << "SA_TEMP_STEPS= " << _nsteps_SA << endl;
      } else if( property == "EQUIL_STEPS"){
        input >> _eq_steps;
        coutf << "EQUIL_STEPS= " << _eq_steps << endl;
      } else if( property == "ENDINPUT" ){
        coutf << "Reading input completed!" << endl;
        break;
      } else cerr << "PROBLEM: unknown input" << endl;
    }
  } else {
    cerr << "PROBLEM: Unable to open INPUT file input.dat" << endl;
    exit(EXIT_FAILURE);
  }
  input.close();
  this->read_configuration(); 
  if(_sim_type==0) this->initialize_velocities();
  coutf << "System initialized!" << endl;
  coutf.close();
  return;
}

// Added Dirac's Delta velocity distribution
void System :: initialize_velocities(){
  double xold, yold, zold;
  if(_restart){
    ifstream cinf;
    cinf.open("../INPUT/CONFIG/conf-1.xyz");
    if(cinf.is_open()){
      string comment;
      string particle;
      int ncoord;
      cinf >> ncoord;
      if (ncoord != _npart){
        cerr << "PROBLEM: conflicting number of coordinates in input.dat & config.xyz not match!" << endl;
        exit(EXIT_FAILURE);
      }
      cinf >> comment;
      for(int i=0; i<_npart; i++){
        cinf >> particle >> xold >> yold >> zold; // units of coordinates in conf.xyz is _side
        _particle(i).setpositold(0, this->pbc(_side(0)*xold, 0));
        _particle(i).setpositold(1, this->pbc(_side(1)*yold, 1));
        _particle(i).setpositold(2, this->pbc(_side(2)*zold, 2));
      }
    } else {
      cerr << "PROBLEM: Unable to open INPUT file conf-1.xyz"<< endl;
      exit(EXIT_FAILURE);
    }
    cinf.close();
  } else {
    vec vx(_npart), vy(_npart), vz(_npart);
    vec sumv(_ndim);
    sumv.zeros();
    if(_dist_type == 1){     // Dirac delta velocity distribution (low entropic initial configuration)
      ofstream coutpvs;
      coutpvs.open("../OUTPUT/initial_velocity_distribution.dat");
      coutpvs << "#      V_BIN_CENTER:             COUNTS:" << endl;
      double v_start{sqrt(3.0*_temp)} ; // start velocity 
      for(int i{}; i < _npart; i++){
        int resto = i % 6 ;
        if(resto == 0){
          vx(i) = v_start ;
          vy(i) = 0.0 ;
          vz(i) = 0.0 ;
        } else if(resto == 1){
          vx(i) = -v_start ;
          vy(i) = 0.0 ;
          vz(i) = 0.0 ;
        } else if(resto == 2){
          vx(i) = 0.0 ;
          vy(i) = v_start ;
          vz(i) = 0.0 ;
        } else if(resto == 3){
          vx(i) = 0.0 ;
          vy(i) = -v_start ;
          vz(i) = 0.0 ;
        } else if(resto == 4){
          vx(i) = 0.0 ;
          vy(i) = 0.0 ;
          vz(i) = v_start ;
        } else if(resto == 5){
          vx(i) = 0.0 ;
          vy(i) = 0.0 ;
          vz(i) = -v_start ;
        }
      }
      for (int i=0; i<_npart; i++){
        _particle(i).setvelocity(0, vx(i));
        _particle(i).setvelocity(1, vy(i));
        _particle(i).setvelocity(2, vz(i));
      }
      for(int k{}; k < _n_bins_v; k++){
        coutpvs << setw(20) << _bin_size_v * 0.5 + k * _bin_size_v
                << setw(20) << _measurement(_index_pofv + k) << endl;
      }
      coutpvs.close();
    } else if(_dist_type == 0){ // Typical Maxwell Boltzmann velocity distribution initial configuration (not used in ex 4.2-4.3)
      for (int i=0; i<_npart; i++){
        vx(i) = _rnd.Gauss(0.,sqrt(_temp));
        vy(i) = _rnd.Gauss(0.,sqrt(_temp));
        vz(i) = _rnd.Gauss(0.,sqrt(_temp));
        sumv(0) += vx(i);
        sumv(1) += vy(i);
        sumv(2) += vz(i);
      }
      for (int idim=0; idim<_ndim; idim++) sumv(idim) = sumv(idim)/double(_npart);
      double sumv2 = 0.0, scalef;
      for (int i=0; i<_npart; i++){
        vx(i) = vx(i) - sumv(0);
        vy(i) = vy(i) - sumv(1);
        vz(i) = vz(i) - sumv(2);
        sumv2 += vx(i) * vx(i) + vy(i) * vy(i) + vz(i) * vz(i);
      }
      sumv2 /= double(_npart);
      scalef = sqrt(3.0 * _temp / sumv2);   // velocity scale factor to bring back velocities compatible with the temperature
      for (int i=0; i<_npart; i++){
        _particle(i).setvelocity(0, vx(i)*scalef);
        _particle(i).setvelocity(1, vy(i)*scalef);
        _particle(i).setvelocity(2, vz(i)*scalef);
      }
    } else {  // ADDED BY ME
      cerr << "PROBLEM : start velocity distribution configuration not valid" << endl;
      exit(EXIT_FAILURE);
    }
    for (int i=0; i<_npart; i++){
      xold = this->pbc( _particle(i).getposition(0,true) - _particle(i).getvelocity(0)*_delta, 0);
      yold = this->pbc( _particle(i).getposition(1,true) - _particle(i).getvelocity(1)*_delta, 1);
      zold = this->pbc( _particle(i).getposition(2,true) - _particle(i).getvelocity(2)*_delta, 2);
      _particle(i).setpositold(0, xold);
      _particle(i).setpositold(1, yold);
      _particle(i).setpositold(2, zold);
    }
  }
  return;
}

void System :: initialize_properties(){ // Initialize data members used for measurement of properties

  string property;
  int index_property = 0;
  _nprop = 0;
  //Defining which properties will be measured
  _measure_penergy         = false;
  _measure_penergy_step    = false; // Added by me
  _measure_kenergy         = false;
  _measure_kenergy_step    = false; // Added by me
  _measure_tenergy         = false;
  _measure_tenergy_step    = false; // Added by me
  _measure_pressure        = false;
  _measure_pressure_step   = false; // Added by me
  _measure_temp            = false;
  _measure_temp_step       = false; // Added by me
  _measure_gofr            = false;
  _measure_magnet          = false;
  _measure_magnet_step     = false; // Added by me
  _measure_cv              = false;
  _measure_chi             = false;
  _measure_pofv            = false; // Added by me
  _measure_loc_energy      = false; // Added by me
  _measure_loc_energy_step = false; // Added by me
  _measure_walker          = false; // Added by me

  ifstream input("../INPUT/properties.dat");
  if (input.is_open()){
    while ( !input.eof() ){
      input >> property;
      if( property == "POTENTIAL_ENERGY" ){
        ofstream coutp("../OUTPUT/potential_energy.dat");
        coutp << "#     BLOCK:          ACTUAL_PE:             PE_AVE:              ERROR:" << endl;
        coutp.close();
        _nprop++;
        _index_penergy = index_property;
        _measure_penergy = true;
        index_property++;
      } else if( property == "POTENTIAL_ENERGY_STEP" ){ // properties to print potential energy at each step of simulation (added by me)
        ofstream coutps("../OUTPUT/potential_energy_step.dat");
        coutps << "#           PE_STEP:" << endl;
        coutps.close();
        // _nprop++; // commented to avoid error of size of measurementes vectors (non interessa al vettore delle medie il fatto che vengano misurate le proprietà istantanee dato che saltano fuori dalle misure per le medie)
        _measure_penergy_step = true;
      } else if( property == "KINETIC_ENERGY" ){
        ofstream coutk("../OUTPUT/kinetic_energy.dat");
        coutk << "#     BLOCK:          ACTUAL_KE:             KE_AVE:              ERROR:" << endl;
        coutk.close();
        _nprop++;
        _measure_kenergy = true;
        _index_kenergy = index_property;
        index_property++;
      } else if( property == "KINETIC_ENERGY_STEP" ){ // properties to print kinetic energy at each step of simulation (added by me)
        ofstream coutks("../OUTPUT/kinetic_energy_step.dat");
        coutks << "#           KE_STEP:" << endl;
        coutks.close();
        // _nprop++; // commented to avoid error of size of measurementes vectors (non interessa al vettore delle medie il fatto che vengano misurate le proprietà istantanee dato che saltano fuori dalle misure per le medie)
        _measure_kenergy_step = true;
      } else if( property == "TOTAL_ENERGY" ){
        ofstream coutt("../OUTPUT/total_energy.dat");
        coutt << "#     BLOCK:          ACTUAL_TE:             TE_AVE:              ERROR:" << endl;
        coutt.close();
        _nprop++;
        _measure_tenergy = true;
        _index_tenergy = index_property;
        index_property++;
      } else if( property == "TOTAL_ENERGY_STEP" ){
        ofstream coutts("../OUTPUT/total_energy_step.dat");
        coutts << "#           TE_STEP:" << endl;
        coutts.close();
        // _nprop++; // commented to avoid error of size of measurementes vectors (non interessa al vettore delle medie il fatto che vengano misurate le proprietà istantanee dato che saltano fuori dalle misure per le medie)
        _measure_tenergy_step = true;
      } else if( property == "TEMPERATURE" ){
        ofstream coutte("../OUTPUT/temperature.dat");
        coutte << "#     BLOCK:           ACTUAL_T:              T_AVE:              ERROR:" << endl;
        coutte.close();
        _nprop++;
        _measure_temp = true;
        _index_temp = index_property;
        index_property++;
      } else if( property == "TEMPERATURE_STEP" ){ // properties to print temperature at each step of simulation (added by me)
        ofstream couttes("../OUTPUT/temperature_step.dat");
        couttes << "#            T_STEP:" << endl;
        couttes.close();
        // _nprop++; // commented to avoid error of size of measurementes vectors (non interessa al vettore delle medie il fatto che vengano misurate le proprietà istantanee dato che saltano fuori dalle misure per le medie)
        _measure_temp_step = true;
      } else if( property == "PRESSURE" ){
        ofstream coutpr("../OUTPUT/pressure.dat");
        coutpr << "#     BLOCK:           ACTUAL_P:              P_AVE:              ERROR:" << endl;
        coutpr.close();
        _nprop++;
        _measure_pressure = true;
        _index_pressure = index_property;
        index_property++;
      } else if( property == "PRESSURE_STEP" ){ // properties to print pressure at each step of simulation (added by me)
        ofstream coutprs("../OUTPUT/pressure_step.dat");
        coutprs << "#            P_STEP:" << endl;
        coutprs.close();
        // _nprop++; // commented to avoid error of size of measurementes vectors (non interessa al vettore delle medie il fatto che vengano misurate le proprietà istantanee dato che saltano fuori dalle misure per le medie)
        _measure_pressure_step = true;
      } else if( property == "GOFR" ){
        if(_sim_type > 1){
          cerr << "ERROR: GOFR measurement is typically incompatible with Ising Model simulations." << endl;
          exit(EXIT_FAILURE);
        }
        ofstream coutgr("../OUTPUT/gofr.dat");
        // coutgr << "# DISTANCE:           AVE_GOFR:              ERROR:" << endl;
        coutgr << "#     BLOCK:         BIN_CENTER:        ACTUAL_GOFR:           GOFR_AVE:              ERROR:" << endl;
        coutgr.close();
        input>>_n_bins;
        _nprop+=_n_bins;
        _bin_size = (_halfside.min() )/(double)_n_bins;
        _gofr_normalization.resize(_n_bins);
        _gofr_increment.resize(_n_bins);
        double dV{2.0 * M_PI * _bin_size * _bin_size * _bin_size}; 
        double a{dV* _npart * _rho};
        for(int i{}; i < _n_bins; i++){
          _gofr_normalization(i) = i*i + i + 1.0/3.0;
          _gofr_increment(i) = 1.0/_gofr_normalization(i);
        }
        _gofr_increment /= a; 
        _measure_gofr = true;
        _index_gofr = index_property;
        index_property+= _n_bins;
      } else if( property == "POFV" ){ // FOR EXERCISE 4
        if(_sim_type > 0){
          cerr << "ERROR: POFV measurement is typically incompatible with Monte Carlo (MC) simulations." << endl;
          exit(EXIT_FAILURE);
        }
        ofstream coutpv("../OUTPUT/pofv.dat");
        coutpv << "#     BLOCK:         BIN_CENTER:           ACTUAL_V:              V_AVE:              ERROR:" << endl;
        coutpv.close();
        input>>_n_bins_v;
        _nprop += _n_bins_v;
        _bin_size_v = 3.5*sqrt(3.0*_temp)/(double)_n_bins_v; // Max sampling velocity is set to 3.5 times the mean velocity
        _pofv_normalization = _npart * _bin_size_v; // Normalization constant
        _pofv_increment = 1.0/_pofv_normalization; // Increment normalized to avoid multiple divisions
        _measure_pofv = true;
        _index_pofv = index_property;
        index_property += _n_bins_v;

        // File to create start simulation velocity distribution histogram only for Dirac
        if( _dist_type == 1 ){
          ofstream coutpvs;
          coutpvs.open("../OUTPUT/initial_velocity_distribution.dat");
          coutpvs << "#      V_BIN_CENTER:             COUNTS:" << endl;
          coutpvs.close();
        // _nprop++; // commented to avoid error of size of measurementes vectors (non interessa al vettore delle medie il fatto che vengano misurate le proprietà istantanee dato che saltano fuori dalle misure per le medie)
        }
      } else if( property == "MAGNETIZATION" ){
        ofstream coutm("../OUTPUT/magnetization.dat");
        coutm << "#     BLOCK:           ACTUAL_M:              M_AVE:              ERROR:" << endl;
        coutm.close();
        _nprop++;
        _measure_magnet = true;
        _index_magnet = index_property;
        index_property++;
      } else if( property == "MAGNETIZATION_STEP" ){
        ofstream coutms("../OUTPUT/magnetization_step.dat");
        coutms << "#            M_STEP:" << endl;
        coutms.close();
        // _nprop++; // commented to avoid error of size of measurementes vectors (non interessa al vettore delle medie il fatto che vengano misurate le proprietà istantanee dato che saltano fuori dalle misure per le medie)
        _measure_magnet_step = true;
      } else if( property == "SPECIFIC_HEAT" ){
        ofstream coutcv("../OUTPUT/specific_heat.dat");
        coutcv << "#     BLOCK:          ACTUAL_CV:             CV_AVE:              ERROR:" << endl;
        coutcv.close();
        _nprop++;
        _measure_cv = true;
        _index_cv = index_property;
        index_property++;
        _nprop++;
        _index_H2 = index_property;
        index_property++;
      } else if( property == "SUSCEPTIBILITY" ){
        ofstream coutchi("../OUTPUT/susceptibility.dat");
        coutchi << "#     BLOCK:         ACTUAL_CHI:            CHI_AVE:              ERROR:" << endl;
        coutchi.close();
        _nprop++;
        _measure_chi = true;
        _index_chi = index_property;
        index_property++;
      } else if( property == "LOCAL_ENERGY"){
        if(_print_vmc){
          ofstream coutle("../OUTPUT/hamiltonian.dat");
          coutle << "#     BLOCK:           ACTUAL_H:              H_AVE:              ERROR:" << endl;
          coutle.close();
        }
        _nprop++;
        _measure_loc_energy = true;
        _index_loc_energy = index_property;
        index_property++;
      } else if( property == "LOCAL_ENERGY_STEP" ){
        ofstream coutles("../OUTPUT/hamiltonian_step.dat");
        coutles << "#            H_STEP:" << endl;
        coutles.close();
        // _nprop++; // commented to avoid error of size of measurementes vectors (non interessa al vettore delle medie il fatto che vengano misurate le proprietà istantanee dato che saltano fuori dalle misure per le medie)
        _measure_loc_energy_step = true;
      } else if( property == "WALKER_POSITION" ){
        ofstream coutwp("../OUTPUT/walker.dat");
        coutwp << "#                 X:                  Y:                  Z:" << endl;
        coutwp.close();
        // _nprop++; // commented to avoid error of size of measurementes vectors (non interessa al vettore delle medie il fatto che vengano misurate le proprietà istantanee dato che saltano fuori dalle misure per le medie)
        _measure_walker = true;
      } else if( property == "NO_PRINT_VMC" ){
        _print_vmc = false;
      } else if( property == "NO_PRINT_ACC" ){
        _print_acceptance = false;
      } else if( property == "GS"){
        if(_sim_type != 4){
          cerr << "ERROR: GS wave function sampling is incompatible with ensemble simulations." << endl;
          exit(EXIT_FAILURE);
        }
        ofstream coutgs("../OUTPUT/gs.dat");
        coutgs << "#     BLOCK:         BIN_CENTER:           ACTUAL_V:              V_AVE:              ERROR:" << endl;
        coutgs.close();
        input>>_n_bins_gs;
        _nprop += _n_bins_gs;
        _bin_size_gs = 5.0/_n_bins_gs; // 5.0 is approximately the range in which particle is confined (see plot ground state)
        _gs_normalization = _npart * _bin_size_gs; // Normalization constant
        _gs_increment = 1.0/_gs_normalization; // Increment normalized to avoid multiple divisions
        _measure_gs = true;
        _index_gs = index_property;
        index_property += _n_bins_gs;
      } else if( property == "ENDPROPERTIES" ){
        ofstream coutf;
        coutf.open("../OUTPUT/output.dat",ios::app);
        coutf << "Reading properties completed!" << endl;
        coutf.close();
        break;
      } else cerr << "PROBLEM: unknown property" << endl;
    }
    input.close();
  } else {
    cerr << "PROBLEM: Unable to open INPUT file properties.dat" << endl;
    exit(EXIT_FAILURE);
  }

  if(_print_acceptance){
    ofstream couta("../OUTPUT/acceptance.dat"); // Set the heading line in file ../OUTPUT/acceptance.dat
    couta << "#   N_BLOCK:         ACCEPTANCE:" << endl;
    couta.close();
  }
  
  // according to the number of properties, resize the vectors _measurement,_average,_block_av,_global_av,_global_av2
  _measurement.resize(_nprop);
  _average.resize(_nprop);
  _block_av.resize(_nprop);
  _global_av.resize(_nprop);
  _global_av2.resize(_nprop);
  _average.zeros();
  _global_av.zeros();
  _global_av2.zeros();
  _nattempts = 0;
  _naccepted = 0;
  return;
}

void System :: finalize(){
  this->write_configuration();
  // this->write_configuration_parameters(); // Da capire se lasciarla qui o metterla altrove
  _rnd.SaveSeed();
  ofstream coutf;
  coutf.open("../OUTPUT/output.dat",ios::app);
  coutf << "Simulation completed!" << endl;
  coutf.close();
  return;
}

// Write final configurations as .xyz files in the directory ../OUTPUT/CONFIG/
void System :: write_configuration(){
  ofstream coutf;
  coutf.open("../OUTPUT/CONFIG/config.xyz");
  if(!coutf.is_open()){
    cerr << "write_configuration() PROBLEM: Unable to open config.xyz" << endl;
  } else{
    if(_sim_type < 2){
      coutf << _npart << endl;
      coutf << "#Comment!" << endl;
      for(int i=0; i<_npart; i++){
        coutf << "LJ" << "  " 
              << setprecision(17) << _particle(i).getposition(0,true)/_side(0) << "   " // x
              << setprecision(17) << _particle(i).getposition(1,true)/_side(1) << "   " // y
              << setprecision(17) << _particle(i).getposition(2,true)/_side(2) << endl; // z
      }
    } else if(_sim_type == 4){
      coutf << _npart << endl;
      coutf << "#Comment!" << endl;
      for(int i=0; i<_npart; i++){
        coutf << "VMC" << "  " 
              << setprecision(17) << _particle(i).getposition(0,true) << "   " // x
              << setprecision(17) << _particle(i).getposition(1,true) << "   " // y
              << setprecision(17) << _particle(i).getposition(2,true) << endl; // z
      }
    }
  }
  coutf.close();
  coutf.open("../OUTPUT/CONFIG/conf-1.xyz");
  if(!coutf.is_open()){
    cerr << "write_configuration() PROBLEM: Unable to open conf-1.xyz" << endl;
  } else{
    if(_sim_type < 2){
      coutf << _npart << endl;
      coutf << "#Comment!" << endl;
      for(int i=0; i<_npart; i++){
        coutf << "LJ" << "  " 
              << setprecision(17) << _particle(i).getposition(0,false)/_side(0) << "   " // x
              << setprecision(17) << _particle(i).getposition(1,false)/_side(1) << "   " // y
              << setprecision(17) << _particle(i).getposition(2,false)/_side(2) << endl; // z
      }
    } else if(_sim_type == 4){
      coutf << _npart << endl;
      coutf << "#Comment!" << endl;
      for(int i=0; i<_npart; i++){
        coutf << "VMC" << "  " 
              << setprecision(17) << _particle(i).getposition(0,false) << "   " // x
              << setprecision(17) << _particle(i).getposition(1,false) << "   " // y
              << setprecision(17) << _particle(i).getposition(2,false) << endl; // z
      }
    }
  }
  coutf.close();
  if(_sim_type == 2 or _sim_type == 3) {
    coutf.open("../OUTPUT/CONFIG/config.spin");
    for(int i=0; i<_npart; i++) coutf << _particle(i).getspin() << " ";
    coutf.close();
  }
  return;
}

// Write configuration nconf as a .xyz file in directory ../OUTPUT/CONFIG/
void System :: write_XYZ(int nconf){
  ofstream coutf;
  coutf.open("../OUTPUT/CONFIG/config_" + to_string(nconf) + ".xyz");
  if(coutf.is_open()){
    coutf << _npart << endl;
    coutf << "#Comment!" << endl;
    for(int i=0; i<_npart; i++){
      coutf << "LJ" << "  " 
            << setw(16) << _particle(i).getposition(0,true)          // x
            << setw(16) << _particle(i).getposition(1,true)          // y
            << setw(16) << _particle(i).getposition(2,true) << endl; // z
    }
  } else cerr << "PROBLEM: Unable to open config.xyz" << endl;
  coutf.close();
  return;
}

// Read configuration from a .xyz file in directory ../OUTPUT/CONFIG/
// Modified by me (added scenario with Dirac's Delta initial velocities configuration)
void System :: read_configuration(){
  ifstream cinf;
  cinf.open("../INPUT/CONFIG/config.xyz");
  if(cinf.is_open()){
    string comment;
    string particle;
    double x, y, z;
    int ncoord;
    cinf >> ncoord;
    if (ncoord != _npart){
      cerr << "PROBLEM: conflicting number of coordinates in input.dat & config.xyz not match!" << endl;
      exit(EXIT_FAILURE);
    }
    cinf >> comment;
    if(_dist_type == 1){ // Dirac velocity distribution has to start from a fcc crystal of half size
      for(int i=0; i<_npart; i++){
        cinf >> particle >> x >> y >> z; // units of coordinates in conf.xyz is _side
        _particle(i).setposition(0, this->pbc(0.5*_side(0)*x, 0));
        _particle(i).setposition(1, this->pbc(0.5*_side(1)*y, 1));
        _particle(i).setposition(2, this->pbc(0.5*_side(2)*z, 2));
        _particle(i).acceptmove(); // _x_old = _x_new
      }
    } else {
      for(int i=0; i<_npart; i++){
        cinf >> particle >> x >> y >> z; // units of coordinates in conf.xyz is _side
        _particle(i).setposition(0, x);  // _side(0)*x since config.xyz is not written in reduced units
        _particle(i).setposition(1, y);  // _side(1)*y since config.xyz is not written in reduced units
        _particle(i).setposition(2, z);  // _side(2)*z since config.xyz is not written in reduced units
        _particle(i).acceptmove(); // _x_old = _x_new
      }
    }
  } else {
    cerr << "PROBLEM: Unable to open INPUT file config.xyz"<< endl;
    exit(EXIT_FAILURE);
  }
  cinf.close();
  if(_restart and _sim_type > 1){
    int spin;
    cinf.open("../INPUT/CONFIG/config.spin");
    for(int i=0; i<_npart; i++){
      cinf >> spin;
      _particle(i).setspin(spin);
    }
    cinf.close();
  }
  return;
}

void System :: block_reset(int blk){ // Reset block accumulators to zero
  ofstream coutf;
  if(blk>0 and _print_acceptance){
    coutf.open("../OUTPUT/output.dat",ios::app);
    coutf << "Block completed: " << blk << endl;
    coutf.close();
  }
  _block_av.zeros();
  return;
}

void System :: measure(){ // Measure properties
  _measurement.zeros();
  // POTENTIAL ENERGY, VIRIAL, GOFR ///////////////////////////////////////////
  int bin;
  vec distance;
  distance.resize(_ndim);
  double penergy_temp=0.0, dr; // temporary accumulator for potential energy
  double kenergy_temp=0.0; // temporary accumulator for kinetic energy
  double tenergy_temp=0.0;
  double temp_temp = 0.0; 
  double magnetization=0.0; // temporary accumulator for magnetization
  double chi=0.0; // temporary accumulator for susceptibility
  double virial=0.0;
  if (_measure_penergy or _measure_pressure or _measure_gofr) {
    for (int i=0; i<_npart-1; i++){
      for (int j=i+1; j<_npart; j++){
        distance(0) = this->pbc( _particle(i).getposition(0,true) - _particle(j).getposition(0,true), 0);
        distance(1) = this->pbc( _particle(i).getposition(1,true) - _particle(j).getposition(1,true), 1);
        distance(2) = this->pbc( _particle(i).getposition(2,true) - _particle(j).getposition(2,true), 2);
        dr = sqrt( dot(distance,distance) );
        // GOFR added by me
        if(_measure_gofr){
          int index_r{int(dr/_bin_size)};
          if(index_r < _n_bins){
            _measurement(_index_gofr + index_r) += _gofr_increment(index_r);
          }
        }
        if(dr < _r_cut){
          if(_measure_penergy)  penergy_temp += 1.0/pow(dr,12) - 1.0/pow(dr,6); // SYSTEM POTENTIAL ENERGY
          if(_measure_pressure) virial       += 1.0/pow(dr,12) - 0.5/pow(dr,6); // SYSTEM PRESSURE
        }
      }
    }
  }

  // POFV
  // Added by me
  if (_measure_pofv) {
    vec velocity;
    velocity.resize(_ndim);
    velocity.zeros();
    double vel{0.0};
    int vel_bin_index{0};
    for(int i{}; i < _npart; i++){

      // Get particle velocity
      velocity = _particle(i).getvelocity();

      // Computes velocity magnitude
      vel = sqrt(dot(velocity, velocity));

      // Computes bin index to increase counts per bin 
      vel_bin_index = int(vel/_bin_size_v);

      // Check if index bin related to the velocity falls out of range
      if(vel_bin_index < _n_bins_v ){
        _measurement(_index_pofv + vel_bin_index)+= _pofv_increment;
      }
    }
  }

  // POTENTIAL ENERGY PER PARTICLE //////////////////////////////////////////////////////////
  if (_measure_penergy){
    penergy_temp = _vtail + 4.0 * penergy_temp / double(_npart);
    _measurement(_index_penergy) = penergy_temp;
  }

  // POTENTIAL ENERGY PER PARTICLE PER STEP //////////////////////////////////////////////////////////
  // This method prints at each step penergy when property "POTENTIAL_ENERGY_STEP" is placed in properties.dat
  // Added by me
  if (_measure_penergy_step){
    ofstream coutps("../OUTPUT/potential_energy_step.dat", ios::app);
    coutps << setw(20) << _measurement[_index_penergy] << endl;
    coutps.close();
  }

  // KINETIC ENERGY PER PARTICLE ////////////////////////////////////////////////////////////
  if (_measure_kenergy){
    if(_sim_type == 0){ // LJ NVE uses Verlet algorithm
      for (int i=0; i<_npart; i++) kenergy_temp += 0.5 * dot( _particle(i).getvelocity() , _particle(i).getvelocity() );
      kenergy_temp /= double(_npart); 
    } else if(_sim_type == 1){
      kenergy_temp = (3.0/2.0) * _temp; // Energy equipartition theorem: kin_tot = 3.0/2.0 * npart * k_B * T
    }
    _measurement(_index_kenergy) = kenergy_temp;
  }

  // KINETIC ENERGY PER PARTICLE PER STEP //////////////////////////////////////////////////////////
  // This method prints at each step penergy when property "POTENTIAL_ENERGY_STEP" is placed in properties.dat
  // Added by me
  if (_measure_kenergy_step){
    ofstream coutks("../OUTPUT/kinetic_energy_step.dat", ios::app);
    coutks << setw(20) << _measurement(_index_kenergy) << endl;
    coutks.close();
  }

  // TOTAL ENERGY (kinetic+potential) PER PARTICLE //////////////////////////////////////////
  if (_measure_tenergy){
    if (_sim_type < 2) _measurement(_index_tenergy) = kenergy_temp + penergy_temp;
    else { 
      double s_i, s_j;
      for (int i=0; i<_npart; i++){
        s_i = double(_particle(i).getspin());
        s_j = double(_particle(this->pbc(i+1)).getspin());
        tenergy_temp += - _J * s_i * s_j - 0.5 * _H * (s_i + s_j);
      }
      tenergy_temp /= double(_npart);
      _measurement(_index_tenergy) = tenergy_temp;
      // SPECIFIC HEAT PER PARTICLE /////////////////////////////////////////////////////////////
      // Added by me
      if (_measure_cv){
        _measurement(_index_H2) = tenergy_temp*tenergy_temp; 
      }
    }
  }

  // TOTAL ENERGY PER PARTICLE PER STEP //////////////////////////////////////////
  // This method prints at each step tenergy when property "TOTAL_ENERGY_STEP" is placed in properties.dat
  // Added by me
  if (_measure_tenergy_step){ // added by me
    ofstream coutts("../OUTPUT/total_energy_step.dat", ios::app);
    coutts << setw(20) << _measurement(_index_tenergy) << endl;
    coutts.close();
  } 

  // TEMPERATURE PER PARTICLE ///////////////////////////////////////////////////////////////
  if (_measure_temp and _measure_kenergy){
    if(_sim_type == 0){ // NVE ensemble temperature is computed from kinetik energy
      temp_temp = (2.0/3.0) * kenergy_temp;
    } else if(_sim_type == 1){  // NVT ensemble temperature is fixed by the reservoir
      temp_temp = _temp;
    }
    _measurement(_index_temp) = temp_temp;
  }

  // TEMPERATURE PER PARTICLE PER STEP ///////////////////////////////////////////////////////////////
  // Prints temperature at each step of simulation
  // Added by me
  if (_measure_temp_step){ 
    ofstream couttes("../OUTPUT/temperature_step.dat", ios::app);
    couttes << setw(20) << _measurement(_index_temp) << endl;
    couttes.close();
  }

  // PRESSURE PER PARTICLE //////////////////////////////////////////////////////////////////
  // Added if evaluation using energy equipartition theorem to compute kinetic energy for NVT simulations
  if (_measure_pressure){
    if(_sim_type == 0){
      _measurement(_index_pressure) = _rho * (2.0/3.0) * kenergy_temp + (_ptail*_npart + 48.0*virial/3.0)/_volume;
    }else if(_sim_type == 1){
      _measurement(_index_pressure) = _rho * _temp + (_ptail*_npart + 48.0*virial/3.0)/_volume;
    }
  }

  // PRESSURE PER PARTICLE PER STEP ///////////////////////////////////////////////////////////////
  // Prints pressure at each step of simulation
  // Added by me
  if (_measure_pressure_step){ 
    ofstream coutprs("../OUTPUT/pressure_step.dat", ios::app);
    coutprs << setw(20) << _measurement(_index_pressure) << endl;
    coutprs.close();
  }

  // MAGNETIZATION /////////////////////////////////////////////////////////////
  // Added by me
  if (_measure_magnet){
    for(int i {}; i < _npart; i++){
      magnetization += _particle(i).getspin();
    }
    _measurement(_index_magnet) = double(magnetization)/_npart;
  }

  // MAGNETIZATION PER STEP //////////////////////////////////////////
  // This method prints at each step magnetization when property "MAGNETIZATION_STEP" is placed in properties.dat
  // Added by me
  if (_measure_magnet_step){
    ofstream coutms("../OUTPUT/magnetization_step.dat", ios::app);
    coutms << setw(20) << _measurement(_index_magnet) << endl;
    coutms.close();
  }

  // SUSCEPTIBILITY ////////////////////////////////////////////////////////////
  // TO BE FIXED IN EXERCISE 6
  if (_measure_chi){
    for(int i{}; i < _npart; i++){
      chi += _particle(i).getspin();
    }
    // suceptibility is mean value of magnetization squared times beta
    _measurement(_index_chi) = double(_beta*chi*chi)/_npart;
  }

  // LOCAL ENERGY /////////////////////////////////////////////////////////////
  // Added by me
  if (_measure_loc_energy){
    double x = _particle(0).getposition(0, true);
    double x1 = x - _parameters(0);
    double x2 = x + _parameters(0);
    double d = 1.0/(_parameters(1)*_parameters(1));
    double y1 = 0.5 * d * x1 * x1;
    double y2 = 0.5 * d * x2 * x2;
    double e1 = exp((-1.0)*y1);
    double e2 = exp((-1.0)*y2);
    double h_loc = d * (0.5 - (y1*e1 + y2*e2)/(e1+e2));
    double v = x*x*x*x - 2.5*x*x; // V(x) = x^4 - (5x^2)/2
    _measurement(_index_loc_energy) = h_loc + v;
    // _measurement(_index_loc_energy) = local_energy(x);
  }

  // LOCAL ENERGY PER STEP //////////////////////////////////////////
  // Added by me
  if (_measure_loc_energy_step){
    ofstream coutles("../OUTPUT/hamiltonian_step.dat", ios::app);
    coutles << setw(20) << _measurement(_index_loc_energy) << endl;
    coutles.close();
  }

  // WALKER POSITION //////////////////////////////////////////
  // Added by me
  if (_measure_walker){
    ofstream coutwp("../OUTPUT/walker.dat", ios::app);
    coutwp << setw(20) << _particle(0).getposition(0,true)
           << setw(20) << _particle(0).getposition(1,true) 
           << setw(20) << _particle(0).getposition(2,true) << endl;
    coutwp.close();
  }

  // GROUND STATE WAVE FUNCTION 
  // Added by me
  if(_measure_gs){
    double x = _particle(0).getposition(0,true);
    int bin = int((x + 2.5)/_bin_size_gs); // +2.5 since index = 0 means [-2.5;-2.5+size]
    if(bin >= 0 and bin < _n_bins_gs){
      _measurement(_index_gs + bin) += _gs_increment;
    }
  }

  _block_av += _measurement; //Update block accumulators

  return;
}

void System :: averages(int blk){

  ofstream coutf;
  double average, sum_average, sum_ave2;

  _average     = _block_av / double(_nsteps); // Computes block measurement average dividing by the step made per block
  _global_av  += _average;
  _global_av2 += _average % _average; // % -> element-wise multiplication

  // POTENTIAL ENERGY //////////////////////////////////////////////////////////
  if (_measure_penergy){
    coutf.open("../OUTPUT/potential_energy.dat",ios::app);
    average  = _average(_index_penergy);
    sum_average = _global_av(_index_penergy);
    sum_ave2 = _global_av2(_index_penergy);
    coutf << setw(12) << blk 
          << setw(20) << average  // Average current block 
          << setw(20) << sum_average/double(blk)  // Progressive average over blocks
          << setw(20) << this->error(sum_average, sum_ave2, blk) << endl; // Progressive error over blocks
    coutf.close();
  }

  // KINETIC ENERGY ////////////////////////////////////////////////////////////
  if (_measure_kenergy){
    coutf.open("../OUTPUT/kinetic_energy.dat",ios::app);
    average  = _average(_index_kenergy);
    sum_average = _global_av(_index_kenergy);
    sum_ave2 = _global_av2(_index_kenergy);
    coutf << setw(12) << blk
          << setw(20) << average  // Average current block
          << setw(20) << sum_average/double(blk)  // Progressive average over blocks
          << setw(20) << this->error(sum_average, sum_ave2, blk) << endl; // Progressive error over blocks
    coutf.close();
  }

  // TOTAL ENERGY //////////////////////////////////////////////////////////////
  if (_measure_tenergy){
    coutf.open("../OUTPUT/total_energy.dat",ios::app);
    average  = _average(_index_tenergy);
    sum_average = _global_av(_index_tenergy);
    sum_ave2 = _global_av2(_index_tenergy);
    coutf << setw(12) << blk
          << setw(20) << average  // Average current block
          << setw(20) << sum_average/double(blk)  // Progressive average over blocks
          << setw(20) << this->error(sum_average, sum_ave2, blk) << endl; // Progressive error over blocks
    coutf.close();
  }

  // TEMPERATURE ///////////////////////////////////////////////////////////////
  if (_measure_temp){
    coutf.open("../OUTPUT/temperature.dat",ios::app);
    average  = _average(_index_temp);
    sum_average = _global_av(_index_temp);
    sum_ave2 = _global_av2(_index_temp);
    coutf << setw(12) << blk
          << setw(20) << average  // Average current block
          << setw(20) << sum_average/double(blk)  // Progressive average over blocks
          << setw(20) << this->error(sum_average, sum_ave2, blk) << endl; // Progressive error over blocks
    coutf.close();
  }

  // PRESSURE //////////////////////////////////////////////////////////////////
  if (_measure_pressure){
    coutf.open("../OUTPUT/pressure.dat",ios::app);
    average  = _average(_index_pressure);
    sum_average = _global_av(_index_pressure);
    sum_ave2 = _global_av2(_index_pressure);
    coutf << setw(12) << blk
          << setw(20) << average  // Average current block
          << setw(20) << sum_average/double(blk)  // Progressive average over blocks
          << setw(20) << this->error(sum_average, sum_ave2, blk) << endl; // Progressive error over blocks
    coutf.close();
  }

  // GOFR //////////////////////////////////////////////////////////////////////
  // Added by me
  if (_measure_gofr && blk == _nblocks){
    coutf.open("../OUTPUT/gofr.dat",ios::app);
    for(int i{}; i < _n_bins; i++){
      average = _average(_index_gofr + i);
      sum_average = _global_av(_index_gofr + i);
      sum_ave2 = _global_av2(_index_gofr + i);
      coutf << setw(12) << blk
            << setw(20) << _bin_size * 0.5 + (i * _bin_size)  // Bin center
            << setw(20) << average  // Average current block
            << setw(20) << sum_average/double(blk)  // Progressive average over blocks
            << setw(20) << this->error(sum_average, sum_ave2, blk) << endl; // Progressive error over blocks
    }
    coutf << endl;
    coutf << endl;
    coutf.close();
  }  

  // POFV //////////////////////////////////////////////////////////////////////
  // Added by me
  if (_measure_pofv){
    coutf.open("../OUTPUT/pofv.dat",ios::app);
    for(int i{}; i < _n_bins_v; i++){
      average = _average(_index_pofv + i);
      sum_average = _global_av(_index_pofv + i);
      sum_ave2 = _global_av2(_index_pofv + i);
      coutf << setw(12) << blk
            << setw(20) << _bin_size_v * 0.5 + (i * _bin_size_v)  // Bin center
            << setw(20) << average  // Average current block
            << setw(20) << sum_average/double(blk)  // Progressive average over blocks
            << setw(20) << this->error(sum_average, sum_ave2, blk) << endl; // Progressive error over blocks
    }
    coutf << endl;
    //coutf << endl;
    coutf.close();
  }

  // MAGNETIZATION /////////////////////////////////////////////////////////////
  // Added by me
  if (_measure_magnet){
    coutf.open("../OUTPUT/magnetization.dat",ios::app);
    average  = _average(_index_magnet);
    sum_average = _global_av(_index_magnet);
    sum_ave2 = _global_av2(_index_magnet);
    coutf << setw(12) << blk
          << setw(20) << average  // Average current block
          << setw(20) << sum_average/double(blk)  // Progressive average over blocks
          << setw(20) << this->error(sum_average, sum_ave2, blk) << endl; // Progressive error over blocks
    coutf.close();
  }
  
  // SPECIFIC HEAT /////////////////////////////////////////////////////////////
  // Added by me
  if (_measure_cv){
    coutf.open("../OUTPUT/specific_heat.dat",ios::app);
    double H{_average(_index_tenergy)}; // Hamiltonian block average <H> per particle
    double H2{_average(_index_H2)}; // Hamiltonian squared block average <H^2> per particle
    average = _npart* _beta * _beta * (H2 - H*H); // Specific heat block average per particle
    _average(_index_cv) = average;  // Setting _average vector at _index_cv to cv block average
    _global_av(_index_cv) += average; // Accumulator for specific heat
    _global_av2(_index_cv) += average*average; // Accumulator for specific heat
    sum_average = _global_av(_index_cv); // This could be avoided since I could use directly _global_av(_index_cv) to print and compute error
    sum_ave2 = _global_av2(_index_cv);  // This could be avoided since I could use directly _global_av2(_index_cv) compute error
    coutf << setw(12) << blk
          << setw(20) << average  // Average current block
          << setw(20) << sum_average/double(blk)  // Progressive average over blocks
          << setw(20) << this->error(sum_average, sum_ave2, blk) << endl; // Progressive error over blocks
    coutf.close();
  }

  // SUSCEPTIBILITY ////////////////////////////////////////////////////////////
  // Added by me
  if (_measure_chi){
    coutf.open("../OUTPUT/susceptibility.dat",ios::app);
    average  = _average(_index_chi);
    sum_average = _global_av(_index_chi);
    sum_ave2 = _global_av2(_index_chi);
    coutf << setw(12) << blk
          << setw(20) << average  // Average current block
          << setw(20) << sum_average/double(blk)  // Progressive average over blocks
          << setw(20) << this->error(sum_average, sum_ave2, blk) << endl; // Progressive error over blocks
    coutf.close();
  }

  // LOCAL ENERGY ////////////////////////////////////////////////////////////
  // Added by me
  if (_measure_loc_energy){
    average  = _average(_index_loc_energy);
    sum_average = _global_av(_index_loc_energy);
    sum_ave2 = _global_av2(_index_loc_energy);
    if(_print_vmc){
      coutf.open("../OUTPUT/hamiltonian.dat",ios::app);
      coutf << setw(12) << blk
            << setw(20) << average  // Average current block
            << setw(20) << sum_average/double(blk)  // Progressive average over blocks
            << setw(20) << this->error(sum_average, sum_ave2, blk) << endl; // Progressive error over blocks
      coutf.close();
    }
  }

  // GROUND STATE
  // Added by me
  if (_measure_gs && blk == _nblocks){
    coutf.open("../OUTPUT/gs.dat",ios::app);
    for(int i{}; i < _n_bins_gs; i++){
      average = _average(_index_gs + i);
      sum_average = _global_av(_index_gs + i);
      sum_ave2 = _global_av2(_index_gs + i);
      coutf << setw(12) << blk
            << setw(20) << -2.5 + ((i+0.5) * _bin_size_gs)  // Bin center
            << setw(20) << average  // Average current block
            << setw(20) << sum_average/double(blk)  // Progressive average over blocks
            << setw(20) << this->error(sum_average, sum_ave2, blk) << endl; // Progressive error over blocks
    }
    coutf << endl;
    coutf << endl;
    coutf.close();
  } 

  // ACCEPTANCE ////////////////////////////////////////////////////////////////
  double fraction;
  if(_print_acceptance){
    coutf.open("../OUTPUT/acceptance.dat",ios::app);
    if(_nattempts > 0) fraction = double(_naccepted)/double(_nattempts);
    else fraction = 0.0; 
    coutf << setw(12) << blk << setw(20) << fraction << endl;
    coutf.close();
  }
  return;
}

double System :: error(double acc, double acc2, int blk){
  if(blk <= 1) return 0.0;
  else return sqrt( fabs(acc2/double(blk) - pow( acc/double(blk) ,2) )/double(blk) );
}

int System :: get_nbl(){
  return _nblocks;
}

int System :: get_nsteps(){
  return _nsteps;
}

// Added by me
void System :: print_velocity_distribution(){
  if(_dist_type == 1){
  ofstream coutf;
  coutf.open("../OUTPUT/initial_velocity_distribution.dat", ios::app);
  for(int k{}; k < _n_bins_v; k++){
    coutf << setw(20) << _bin_size_v * 0.5 + k * _bin_size_v
          << setw(20) << _measurement(_index_pofv + k) << endl;
  }
  coutf.close();
  return;
  }
}

// Added by me
void System :: vmc(){

  double h = 0.0;
  double h2 = 0.0;
  double err = 0.0;
  double last_walk_pos = 0.0;

  // Equilibration routine done every time we change parameter values or temperature
  for(int i{}; i < _eq_steps; i++){
    step();
  }
  // Reset all variable
  _block_av.zeros();
  _average.zeros();
  _global_av.zeros();
  _global_av2.zeros();

  // VMC routine
  for(int i{}; i < _nblocks; i++){
    for(int j{}; j < _nsteps; j++){
      step();
      measure();
    }
    averages(i+1);
    block_reset(i+1);
  }

  h =  _global_av(_index_loc_energy)/_nblocks;
  h2 =  _global_av2(_index_loc_energy)/_nblocks;
  err = this->error(h, h2, _nblocks);
  last_walk_pos = _particle(0).getposition(0, true);

  // Saving VMC value computed
  _final_h = h;
  _final_err = err;
  _final_x = last_walk_pos; 

  return;

}

// Added by me
// Routine performing _nsteps_SA at fixed temperature
void System :: SA_fixed_temp_step(){

  _naccepted_SA = 0;
  
  vmc();
  double h_current = _final_h;
  double err_current = _final_err;
  double x_current = _final_x;
  _parameters_current = _parameters;

  for(int i{}; i < _nsteps_SA; i++){
    // print_data_step(i);
    for(int l{}; l < _nparam; l++){
      _parameters(l) = _parameters_current(l) + _rnd.Rannyu(-1.0, 1.0)*_delta_parameters(l);
      if(_parameters(1) <= 1e-6) _parameters(1) = 1e-6;
    }

    _particle(0).setposition(0, x_current);
    vmc();

    double h_trial = _final_h;
    double err_trial = _final_err;
    double x_trial = _final_x;

    double delta_h = h_trial - h_current;
    double acceptance = exp((-1.0)*_beta*delta_h);
    if(_rnd.Rannyu(0.0, 1.0) < acceptance){
      _h_best = h_trial;
      _err_best = err_trial;
      x_current = x_trial;
      _x_best = x_trial;
      _parameters_current = _parameters;
      _naccepted_SA++;
    }else{
      _h_best = h_current;
      _err_best = err_current;
      // _parameters = _parameters_current;
      _x_best = x_current;
    }
  }
  // print_data_step(_nsteps_SA);

}

// Added by me
void System :: SA(){
  header_data();
  for(int i{}; i < _n_temp; i++){
    SA_fixed_temp_step();
    print_data();
    // _temp -= _delta_temp;
    _temp *= _delta_temp;
    _beta = 1.0/_temp;
  }
  finalize();
}

// Added by me
void System :: print_data_step(int step){
  ofstream coutf;
  if(step == 0){
    coutf.open("../OUTPUT/data_" + to_string(_temp) + ".dat");
    if(coutf.is_open()){
      coutf << "#       TEMPERATURE:            SA_STEP:                 MU:              SIGMA:             AVE_LE:              ERROR:      LAST_X_WALKER:" << endl;
      coutf << setw(20) << _temp
            << setw(20) << step
            << setw(20) << _parameters(0)
            << setw(20) << _parameters(1)
            << setw(20) << _h_best
            << setw(20) << _err_best
            << setw(20) << _x_best << endl;
      
    } else cerr << "PROBLEM: Unable to open ../OUTPUT/data_" + to_string(_temp) + ".dat" << endl;
  }else{
    coutf.open("../OUTPUT/data_" + to_string(_temp) + ".dat", ios::app);
    if(coutf.is_open()){
      coutf << setw(20) << _temp
            << setw(20) << step
            << setw(20) << _parameters(0)
            << setw(20) << _parameters(1)
            << setw(20) << _h_best
            << setw(20) << _err_best
            << setw(20) << _x_best << endl;
      
    } else cerr << "PROBLEM: Unable to open ../OUTPUT/data_" + to_string(_temp) + ".dat" << endl;
  }
  coutf.close();
  return;
}

// Added by me
void System :: header_data(){
  ofstream coutf;
  coutf.open("../OUTPUT/data.dat");
  if(coutf.is_open()){
    coutf << "#       TEMPERATURE:                 MU:              SIGMA:             AVE_LE:              ERROR:         ACCEPTANCE:      LAST_X_WALKER:" << endl;
  } else cerr << "PROBLEM: Unable to open ../OUTPUT/data.dat" << endl;
  coutf.close();
  return;
}

// Added by me
void System :: print_data(){
  ofstream coutf;
  coutf.open("../OUTPUT/data.dat", ios::app);
  if(coutf.is_open()){
    coutf << setw(20) << _temp
          << setw(20) << _parameters(0)
          << setw(20) << _parameters(1)
          << setw(20) << _h_best
          << setw(20) << _err_best
          << setw(20) << double(_naccepted_SA)/_nsteps_SA
          << setw(20) << _x_best << endl;
    
  } else cerr << "PROBLEM: Unable to open ../OUTPUT/data.dat" << endl;
  coutf.close();
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
