#include "threadex.h"
using namespace std;

// ======== PRODUCER AND CONSUMER THREADS =====
// ======== USING THE SHARED BUFFERS ==========
// ======== AND WITH EXCLUSIVE ACCESS =========
// ======== LOCKS ON I/O ======================


// create the shared data buffer, in this case
//    it is a circular buffer for up to 20 floats
const int BUFFERSIZE = 20;
SharedBuffer<float> Buffer(BUFFERSIZE);

// determine how many consumer and producer threads there will be,
const int NUMPRODUCERS = 5;
const int NUMCONSUMERS = 5;

// a shared (global) communication access lock
//   (so multiple threads don't try to write to 
//    the display simultaneously or read from the 
//    keyboard simultaneously)
pthread_mutex_t comm_access;

// a routine to initialize the communication lock
void  CommInit() 
{
  // initialize the communication semaphore
  pthread_mutex_init(&comm_access, NULL);

  // unlock the semaphore (make io access available)
  pthread_mutex_unlock(&comm_access);
}

// the code for an individual producer thread,
//     it determines how many values it should obtain
//        from the user,
//     then gets each value, storing it in the shared buffer
// Note the use of void* as the type for a single parameter
//     and the return value (make the parameter a struct if
//     you need to pass multiple values to the Producer)
void* Producer(void* ProducerID) 
{
   // in our example, we expect the data parameter to 
   //    represent an id for the specific producer thread
   long myid = (long)(ProducerID);


   // the thread behaviour will be to ask the user
   //     how many floats they wish to enter,
   // then we'll grab that many from them
   //     and store them in the shared buffer
   int numvalues;
   string value_entry;

   // first, request control of keyboard/display
   pthread_mutex_lock(&comm_access);

   // find out how much data they wish to enter
   cout << "This is producer " << myid;
   cout << ", how many values should I produce?" << endl;
   cin >> value_entry;

   // release control of I/O for a while
   pthread_mutex_unlock(&comm_access);

   // convert whatever the user entered into an int
   // (we read it as a string for robustness)
   numvalues = atoi(value_entry.c_str());
   
   // get each of the floats the user wishes to enter
   for (int i = 0; i < numvalues; i++) {

      // storage for the next float
      float nextvalue;

      // request control of I/O
      pthread_mutex_lock(&comm_access);

      // get the next float from the user
      cout << "Please enter value " << (i+1) << " for producer ";
      cout << myid << endl;
      cin >> value_entry;

      // release control of I/O
      pthread_mutex_unlock(&comm_access);

      // convert the value entered to a float
      // (again, we read as a string for robustness)
      nextvalue = atoi(value_entry.c_str());

      // store the value in the circular buffer
      Buffer.Store(nextvalue);
   }

   // request control of I/O
   pthread_mutex_lock(&comm_access);

   // let the user know you're done
   cout << "Producer " << myid << " complete" << endl;

   // release control of I/O and terminate the thread
   pthread_mutex_unlock(&comm_access);
   pthread_exit(NULL);
}

// the code for an individual consumer thread,
//     it determines how many values it should lookup
//        for the user,
//     then gets each value from the shared buffer
//        and displays it for the user
// Note the use of void* as the type for a single parameter
//     and the return value (make the parameter a struct if
//     you need to pass multiple values to the Producer)
void *Consumer(void* ConsumerID) 
{
   int numvalues;
   string value_entry;

   // convert the passed parameter to our userid
   long myid = (long)(ConsumerID);

   // request control of user I/O
   pthread_mutex_lock(&comm_access);

   // ask the user how many floats they wish this thread
   //     to extract from the buffer
   cout << "This is consumer " << myid;
   cout << ", how many values should I retrieve?" << endl;
   cin >> value_entry;

   // release control of user I/O
   pthread_mutex_unlock(&comm_access);

   // convert the value they entered to an int
   // (it was read as a string for robustness)
   numvalues = atoi(value_entry.c_str());
   
   // extract each of the values from the queue
   for (int i = 0; i < numvalues; i++) {

      // extract the next value from the queue
      float nextvalue = Buffer.Retrieve();

      // request control of user I/O
      pthread_mutex_lock(&comm_access);

      // inform the user of the value you extracted
      cout << "Consumer " << myid << "\'s " << (i+1);
      cout << "\'th reported value is " << nextvalue << endl;

      // release control of user I/O
      pthread_mutex_unlock(&comm_access);
   }

   // request control of user I/O
   pthread_mutex_lock(&comm_access);

   // let them know this thread is finished extracting
   cout << "Consumer " << myid << " complete" << endl;

   // release control of user I/O and terminate the thread
   pthread_mutex_unlock(&comm_access);
   pthread_exit(NULL);
}

// sample central control,
//    setting up each of the threads as well as the
//    storage buffer and communication semaphore
int main()
{
  // initialize and unlock the communication semaphore
  CommInit();

  // create storage/control info for each of the
  //    producer and consumer threads
  pthread_t producers[NUMPRODUCERS];
  pthread_t consumers[NUMCONSUMERS];

  // create storage/control info for this (main) thread
  pthread_t whoami = pthread_self();

  // variables for thread ids and status codes
  long p, c, errcode;


  // store the id's for each of the producers
  long producerids[NUMPRODUCERS];

  // for each producer, create a thread and store its id:
  for (p = 0; p < NUMPRODUCERS; p++) {
      // give it an id
      producerids[p] = p;

      // create the thread, passing
      //     the pthread_t struct
      //     the thread attributes object
      //     the void* function to be run as a thread
      //     a void* argument to be passed to the function being run
      errcode = pthread_create(&producers[p], NULL, Producer, 
                               ((void*)(producerids[p])));

      // if the creation failed then identify the error
      if (errcode != 0) {
         // request control of user I/O
         pthread_mutex_lock(&comm_access);

         // display the error message
         cout << "ERROR: producer " << p << " creation returned error code ";
         cout << errcode << endl;

         // release control of user I/O
         pthread_mutex_unlock(&comm_access);
         break;    
      }
  }

  // store the ids for each of the consumer threads
  long consumerids[NUMCONSUMERS];

  // generate each of the consumer threads
  for (c = 0; c < NUMCONSUMERS; c++) {

      // assign it an id
      consumerids[c] = c;

      // create the thread
      errcode = pthread_create(&consumers[c], NULL, Consumer, 
                               ((void*)(consumerids[c])));

      // if creation failed then generate an error message
      if (errcode != 0) {
         // request control of user I/O
         pthread_mutex_lock(&comm_access);
 
         // display the error message
         cout << "ERROR: consumer " << c << " creation returned error code ";
         cout << errcode << endl;

         // release control of user I/O
         pthread_mutex_unlock(&comm_access);
         break;    
      }
  }

  // request control of user I/O
  pthread_mutex_lock(&comm_access);

  // let the user know that all the producers and consumers
  //     have been created (or at least attempted)
  cout << "In main routine, all threads created" << endl;

  // release control of user I/O
  pthread_mutex_unlock(&comm_access);
 
  // now join main with all the other threads,
  //     so main blocks until they are ALL completed
  int status;

  // join main with each producer 
  for (p = 0; p < NUMPRODUCERS; p++) {
      errcode = pthread_join(producers[p], (void**)&status);

      // if the join fails issue an error message
      if (errcode != 0) {
         // request control of user I/O
         pthread_mutex_lock(&comm_access);

         // display the error message
         cout << "ERROR: producer " << p << " rejoin returned error code ";
         cout << errcode << endl;

         // release control of user I/O
         pthread_mutex_unlock(&comm_access);
      }
  }

  // join main with each of the consumer threads
  for (c = 0; c < NUMCONSUMERS; c++) {
      errcode = pthread_join(consumers[c], (void**)&status);

      // if the join fails generate an error message
      if (errcode != 0) {
         // request control of user I/O
         pthread_mutex_lock(&comm_access);
 
         // display the error message
         cout << "ERROR: consumer " << c << " rejoin returned error code ";
         cout << errcode << endl;

         // release control of user I/O
         pthread_mutex_unlock(&comm_access);
      }
  }

  // request control of user I/O
  pthread_mutex_lock(&comm_access);

  // let the user know that all producers and consumers are done
  cout << "In main routine, all threads completed" << endl;

  // release control of user I/O
  pthread_mutex_unlock(&comm_access);
 
  // free the communication semaphore and exit the main routine
  pthread_mutex_destroy(&comm_access);
  pthread_exit(NULL);
}
