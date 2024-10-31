# inc-inc
In C. In C. In RC.

This project aims to provide a very specific type of sequencer for the RC2014. 
On the way to developing it for the 2024 Retro Challenge, a 'quick win' idea for a
standard midi file player came to the fore, so for now most of the code here provides
functionality for the 'MIDIJUKE.COM' MIDI file jukebox program. 

The application requires the "MIDI IN" midi interface developed by Shiela Dixon to operate.
I has been tested so far on ROMWBW on a Zed Pro RC2014, using the original version of the
interface (https://www.tindie.com/products/shieladixon/midi-module-designed-for-rc2014/).
Since Shiela's own RC2024 challenge was to improve on the original model, I cannot say that
this code will work on the new version directly, but hopefully will with minor tweaks.

I compiled it with:

`zcc +cpm -create-app -o midijuke midijuke.c -DAMALLOC`


 - `rc_midi.c` contains generic midi routines and definitions:
 	- createEvent - create a midi event that includes a delta time to wait until playback, a standard midi status byte and data bytes plus a pointer to the next event
 	- createQueue - creates a queue of midi events as a linked list
 	- enqueue - adds a midi event to the queue
 	- dequeue - removes a midi event from the queue, making its data available for use
 	- wait_delta - a bit of shonky code to wait a certain amount of time specified by the delta value in the midi events, subject to a certain weighting that depends on the clock speed of the system. I'm sure there must be a better way than this, but for now it is beyond my knowledge and there was only so much I could take in this time around!
 	- bdos_read_portB - reads a midi byte - not currently used
 	- bdos_write_portB - writes to the midi port a single byte
 	- send_MIDI_message - sends a whole midi message to the port, with one or two data bytes sent after the status byte, depending on the value of the 2nd data byte (255=byte not sent) 

 - `rc_smf.c`  contains code for loading standard midi files
 	- WriteVarLen - writes a variable length byte stream to a file - unused here
 	- ReadVarLen - reads a variable length stream of bytes according to the midi file spec into an unsigned 32 bit integer
 	- smf_to_queue - loads a midi file (format 0 or 1) and converts it into a set of queues, one per track. Currently standard midi events are handled, so note-on/off, program and control changes, etc. as well as some of the meta events. Sysex is not handled. A pointer to a midi file header chunk structure is returned containing certain metadata items about the file
 	
## MIDIJUKE - Standard MIDI file player for RC2014
 	
 - `midijuke.c` - contains basic midi jukebox code. Call it with midijuke midifile.mid midi2.mid midi3.mid and it will play the midi files in turn, one after the other. As it plays it shows some basic info about track names, if provided.
 	


