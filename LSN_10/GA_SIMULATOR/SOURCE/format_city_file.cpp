#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>

using namespace std;

int main(){
	
	double x,y;
	int idx = 1;
	ifstream in("../INPUT/italy_prov.dat");
	ofstream out("../INPUT/italy_prov_mod.dat");
	out << "#  CITY_INDEX:                       X:                       Y:" << endl;
	if(!out.is_open()){
                cerr << "ERROR: Not possible to open \"../INPUT/italy_prov_mod.dat\" input file." << endl;
                exit(99);    
        }
	if(!in.is_open()){
		cerr << "ERROR: Not possible to open \"../INPUT/italy_prov.dat\" input file." << endl;
		exit(99);
	}while(!in.eof()){
		in >> x >> y;
		out << setw(14) << idx
		    << setprecision(17) << setw(25) << x
		    << setprecision(17) << setw(25) << y << endl;
		idx++;
	}
	in.close();
	out.close();

	return 0;
}
