all: fastcode

fastcode: fastcode.cpp utils.cpp
	g++ $^ -o fastcode
