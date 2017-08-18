IDIR=./include
IDIRRPI=/opt/vc/include/
CC=gcc
CFLAGS=-I$(IDIR) -I$(IDIRRPI) -std=gnu11 -O3

ODIR=./obj
LDIR=./lib

LIBS=-lm /opt/vc/lib/libmmal_vc_client.so /opt/vc/lib/libmmal.so /opt/vc/lib/libmmal_components.so /opt/vc/lib/libmmal_util.so /opt/vc/lib/libmmal_core.so -pthread
#-pg

_DEPS=downsampler.h entropic_enc.h frame_encoder.h globals.h imgUtil.h quantizer.h camera_reader.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ=downsampler.o entropic_enc.o frame_encoder.o imgUtil.o quantizer.o quantizer_tester.o camera_reader.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

lhepi: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~