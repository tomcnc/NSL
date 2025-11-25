/****************************************************************
*****************************************************************
    _/    _/  _/_/_/  _/       Numerical Simulation Laboratory
   _/_/  _/ _/       _/       Physics Department
  _/  _/_/    _/    _/       Universita' degli Studi di Milano
 _/    _/       _/ _/       Prof. D.E. Galli
_/    _/  _/_/_/  _/_/_/_/ email: Davide.Galli@unimi.it
*****************************************************************
*****************************************************************/

#pragma once

// This class contains functions for generating random numbers using the RANNYU algorithm
class Random {

private:
  int m1,m2,m3,m4,l1,l2,l3,l4,n1,n2,n3,n4;

protected:

public:
  // Default constructor
  Random();
  // Destructor
  ~Random();
  // Method to set the seed for the RNG
  void SetRandom(int * , int, int);
  // Method to save the seed to a file
  void SaveSeed();
  // Method to sample a random number in the range [0,1)
  double Rannyu(void);
  // Method to sample a random number in the range [min,max)
  double Rannyu(double min, double max);
  // Method to sample a random number with a Gaussian distribution
  double Gauss(double mean, double sigma);

  //added methods

  // Routine to set seed and primes using first row of Primes
  void StartRandom(void);
  // Routine to set seed and primes using row row of Primes
  void StartRandom(int row);
  // Routine to set seed from previous simulation using first row of Primes
  void RestartRandom(void);
  // Routine to set seed from previous simulation using row row of Primes
  void RestartRandom(int row);
  // Method to sample a random number with an exponential distribution
  double Exponential(double lambda);
  // Method to sample a random number with a Lorentzian distribution
  double Lorentzian(double gamma, double centervalue);
  // Method to sample a random angle
  double RanAngle();

};



/****************************************************************
*****************************************************************
    _/    _/  _/_/_/  _/       Numerical Simulation Laboratory
   _/_/  _/ _/       _/       Physics Department
  _/  _/_/    _/    _/       Universita' degli Studi di Milano
 _/    _/       _/ _/       Prof. D.E. Galli
_/    _/  _/_/_/  _/_/_/_/ email: Davide.Galli@unimi.it
*****************************************************************
*****************************************************************/
