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

#include <sys/socket.h>

//void put_bits(int data);
int read_socket(int fd, struct sockaddr *remote, socklen_t *long_remote, unsigned long int * data, int long_data);
int write_socket (int fd, struct sockaddr *remote,	socklen_t long_remote, unsigned long int *data, int long_data);
void sendData();
void stream_line(uint8_t ** bits, int bits_lenght, int line);
void stream_frame();
void lanza_streamer_subframe(int startline,int separation);
void init_streamer();
