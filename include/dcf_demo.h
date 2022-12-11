#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "string.h"
#include "dcf_interface.h"
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#define PASSED 0
#define FAILED -1

extern bool isleader;


#ifndef STRUCT_INPUTBUFFER
typedef struct
{
    char *buffer;
    size_t bufferLength;
    ssize_t inputLength;

} inputBuffer;

#define STRUCT_INPUTBUFFER 
#endif

inputBuffer *newInputBuffer();

void readCommand(inputBuffer *buf);

void Print_REPL();

void Print_Prompt();

int DCFTest_exit();

int DCFTest_set_param(int node_id);

void DCFTest_write(bool isleader, inputBuffer *input_buffer, char *writeContents, unsigned long long *writeIndex);

int DCFTest_read(unsigned int streamId, unsigned long long ReadIndex, char *ReadBuffer, unsigned int ReadLength);

int DCFTest_start(int node_id, char *dcf_start_config);

int DCFTest_add_node(unsigned int AddNode_id, char* Addip, unsigned int Addport);

int DCFTest_remove_node(int AddNode_id);

void DCFTest_index(int node_id);

int DCFTest_query(char *query_buffer, unsigned int length);
