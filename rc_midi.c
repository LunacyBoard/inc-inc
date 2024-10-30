// Main midi handling code for sending messages and queueing of events
// With thanks to Shiela Dixon for initial BDOS codes

#include "rc_midi.h"
#include <stdio.h> 
#include <stdlib.h>
#include <stdint.h>
#include <cpm.h>
#include <stdbool.h>

//#define DEBUG

Event* createEvent(uint32_t delta, uint8_t status, uint8_t data_byte_1, uint8_t data_byte_2)
{
    Event* midi_event = (Event*)malloc(sizeof(Event));
    midi_event->delta = delta;
    midi_event->status = status;
    midi_event->data_byte_1 = data_byte_1;
    midi_event->data_byte_2 = data_byte_2;
    midi_event->next = NULL;
    return midi_event;
}

Queue* createQueue()
{
    // Reserve memory for queue and set head & tail to empty
    #ifdef DEBUG
    printf("Create Q - \n");
    #endif
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->head = q->tail = NULL;
    q->size = 0;
    return q;
}

void enqueue(Queue* q, uint32_t delta, uint8_t status, uint8_t data_byte_1, uint8_t data_byte_2)
{
    // If queue is empty, the new event is both the head & tail
    Event* midi_event = createEvent(delta, status, data_byte_1, data_byte_2);
    q->size++;
    #ifdef DEBUG
    printf("Q %d   ",(uint16_t)q->size);
    #endif
    if (q->tail == NULL) {
        q->head = q->tail = midi_event;
        return;
    }
    // Add the new node at the tail of the queue and
    q->tail->next = midi_event;
    q->tail = midi_event;
}

Event* dequeue(Queue* q)
{

    // If queue is empty, return
 //   if (isEmpty(q)) {
 //       printf("Queue Underflow\n");
 //       return;
 //   }
    // Store previous head and shuffle forward
    Event* temp = q->head;
    q->head = q->head->next;

    // If head becomes null, tail must also be
    if (q->head == NULL)
        q->tail = NULL;
    q->size--;
    #ifdef DEBUG
    printf("\nQ %d   ",q->size);
    #endif
    // Return the event at the front
    return temp;
}


void wait_delta(uint32_t delta, uint16_t delta_weight){

    int d = 0;
    while (delta > 0){
        
        for (uint16_t wd = 0; wd < delta_weight; wd++) {
            // do something to waste time
            d++;
        }
        #ifdef DEBUG
        if(delta % 100 == 0)
            printf("~%d~",(uint16_t)(delta/100));
        #endif
        delta--;
    }
}


uint8_t bdos_read_portB(){
    
    return bdos(CPM_RRDR,0);

}


void bdos_write_portB(uint8_t d){
    
    #ifdef DEBUG
    printf(" %x",d);
    #endif
    bdos(CPM_WPUN,d);
    
}

void send_MIDI_message(uint8_t status, uint8_t data_byte_1, uint8_t data_byte_2){
    
    bdos_write_portB(status);
    bdos_write_portB(data_byte_1); // First data byte is always output
    if(data_byte_1 & ~0x80)
        bdos_write_portB(data_byte_2); // If 2nd byte is 255 then it is not output
}



// test it works
/*
void main()
{
    // Create an event queue
    Queue* q = createQueue();

    enqueue(q, 12345, 0xFF,0,0x00);
    enqueue(q, 12, 0xF0,0,0x00);

    Event* get;
    get = dequeue(q);

    printf("got it %d \n",(int)get->delta);
    get = dequeue(q);

    printf("got it %d \n",(int)get->delta);
}
*/
/*
void main()
{
    printf("START");
    wait_delta(480*4*60, 8);
    printf("END");

printf("START");
    wait_delta(480*4*60, 30);
    printf("END");

    return 0;
}
*/