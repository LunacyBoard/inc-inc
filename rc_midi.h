#ifndef RC_MIDI_ROUTINES
#define RC_MIDI_ROUTINES

// Meta event status byte and type identifiers
#define META_EVENT      0xFF

#define MTA_SEQNO       0x00
#define MTA_TX_TEXT     0x01
#define MTA_TX_COPYRT   0x02
#define MTA_TX_SEQNAME  0x03
#define MTA_TX_INSNAME  0x04
#define MTA_TX_LYRIC    0x05
#define MTA_TX_MARKER   0x06
#define MTA_TX_CUE      0x07
#define MTA_SEQ_SPEC    0x7F
#define MTA_CH_PREFIX   0x20
#define MTA_EOF_TRACK   0x2F
#define MTA_SET_TEMPO   0x51
#define MTA_SMPTE_OFF   0x54
#define MTA_TIME_SIG    0x58
#define MTA_KEY_SIG     0x59

// Channel & Sys
#define NOTE_OFF    0x80
#define NOTE_ON     0x90
#define POLY_PRESS  0xA0
#define CTRL_CHG    0xB0
#define PROG_CHG    0xC0
#define CH_PRESS    0xD0
#define PITCH_BEND  0xE0
#define SYS_COMM    0xF0


#define CYCLES_PM 2844420UL

//#include <math.h>
#include <stdint.h>
//#include <stdlib.h>

// MIDI event contains status byte plus 0-2 data bytes, here tagged with the time it will be played and link to next event
typedef struct Event_OLD
{
    uint32_t delta;
    unsigned char status;
    int num_data;
    unsigned char * data;
    struct Event* next;
} Event_OLD;

typedef struct Event
{
    uint32_t delta;
    uint8_t status;
    uint8_t data_byte_1;
    uint8_t data_byte_2;
    struct Event* next;
} Event;

// The actual queue of nodes
typedef struct Queue
{
    Event *head, *tail;
    int size;
} Queue;

// MIDI Queue functions
Event* createEvent(uint32_t delta, uint8_t status, uint8_t data_byte_1, uint8_t data_byte_2);
Queue* createQueue();
void enqueue(Queue* q, uint32_t delta, uint8_t status, uint8_t data_byte_1, uint8_t data_byte_2);
Event* dequeue(Queue* q);


// CP/M BDOS Port access
uint8_t bdos_read_portB();
void bdos_write_portB(uint8_t d);

// MIDI Playback
void send_MIDI_message(uint8_t status, uint8_t data_byte_1, uint8_t data_byte_2);


void wait(uint16_t siWaitNow);
void wait_delta(uint32_t delta, uint16_t delta_weight);

#endif // RC_MIDI_ROUTINES