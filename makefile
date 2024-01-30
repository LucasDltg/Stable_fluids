main: main.o stable_fluid.o
	g++ main.o stable_fluid.o -o main -lSDL2 -lSDL2_ttf

stable_fluid.o: stable_fluid.cpp
	g++ stable_fluid.cpp -c -o stable_fluid.o

main.o: main.cpp
	g++ main.cpp -c -o main.o

clean:
	rm -rf *.o
	rm -f main