
#Include directory
IDIR=./

#Includes for mmal
IDIRRPI=/opt/vc/include/

#Compiler
CC=gcc

CFLAGS=-I$(IDIR) -I$(IDIRRPI) -std=gnu11 -O3

#Object directory
ODIR=./

#Libraries directories
LDIR=./
LIBS=-lm /opt/vc/lib/libmmal_vc_client.so /opt/vc/lib/libmmal.so /opt/vc/lib/libmmal_components.so /opt/vc/lib/libmmal_util.so /opt/vc/lib/libmmal_core.so -pthread
#-pg

#dependencies ( headers)
_DEPS=entropic_decoder_test.h entropic_decoder.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

#objects (files to compile), be careful to add only one file with main 
_OBJ= entropic_decoder_test.o entropic_decoder.o

#OBJ is concat ODIR and object file names
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

#compiling
$(ODIR)/%.o: %.c $(DEPS) #$(STB_image)
	$(CC) -c -o $@ $< $(CFLAGS)

#linking to get target executable
lhepi: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~