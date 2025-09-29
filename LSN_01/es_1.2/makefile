CXXFLAGS = -std=gnu++2b -g -O3 -Wall --pedantic
FMTINCS = -lfmt
AFLAGS = -larmadillo

main.exe : main.o ../../generator/random.o ../../functions.h
	g++ $(FMTINCS) $(AFLAGS) main.o ../../generator/random.o -o $@

main.o : main.cpp ../../functions.h
	g++ main.cpp $(CXXFLAGS) -c -o $@

random.o : ../../generator/random.cpp ../../generator/random.h
	g++ $(CXXFLAGS) -c ../../generator/random.cpp -o $@

remove_o :
	rm main.o

remove_exe :
	rm main.exe

remove_output :
	rm OUTPUT/*.data
