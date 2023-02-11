/**
 * Author: Fábio Carneiro
 * Author Email: fabiolucas.carneiro@gmail.com
 * Author GitHub: https://github.com/fabiocfabini
 * Created:   02.11.2022
 *  
 *
 * MIT License
 * 
 * Copyright (c) 2022 Fábio Carneiro
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * 
 * STB LIBRARY DESCRIPTION:
 * 
 * This library calculates a sized moving average of a given stream of values. The interface is minimal. One must only initialize the Queue objects in the main program followed by the use of get_next_value.
 **/

#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdint.h>

#define QUEUE_CAP (3)
typedef struct queue {
    uint8_t is_active: 1;      // (1b) This is a flag to indicate if has any data

    uint8_t start;             // (1B) This is the start index of the queue 
    uint8_t end;               // (1B) This is the end index of the queue
    uint8_t size;              // (1B) This is the number of elements in the queue
    uint16_t sum;               // (1B) This is the sum of all elements in the queue
    uint8_t data[QUEUE_CAP];   // (1B) This is the data array
}Queue;                     // The size of this struct in bytes is: 4 + QUEUE_CAP + 0.1

/*
 * @brief This function initializes the queue
 *
 * @param q This is the queue to be initialized
*/
void init_queue(Queue *q);

/*
 * @brief 
 *      This function will add a new element to the queue if there is space
 *      and update the queue's sum and size.
 *
 * @param    q The queue to add the element to
 * @param data The value to add to the queue
*/
void enqueue(Queue *q, uint8_t data);

/*
 * @brief 
 *      This function will remove the oldest element from the queue if there is
 *      data and update the queue's sum and size.
 *
 * @param    q The queue to remove the element from
 * @param data The removed value
*/
void dequeue(Queue *q, uint8_t *data);

/*
 * @brief 
 *      This function recieves the accelerometer data, stores it in a queue and returns the average
 *      of all the data in the queue. If the queue is full, the oldest data is removed and the new data
 *      is added to the queue.
 * 
 * @param    q  The queue to store the data in. This can be any of the three queues.
 * @param data  The byte of data to be stored in the queue. This byte came from the accelerometer.
 * 
 * 
 * @return   The average of all the data in the queue.
 */
uint8_t get_next_value(Queue* q, uint8_t data);

#endif //_QUEUE_H_

#ifdef QUEUE_IMPLEMENTATION

static Queue Q_X = {0};     // Global Queue For Accelerometer X axis
static Queue Q_Y = {0};     // Global Queue For Accelerometer Y axis
static Queue Q_Z = {0};     // Global Queue For Accelerometer Z axis

void init_queue(Queue *q) {
    q->start = 0;
    q->end = 0;
    q->sum = 0;
    q->size = 0;
}

void enqueue(Queue *q, uint8_t data) {
    if (q->size < QUEUE_CAP) {
        q->data[q->end] = data;
        q->sum += data;
        q->end = (q->end + 1) % QUEUE_CAP;
        q->size++;
    }
}

void dequeue(Queue *q, uint8_t *data) {
    if (q->size > 0) {
        *data = q->data[q->start];
        q->sum -= *data;
        q->start = (q->start + 1) % QUEUE_CAP;
        q->size--;
    }
}

uint8_t get_next_value(Queue* q, uint8_t data) {
#ifdef __QMA
    uint8_t average = 0;

    /*
    At this point the queue can have at most (QUEUE_CAP - 1) elements in it.
    The value of sum is the sum of all the elements in the queue.
    */
    enqueue(q, data);

    /*
    If q->is_active is 0, it means we are reading the accelerometer for the first time. Therefore, we
    we don't have enough data to calculate the average. We return the first value in the queue, set
    q->is_active to 1 and q->sum to the first value in the queue.
    */
    if (!q->is_active) {
        q->is_active = 1;
        return q->sum;
    }

    /*
    If we reach this point, then there are 2 possibilities:
    1. The queue is full. 
    2. The queue is not full.

    In case 1, we calculate the average of all the data in the queue (q->sum/q->size), remove the oldest
    data from the queue and return the average.

    In case 2, we calculate the average of all the data in the queue (q->sum/q->size) and return the average.
    */
    if (q->size == QUEUE_CAP) {
        average = q->sum / QUEUE_CAP;
        dequeue(q, &data);
    }else {
        average = q->sum / q->size;
    }

    /*
    At this point the queue can have at most (QUEUE_CAP - 1) elements in it.
    The value of q->sum is the sum of all the data in the queue.
    */
    return average;
#else
    return data;
#endif
}

#endif // QUEUE_IMPLEMENTATION
