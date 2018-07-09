# Makefile for compiling register-Iiterval creation codes
  all: main.cpp
	find . -type f | xargs -n 5 touch
	g++ -std=c++11 -o RegisterIntervalCreation main.cpp

  clean:
	find . -type f | xargs touch
	rm -rf RegisterIntervalCreation
