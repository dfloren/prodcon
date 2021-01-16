CC=g++
FLAGS=-std=c++11 -Wall -O
LDFLAGS = -lpthread
SRC=prodcon.cpp util.cpp tands.cpp
OBJ=$(SRC:.cpp=.o)

prodcon: $(OBJ)
	$(CC) $(FLAGS) -o prodcon $(OBJ) $(LDFLAGS)
prodcon.o: prodcon.cpp
	$(CC) $(FLAGS) -c prodcon.cpp $(LDFLAGS)
util.o: util.cpp util.h
	$(CC) $(FLAGS) -c util.cpp $(LDFLAGS)
tands.o: tands.cpp
	$(CC) $(FLAGS) -c tands.cpp $(LDFLAGS)

clean:
	@-rm -f *.o prodcon
cleanlogs:
	@-rm *.log
