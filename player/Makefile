CC=gcc
CFLAGS= -std=gnu11 -Wall -O3
LDFLAGS= 
LIB = -lSDL2 -lm
SOURCES=entropic_decoder.c quantizer_decoder.c upsampler_decoder.c player_decoder.c decoder.c get_bits.c main.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=lhe_rpi_player

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS) $(LIB) 

clean :
	rm -f $(EXECUTABLE) *.o	