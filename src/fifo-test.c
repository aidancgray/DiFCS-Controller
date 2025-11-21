// #include "fifo.h"
/*!
   \@file Fifo.h
   \@brief header file for spi fifo buffer
   \@author Jason Berger
   \@date 05/03/2017
*/

// #pragma once

#include <stdint.h>

#define FIFO_OK 0
#define FIFO_OVERFLOW -1
#define FIFO_UNDERFLOW -2
/**
 *@brief creates fifo
 *@param name : name of fifo buffer
 *@param len : max number of elements in buffer
 *@param type : datatype
 *@return "Return of the function"
 */
#define FIFO_DEF(name,len,type)     \
  type name##_data[len];            \
  fifo_t name = {                      \
        .mBuffer = name##_data,        \
        .mHead = 0,                 \
        .mTail = 0,                 \
        .mMaxLen = len,             \
        .mCount = 0,                \
        .mObjSize = sizeof(type)	\
    };

typedef struct {
    uint8_t *  mBuffer; //pointer to data buffer
    int mHead;      //next address to write to
    int mTail;      //next address to read from
    int mCount;     //number of items in fifo
    int mObjSize;   //size of obj in buffer
    int mMaxLen;
    uint8_t mLock;     //lock for fifo that is accessed by multiple threads
} fifo_t;


/**
 * @brief initialize fifo
 * @param pFifo ptr to fifo_t
 * @param depth number of items to to store in fifo
 * @param width size of items in fifo (in bytes)
 */
void fifo_init(fifo_t* pFifo, int depth, int width);

/**
 * @brief deinitialize fifo
 * @param pFifo ptr to fifo
 */
void fifo_deinit(fifo_t* pFifo);

/**
 * @brief push a single 'object' into the fifo
 * @param pFifo ptr to fifo
 * @param data ptr to obj to add to fifo
 * @return FIFO_OK on success  
 */
int fifo_push(fifo_t* pFifo, void* data);

/**
 * @brief pops the next obj from the fifo
 * 
 * @param pFifo ptr to fifo
 * @param data ptr to store obj
 * @return FIFO_OK on success  
 */
int fifo_pop(fifo_t* pFifo, void* data);

/**
 * @brief push a buffer of objects into fifo
 * @param pFifo ptr to fifo
 * @param data ptr to objects to add to fifo
 * @param len number of objects to add to fifo
 * @return FIFO_OK on success 
 */
int fifo_push_buf(fifo_t* pFifo, void* data, int len);

/**
 * @brief retrieve a buffer of objects from the fifo
 * @param pFifo ptr to fifo
 * @param data ptr to store objects
 * @param len number of objects to retrieve
 * @return FIFO_OK on success 
 */
int fifo_pop_buf(fifo_t* pFifo, void* data, int len);

/**
 * @brief deletes objects from fifo
 * @param pFifo ptr to fifo
 * @param len number of objects to remove
 * @return int 
 */
int fifo_clear(fifo_t* pFifo, int len);


int fifo_peek(fifo_t* pFifo, void* data, int idx);
int fifo_peek_buf(fifo_t* pFifo, void* data, int len);
int fifo_checksum(fifo_t* pFifo, int offset, int len);
//---------------------------------------------------------------------------//
/*!
   \@file Fifo_c.case
   \@brief pure C implementation of fifo
   \@author Jason Berger
   \@date 05/03/2017
*/
#include <stdlib.h>
#include <string.h>

#define FIFO_LOCK //pFifo->mLock =1//while(pFifo->lock){delay_ms(1);} pFifo->lock = 1
#define FIFO_UNLOCK //pFifo->mLock = 0

#define fifo_min(X,Y) (((X) < (Y)) ? (X) : (Y))
#define fifo_max(X,Y) (((X) > (Y)) ? (X) : (Y))


void fifo_init(fifo_t* pFifo, int depth, int width)
{
  pFifo->mBuffer = (uint8_t*) malloc(depth * width);
  pFifo->mHead = 0;
  pFifo->mTail = 0;
  pFifo->mMaxLen = depth;
  pFifo->mCount = 0;
  pFifo->mObjSize = width;
  pFifo->mLock = 0;
}

void fifo_deinit(fifo_t* pFifo)
{
  free(pFifo->mBuffer);
}

int fifo_push( fifo_t* pFifo, void* data)
{
	FIFO_LOCK;

    if (pFifo->mCount >= pFifo->mMaxLen) /* check if fifo is full */
    {
		    FIFO_UNLOCK;
        return FIFO_OVERFLOW;
    }

    int next = pFifo->mHead + 1;                          /*next is where head will point to after this write*/

    if (next >= pFifo->mMaxLen)                           /* wrap head at end of buffer*/
    {
        next = 0;
    }

    pFifo->mCount++;                                      /* increment count*/

    int addr = pFifo->mHead * pFifo->mObjSize;            /* get address of current object*/
    memcpy(&pFifo->mBuffer[addr], data, pFifo->mObjSize); /* Copy object into next     */


    pFifo->mHead = next;                                  /* set head to next */

    FIFO_UNLOCK;

    return FIFO_OK;  // return success to indicate successful push.
}

int fifo_pop( fifo_t* pFifo, void* data)
{
	FIFO_LOCK;
    // if the head isn't ahead of the tail, we don't have any characters
    if (pFifo->mCount <= 0) // check if circular buffer is empty
    {
		    FIFO_UNLOCK;
        return FIFO_UNDERFLOW;          // and return with an error
    }

    // next is where tail will point to after this read.
    int next = pFifo->mTail + 1;
    if (next >= pFifo->mMaxLen)
    {
        next = 0;
    }
    int addr = pFifo->mTail * pFifo->mObjSize;
    memcpy(data,&pFifo->mBuffer[addr],pFifo->mObjSize);
    pFifo->mTail = next;             // tail to next data offset.

    if(pFifo->mCount > 0)
    {
        pFifo->mCount--;
    }
    FIFO_UNLOCK;

    return FIFO_OK;  // return success to indicate successful push.
}

int fifo_push_buf( fifo_t* pFifo, void* data, int len)
{
  int result = 0;
  uint8_t* cast = (uint8_t*) data;
  for(int i=0; i < len; i++)
  {
    result |= fifo_push(pFifo,&cast[i * pFifo->mObjSize]);
  }
  return result;
}

int fifo_pop_buf( fifo_t* pFifo, void* data, int len)
{
  int result = 0;
  uint8_t* cast = (uint8_t*) data;
  for(int i=0; i < len; i++)
  {
    result |= fifo_pop(pFifo, &cast[i * pFifo->mObjSize]);
  }
  return result;
}

int fifo_clear( fifo_t* pFifo, int len)
{
  //create trash bin based on objsize
  uint8_t* trash = (uint8_t*)malloc(pFifo->mObjSize);

  if(len > pFifo->mCount)
    len = pFifo->mCount;

  //there are more effecient ways to do this
  //but it should be a rare case and this utilizes locks in place
  while(len > 0)
  {
    fifo_pop(pFifo,trash);
    len--;
  }
  //free up  trash memory
  free(trash);

  return len;
}

int fifo_peek( fifo_t* pFifo, void* data, int idx)
{
  FIFO_LOCK;
  if(pFifo->mCount <= idx)
  {
        FIFO_UNLOCK;
        return -1;
  }

  int addr = pFifo->mTail + idx; //

  //handle buffer wrapping
  if(addr > pFifo->mMaxLen)
  {
      addr -= pFifo->mMaxLen;
  }

  memcpy(data,&pFifo->mBuffer[addr* pFifo->mObjSize],pFifo->mObjSize);
  FIFO_UNLOCK;
  return 0;  // return success to indicate successful push.
}

int fifo_peek_buf(fifo_t* pFifo, void* data, int len)
{

  FIFO_LOCK;
  int addr = pFifo->mTail;
  uint8_t* cast = (uint8_t*) data;

  len = fifo_min(pFifo->mCount, len);

  len*=pFifo->mObjSize;

  for(int i=0; i < len; i++)
  {
    cast[i] =  pFifo->mBuffer[addr++];

    //wrap;
    if(addr == pFifo->mMaxLen)
    {
      addr =0;
    }
  }

  //we return as an int so we can send -1 to indicate there isnt enough data
  FIFO_UNLOCK;
  return len;
}

int fifo_checksum(fifo_t* pFifo, int offset,  int len)
{
  FIFO_LOCK;
  uint16_t sum = 0;
  int addr = pFifo->mTail + offset;

  if(pFifo->mCount < (len + offset))
  {
    FIFO_UNLOCK;
    return -1;
  }

  for(int i=0; i < len; i++)
  {
    sum+= pFifo->mBuffer[addr++];

    //wrap
    if(addr == pFifo->mMaxLen)
    {
      addr =0;
    }
  }


  //we return as an int so we can send -1 to indicate there isnt enough data
  FIFO_UNLOCK;
  return (int) sum;
}
//---------------------------------------------------------------------------//
#include <stdio.h>

#define BUFFER_SIZE 10

fifo_t myFifo;
int testData[] = {3, 5, 4, 1, 47, 2, 6, 8, 7, 9};
int peekBuf[BUFFER_SIZE];
int peekData;

float average=0;

// int compar(const void *arg1, const void *arg2);

int compar(const void *arg1, const void *arg2){
    if ((* (int *) arg1) < (* (int *) arg2)) return -1;
    else if ((* (int *) arg1) == (* (int *) arg2)) return 0;
    else return 1;
}

void main(void)
{
    fifo_init(&myFifo, BUFFER_SIZE, sizeof(int));
    
    for(int i = 0; i < (sizeof(testData)/sizeof(*testData)); i++){
        fifo_push(&myFifo, &testData[i]);
    }
    
    fifo_peek_buf(&myFifo, &peekBuf, BUFFER_SIZE);

    for(int i=0; i<BUFFER_SIZE; i++){
        fifo_peek(&myFifo, &peekData, i);
        *(peekBuf+i) = peekData;
    }
    
    qsort (peekBuf, sizeof(peekBuf)/sizeof(*peekBuf), sizeof(*peekBuf), compar);

    for(int i=2; i<BUFFER_SIZE-2; i++){
        average += *(peekBuf+i);
    }

    average /= BUFFER_SIZE-4;
    printf("AVG: %.3f\n", average);

}