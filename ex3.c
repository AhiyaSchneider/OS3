#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h> 
#include <unistd.h>

#define MAX_LINE_CONF 150
#define CO_EDIT_QUEUES 3
#define SPORTS_QUEUE 0
#define NEWS_QUEUE 1
#define WEATHER_QUEUE 2

typedef struct UnBoundItem UnBoundItem;

/**
 * BoundedQueue - A data structure representing a bounded buffer with thread-safe access.
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
} BoundedQueue;

typedef enum {
    SPORTS,
    WEATHER,
    NEWS
} MessageType;

//un bounded queue:
typedef struct UnBoundItem {
    char* article;
    UnBoundItem* next;
    UnBoundItem* prev;
} UnBoundItem;

typedef struct {
    UnBoundItem* head;
    UnBoundItem* tail;
    pthread_mutex_t mutex;
} UnBoundQueue;

typedef struct {
    int threadNum;
    int numOfArticles;
    BoundedQueue* queue;
} ProducerArguments;

typedef struct {
    UnBoundQueue** coEditQueues;
    int numQueues;
    BoundedQueue** queues;
} DispatcherArguments;

typedef struct {
    UnBoundQueue* unBoundQueue;
    BoundedQueue* screenQueue;
} CoEditorArguments;

typedef struct {
    BoundedQueue* screenQueue;
} ScreenManagerArguments;

//bounded queue:
/**
 * Bounded_Buffer - Create a new bounded buffer with the specified size.
 *
 * @param size: The size of the bounded buffer.
 *
 * @return: A pointer to the newly created BoundedQueue object representing the bounded buffer.
 */
BoundedQueue* Bounded_Buffer(int size) {
    BoundedQueue* newQ = malloc(sizeof(BoundedQueue));
    newQ->size = size;
    newQ->front = 0;
    newQ->rear = 0;
    newQ->articlessArr = (char**)malloc(size * sizeof(char*));
    pthread_mutex_init(&(newQ->mutex), NULL);  // Initialize the mutex
    return newQ;
}

/**
 * insert - Insert a new object into the bounded buffer.
 *
 * @param s: A pointer to the object to be inserted into the bounded buffer.
 * @param queue: A pointer to the BoundedQueue object representing the bounded buffer.
 */
void insert(char* s, BoundedQueue* queue) {
    //check if it possible to insert a new
    while ((queue->rear + 1) % queue->size == queue->front) {
        // printf("stayee\n");
        continue;

    }
    // Lock the mutex to ensure exclusive access to the queue
    pthread_mutex_lock(&(queue->mutex));
    
    queue->articlessArr[queue->rear] = s;

    // Increment the rear index and wrap around if necessary
    queue->rear = (queue->rear + 1) % queue->size;    // Insert the item into the queue

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
char* removeItem(BoundedQueue* queue) {
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

//un bounded queue:
UnBoundQueue* unBoundQueue(){
    UnBoundQueue* queue = malloc(sizeof(UnBoundQueue));
    queue->head = NULL;
    queue->tail = NULL;
    pthread_mutex_init(&(queue->mutex), NULL);
    return queue;
}

/**
 * insert - Insert a new object into the unbounded queue.
 *
 * @param s: A pointer to the object to be inserted into the unbounded queue.
 * @param queue: A pointer to the UnBoundQueue object representing the unbounded queue.
 */
void insertToUnBound(char* s, UnBoundQueue* queue) {
    UnBoundItem* newItem = malloc(sizeof(UnBoundItem));
    newItem->article = s;
    newItem->next = NULL;

    pthread_mutex_lock(&(queue->mutex));

    if (queue->head == NULL) {
        queue->head = newItem;
        queue->tail = newItem;
    } else {
        queue->tail->next = newItem;
        queue->tail = newItem;
    }

    pthread_mutex_unlock(&(queue->mutex));
}

/**
 * removeItem - Remove the first object from the unbounded queue and return it.
 *
 * @param queue: A pointer to the UnBoundQueue object representing the unbounded queue.
 *
 * @return: A pointer to the removed object from the unbounded queue. Returns NULL
 *          if the unbounded queue is empty.
 */
char* removeItemFromUnBound(UnBoundQueue* queue) {
    pthread_mutex_lock(&(queue->mutex));

    if (queue->head == NULL) {
        pthread_mutex_unlock(&(queue->mutex));
        return NULL; // Queue is empty
    }

    UnBoundItem* itemToRemove = queue->head;
    char* item = itemToRemove->article;

    queue->head = itemToRemove->next;
    free(itemToRemove);

    if (queue->head == NULL) {
        queue->tail = NULL;
    }

    pthread_mutex_unlock(&(queue->mutex));

    return item;
}


//the code:
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
            // printf("1111the char:    %c, num:  %d  count:   %d\n", buffer[0], buffer[0], count);
            if( buffer[0] == '\n' || buffer[0] == ' ') {
                if (count == 1) {
                    break;
                } else if(count == 3) {
                    numThreads++;
                    count = 0;
                } else if(count == 0) {
                    continue;
                }

            }
            count++;
            if (count > 2){
                count = count %3;
                numThreads++;
            }
        }
    }
    // printf("num of threads:    %d\n", numThreads);
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

    if(buffer[0] == ' ' || buffer[0] == '\n'){
        if (fgets(buffer, sizeof(buffer), configFile) == NULL) {
            fprintf(stderr, "Failed to read the line.\n");
            return 1;
        }
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
int openFile(FILE** configFile, int argc, char* argv[]){
    if (argc != 2) {
        printf("no confiig\n");
        fprintf(stderr, "Usage: ./program_name <config_file>\n");
        return 1;
    }

    *configFile = fopen(argv[1], "r");
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
    pthread_t threadId = pthread_self();
    ProducerArguments* prodArgs = (ProducerArguments*)arg;
    int threadNum = prodArgs->threadNum;
    int numOfArticles = prodArgs->numOfArticles;
    BoundedQueue* queue = prodArgs->queue;
    
    // printf("thread number:   %d,   number of articles:    %d\n",threadNum, numOfArticles);
    int i = 0, randNum, commomCounter =0;
    int sportsCounter = 0, newsCounter = 0, weatherCounter = 0;
    for (i = 0; i < numOfArticles; i++)
    {
        randNum = rand() % 3;  // Randomly generate num as 0-2
        char* type;
        switch (randNum) {
            case SPORTS:
                commomCounter = sportsCounter;
                sportsCounter++;
                type = "SPORTS";
                break;
            case WEATHER:
                commomCounter = weatherCounter;
                weatherCounter++;
                type = "WEATHER";
                break;
            case NEWS:
                commomCounter = newsCounter;
                newsCounter++;
                type = "NEWS";
                break;
            default:
                type = "UNKNOWN";
                break;
        }
        // printf("the type is: %s  random number is:   %d\n", type, randNum);
        char message[100];
        sprintf(message, "Producer %d %s %d", threadNum, type, commomCounter);
        //allocate memory on heap so it remain
        char* messagePtr = malloc((strlen(message) + 1) * sizeof(char));
        strcpy(messagePtr, message);
        //insert message to queue
        insert(messagePtr, queue);
        printf("in1  Generated article: %s\n", messagePtr);
    }
    char* finishMessage = malloc(strlen("Done") + 1);  // +1 for the null terminator
    strcpy(finishMessage, "Done");
    insert(finishMessage, queue);
    printf("DONE INSERTED\n");
    free(prodArgs);
    pthread_exit(NULL);
}


void producerGenerate(int threadsNum, FILE* configFile, BoundedQueue** boundQueuesArr){
    //make the N producers threads and feed the queues
    //create the dispatcher
    int i = 0;
    pthread_t threads[threadsNum];
    //make the threads and send them and a quqeue to func
    rewind(configFile);
    for (i = 0; i < threadsNum; i++)
    {
        int config[2];
        //put in config[0] the num Of Articles and in config[1] the size of bounded
        if(getInfoConfig(configFile, config)) {
            printf("failed to put data in config\n");
            // return 1;
            //deal with failure here
        }
        boundQueuesArr[i] = Bounded_Buffer(config[1]);

        ProducerArguments* producerArgs = malloc(sizeof(ProducerArguments));
        producerArgs->threadNum = i;
        producerArgs->numOfArticles = config[0];
        producerArgs->queue = boundQueuesArr[i];
        pthread_create(&threads[i], NULL, producer, producerArgs);
    }
}


bool boundQueueRead(bool* isDone, int i, BoundedQueue* boundedQueue, UnBoundQueue** unBoundQueues) {
    //check if this queue finished
    if(isDone[i]) {
        return true;
    }
    if (boundedQueue == NULL)
    {
        return false;
    }
    
    char* item = removeItem(boundedQueue); // Assuming you have a removeItem function for removing items from the queue
    //the remove return null if the queue is empty
    if (item != NULL) {
        //check if the item is "Done"
        if (strcmp(item, "Done") == 0) {
            free(item);
            return true;
        } else {
            // Check the type of the message
            printf("out2 from the queue :   %s\n", item);
            if (strstr(item, "SPORTS") != NULL) {
                // Process sports message
                insertToUnBound(item, unBoundQueues[SPORTS_QUEUE]);
            } else if (strstr(item, "WEATHER") != NULL) {
                // Process weather message
                insertToUnBound(item, unBoundQueues[WEATHER_QUEUE]);
            } else if (strstr(item, "NEWS") != NULL) {
                // Process news message
                insertToUnBound(item, unBoundQueues[NEWS_QUEUE]);
            } else {
                // Unknown type
                printf("SOMETHING FAILED\n");
            }
        }
    }
    return false;
}


/**
 * Dispatcher function that processes items from multiple queues until all queues are done.
 *
 * @param arg The argument passed to the thread containing the queues and the number of queues.
 * @return NULL
 */
void* dispatcher(void* arg){
    DispatcherArguments* dispArgs = (DispatcherArguments*)arg;
    UnBoundQueue** coEditQueues = dispArgs->coEditQueues;
    int numQueues = dispArgs->numQueues;
    BoundedQueue** queues = dispArgs->queues;

    bool* isDone = calloc(numQueues, sizeof(bool));  // Allocate the boolean array
    bool finish = false;

    // printf("num of queues:   %d\n", numQueues);
    // Check if the allocation was successful
    if (isDone == NULL) {
        // Handle the error
        fprintf(stderr, "Failed to allocate memory for the boolean array.\n");
    }
    while (!finish) {
        finish = true;
        // printf("looping\n");
        for (int i = 0; i < numQueues; i++) {
            // printf("queue number:   %d\n", i);
            isDone[i] = boundQueueRead(isDone, i, queues[i], coEditQueues);
            if(finish && !isDone[i]) {
                finish = false;
            }
        }
    }
            char* item1 = NULL;
            char* item2 = NULL;
            char* item3 = NULL;
            item1 = malloc(strlen("Done") + 1);  // +1 for null terminator
            item2 = malloc(strlen("Done") + 1);
            item3 = malloc(strlen("Done") + 1);

            if (item1 != NULL && item2 != NULL) {
                strcpy(item1, "Done");
                strcpy(item2, "Done");
                strcpy(item3, "Done");
            }
            insertToUnBound(item1, coEditQueues[SPORTS_QUEUE]);
            insertToUnBound(item2, coEditQueues[WEATHER_QUEUE]);
            insertToUnBound(item3, coEditQueues[NEWS_QUEUE]);
            printf("out2   DONE\n");
    //when finish send 3 queues "DONE"
    free(dispArgs);
    free(isDone);
    pthread_exit(NULL);
}


void dispatcherGenerate(int threadsNum, BoundedQueue** boundQueuesArr, UnBoundQueue** unBoundQueuesArr) {
    int i;
    for (i = 0; i < CO_EDIT_QUEUES; i++)
    {
        unBoundQueuesArr[i] = unBoundQueue();
    }

    DispatcherArguments* dispatcerArgs = malloc(sizeof(DispatcherArguments));
    dispatcerArgs->coEditQueues = unBoundQueuesArr;
    dispatcerArgs->numQueues = threadsNum;
    dispatcerArgs->queues = boundQueuesArr;

    pthread_t dispatcherThread;
    pthread_create(&dispatcherThread, NULL, dispatcher, dispatcerArgs);
}


void* coEditor(void* arg){
    CoEditorArguments* coEArgs = (CoEditorArguments*)arg;
    UnBoundQueue* unBoundQueue = coEArgs->unBoundQueue;
    BoundedQueue* screenQueue = coEArgs->screenQueue;
    bool done = false;
    while (!done) {
        char* item = removeItemFromUnBound(unBoundQueue);
        if (item != NULL) {
            if (strcmp(item, "Done") == 0){
                printf("3 out     DONE\n");
                insert(item, screenQueue);
                done = true;
                break;
            }
            usleep(100000); // Wait for 0.1 second (100000 microseconds)
            insert(item, screenQueue);
        } else {
        }
    }
    free(coEArgs);
    pthread_exit(NULL);
}


void coEditorsGenerate(FILE* configFile, UnBoundQueue** unBoundQueuesArr, BoundedQueue* screenQueue) {
    

    // Transfer items from unBoundQueuesArr to screenQueue using multiple threads
    pthread_t threads[3];
    for (int i = 0; i < 3; i++) {
        CoEditorArguments* coEditArgs = malloc(sizeof(CoEditorArguments));
        coEditArgs->screenQueue = screenQueue;
        coEditArgs->unBoundQueue = unBoundQueuesArr[i];

        // Create a separate thread for each co-editor
        pthread_create(&threads[i], NULL, coEditor, coEditArgs);
    }
}


void* screenManager(void* arg) {
    ScreenManagerArguments* screArgs = (ScreenManagerArguments*)arg;
    BoundedQueue* screenQueue = screArgs->screenQueue;
    int finishCounter = 0;
    
    while (finishCounter < 3) {
        // Remove an item from the screenQueue
        while (screenQueue == NULL)
        {
            continue;
        }
        
        char* item = removeItem(screenQueue);

        // Check if the queue is empty
        if (item == NULL) {
            // Handle the case when the queue is empty (e.g., wait or exit the loop)
            continue;
        }
        
        // Check if the item is "DONE"
        if (strcmp(item, "Done") == 0) {
            // printf("out3  DONE\n");
            finishCounter++;
            free(item);
            continue;
        }
        
        // Print the item to the screen
        printf("6out   screenQueue: %s\n", item);

        // Free the memory allocated for the item
        free(item);
    }
    free(screArgs);
    pthread_exit(NULL);
}


void freeQueues(BoundedQueue** qTBQS, int threads, UnBoundQueue** qTUBQS, BoundedQueue* q) {
    int i;
    for (i = 0; i < threads; i++)
    {
        free(qTBQS[i]->articlessArr);
        free(qTBQS[i]);
    }
    free(qTBQS);
    for (i = 0; i < CO_EDIT_QUEUES; i++)
    {
        free(qTUBQS[i]);
    }
    free(qTUBQS);
    free(q->articlessArr);
    free(q);
}


int main(int argc, char* argv[]) {
    printf("started\n");
    //check if there are inough param and if the file can be opened.
    FILE* configFile;
    if (openFile(&configFile, argc, argv)) {
        return 1;
    }
    //check how many threads i will need and than creat queue array this length
    int threadsNum = numOfProducers(configFile);

    // printf("number of threads is:  %d\n", threadsNum);

    BoundedQueue** boundQueuesArr = malloc(threadsNum * sizeof(BoundedQueue*));
    producerGenerate(threadsNum, configFile,boundQueuesArr);

    //implement dispatcher and three unbound queue
    UnBoundQueue** unBoundQueuesArr = malloc(CO_EDIT_QUEUES * sizeof(UnBoundQueue*));
    dispatcherGenerate(threadsNum, boundQueuesArr, unBoundQueuesArr);

    //when read 3 failed - read only one number so generate for each N,W,S editors a bounded queue
    //imp co-editors:
    BoundedQueue* screenQueue;
    // Read the size from the last line of the configFile
    int size;
    fseek(configFile, -sizeof(int), SEEK_END);
    fread(&size, sizeof(int), 1, configFile);
    fclose(configFile);

    // Create the bounded buffer with the specified size
    screenQueue = Bounded_Buffer(size);
    coEditorsGenerate(configFile, unBoundQueuesArr, screenQueue);

    //implement screen maneger
    ScreenManagerArguments* screenArgs = malloc(sizeof(ScreenManagerArguments));
    screenArgs->screenQueue = screenQueue;
    pthread_t screenManagerThread;
    pthread_create(&screenManagerThread, NULL, screenManager, (void*)screenArgs);

    pthread_join(screenManagerThread, NULL);
    freeQueues(boundQueuesArr, threadsNum, unBoundQueuesArr, screenQueue);
    return 0;
}
