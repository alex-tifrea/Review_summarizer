CC = g++
CFLAGS=-Wall -g -I/usr/include/python2.7
SRC = Interogate.cpp IO.cpp Main.cpp NgramEntry.cpp Worker.cpp
OBJ = Interogate.o IO.o Main.o NgramEntry.o Worker.o
EXE = main

all: $(SRC) $(EXE)

$(EXE): $(OBJ)
	$(CC) $(OBJ) -o $(EXE)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $^

.PHONY: clean
clean:
	rm -f *.o main
