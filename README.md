# OS3
Article Processing and Distribution System

This program implements a multi-threaded system for managing articles and distributing them to different queues for processing.
The program includes the following components:

# 1. Bounded Queue:
   - A data structure representing a bounded buffer with thread-safe access.
   - Supports insertion and removal of articles from the buffer.
   - Uses semaphores and mutexes for synchronization.

# 2. Unbounded Queue:
   - A data structure representing an unbounded queue for storing articles.
   - Supports insertion and removal of articles from the queue.
   - Uses mutexes for synchronization.

# 3. Producer Threads:
   - Generate articles with random types and store them in bounded queues.
   - Each producer thread has a specified number of articles to generate.
   - Articles are randomly assigned a type (SPORTS, WEATHER, or NEWS).
   - Articles are inserted into the corresponding bounded queue.

# 4. Dispatcher Thread:
   - Manages the distribution of articles from bounded queues to unbounded queues.
   - Receives articles from bounded queues and inserts them into the appropriate unbounded queue based on their type.
   - Uses mutexes for synchronization.

# 5. Co-Editor Threads:
   - Retrieve articles from unbounded queues and process them.
   - Each co-editor thread is assigned to a specific unbounded queue.
   - Retrieves articles from the assigned unbounded queue and performs processing on them.

# 6. Screen Manager Thread:
   - Retrieves articles from the screen queue and displays them on the screen.
   - The screen queue contains articles that are ready to be displayed.

# Main Functions and Data Structures:

- BoundedQueue: Represents a bounded buffer with thread-safe access.
- UnBoundQueue: Represents an unbounded queue for storing articles.
- ProducerArguments: Contains arguments for the producer thread.
- DispatcherArguments: Contains arguments for the dispatcher thread.
- CoEditorArguments: Contains arguments for the co-editor thread.
- ScreenManagerArguments: Contains arguments for the screen manager thread.

# Usage:

1. Compile the program with the Makefile: "make"
2. run the program: "ex3.out conf.txt"
   - Provide a configuration file as an argument. The configuration file specifies the number of articles to generate and the size of        the bounded queue.

Note: This README provides a high-level overview of the code. Please refer to the source code for detailed implementation and function descriptions.


