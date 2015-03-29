CC = g++
CFLAGS=-Wall -g -lm -std=c++11
SRC = InterogateNGRAM.cpp IO.cpp main.cpp NgramEntry.cpp Worker.cpp InterogateCoreNLP.cpp
HEADERS = InterogateNGRAM.h IO.h NgramEntry.h Worker.h InterogateCoreNLP.h
OBJ = InterogateNGRAM.o IO.o main.o NgramEntry.o Worker.o InterogateCoreNLP.o
EXE = main

build: $(SRC) $(EXE)

$(EXE): $(OBJ)
	$(CC) $(OBJ) -o $(EXE)

%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $<

run: $(EXE)
	time ./$(EXE)

.PHONY: clean
clean:
	rm -f *.o $(EXE)
