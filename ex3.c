#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/**
 * Queue - A data structure representing a bounded buffer with thread-safe access.
 *
 * @field size: The size of the bounded buffer.
 * @field front: The index of the front element in the bounded buffer. Initialized to -1
 *              to indicate an empty queue.
 * @field rear: The index of the rear element in the bounded buffer. Initialized to -1
 *             to indicate an empty queue.
 * @field newsArr: A pointer to an array of char pointers representing the bounded buffer.
 * @field mutex: A mutex for providing thread-safe access to the bounded buffer.
 */
typedef struct {
    int size;
    int front; // initialize to -1 so indicate the queue is empty
    int rear;  // initialize to -1 so indicate the queue is empty will point to last in queue
    char** newsArr; 
    pthread_mutex_t mutex;  // Mutex for thread-safe access
} Queue;

/**
 * Bounded_Buffer - Create a new bounded buffer with the specified size.
 *
 * @param size: The size of the bounded buffer.
 *
 * @return: A pointer to the newly created Queue object representing the bounded buffer.
 */
Queue* Bounded_Buffer(int size) {
    Queue* newQ;
    newQ->size = size;
    newQ->front = -1;
    newQ->rear = -1;
    newQ->newsArr = (char**)malloc(size * sizeof(char*));
    pthread_mutex_init(&(newQ->mutex), NULL);  // Initialize the mutex
}

/**
 * insert - Insert a new object into the bounded buffer.
 *
 * @param s: A pointer to the object to be inserted into the bounded buffer.
 * @param queue: A pointer to the Queue object representing the bounded buffer.
 */
void insert(char* s, Queue* queue) {
    //check if it possible to insert a new
    while ((queue->rear + 1) % queue->size == queue->front) {
        continue;
    }
    // Lock the mutex to ensure exclusive access to the queue
    pthread_mutex_lock(&(queue->mutex));
    // Increment the rear index and wrap around if necessary
    queue->rear = (queue->rear + 1) % queue->size;    // Insert the item into the queue
    queue->newsArr[queue->rear] = s;
    // Unlock the mutex to allow other threads to access the queue
    pthread_mutex_unlock(&(queue->mutex));
}

/**
 * remove - Remove the first object from the bounded buffer and return it.
 *
 * @param queue: A pointer to the Queue object representing the bounded buffer.
 *
 * @return: A pointer to the removed object from the bounded buffer. Returns NULL
 *          if the bounded buffer is empty.
 */
char* remove(Queue* queue) {
    char* item = NULL;

    // Check if the queue is empty
    while (queue->front == queue->rear) {
        continue;
    }
    // Lock the mutex to ensure exclusive access to the queue
    pthread_mutex_lock(&(queue->mutex));
    
    // Retrieve the item from the queue
    item = queue->newsArr[queue->front];

    // Increment the front index and wrap around if necessary
    queue->front = (queue->front + 1) % queue->size;

    // Unlock the mutex to allow other threads to access the queue
    pthread_mutex_unlock(&(queue->mutex));

    return item;
}

// Implement the different threads: Producer and Dispatcher and Screen 
//    manager.




int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./program_name <config_file>\n");
        return 1;
    }

    FILE* configFile = fopen(argv[1], "r");
    if (!configFile) {
        fprintf(stderr, "Failed to open the configuration file.\n");
        return 1;
    }
    //try to read 3 lines from file if success send them as arg to new thread

    //make the N producers threads and feed the queues

    //create the dispatcher

    //when read 3 failed - read only one number so generate for each N,W,S editors a bounded queue

    //generate 3 co-editors threads and get from the queues 

    //generate screen manager thread
    return 0;
}
