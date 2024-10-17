// Routines for handling of Standard MIDI files

//  zcc +cpm -create-app -o qtest rc_smf.c rc_midi.c -DAMALLOC

#include <stdint.h>
#include <stdio.h> 
#include <stdlib.h>
#include <float.h>
#include <byteswap.h>
#include "rc_smf.h"
#include "rc_midi.h"

// #define DEBUG


// Routines for handling variable length numbers
// from https://www.cs.cmu.edu/~music/cmsip/readings/Standard-MIDI-file-format-updated.pdf

void WriteVarLen(register uint32_t value, FILE *outfile)
{
    register uint32_t buffer;
    buffer = value & 0x7f;

    while ((value >>= 7) > 0)
    {
        buffer <<= 8;
        buffer |= 0x80;
        buffer += (value & 0x7f);
    }
    while (1==1)
    {
        putc(buffer, outfile);
        if (buffer & 0x80)
            buffer >>= 8;
        else
            break;
    }
}
uint32_t ReadVarLen(FILE *infile, uint8_t *bytes_read)
{
    register uint32_t value;
    register uint8_t c;

    *bytes_read = 1;
    if ((value = getc(infile)) & 0x80)
    {
        //printf("init val:%x ", (uint16_t)value);
        value &= 0x7f;
        //printf("2nd val:%x ", (uint16_t)value);
        do
        {
        value = (value << 7) + ((c = getc(infile)) & 0x7f);
        *bytes_read = *bytes_read + 1;
        //printf("loop val:%d, c:%d ", (uint16_t)value,c);
        } while (c & 0x80);
    }
    return (value);
}

//
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

int main(int argc, char *argv[])
{
    // Open file
    FILE *fptr;

    char * smf_name;
    smf_name = argc > 1 ? argv[1] : "INC0.MID";
    printf("Loading file: %s\n", smf_name);

    if ((fptr = fopen(smf_name,"rb")) == NULL){
       printf("Error! opening file %s", smf_name);
       // Program exits if the file pointer returns NULL.
       exit(1);
   }

    headerChunk head_chunk;

    // Read the Header chunk and convert 32 & 16 bit values to wee endian for z80
    fread(&head_chunk, sizeof(headerChunk), 1, fptr); 
    head_chunk.length =     bswap_32(head_chunk.length); 
    head_chunk.format =     bswap_16(head_chunk.format);  
    head_chunk.ntrks =      bswap_16(head_chunk.ntrks);   
    head_chunk.division =   bswap_16(head_chunk.division);
    // Print the header info - convert chunk length to 16 bit as printf here cannot handle 32 bit
    printf("%s length size %d, %d fmt:%d trks:%d ppqn:%d\n",head_chunk.type, sizeof(head_chunk.length)
        , (uint16_t)head_chunk.length
        , head_chunk.format
        , head_chunk.ntrks
        , head_chunk.division);

    // Read through each of the tracks
    
    trackChunk *track_chunks = malloc(head_chunk.ntrks * sizeof *track_chunks);
    Queue *track_queues[MAX_QUEUES];
    
    printf("Malloc %d\n",(head_chunk.ntrks * sizeof *track_chunks));
    uint8_t get_event_byte, status_byte, running, var_len_bytes, meta_type, channel, data1, data2, end_of_track;
    uint32_t event_cnt, get_delta, meta_length, d_weight,meta_tempo;
    uint16_t tempo;
    tempo = (uint16_t)120; // tempo default 120 bpm
    d_weight = CYCLES_PM / 97600U; // (uint32_t)head_chunk.division * (uint32_t)tempo;  // 57600U; 
    printf("tempo:%d, ppqn:%d, delta_weight:%d \n", tempo, head_chunk.division, (uint16_t)d_weight);
    uint8_t data_bytes[6];

    
    // Loop through the number of tracks, adding each track to the relevant queue

    for(uint16_t tk = 0; tk < head_chunk.ntrks; tk++)
    {
        // Create an array of event queues
        //Queue *q = createQueue();
        track_queues[tk] = createQueue();
        fread(&track_chunks[tk], sizeof(trackChunk), 1, fptr); 
        track_chunks[tk].length =  bswap_32(track_chunks[tk].length); 
        
        #ifdef DEBUG
        printf("%d:%s, %d \n",(tk + 1), track_chunks[tk].type, (uint16_t)track_chunks[tk].length);
        #endif

        printf("Loading track : %d (size:%d) \n",tk,(uint16_t)track_chunks[tk].length);
        running = 0;
        event_cnt = 0;
        end_of_track = 0;
        while(event_cnt < track_chunks[tk].length && end_of_track == 0)
        {
            // Get the delta time for the midi event, increase count by number of bytes in variable length field
            get_delta = ReadVarLen(fptr, &var_len_bytes);
            #ifdef DEBUG
            printf("delta:%d_", (uint16_t)get_delta, var_len_bytes);
            #endif
            event_cnt += var_len_bytes;

            // Get the next byte and if it is a status byte then update the status, else set data1 to the value
            fread(&get_event_byte, sizeof(get_event_byte), 1, fptr);
            event_cnt += 1;
            if((get_event_byte & 0x80) > 0)
            {
                status_byte = get_event_byte;
                // event is status byte, so if not reserved type then load the next data byte too
                if((status_byte & 0xF0) == 0xF0)
                {
                    #ifdef DEBUG
                    printf("<System Event %x>",status_byte);
                    #endif
                }
                else
                {
                    fread(&data1, sizeof(data1), 1, fptr);
                    event_cnt += 1;
                }
            }
            else
            {   
                data1 = get_event_byte;
                running = 1;
            }
            
            // Handle the status byte, ignoring the channel at this point
            switch (status_byte & 0xF0)
            {
            case SYS_COMM:
                // Here we handle meta events, sysex, system realtime, etc.
                switch(status_byte)
                {
                case META_EVENT:
                    // Depending on the type of meta event, we need additional bytes
                    
                    fread(&meta_type, sizeof(meta_type), 1, fptr);
                    
                    meta_length = ReadVarLen(fptr, &var_len_bytes);
                    //#ifdef DEBUG
                    printf("<Meta Event %x> %d\n", meta_type, (uint16_t)meta_length);
                    //#endif
                    event_cnt = event_cnt + meta_length + var_len_bytes;
                    uint8_t *meta_bytes = malloc(sizeof(uint8_t) * meta_length);

                    if(meta_length > 0)
                    {    
                        fread(meta_bytes, sizeof(uint8_t), (size_t)meta_length, fptr);
                        #ifdef DEBUG
                        printf("event_cnt=%d, meta_length=%d, var_len_bytes=%d\n",(uint16_t)event_cnt, (uint16_t)meta_length, (uint16_t)var_len_bytes);
                        printf("Meta Text Event:%x,%d\n -->", meta_type, (uint16_t)meta_length);
                        
                        for(uint16_t show_bytes = 0; show_bytes < meta_length; show_bytes++)
                        {
                            printf("%x ", meta_bytes[show_bytes]);
                        }
                        printf("\n");
                        #endif
                       
                    }

                    // For now skip over these meta events
                    switch (meta_type)
                    {
                    case MTA_SEQNO:
                        break;
                    case MTA_TX_TEXT:   
                    case MTA_TX_COPYRT: 
                    case MTA_TX_SEQNAME:
                    case MTA_TX_INSNAME:
                    case MTA_TX_LYRIC:  
                    case MTA_TX_MARKER: 
                    case MTA_TX_CUE:
                        printf("META text: %s \n", meta_bytes);
                        break;
                    case MTA_SEQ_SPEC:
                    case MTA_CH_PREFIX:
                        break;
                    case MTA_EOF_TRACK:
                        end_of_track = 1;
                        #ifdef DEBUG
                        printf("\n-- END OF TRACK %d --\n",tk);
                        #endif
                        break;
                    case MTA_SET_TEMPO:
                        //367649
                        meta_tempo = ((uint32_t)meta_bytes[0]<<16)|((uint32_t)meta_bytes[1]<<8)|((uint32_t)meta_bytes[2]);
                        //float bpm = (60.0 / (float)meta_tempo * 1e6);
                        printf("Meta Tempo = %d %d %d -> %d -> %d bpm \n", meta_bytes[0],meta_bytes[1],meta_bytes[2],(uint16_t)meta_tempo, 0);
                    case MTA_SMPTE_OFF:
                    case MTA_TIME_SIG:
                    case MTA_KEY_SIG:
                    default:
                        break;
                    }
                     free(meta_bytes);
                    
                    break;
                }
                break;

            case NOTE_ON:
                fread(&data2, sizeof(data2), 1, fptr);
                event_cnt++;
                channel = status_byte & 0x0F;
                #ifdef DEBUG
                printf("(%d)  Note On:%d,%d,%d\n",(uint16_t)event_cnt,channel,data1, data2);
                #endif
                enqueue(track_queues[tk], get_delta, status_byte, data1, data2);
                break;

            case NOTE_OFF:
                fread(&data2, sizeof(data2), 1, fptr);
                event_cnt++;
                channel = status_byte & 0x0F;
                #ifdef DEBUG
                printf("(%d)  Note Off:%d,%d,%d\n",(uint16_t)event_cnt,channel,data1, data2);
                #endif
                enqueue(track_queues[tk], get_delta, status_byte, data1, data2);
                break;

            case POLY_PRESS :
                fread(&data2, sizeof(data2), 1, fptr);
                event_cnt++;
                channel = status_byte & 0x0F;
                #ifdef DEBUG
                printf("(%d)  Poly Pressure:%d,%d,%d\n",(uint16_t)event_cnt,channel,data1, data2);
                #endif
                enqueue(track_queues[tk], get_delta, status_byte, data1, data2);
                break;

            case CTRL_CHG   :
                fread(&data2, sizeof(data2), 1, fptr);
                event_cnt++;
                channel = status_byte & 0x0F;
                #ifdef DEBUG
                printf("(%d)  Control Change:%d,%d,%d\n",(uint16_t)event_cnt,channel,data1, data2);
                #endif
                enqueue(track_queues[tk], get_delta, status_byte, data1, data2);
                break;

            case PROG_CHG   :
                // No second data byte
                channel = status_byte & 0x0F;
                #ifdef DEBUG
                printf("(%d)  Program Change:%d,%d,-\n",(uint16_t)event_cnt,channel,data1);
                #endif
                enqueue(track_queues[tk], get_delta, status_byte, data1, 255);
                break;

            case CH_PRESS   :
                // No second data byte
                channel = status_byte & 0x0F;
                #ifdef DEBUG
                printf("(%d)  Channel Pressure:%d,%d,-\n",(uint16_t)event_cnt,channel,data1);
                #endif
                enqueue(track_queues[tk], get_delta, status_byte, data1, 255);
                break;

            case PITCH_BEND :
                fread(&data2, sizeof(data2), 1, fptr);
                event_cnt++;
                channel = status_byte & 0x0F;
                #ifdef DEBUG
                printf("(%d)  Pitch Bend:%d,%d,%d\n",(uint16_t)event_cnt,channel,data1, data2);
                #endif
                enqueue(track_queues[tk], get_delta, status_byte, data1, data2);
                break;
               
            default:
                break;
            }
            // End of event cycle
        }
        #ifdef DEBUG
        printf("\n---end of track---\n\n");
        #endif
    }

    free(track_chunks);

    fclose(fptr); 

    // Test the queues
    Event *next_q_event[MAX_QUEUES];
    // Event *get = malloc(sizeof(Event));

    uint16_t running_tracks = 0;
    uint32_t next_delta = INT32_MAX; // Running count of the shortest delta value in the next events

    // Load the first event from each track queue and identify the first
    for(uint16_t tk = 1; tk < head_chunk.ntrks; tk++)
    {
        if(track_queues[tk]->size > 0)
            running_tracks++;

        next_q_event[tk] = dequeue(track_queues[tk]);
        if(next_q_event[tk]->delta < next_delta)
            next_delta = next_q_event[tk]->delta;
    }

    while(running_tracks > 0)
    {
        // Adjust the stored deltas by the amount of the shortest (next) delta
        for(uint16_t tk = 1; tk < head_chunk.ntrks; tk++)
        { 
            next_q_event[tk]->delta -= next_delta;
        }

        // Wait for the specified time
        wait_delta(next_delta, (uint16_t)d_weight);

        // Reset the next delta counter and recalculate
        // at the same time play any items with delta == 0
        next_delta = INT32_MAX;
        running_tracks = 0;
        for(uint16_t tk = 1; tk < head_chunk.ntrks; tk++)
        { 
            if(next_q_event[tk]->delta == 0)
            {
                // Send the midi message and replace the event with the next one in the respective queue
                send_MIDI_message(next_q_event[tk]->status, next_q_event[tk]->data_byte_1, next_q_event[tk]->data_byte_2);

                while(track_queues[tk]->size > 0 && next_q_event[tk]->delta == 0)
                {
                    next_q_event[tk] = dequeue(track_queues[tk]);
                    // keep playing any more dequeued events with zero delta time
                    if(next_q_event[tk]->delta == 0)
                    {
                        send_MIDI_message(next_q_event[tk]->status, next_q_event[tk]->data_byte_1, next_q_event[tk]->data_byte_2);
                    }
                }
            }
            // recalculate the shortest delta and number of tracks still running
            if(next_q_event[tk]->delta < next_delta)
                next_delta = next_q_event[tk]->delta;
            if(track_queues[tk]->size > 0)
                running_tracks++;
            printf("TR%d:%d -- ",tk,(uint16_t)track_queues[tk]->size);
        }
        printf("Running tracks:%d \n", running_tracks);
    }
    // DONE!
    free(track_chunks);
    free(track_queues);
    printf("Ta daaaaa!");
    return 0;
}

