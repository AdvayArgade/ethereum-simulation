all: program.exe

program.exe: main.o
	g++ main.o -o program.exe

main.o: main.cpp
	g++ -c main.cpp

clean: 
	rm -f *.o *.exe