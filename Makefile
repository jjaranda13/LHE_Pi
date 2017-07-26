IDIR=./include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=./obj
LDIR=./lib

LIBS=-lm 
#-pg

_DEPS=downsampler.h entropic_enc.h frame_encoder.h globals.h imgUtil.h quantizer.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ=downsampler.o entropic_enc.o frame_encoder.o imgUtil.o quantizer.o quantizer_tester.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

lhepi: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~