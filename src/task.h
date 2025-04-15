#ifndef	__TASK_H__
#define __TASK_H__

#define SIZE_PKT	256
#define	SIZE_BUF_S	8192
#define	SIZE_BUF_U	8192
#define	SIZE_EPOLL	2
#define	SIZE_TASK	16

#define TIMEOUT		8000

int task_init();
int task_end();
int task_isRun();
int task_stop();
int task_create(sConfig *pTask);
int task_join(sConfig *pTask);


#endif // __TASK_H__
