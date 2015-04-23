CC = g++
CFLAGS=-Wall -g -lm -Wno-unused-result -std=c++11 -O3
SRC = InterogateNGRAM.cpp IO.cpp main.cpp NgramEntry.cpp Worker.cpp InterogateCoreNLP.cpp POS.cpp Topics.cpp
HEADERS = InterogateNGRAM.h IO.h NgramEntry.h Worker.h InterogateCoreNLP.h POS.h Topics.h
OBJ = InterogateNGRAM.o IO.o main.o NgramEntry.o Worker.o InterogateCoreNLP.o POS.o Topics.o
EXE = main
NOTIFY=""
ERROR=""

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	NOTIFY=true
	ERROR=true
endif
ifeq ($(UNAME_S),Darwin)
	NOTIFY=say "Boss, I am done"
	ERROR=say "Error Error Error"
endif

build: $(SRC) $(EXE)

$(EXE): $(OBJ)
	$(CC) $(OBJ) -o $(EXE)

%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $<

run: $(EXE)
	(time ./$(EXE) || $(ERROR)) && $(NOTIFY)

.PHONY: clean
clean:
	rm -f *.o $(EXE)
