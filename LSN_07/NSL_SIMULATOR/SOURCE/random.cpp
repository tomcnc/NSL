/****************************************************************
*****************************************************************
    _/    _/  _/_/_/  _/       Numerical Simulation Laboratory
   _/_/  _/ _/       _/       Physics Department
  _/  _/_/    _/    _/       Universita' degli Studi di Milano
 _/    _/       _/ _/       Prof. D.E. Galli
_/    _/  _/_/_/  _/_/_/_/ email: Davide.Galli@unimi.it
*****************************************************************
*****************************************************************/

#include <iostream>
#include <fstream>
#include <cfloat> // for DBL_EPSILON
#include <cmath>
#include <cstdlib>
#include <string>
#include "random.h"

using namespace std;

Random :: Random(){}
// Default constructor, does not perform any action

Random :: ~Random(){}
// Default destructor, does not perform any action

void Random :: SaveSeed(){
   // This function saves the current state of the random number generator to a file "seed.out"
   ofstream WriteSeed;
   WriteSeed.open("../OUTPUT/seed.out");
   if (WriteSeed.is_open()){
      WriteSeed << "RANDOMSEED	" << l1 << " " << l2 << " " << l3 << " " << l4 << endl;;
   } else cerr << "Random :: SaveSeed() PROBLEM: Unable to open random.out" << endl;
  WriteSeed.close();
  return;
}

double Random :: Gauss(double mean, double sigma) {
   // This function generates a random number from a Gaussian distribution with given mean and sigma
   double s=Rannyu();
   double t=Rannyu();
   double x=sqrt(-2.*log(1.-s))*cos(2.*M_PI*t);
   return mean + x * sigma;
}

double Random :: Rannyu(double min, double max){
   // This function generates a random number in the range [min, max)
   return min+(max-min)*Rannyu();
}

double Random :: Rannyu(void){
  // This function generates a random number in the range [0,1)
  const double twom12=0.000244140625;
  int i1,i2,i3,i4;
  double r;

  i1 = l1*m4 + l2*m3 + l3*m2 + l4*m1 + n1;
  i2 = l2*m4 + l3*m3 + l4*m2 + n2;
  i3 = l3*m4 + l4*m3 + n3;
  i4 = l4*m4 + n4;
  l4 = i4%4096;
  i3 = i3 + i4/4096;
  l3 = i3%4096;
  i2 = i2 + i3/4096;
  l2 = i2%4096;
  l1 = (i1 + i2/4096)%4096;
  r=twom12*(l1+twom12*(l2+twom12*(l3+twom12*(l4))));

  return r;
}

void Random :: SetRandom(int * s, int p1, int p2){
  // This function sets the seed and parameters of the random number generator
  m1 = 502;
  m2 = 1521;
  m3 = 4071;
  m4 = 2107;
  l1 = s[0];
  l2 = s[1];
  l3 = s[2];
  l4 = s[3];
  n1 = 0;
  n2 = 0;
  n3 = p1;
  n4 = p2;

  return;
}

//added implementation methods

// This method is setting seed and primes to activate the generator
void Random :: StartRandom(void){
   int seed[4] ;
   int p1, p2 ;
   ifstream Primes("../INPUT/Primes") ;
   if(Primes.is_open()){
      Primes >> p1 >> p2 ;
   }else{
      cerr << "Random :: StartRandom(void) ERROR : Not possible to open Primes file" << endl ;
   }
   // cout << "Primes value used from file Primes : " << p1 << " " << p2 << endl;
   Primes.close() ;

   ifstream in("../INPUT/seed.in") ;
   string property ;
   if(in.is_open()){
      while(!in.eof()){
         in >> property ;
         if(property == "RANDOMSEED"){
            in >> seed[0] >> seed[1] >> seed[2] >> seed[3] ;
            SetRandom(seed, p1, p2) ;
         }
      }
      in.close() ;
   } else {
      cerr << "Random :: StartRandom(void) ERROR : Unable to open seed.in file" << endl ;
   }
   // cout << "Primes value used from file \"seed.in\" : " << seed[0] << " " << seed[1] << " " << seed[2] << " " << seed[3] << endl;
}

// This method is setting seed and primes to activate the generator reading file Primes at row "row"
// first row is linked to index 0, second raw to index 1 and so on
void Random :: StartRandom(int row){
   int seed[4] ;
   int p1, p2 ;
   ifstream Primes("../INPUT/Primes") ;
   if(Primes.is_open()){
      for(int i{}; i < row; i++){
         Primes >> p1 >> p2 ;
      }
      Primes >> p1 >> p2 ;
   }else{
      cerr << "Random :: StartRandom(void) ERROR : Not possible to open Primes file" << endl ;
   }
   Primes.close() ;
   //cout << "Primes value used from file \"Primes\" : " << p1 << " " << p2 << endl;

   ifstream in("../INPUT/seed.in") ;
   string property ;
   if(in.is_open()){
      while(!in.eof()){
         in >> property ;
         if(property == "RANDOMSEED"){
            in >> seed[0] >> seed[1] >> seed[2] >> seed[3] ;
            SetRandom(seed, p1, p2) ;
         }
      }
      in.close() ;
   }else {
      cerr << "Random :: StartRandom(void) ERROR : Unable to open seed.in file" << endl ;
   }
   //cout << "Primes value used from file \"seed.in\" : " << seed[0] << " " << seed[1] << " " << seed[2] << " " << seed[3] << endl;
}

// This method is setting seed and primes to activate the generator from previous sequence using first row of Primes
void Random :: RestartRandom(void){
   int seed[4] ;
   int p1, p2 ;
   ifstream Primes("../INPUT/Primes") ;
   if(Primes.is_open()){
      Primes >> p1 >> p2 ;
   }else{
      cerr << "Random :: StartRandom(void) ERROR : Not possible to open Primes file" << endl ;
   }
   Primes.close() ;
   //cout << "Primes value used from file \"Primes\" : " << p1 << " " << p2 << endl;

   ifstream in("../INPUT/seed.out") ;
   string property ;
   if(in.is_open()){
      while(!in.eof()){
         in >> property ;
         if(property == "RANDOMSEED"){
            in >> seed[0] >> seed[1] >> seed[2] >> seed[3] ;
            SetRandom(seed, p1, p2) ;
         }
      }
      in.close() ;
   } else {
      cerr << "Random :: StartRandom(void) ERROR : Unable to open seed.out file" << endl ;
   }
   //cout << "Primes value used from file \"seed.out\" : " << seed[0] << " " << seed[1] << " " << seed[2] << " " << seed[3] << endl;
}

// This method is setting seed and primes to activate the generator from previous sequence using row "row" of Primes
void Random :: RestartRandom(int row){
   int seed[4] ;
   int p1, p2 ;
   ifstream Primes("../INPUT/Primes") ;
   if(Primes.is_open()){
      for(int i{}; i < row; i++){
         Primes >> p1 >> p2 ;
      }
      Primes >> p1 >> p2 ;
   }else{
      cerr << "Random :: StartRandom(void) ERROR : Not possible to open Primes file" << endl ;
   }
   Primes.close() ;
   //cout << "Primes value used from file \"Primes\" : " << p1 << " " << p2 << endl;

   ifstream in("../INPUT/seed.out") ;
   string property ;
   if(in.is_open()){
      while(!in.eof()){
         in >> property ;
         if(property == "RANDOMSEED"){
            in >> seed[0] >> seed[1] >> seed[2] >> seed[3] ;
            SetRandom(seed, p1, p2) ;
         }
      }
      in.close() ;
   } else {
      cerr << "Random :: StartRandom(void) ERROR : Unable to open seed.out file" << endl ;
   }
   //cout << "Primes value used from file \"seed.out\" : " << seed[0] << " " << seed[1] << " " << seed[2] << " " << seed[3] << endl;
}

// Method to sample an exponential distribution
double Random::Exponential(double lambda){
   if(lambda <= 0){
      cerr << "Random::Exponential(double lambda) ERROR : lambda has to be positive" << endl ;
   }
   return ((-1.) * log(1. - Rannyu())) / lambda ;
}

// Method to sample a Lorentzian distribution
double Random::Lorentzian(double gamma, double center){
   if(gamma <= 0){
      cerr << "Random::Lorentzian(double gamma, double centervalue) ERROR : gamma has to be positive" << endl ;
   }
   return center + gamma * (tan(M_PI * (Rannyu() - 0.5))) ;
}

// function that returns sign of a double
double sign(double x){
   if(x == 0.0) return 0.0;
   else if(x < 0.0) return -1.0;
   else return 1.0;
}

// Method to sample an uniformly distributed angle in [0, 2pi]
double Random::RanAngle(){
   double x, y, mod;
   while(true){
      // sample of a square with side = 2
      x = Rannyu(-1.0, 1.0);
      y = Rannyu(-1.0, 1.0);
      mod = sqrt(x*x + y*y);
      // check if point is inside unit circle and not the (0,0)
      if(mod <= 1.0 && mod != 0){
         // sign(y) samples in [-pi, pi] 
         return sign(y)*acos(x/mod);
      }
   }
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
