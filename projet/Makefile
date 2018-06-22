CC=g++
FLAGS= -g -Wall -c -std=c++11 -pthread -D_REENTRANT -I.
OBJ= obj/client.o

all:client

bin/client: $(OBJ)
	$(CC) $(OBJ) -o bin/client

obj/main.o: $(OBJ)
	$(CC) $(FLAGS) src/main.cpp -o src/main.cpp



clean:
	rm obj/*.o



