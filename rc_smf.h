#ifndef RC_MIDI_FILES
#define RC_MIDI_FILES

#include <stdio.h>

void WriteVarLen(register uint32_t value, FILE *outfile);

uint32_t ReadVarLen(FILE *infile, uint8_t *bytes_read);

#define MAX_QUEUES 100



#endif // RC_MIDI_FILES