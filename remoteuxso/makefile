# Makefile for remoteux

PROGNAME = libremoteux.so
EXENAME= test.exe
CC = g++
CXX = g++

OBJ = *.cpp

#debug
CCFLAGS = -Xlinker -rpath -Xlinker -lutil -g -shared -DDEBUG
#release
#CCFLAGS = -lutil -shared -O3 -g

all:  clean $(PROGNAME) $(EXENAME) install 

remoteuxso: clean $(PROGNAME) 

$(EXENAME):
	$(CXX) -Xlinker -rpath -Xlinker -g -DDEBUG -lutil -o $@ main.cpp ./$(PROGNAME)

$(PROGNAME):
	$(CXX) $(CCFLAGS) $(OBJ) -o $(PROGNAME) 
	
clean:
	rm -f  *.so *~; 
	rm -f $(PROGNAME) $(EXENAME); 

######### install
install : 
	rm -f /usr/lib/$(PROGNAME)
	ln $(PROGNAME) /usr/lib;
	ldconfig
