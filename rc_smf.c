// Routines for handling of Standard MIDI files

#include <stdint.h>
#include <stdio.h> 
#include <stdlib.h>
#include <byteswap.h>

// Routines for handling variable length numbers
// from https://www.cs.cmu.edu/~music/cmsip/readings/Standard-MIDI-file-format-updated.pdf

WriteVarLen(value, outfile) register long value;
{
    register long buffer;
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
int32_t ReadVarLen(infile)
{
    register int32_t value;
    register uint8_t c;
    if ((value = getc(infile)) & 0x80)
    {
        value &= 0x7f;
        do
        {
        value = (value << 7) + ((c = getc(infile))) & 0x7f;
        } while (c & 0x80);
    }
    return (value);
}


//
typedef struct headerChunk 
{
    char type[4];
    //char rest[10];
    uint32_t length;
    uint16_t format;
    uint16_t ntrks;
    uint16_t division;
} headerChunk;

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

    // Read the Header chunk
    fread(&head_chunk, sizeof(struct headerChunk), 1, fptr); 
    //printf("\n %s -- ", head_chunk.type);
    //for(int i = 0; i < 10; i++)
    //{
    //    printf("%d.",(uint8_t)head_chunk.rest[i]);
    //}

    printf("%s -- ", head_chunk.type);
    printf("__%d_%d__", bswap_32(head_chunk.length),head_chunk.length);
    printf("_%d_", bswap_16(head_chunk.format));
    printf("_%d_", bswap_16(head_chunk.ntrks));
    printf("_%d_", bswap_16(head_chunk.division));

    //printf("%s -- %d %d %d %d %d %d %d \n", head_chunk.type, head_chunk.l1, head_chunk.l2, head_chunk.l3, head_chunk.l4, head_chunk.format, head_chunk.ntrks, head_chunk.division);

    fclose(fptr); 

if ((fptr = fopen("INC1.MID","rb")) == NULL){
       printf("Error! opening file INC1.MID");

       // Program exits if the file pointer returns NULL.
       exit(1);
   }

    

    // Read the Header chunk
    fread(&head_chunk, sizeof(struct headerChunk), 1, fptr); 
    //printf("\n %s -- ", head_chunk.type);
    //for(int i = 0; i < 10; i++)
    //{
    //    printf("%d.",(uint8_t)head_chunk.rest[i]);
    //}
    printf("%s -- %d %d %d %d \n", head_chunk.type, bswap_32(head_chunk.length), bswap_16(head_chunk.format), bswap_16(head_chunk.ntrks), bswap_16(head_chunk.division));
    //printf("%s -- %d %d %d %d %d %d %d \n", head_chunk.type, head_chunk.l1, head_chunk.l2, head_chunk.l3, head_chunk.l4, head_chunk.format, head_chunk.ntrks, head_chunk.division);

    fclose(fptr); 

if ((fptr = fopen("INC2.MID","rb")) == NULL){
       printf("Error! opening file %s", smf_name);

       // Program exits if the file pointer returns NULL.
       exit(1);
   }

    

    // Read the Header chunk
    fread(&head_chunk, sizeof(struct headerChunk), 1, fptr); 
    //printf("\n %s -- ", head_chunk.type);
    //for(int i = 0; i < 10; i++)
    //{
    //    printf("%d.",(uint8_t)head_chunk.rest[i]);
    //}
    printf("%s -- %d %d %d %d \n", head_chunk.type, (uint32_t)bswap_32(head_chunk.length), (uint16_t)bswap_16(head_chunk.format), (uint16_t)bswap_16(head_chunk.ntrks), (uint16_t)bswap_16(head_chunk.division));
    //printf("%s -- %d %d %d %d %d %d %d \n", head_chunk.type, head_chunk.l1, head_chunk.l2, head_chunk.l3, head_chunk.l4, head_chunk.format, head_chunk.ntrks, head_chunk.division);

    fclose(fptr); 




    return 0;
}

