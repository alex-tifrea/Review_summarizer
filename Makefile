CC = g++
CFLAGS=-Wall -g
SRC = Interogate.cpp IO.cpp main.cpp NgramEntry.cpp Worker.cpp
HEADERS = Interogate.h IO.h NgramEntry.h Worker.h
OBJ = Interogate.o IO.o main.o NgramEntry.o Worker.o
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
