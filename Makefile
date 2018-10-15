
#Include directory
IDIR=./include

#Includes for mmal
IDIRRPI=/opt/vc/include/

#Compiler
CC=gcc

CFLAGS=-I$(IDIR) -I$(IDIRRPI) -std=gnu11 -mcpu=cortex-a53 -mfpu=neon-fp-armv8 -O3

#Object directory
ODIR=./obj

#Libraries directories
LDIR=./lib
LIBS=-lm /opt/vc/lib/libmmal_vc_client.so /opt/vc/lib/libmmal.so /opt/vc/lib/libmmal_components.so /opt/vc/lib/libmmal_util.so /opt/vc/lib/libmmal_core.so -pthread
#-pg

#dependencies ( headers)
_DEPS=downsampler.h entropic_enc.h frame_encoder.h globals.h imgUtil.h quantizer.h camera_reader.h downsampler_simd.h video_encoder_simd.h streamer.h http_api.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

#STB_image = ./stb_image/stb_image.h ./stb_image/stb_image_write.h

#objects (files to compile), be careful to add only one file with main
#_OBJ= downsampler.o entropic_enc.o frame_encoder.o imgUtil.o quantizer.o camera_downsampler_test.o camera_reader.o quantizer_tester.o video_encoder.o

_OBJ= downsampler.o downsampler_simd.o entropic_enc.o frame_encoder.o imgUtil.o quantizer.o camera_reader.o video_encoder_simd.o video_encoder.o streamer.o http_api.o main.o
#quantizer_tester.o

#OBJ is concat ODIR and object file names
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))
#compiling assembly files
$(ODIR)/%.o: %.s $(DEPS) #$(STB_image)
	$(CC) -c -o $@ $< $(CFLAGS)
#compiling c files
$(ODIR)/%.o: %.c $(DEPS) #$(STB_image)
	$(CC) -c -o $@ $< $(CFLAGS)

#linking to get target executable
lhepi: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~