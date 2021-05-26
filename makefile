IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=obj
LDIR =../lib

LIBS=-lm

_DEPS = hellomake.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = hellomake.o hellofunc.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

BUILD_SERVER=build_server
BUILD_CLIENT=build_client

makeall: screen-worms-server screen-worms-client


screen-worms-server: build
	cp build/screen-worms-server .

screen-worms-client: build
	cp build/screen-worms-client .

build:
	mkdir build
	cmake -S . -B build
	cd build && make

clean:
	rm -f screen-worms-client
	rm -f screen-worms-server
	rm -r -f build
	#rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~