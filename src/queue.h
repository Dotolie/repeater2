#ifndef	__QUEUE_H__
#define __QUEUE_H__

#define MAX_SIZE 8192 // 큐의 크기 정의

typedef struct {
    char items[MAX_SIZE];
    int front, rear;
} Queue;


void queue_init(Queue* q);
int queue_isFull(Queue* q);
int queue_isEmpty(Queue* q);
int queue_size(Queue* q);
int queue_enqueue(Queue* q, char value);
int queue_dequeue(Queue* q, char *value);
void queue_display(Queue* q);

int queue_push(Queue* q, char *pData, int nLen);
int queue_pull(Queue* q, char *pData, int nSize);

#endif	// __QUEUE_H__
