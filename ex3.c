#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

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
char* remove(Queue* queue) {
    char* item = NULL;

    // Check if the queue is empty
    while (queue->front == queue->rear) {
        continue;
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
 * generateArticles - Generates articles with random types and stores them in the queue.
 * 
 * @param arg: Pointer to the thread number.
 * @param threadNum: Thread number.
 * @param numOfArticles: Number of articles to generate.
 * @return: None.
 */
void* generateArticles(void* arg, int threadNum, int numOfArticles) {
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

        // queueArr[threadNum].insert(message);
        printf("Generated article: %s\n", message);
    }
    pthread_exit(NULL);
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

int (FILE* configFile, int* ret){
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

int openFile(FILE* configFile, int argc, char* argv[]){
    if (argc != 2) {
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

// Write the main function which reads the configuration file and 
//    initiates the system.
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
    int i = 0, config[2];
    for (i = 0; i < threadsNum; i++)
    {
        //put in config[0] the num Of Articles and in config[1] the size of bounded
        if(getInfoConfig(configFile, config)) {
            return 1;
        }
        queuesArr[i] = Bounded_Buffer(config[1]);

        
    }
    


    //when read 3 failed - read only one number so generate for each N,W,S editors a bounded queue

    //generate 3 co-editors threads and get from the queues 

    //generate screen manager thread
    return 0;
}
