#include "my_malloc.h"

// add the node to the list

void addNode(node* tempNode, node** FreedHeadNode){
    if(*FreedHeadNode == NULL){
        *FreedHeadNode = tempNode;
    }else if(tempNode < *FreedHeadNode){
        tempNode->next = *FreedHeadNode;
        (*FreedHeadNode)->prev = tempNode;
        *FreedHeadNode = tempNode;
    }else{
        node* iterNode = *FreedHeadNode;
        while (iterNode->next != NULL)
        {
            if(iterNode->next > tempNode){
                break;
            }
            iterNode = iterNode->next;
        }
        tempNode->next = iterNode->next;
        if(tempNode->next != NULL){
            tempNode->next->prev = tempNode;
        }
        tempNode->prev = iterNode;
        iterNode->next = tempNode;
    }
    tempNode->isFreed = 1;
}

void removeNode(node* tempNode, node** FreedHeadNode){
    if(tempNode == *FreedHeadNode){
        *FreedHeadNode = tempNode->next;
    }else{
        tempNode->prev->next = tempNode->next;   // the next should be initialized as NULL
        if(tempNode->next != NULL){
            tempNode->next->prev = tempNode->prev;
        }
    }
    tempNode->next = NULL;
    tempNode->prev = NULL;
    tempNode->isFreed = 0;
}


/**
 * only could be use for the block has larger size then newSize; firstly find node in the list than try split it
 */
node* tryToSplitNode(node* tempNode, size_t newSize, node** FreedHeadNode){
    if(tempNode->size > (newSize + sizeof(node))){
        tempNode->size = newSize;
        node *newNode = (node*)((char*)tempNode + tempNode->size);
        newNode->size = tempNode->size - newSize - sizeof(node);
        newNode->next = NULL;
        newNode->prev = NULL;
        newNode->isFreed = 1;
        addNode(newNode, FreedHeadNode);
    }
    removeNode(tempNode, FreedHeadNode);
    return tempNode;
}




void* bf_malloc(size_t newSize, node** FreedHeadNode){
    node* tempNode = *FreedHeadNode;
    int bestSize = -1; // the smallest size which meet the request
    node* targetNode = NULL;
    while(tempNode != NULL){
        if(tempNode->size >= newSize && (bestSize == -1 || tempNode->size < bestSize)){
            bestSize = tempNode->size;
            targetNode = tempNode;
        }
        tempNode = tempNode->next;
    }
    if(targetNode == NULL){
        pthread_mutex_lock(&sbrkMutex);
        targetNode = (node*)sbrk(newSize + sizeof(node));
        pthread_mutex_unlock(&sbrkMutex);
        targetNode->prev = NULL;
        targetNode->next = NULL;
        targetNode->size = newSize;
    }else{
        targetNode = tryToSplitNode(targetNode, targetNode->size, FreedHeadNode);
    }
    targetNode->isFreed = 0;
    return (char*)targetNode + sizeof(node);
}

void bf_free(void* tempPtr, node** FreedHeadNode){
    node* tempNode = (node*)((char*)tempPtr - sizeof(node));
    tempNode->isFreed = 1;
    addNode(tempNode, FreedHeadNode);


    // merge the next node
    if(tempNode->next == (node*)((char*)tempNode + sizeof(node) + tempNode->size) && tempNode->next->isFreed == 1){
        tempNode->size += tempNode->next->size + sizeof(node);
        removeNode(tempNode->next, FreedHeadNode);
    }


    // merge the previous node
    if(tempNode->prev != NULL && tempNode->prev == (node*)((char*)tempNode - sizeof(node) - tempNode->prev->size) && tempNode->prev->isFreed == 1){
        tempNode->prev->size += tempNode->size + sizeof(node);
        removeNode(tempNode, FreedHeadNode);
    }
}



void* ts_malloc_lock(size_t size){
    pthread_mutex_lock(&mutex);
    void* temp = bf_malloc(size, &FreedHeadNode);
    pthread_mutex_unlock(&mutex);
    return temp;
}

void ts_free_lock(void* ptr){
    pthread_mutex_lock(&mutex);
    bf_free(ptr, &FreedHeadNode);
    pthread_mutex_unlock(&mutex);
}

void* ts_malloc_nolock(size_t size){
    bf_malloc(size, &localHeadNode);
}

void ts_free_nolock(void* ptr){
    bf_free(ptr, &localHeadNode);
}

// unsigned long get_largest_free_data_segment_size(){
//     node* tempNode = FreedHeadNode;
//     size_t maxSize = 0;
//     while(tempNode != NULL){
//         if(tempNode->size > maxSize){
//             maxSize = tempNode->size;
//         }
//         tempNode = tempNode->next;
//     }
//     return maxSize;
// }

// unsigned long get_total_free_size(){
//     node* tempNode = FreedHeadNode;
//     size_t sumSize = 0;
//     while(tempNode != NULL){
//         sumSize += tempNode->size;
//         tempNode = tempNode->next;
//     }
//     return sumSize;
// }


