CC=gcc
CFLAGS= -std=gnu11 -Wall -Wextra -O3 -fopenmp
LDFLAGS= 
LIB = -lSDL2 -lm -fopenmp
SOURCES=entropic_decoder.c quantizer_decoder.c upsampler_decoder.c player_decoder.c decoder.c get_bits.c image_yuv.c main.c
OBJECTS=$(SOURCES:.c=.o)
OBJECTS_DEBUG=$(SOURCES:.c=_g.o)
EXECUTABLE=lhe_rpi_player

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS) $(LIB) 

debug: CFLAGS = -std=gnu11 -Wall -Wextra -Og -ggdb -fprofile-arcs -ftest-coverage -pg -fsanitize=address -fsanitize=leak -fsanitize=undefined
debug: LIB += -pg -lgcov -fsanitize=address -fsanitize=leak -fsanitize=undefined

debug: all

clean :
	rm -f $(EXECUTABLE) *.o *.gcda *.gcno *.gcov gmon.out