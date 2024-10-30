#ifndef RC_MIDI_FILES
#define RC_MIDI_FILES

#include <stdio.h>
#include "rc_midi.h"

#define DELTA_ADJUST 150
#define MAX_QUEUES 100

typedef struct 
{
    unsigned char type[4];
    uint32_t length;
    uint16_t format;
    uint16_t ntrks;
    uint16_t division;
} headerChunk;

typedef struct 
{
    unsigned char type[4];
    uint32_t length;
} trackChunk;

typedef struct
{
    unsigned char **track_name;
    unsigned char **inst_name;
    uint16_t *tempo_map; 
} trackMeta;


void WriteVarLen(register uint32_t value, FILE *outfile);

uint32_t ReadVarLen(FILE *infile, uint8_t *bytes_read);

headerChunk *smf_to_queue(char *smf_filename, Queue **track_queuesm, trackMeta *meta_data_out);

#endif // RC_MIDI_FILES