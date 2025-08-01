all:
	g++ -c -o vctr.o vctr.cpp -std=c++23
	g++ -c -o global_vars.o global_variables.cpp -std=c++23
	g++ -c -o main.o main.cpp -std=c++23
	g++ -o program.out main.o vctr.o global_vars.o -lraylib -std=c++23
