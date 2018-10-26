/**
 * @file streamer.h
 * @author JRD
 * @date Jul 2017
 * @brief image utilities.
 *
 * This module makes udp socket for exchange of bits between raspberry and pc
 *
 * @see https://github.com/jjaranda13/LHE_Pi
 */


void stream_line(uint8_t ** bits, int bits_lenght, int line, int line_type);
void stream_frame();
void lanza_streamer_subframe(int startline,int separation);
void init_streamer();
void send_nal();

/*
The header is 4 bytes:
----|----| -------- ----|----  --------
pppx|pppy|  width       | height

pppx -> 4 bits (from 0 to 16)
pppy -> 4 bits (from 0 to 16)
width -> 12 bits (from  0 to 4096).
height -> 12 bits (from  0 to 4096).

*/
void send_frame_header(int width, int height , int pppx, int pppy);

void send_fake_newline();
