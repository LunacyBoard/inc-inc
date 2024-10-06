// qtest.c - test program to make use of each module as it is ready

// zcc +cpm -create-app --list -o qtest rc_midi.c qtest.c -DAMALLOC

#include "rc_midi.h"
#include <stdio.h> 
#include <stdlib.h>
#include <time.h>
#include <math.h>


int main(int argc, char *argv[])
{
    
    // Load the specified MIDI file
    char * smf_name;
    smf_name = argc > 1 ? argv[1] : "DEFAULT.MID";
    printf("Loading file: %s\n", smf_name);

    // Create an event queue
    Queue* q = createQueue();

   // enqueue(q, 200, NOTE_ON | 1, 36, 0x7f);
   // enqueue(q, 200, NOTE_OFF| 1, 36, 0);
   // enqueue(q, 200, NOTE_ON | 1, 38, 0x7f);
   // enqueue(q, 200, NOTE_OFF| 1, 38, 0);
   // enqueue(q, 200, NOTE_ON | 1, 42, 0x7f);
   // enqueue(q, 200, NOTE_OFF| 1, 42, 0);
    unsigned char non[] = {26, 0x7f};
    unsigned char noff[] = {26, 0};
    unsigned char non2[] = {36, 0x7f};
    unsigned char noff2[] = {36, 0};
    
    enqueue(q, 2, NOTE_ON | 1, 2, non);
    enqueue(q, 2, NOTE_OFF| 1, 2, noff);
    enqueue(q, 2, NOTE_ON | 1, 2, non2);
    enqueue(q, 2, NOTE_OFF| 1, 2, noff2);
    enqueue(q, 2, NOTE_ON | 1, 2, non);
    enqueue(q, 2, NOTE_OFF| 1, 2, noff);

    clock_t time_req, time_st;
    time_st = clock();
    printf("cps %li \n",(long)time_st);
    time_req = clock() - time_st;

    // Test the queue
    while(q->size > 0)
    {
        Event* get;
        get = dequeue(q);
        printf("got it %li \n",(long)get->delta);
        while((long)time_req < (long)get->delta)
        {
        time_req = clock() - time_st;
        //printf("clocks %d \n",(int)time_req);
        }
        send_MIDI_message(get->status, get->num_data, get->data);
        free(get);
        time_st = clock();
    }
 
    return 0;
}

