#ifndef RC_MIDI_ROUTINES
#define RC_MIDI_ROUTINES

#define NOTE_ON     0x90
#define NOTE_OFF    0x80
#define POLY_PRESS  0xA0
#define CTRL_CH     0xB0
#define PROG_CH     0xC0
#define CH_PRESS    0xD0

//#include <math.h>
#include <stdint.h>
//#include <stdlib.h>

// MIDI event contains status byte plus 0-2 data bytes, here tagged with the time it will be played and link to next event
typedef struct Event
{
    long delta;
    unsigned char status;
    int num_data;
    unsigned char * data;
    struct Event* next;
} Event;

// The actual queue of nodes
typedef struct Queue 
{
    Event *head, *tail;
    int size;
} Queue;

// MIDI Queue functions
Event* createEvent(long delta, unsigned char status, int num_data, unsigned char * data);
Queue* createQueue();
void enqueue(Queue* q, long delta, unsigned char status, int num_data, unsigned char * data);
Event* dequeue(Queue* q);


// CP/M BDOS Port access
uint8_t bdos_read_portB();
void bdos_write_portB(uint8_t d);

// MIDI Playback
void send_MIDI_message(uint8_t msg, uint8_t num_data, uint8_t * data);


#endif // RC_MIDI_ROUTINES