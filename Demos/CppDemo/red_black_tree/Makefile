all:
	g++ -c red_black_tree.cpp -I .
	ar crv libred_black_tree.a red_black_tree.o
	g++ main.cpp libred_black_tree.a -o main

clean:
	rm libred_black_tree.a red_black_tree.o
