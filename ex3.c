#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h> 

#define MAX_LINE_CONF 150

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
    char** articlessArr; 
    pthread_mutex_t mutex;  // Mutex for thread-safe access
} Queue;

typedef struct {
    int threadNum;
    int numOfArticles;
    Queue* queue;
} ThreadArguments;

typedef enum {
    SPORTS,
    WEATHER,
    NEWS
} MessageType;

/**
 * Bounded_Buffer - Create a new bounded buffer with the specified size.
 *
 * @param size: The size of the bounded buffer.
 *
 * @return: A pointer to the newly created Queue object representing the bounded buffer.
 */
Queue* Bounded_Buffer(int size) {
    Queue* newQ = malloc(sizeof(Queue));
    newQ->size = size;
    newQ->front = -1;
    newQ->rear = -1;
    newQ->articlessArr = (char**)malloc(size * sizeof(char*));
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
    queue->articlessArr[queue->rear] = s;
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
char* removeItem(Queue* queue) {
    char* item = NULL;

    // Check if the queue is empty
    while (queue->front == queue->rear) {
        NULL;
    }
    // Lock the mutex to ensure exclusive access to the queue
    pthread_mutex_lock(&(queue->mutex));
    
    // Retrieve the item from the queue
    item = queue->articlessArr[queue->front];

    // Increment the front index and wrap around if necessary
    queue->front = (queue->front + 1) % queue->size;

    // Unlock the mutex to allow other threads to access the queue
    pthread_mutex_unlock(&(queue->mutex));

    return item;
}

/**
 * numOfProducers - Determines the number of threads based on the configuration file.
 * 
 * @param configFile: Pointer to the configuration file.
 * @return: The number of threads based on the configuration file.
 */
int numOfProducers(FILE* configFile) {
    int numThreads = 0;
    int count = 0;
    char buffer[MAX_LINE_CONF];
    
    // Read the file line by line
    while (fgets(buffer, sizeof(buffer), configFile) != NULL) {
        // Ignore empty lines
        if (strlen(buffer) > 1) {
            count++;
            // Check if three lines have been encountered
            if (count == 3) {
                numThreads++;
                count = 0; // Reset the count
            }
        }
    }
    return numThreads;
}

/**
 * Retrieves the configuration information from the given config file.
 *
 * @param configFile The file pointer to the configuration file.
 * @param ret An integer array to store the retrieved configuration values.
 *            ret[0] will be set to the number of articles to generate.
 *            ret[1] will be set to the size of the queue.
 * @return 0 if the configuration information is successfully retrieved, 1 otherwise.
 */
int getInfoConfig(FILE* configFile, int* ret){
    // Skip the producer number line
    char buffer[MAX_LINE_CONF];
    if (fgets(buffer, sizeof(buffer), configFile) == NULL) {
        fprintf(stderr, "Failed to read the line.\n");
        return 1;
    }

    // Read the number from the next line as number of articles to generate
    int numOfArticles;
    if (fgets(buffer, sizeof(buffer), configFile) == NULL) {
        fprintf(stderr, "Failed to read the number.\n");
        return 1;
    }
    if (sscanf(buffer, "%d", &numOfArticles) != 1) {
        fprintf(stderr, "Failed to parse the number.\n");
        return 1;
    }

    // Read the number from the next line as the bound of the queue
    int queueSize;
    if (fgets(buffer, sizeof(buffer), configFile) == NULL) {
        fprintf(stderr, "Failed to read the number.\n");
        return 1;
    }
    if (sscanf(buffer, "%d", &queueSize) != 1) {
        fprintf(stderr, "Failed to parse the number.\n");
        return 1;
    }
    // Update the values in the ret array
    ret[0] = numOfArticles;
    ret[1] = queueSize;
    return 0;
}

/**
 * Opens the configuration file for reading.
 *
 * @param configFile The file pointer to store the opened configuration file.
 * @param argc The number of command-line arguments.
 * @param argv An array of command-line arguments.
 * @return 0 if the configuration file is successfully opened, 1 otherwise.
 */
int openFile(FILE* configFile, int argc, char* argv[]){
    if (argc != 2) {
        printf("no confiig\n");
        fprintf(stderr, "Usage: ./program_name <config_file>\n");
        return 1;
    }

    configFile = fopen(argv[1], "r");
    if (!configFile) {
        fprintf(stderr, "Failed to open the configuration file.\n");
        return 1;
    }
    return 0;
}

/**
 * generateArticles - Generates articles with random types and stores them in the queue.
 * 
 * @param arg: Pointer to the thread arguments.
 * @return: None.
 */
void* producer(void* arg) {
    ThreadArguments* args = (ThreadArguments*)arg;
    int threadNum = args->threadNum;
    int numOfArticles = args->numOfArticles;
    Queue* queue = args->queue;

    int i = 0, randNum;
    for (i = 0; i < numOfArticles; i++)
    {
        randNum = rand() % 3;  // Randomly generate num as 0-2
        char* type;
        switch (randNum) {
            case SPORTS:
                type = "SPORTS";
            case WEATHER:
                type = "WEATHER";
            case NEWS:
                type = "NEWS";
            default:
                type = "UNKNOWN";
            }
        char message[100];
        sprintf(message, "Producer %d %s %d", threadNum, type, i);
        //allocate memory on heap so it remain
        char* messagePtr = malloc((strlen(message) + 1) * sizeof(char));
        strcpy(messagePtr, message);
        //insert message to queue
        insert(message, queue);
        printf("Generated article: %s\n", message);
    }
    char* finishMessage = malloc(strlen("Done") + 1);  // +1 for the null terminator
    strcpy(finishMessage, "Done");
    insert(finishMessage, queue);
    pthread_exit(NULL);
}

void* dispatcher(void* arg){
    Queue** queues = (Queue**)arg; // Cast the argument to the appropriate type
    int numQueues = *((int*)arg + 1); // Get the number of queues from the argument

    bool* isDone = malloc(numQueues * sizeof(bool));  // Allocate the boolean array
    bool finish = false;
    // Check if the allocation was successful
    if (isDone == NULL) {
        // Handle the error
        fprintf(stderr, "Failed to allocate memory for the boolean array.\n");
        // Additional error handling or cleanup code if needed
        // ...
    } else {
        // Initialize all elements to false
        for (int i = 0; i < numQueues; i++) {
            isDone[i] = false;
        }
    }

    while (!finish) {
        finish = true;
        for (int i = 0; i < numQueues; i++) {
            //check if this queue finished
            if(isDone[i]) {
                continue;
            }
            char* item = removeItem(queues[i]); // Assuming you have a removeItem function for removing items from the queue
            //the remove return null if the queue is empty
            if (item != NULL) {
                // Process the item
                //check if the item is "Done"
                if (strcmp(item, "Done") == 0) {
                    isDone[i] = true;
                }
                printf("Processed item from queue %d: %s\n", i, item);
            }
            if(finish && !isDone[i]) {
                finish = false;
            }
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    //check if there are inough param and if the file can be opened.
    FILE* configFile;
    if (openFile(configFile, argc, argv)) {
        return 1;
    }

    //check how many threads i will need and than creat queue array this length
    int threadsNum = numOfProducers(configFile);
    Queue** queuesArr = malloc(threadsNum * sizeof(Queue*));

    //make the N producers threads and feed the queues
    //create the dispatcher
    int i = 0;
    pthread_t threads[threadsNum];
    //make the threads and send them and a quqeue to func
    for (i = 0; i < threadsNum; i++)
    {
        int config[2];
        //put in config[0] the num Of Articles and in config[1] the size of bounded
        if(getInfoConfig(configFile, config)) {
            return 1;
        }
        queuesArr[i] = Bounded_Buffer(config[1]);

        ThreadArguments* args = malloc(sizeof(ThreadArguments));
        args->threadNum = i;
        args->numOfArticles = config[0];
        args->queue = queuesArr[i];

        pthread_create(&threads[i], NULL, producer, args);
    }

     

    
    

    //implement dispatcher and three
    //when read 3 failed - read only one number so generate for each N,W,S editors a bounded queue



    //generate 3 co-editors threads and get from the queues 
    //implement screen maneger
    //generate screen manager thread
    return 0;
}
