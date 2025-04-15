#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "queue.h"


void queue_init(Queue* q)
{
    q->front = 0;
    q->rear = 0;
}

int queue_isFull(Queue* q)
{
    return (q->rear + 1) % MAX_SIZE == q->front;
}

int queue_isEmpty(Queue* q)
{
    return q->front == q->rear;
}

int queue_size(Queue* q)
{
	if (q->rear >= q->front) {
		return (q->rear - q->front);
	}
	else {
		return (MAX_SIZE - q->front + q->rear );
	}
}

int queue_enqueue(Queue* q, char value)
{
	q->items[q->rear] = value;
    q->rear = (q->rear + 1) % MAX_SIZE;

    if (q->rear == q->front) {
    	q->front = (q->front + 1) % MAX_SIZE;
    }
//    DBG("Enqueued: %d\n", value);

    return 0;
}

int queue_dequeue(Queue* q, char *pData)
{
    if (queue_isEmpty(q)) {
        printf("Queue is empty!\n");
        return 0;
    }

    char value = q->items[q->front];

    q->front = (q->front + 1) % MAX_SIZE;

    *pData = value;
//    DBG("Dequeued: %d\n", value);
    return 1;
}

void queue_display(Queue* q)
{
    if (queue_isEmpty(q)) {
        DBG("Queue is empty!\n");
        return;
    }

//    DBG("Queue: ");
    int i = q->front;
    while (1) {
        DBG("%c", q->items[i]);
        if (i == q->rear) break;
        i = (i + 1) % MAX_SIZE;
    }
    DBG("\n");
}

int queue_push(Queue* q, char *pData, int nLen)
{
	int i = 0;
	for (i=0;i<nLen;i++) {
		if ( queue_enqueue(q, pData[i]) < 0 ) {
			return -2;
		}
	}
	return i;
}

int queue_pull(Queue* q, char *pData, int nSize )
{
	int len = 0;


	if (q->front == q->rear) {
		len = 0;
	}
	else {
		if (q->rear > q->front) {
			len = q->rear - q->front + 1;
		}
		else {
			len = MAX_SIZE - q->front + q->rear;
		}
	}



	if (len < nSize) {
		printf("pull----len=%d, f=%d, r=%d\n", len, q->front, q->rear);
		return 0;
	}
	else {
		for (int i=0;i<nSize;i++) {
			queue_dequeue(q, &pData[i]);
		}
		printf("----deque f=%d, r=%d\n", q->front, q->rear);
	}

	return nSize;
}



//int main() {
//    Queue q;
//    initializeQueue(&q);
//
//    enqueue(&q, 10);
//    enqueue(&q, 20);
//    enqueue(&q, 30);
//    enqueue(&q, 40);
//    enqueue(&q, 50); // 큐가 가득 참
//    enqueue(&q, 60); // 삽입 시도 (실패)
//
//    displayQueue(&q);
//
//    dequeue(&q);
//    dequeue(&q);
//
//    displayQueue(&q);
//
//    enqueue(&q, 60);
//    enqueue(&q, 70);
//
//    displayQueue(&q);
//
//    return 0;
//}
