CC = g++
CFLAGS=-Wall -g -I/usr/include/python2.7
SRC = Interogate.cpp IO.cpp Main.cpp NgramEntry.cpp Worker.cpp
HEADERS = Interogate.h IO.h NgramEntry.h Worker.h
OBJ = Interogate.o IO.o Main.o NgramEntry.o Worker.o
EXE = main

build: $(SRC) $(EXE)

$(EXE): $(OBJ)
	$(CC) $(OBJ) -o $(EXE)

%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $<

run: $(EXE)
	./main

.PHONY: clean
clean:
	rm -f *.o main
