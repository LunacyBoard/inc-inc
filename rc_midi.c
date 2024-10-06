// Main midi handling code for sending messages and queueing of events
// With thanks to Shiela Dixon for initial BDOS codes

#include "rc_midi.h"
#include <stdio.h> 
#include <stdlib.h>
#include <stdint.h>
#include <cpm.h>
#include <stdbool.h>

Event* createEvent(long delta, unsigned char status, int num_data, unsigned char * data)
{
    Event* midi_event = (Event*)malloc(sizeof(Event));
    midi_event->delta = delta;
    midi_event->status = status;
    midi_event->num_data = num_data;
    midi_event->data = data;
    midi_event->next = NULL;
    return midi_event;
}

Queue* createQueue()
{
    // Reserve memory for queue and set head & tail to empty
    printf("Create Q - \n");
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->head = q->tail = NULL;
    q->size = 0;
    return q;
}

void enqueue(Queue* q, long delta, unsigned char status, int num_data, unsigned char * data)
{
    // If queue is empty, the new event is both the head & tail
    Event* midi_event = createEvent(delta, status, num_data, data);
    q->size++;
    printf("Enqueue: size %d \n",q->size);
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
    printf("Dequeued: size %d \n",q->size);
    // Return the event at the front
    return temp;
}





uint8_t bdos_read_portB(){
    
    return bdos(CPM_RRDR,0);

}


void bdos_write_portB(uint8_t d){
    
    bdos(CPM_WPUN,d);
    
}

void send_MIDI_message(uint8_t msg, uint8_t num_data, uint8_t * data){
    
    bdos_write_portB(msg);
    //bdos_write_portB(note);
    //bdos_write_portB(vel);
    for(int i = 0; i < num_data; i++)
    {
        bdos_write_portB(data[i]);
    }
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