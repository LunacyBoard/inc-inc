// Routines for handling of Standard MIDI files

//  zcc +cpm -create-app -o midijuke midijuke.c -DAMALLOC

#include <stdint.h>
#include <stdio.h> 
#include <stdlib.h>
//#include <float.h>
#include <string.h>
//#include <byteswap.h>
#include "rc_smf.h"
#include "rc_midi.h"

//#define DEBUG

int main(int argc, char *argv[])
{
    // Open file
    char smf_name[20] = "INC0.MID";
    int8_t juke_cnt, juke_max;
    juke_max = argc;
    juke_cnt = 1;


    do {
        if(argc > 1)
        {
            strcpy(smf_name, argv[juke_cnt]);
        }
        printf("File %d of %d \n", juke_cnt, juke_max-1);

        Queue *track_queues[MAX_QUEUES];
        trackMeta *meta_data;

        // Populate the MIDI queues from the loaded file
        headerChunk *head_chunk = smf_to_queue(smf_name, track_queues, meta_data);

        // Test the queues
        Event *next_q_event[MAX_QUEUES];
        // Event *get = malloc(sizeof(Event));

        uint16_t running_tracks = 0;
        uint32_t next_delta = INT32_MAX; // Running count of the shortest delta value in the next events
        uint32_t d_weight = 17;

        // Load the first event from each track queue and identify the first
        printf("\201\205\205\205\205\205\205\205\205\205\205\205\205\187\n");
        printf("\186 %s   \186\n",smf_name);
        printf("\200\205\205\205\205\205\205\205\205\205\205\205\205\188\n");
        for(uint16_t tk = 1; tk < head_chunk->ntrks; tk++)
        {
            if(track_queues[tk]->size > 0)
                running_tracks++;

            next_q_event[tk] = dequeue(track_queues[tk]);
            if(next_q_event[tk]->delta < next_delta)
                next_delta = next_q_event[tk]->delta;

            // Display track metadata
            printf("(%d) %s: %s\n",tk , meta_data->track_name[tk], meta_data->inst_name[tk]);
        }

        while(running_tracks > 0)
        {
            // Adjust the stored deltas by the amount of the shortest (next) delta
            for(uint16_t tk = 1; tk < head_chunk->ntrks; tk++)
            { 
                next_q_event[tk]->delta -= next_delta;
            }

            // Wait for the specified time
            wait_delta(next_delta, (uint16_t)d_weight);

            // Reset the next delta counter and recalculate
            // at the same time play any items with delta == 0
            next_delta = INT32_MAX;
            running_tracks = 0;
            for(uint16_t tk = 1; tk < head_chunk->ntrks; tk++)
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
        
        free(track_queues);
        printf("Ta daaaaa!");

        juke_cnt++;
   } while(juke_cnt < juke_max);

    return 0;
}

