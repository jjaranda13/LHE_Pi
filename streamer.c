#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

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

#include <time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <pthread.h>

#include "include/globals.h"
#include "include/streamer.h"
int total_frames=0;
int total_bytes=0;
bool newframe=false;
int line_type=0;

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void init_streamer()
{
    pthread_mutex_init(&stream_subframe_mutex,NULL);

    for (int i = 0; i< 8; i++)
    {
        pthread_cond_init(&stream_subframe_sync_cv[i], NULL);
        stream_subframe_sync[i] = 0;
    }
    stream_subframe_sync[7] = 3;
    pthread_mutex_init(&stream_subframe_sync_mtx, NULL);

    nal_byte_counter=0;

    frame_byte_counter=0;
    total_bytes=0;
    total_frames=0;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void *mytask_stream(void *arg)
{
    struct thread_streamer_info *tinfo = arg; //(int start, int separation, int num_threads)
    int start=tinfo->start;
    int separation=tinfo->separation;

    int subframe = start %8;
    int previous_subframe = subframe - 1;
    previous_subframe = previous_subframe < 0 ? 8 + previous_subframe: previous_subframe;

    pthread_mutex_lock(&stream_subframe_sync_mtx);
    if(stream_subframe_sync[previous_subframe] != num_threads)
    {
        pthread_cond_wait (&stream_subframe_sync_cv[previous_subframe],&stream_subframe_sync_mtx);
    }
    pthread_mutex_unlock(&stream_subframe_sync_mtx);

    pthread_mutex_lock(&stream_subframe_mutex);
    //inteligent discard
    bool discard=true;

    //luminancias
    int line=start;

    if (line==0 ) newframe=true;//flag de nuevo frame

    while (line<height_down_Y)
    {
        if (!discard || line%2==0|| inteligent_discard_Y[line]==false)
        {
            line_type=0;
            stream_line(bits_Y, tam_bits_Y[line],line);
        }

        line+=separation;
    }
    //Crominancias
    line=start;
    while (line<height_down_UV)
    {
        if (!discard || line%2==1 || inteligent_discard_U[line]==false);
        {
            line_type=1;
            stream_line(bits_U, tam_bits_U[line],line);
        }
        if (!discard && line%2==1 || inteligent_discard_V[line]==false);
        {
            line_type=2;
            stream_line(bits_V, tam_bits_V[line],line);
        }
        line+=separation;
    }

    fflush(stdout);
    pthread_mutex_unlock(&stream_subframe_mutex);

    pthread_mutex_lock(&stream_subframe_sync_mtx);
    stream_subframe_sync[subframe]++;
    if (stream_subframe_sync[subframe] == num_threads)
    {
        pthread_cond_broadcast(&stream_subframe_sync_cv[subframe]);
        stream_subframe_sync[previous_subframe] = 0;
    }
    pthread_mutex_unlock(&stream_subframe_sync_mtx);

    pthread_exit(NULL);
}


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void lanza_streamer_subframe(int startline,int separation)
{
pthread_attr_t attr;
pthread_attr_init(&attr);
pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

//struct thread_streamer_info tinfo;

tsinfo[startline].start=startline;
tsinfo[startline].separation=separation;

//crear thread
//pthread_t mithread;

//printf ("streaming %d \n", startline);
//pthread_create(&mithread, &attr, &mytask_stream, &tsinfo[startline]);

pthread_create(&streamer_thread[startline], &attr, &mytask_stream, &tsinfo[startline]);
//pthread_join(mithread, NULL);


}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void stream_line(uint8_t ** bits, int bits_lenght, int line)
{
    uint8_t line_low, line_high, line_size_bytes;

#if DEBUG_RANDOM_LOSES > 0
    int random = rand() % 100;
    if (number < random)
    {
        return;
    }
#endif /* DEBUG_RANDOM_LOSES > 0 */

    int line_aux=line;

    line_low = (uint8_t) line_aux;
    line_high = (uint8_t) (line_aux >> 8);
    //line_high &= ~(0xC0); // Set line_type to zeros

    if (line_type == 0) // Y component
    {
        line_high |= 0x60;//0xC0;
    }
    else if (line_type == 1) // U component
    {
        line_high |= 0x40;//0x80;
    }
    else // When line_type == 2  V component
    {
        line_high |= 0x20;//0x40;
    }

     //if (line_high==0) line_high=1;
     //if (line_low==0) line_low=1;

    //line_high=1;
    //line_low=1;

    line_size_bytes = (bits_lenght%8 == 0)? bits_lenght/8 : (bits_lenght/8)+1;

    if (newframe)
    {
        total_frames += 1;
        total_bytes += frame_byte_counter;

        if (total_frames==30)
        {
            fprintf (stderr,"INFO: Frame Stats-> Bytes: %5d ", total_bytes/30);
            total_frames = 0;
            total_bytes = 0;
        }
        frame_byte_counter=0;

        newframe=false;
    }
    frame_byte_counter+=line_size_bytes;

    if (nal_byte_counter>1000)
    {

       const uint8_t frame[] = {0x00, 0x00, 0x00,0x01, 0x65}; //nal tipo 5
       //son 3 bits, 1 bit para el forbiden cero, dos para ref idc y 5 bits para el tipo
       // es decir  xxx xx xxxxx -> 0 11 00001 nal tipo 1 (coded slice of a non idr picture)
       //-> 0 11 00111 nal tipo 7 (sequence parameter set)
       //-> 0 11 00101 nal tipo 5 (coded slice of idr picture)
       fwrite(&frame,sizeof(uint8_t),5,stdout);
       nal_byte_counter=line_size_bytes;
    }
    else nal_byte_counter+=line_size_bytes;

    fwrite(&line_high,sizeof(uint8_t),1,stdout);
    fwrite(&line_low,sizeof(uint8_t),1,stdout);
    fwrite(bits[line], sizeof(uint8_t), line_size_bytes, stdout);
    //fflush(stdout);
    //fclose(stdout);

}

void stream_frame()
{
    int const separacion=8;
    int line;

    for(int i = 0; i < separacion; i++)
    {
        line = i;
        while (line < height_down_Y)
        {
            if (inteligent_discard_Y[line] == false)
            {
                stream_line(bits_Y, tam_bits_Y[line], line);
            }
            line = line + separacion;
        }
    }

    for(int i =0 ; i <separacion; i++)
    {
        line =i;
        while (line<height_down_UV)
        {
            if (inteligent_discard_U[line] == false)
            {
                stream_line(bits_U, tam_bits_U[line],line);
            }
            line=line+separacion;
        }
    }
    for(int i =0 ; i <separacion; i++)
    {
        line =i;
        while (line<height_down_UV)
        {
            if (inteligent_discard_V[line] == false)
            {
                stream_line(bits_V, tam_bits_V[line],line);
            }
            line=line+separacion;
        }
    }
}
