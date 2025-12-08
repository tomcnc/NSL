CC = mpic++
CXXFLAGS = -std=gnu++2b -O3 -g -Wall --pedantic
AFLAGS = -larmadillo

all: GA.exe

GA.exe : GA.o population.o chromosome.o random.o
	$(CC) GA.o population.o chromosome.o random.o $(AFLAGS) -o $@

GA.o : GA.cpp population.h
	$(CC) GA.cpp $(CXXFLAGS) -c -o $@

population.o : population.cpp population.h chromosome.h random.h
	$(CC) population.cpp $(CXXFLAGS) -c -o $@

chromosome.o : chromosome.cpp chromosome.h
	$(CC) $(CXXFLAGS) -c chromosome.cpp -o $@

random.o : random.cpp random.h
	$(CC) $(CXXFLAGS) -c random.cpp -o $@

clean_o :
	rm *.o

clean_exe :
	rm *.exe

remove :
	rm ../OUTPUT/BEST_LOSS/*.*
	rm ../OUTPUT/BEST_PATH/*.*
	rm ../OUTPUT/HALF_POP_MEAN_LOSS/*.*
	rm ../OUTPUT/BEST_CITY_DISTRIBUTION/*.*
	rm ../OUTPUT/STARTING_POPULATION/*.*
	rm ../OUTPUT/*.*
