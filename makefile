%: %.cpp
	g++ -o $@ $^ -std=c++17 -pthread -lrt -lstdc++fs
clean:
	rm -f ejercicio2 ejercicio2.o