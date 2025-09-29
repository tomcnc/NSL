CXXFLAGS = -std=gnu++2b -O3 -g -Wall --pedantic
FMTINCL = -lfmt
AFLAGS = -larmadillo

main.exe : main.o ../../generator/random.o
	g++ main.o ../../generator/random.o $(FMTINCL) $(AFLAGS) -o $@

main.o : main.cpp ../../functions.h
	g++ main.cpp $(CXXFLAGS) -c -o $@

random.o : ../../generator/random.cpp ../../generator/random.h
	g++ $(CXXFLAGS) -c ../../generator/random.cpp -o ../../generator/random.o

remove_o :
	rm main.o

remove_exe :
	rm main.exe

remove_output :
	rm OUTPUT/*.data