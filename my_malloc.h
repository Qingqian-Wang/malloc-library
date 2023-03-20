#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


// the main structure of node
typedef struct _node
{
    struct _node* prev;
    struct _node* next;
    size_t size;
    size_t isFreed; //1 means freed, 0 means invalid
} node;


node* FreedHeadNode = NULL;

__thread node* localHeadNode = NULL;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sbrkMutex = PTHREAD_MUTEX_INITIALIZER;

// add the node to the list




/**
 * only could be use for the block has larger size then newSize; firstly find node in the list than try split it
 */


void* ts_malloc_lock(size_t size);

void ts_free_lock(void* ptr);

void* ts_malloc_nolock(size_t size);

void ts_free_nolock(void* ptr);


unsigned long get_largest_free_data_segment_size();

unsigned long get_total_free_size();


