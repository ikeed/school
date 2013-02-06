
#ifndef threadex_h
#define threadex_h

#include <iostream>
#include <cstdlib>
#include <pthread.h>
using namespace std;

// ======= CLASS TO ALLOW MULTIPLE THREADS TO STORE/RETRIEVE =======
// ======= FROM A SHARED (CIRCULAR) BUFFER OF TEMPLATED DATA =======

// Use as follows:
//
// create the shared buffer for the desired number of elements
//       of whatever data type using:
// SharedBuffer<desired_data_type> YourBufferName(desired_buffer_size);
//
// Safely extract one element from the buffer using
//    X = YourBufferName.Retrieve();
// note that if no values are in the buffer 
//      then this will block the current thread and wait for some,
//      but will not block other threads from trying to store
//      or retrieve
//
// Safely insert one element into the buffer using
//   YourBufferName.Store(desired_value);
// note that if the buffer is full
//      then this will block the current thread until space is 
//      available, but will not block other threads from trying
//      to store or retrieve

template <class DType>
class SharedBuffer {
   public:
      SharedBuffer(int s = 0); // s specified available buffer size
      ~SharedBuffer();        

      DType Retrieve();        // dequeue front item 
                               // (keeps trying til there is one)
      void Store(DType data);  // enqueue at back 
                               // (keeps trying til space available)

   private:
      DType *buffer; // pointer to allocated buffer space

      int front, back;    // positions of first and last items
      int size, current;  // total queue size, items currently enqueued 

      pthread_mutex_t buffer_access, index_access; // semaphores to
                          // restrict access to buffer storage and
                          // buffer indexes (front, back, size, current)
};

// keep trying (without blocking other threads) to store data
//    at the back of the queue
template <class DType>
void SharedBuffer<DType>::Store(DType data) 
{
      // can't store if buffer is unallocated
      if (buffer == NULL) return;

      // keep trying til successful
      bool complete = false;
      while (!complete) {
         // wait for access to the index positions
         pthread_mutex_lock(&index_access);

         // if the queue isn't full add the item
         if (current < size) {
            // adjust the current size of the queue
            current++;

            // adjust the back of queue position, 
            //    wrapping around if necessary
            int fillposition = back++;
            if (back == size) back = 0;

            // request access to the queue storage
            pthread_mutex_lock(&buffer_access);

            // release access to the queue indexes
            pthread_mutex_unlock(&index_access);

            // copy the data across
            buffer[fillposition] = data;

            // release access to the queue storage
            pthread_mutex_unlock(&buffer_access);
            complete = true;
         }

         // if queue is full then release the index positions
         else {
            // (so that sooner or later a consumer thread will
            //  be able to take something out)
            pthread_mutex_unlock(&index_access);
         }
      }
}

// keep trying (without blocking other threads) to extract
//   front item from the queue
template <class DType>
DType SharedBuffer<DType>::Retrieve() 
{
      // can't get data out if the queue is unallocated
      if (buffer == NULL) return 0;

      // keep trying til successful
      DType data;
      bool complete = false;
      while (!complete) {
         // request access to the queue index positions
         pthread_mutex_lock(&index_access);

         // if the queue isn't empty extract the front item
         if (current > 0) {
            // adjust the front of queue pointer,
            //    wrapping around if necessary
            int removeposition = front++;
            if (front == size) front = 0;

            // decrement the number of currently stored items
            current--;

            // request access to the queue storage
            pthread_mutex_lock(&buffer_access);

            // release access to the queue index positions
            pthread_mutex_unlock(&index_access);

            // copy out the data
            data = buffer[removeposition];

            // release access to the queue storage
            pthread_mutex_unlock(&buffer_access);
            complete = true;
         }

         // otherwise (queue is empty) release access
         else {
            // so a store thread can eventually enqueue something
            pthread_mutex_unlock(&index_access);
         }
      }
      // return the data extracted
      return data;
}

// destroy the queue
template <class DType>
SharedBuffer<DType>::~SharedBuffer() 
{
      // destroy the semaphores
      pthread_mutex_destroy(&index_access);
      pthread_mutex_destroy(&buffer_access);

      // delete the allocated queue storage
      delete buffer;
 }

// allocate a circular queue,
//    s specifies the maximum number of items which
//    can be stored at once
template <class DType>
SharedBuffer<DType>::SharedBuffer(int s) 
{
      // initially front and back refer to the front item,
      //    and 0 items are currently enqueued
      front = 0;
      back = 0;
      current = 0;

      // try to allocate storage for the queue
      //    and record the size allocated
      buffer = NULL;
      if (s > 0) buffer = new DType[s];
      if (buffer != NULL) size = s;
      else size = 0;

      // create semaphores for access to the buffer
      //    and for the index positions (front, back, current)
      pthread_mutex_init(&buffer_access, NULL);
      pthread_mutex_init(&index_access, NULL);

      // unlock both semaphores (make the resources available)
      pthread_mutex_unlock(&buffer_access);
      pthread_mutex_unlock(&index_access);
}

#endif

