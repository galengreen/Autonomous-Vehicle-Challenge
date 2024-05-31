avc: avc.o
	c++ -o avc avc.o -le101
avc.o: avc.cpp
	c++ -c avc.cpp
