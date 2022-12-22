#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdint.h>

#define QUEUE_CAP (5)
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

// Dummy Stream off data to simulate the accelerometer
uint8_t queue_stream[55] = {1,2,3,4,5,6,7,8,9,8,7,6,5,4,3,2,1,2,3,4,5,5,6,6,6,6,6,5,4,3,2,1,2,3,4,5,6,7,8,8,9,9,9,8,7,6,5,4,3,2,1,2,3,4,5};

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