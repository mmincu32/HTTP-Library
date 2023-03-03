all: build run clean

build: client.o requests.o helpers.o buffer.o
	sudo apt-get update -y
	sudo apt-get install -y nlohmann-json-dev
	g++ -o client client.o requests.o helpers.o buffer.o -Wall

client.o: client.cpp
	g++ -c client.cpp

requests.o: requests.cpp
	g++ -c requests.cpp

helpers.o: helpers.cpp
	g++ -c helpers.cpp

buffer.o: buffer.cpp
	g++ -c buffer.cpp

run: client
	./client

clean:
	rm -f *.o client
