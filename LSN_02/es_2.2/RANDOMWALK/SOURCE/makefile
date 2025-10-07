CC = g++
CXXFLAGS = -std=gnu++2b -O3 -g -Wall --pedantic
AFLAGS = -larmadillo

main.exe : main.o random_walk.o random.o
	$(CC) main.o random_walk.o random.o $(AFLAGS) -o $@

main.o : main.cpp random_walk.h
	$(CC) main.cpp $(CXXFLAGS) -c -o $@

random_walk.o : random_walk.cpp random_walk.h random.h
	$(CC) $(CXXFLAGS) -c random_walk.cpp -o $@

random.o : random.cpp random.h
	$(CC) $(CXXFLAGS) -c random.cpp -o $@

remove_o :
	rm *.o

remove_exe :
	rm *.exe

remove_output :
	rm ../OUTPUT/*.*
	rm ../OUTPUT/RW/*.*
	rm ../OUTPUT/SQRT_MEAN_DISTANCE2/*.*
