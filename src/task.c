#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>

#include "config.h"
#include "debug.h"
#include "uart.h"
#include "task.h"






static inline void sending(sConfig *p, char *data, int n)
{
	static int	flip = 0;
	static int 	idx = 0;
	static char buf[2][SIZE_PKT];

	if (n > 0) {
		for(int k=0;k<n;k++) {
			buf[flip][idx] = data[k];
			idx++;
			if (idx >= SIZE_PKT) {
				send(p->client_fd, buf[flip], SIZE_PKT, 0);
				flip = (flip + 1) % 2;
				idx = 0;
			}
		}
	}
	else {
		if (idx != 0) {
			send(p->client_fd, buf[flip], idx, 0);
			flip = (flip + 1) % 2;
			idx = 0;
		}
	}
}


void *task_worker(void *pArg)
{
	int option = 1;
	int ret = -1;
	int len = 0;
	int lenS = 0;
	int n = 0;
	int timeout = 100;


	char uart_buffer[SIZE_BUF_U];
	char socket_buffer[SIZE_BUF_S];

	struct epoll_event event;
	struct epoll_event events[SIZE_EPOLL];

	sConfig *p = (sConfig*)pArg;
	socklen_t size = 0;


	size = sizeof(struct sockaddr_in);

	p->epoll_fd = epoll_create(SIZE_EPOLL);
	p->server_fd = socket(PF_INET,SOCK_STREAM,0);
	if (p->server_fd < 0) {
		printf("error:server socker error\n");
		return NULL;
	}

	fcntl(p->server_fd, F_SETFL, O_NONBLOCK);

	setsockopt( p->server_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

	memset(&(p->host_addr), 0, sizeof(struct sockaddr_in));
    p->host_addr.sin_family = AF_INET;
    p->host_addr.sin_port = htons(PORT_BASE + p->id);
    p->host_addr.sin_addr.s_addr = 0;


    if (bind(p->server_fd, (struct sockaddr *)&(p->host_addr), sizeof(struct sockaddr)) < 0) {
    	printf("error: server bind error\n");
    	return NULL;
    }


	p->isRun = 1;
    while(p->isRun) {
        listen(p->server_fd, 3);

        p->client_fd = accept(p->server_fd, (struct sockaddr *)&(p->client_addr), &size);
        if (p->client_fd < 0) {
//        	DBG("accept:wait fd=%d\n", p->server_fd);
        	usleep(100000);
        	continue;
        }
        // epoll add for socket
        event.events = EPOLLIN ;
    	event.data.fd = p->client_fd;
    	ret = epoll_ctl(p->epoll_fd, EPOLL_CTL_ADD,  p->client_fd, &event);
        DBG("client fd=%d, ip=%s, port=%d\n", p->client_fd, inet_ntoa(p->client_addr.sin_addr), ntohs(p->client_addr.sin_port));

//        send(p->client_fd, "connected\n", 10, 0);

        ret = uart_open(p);
        if (ret==0) {
            // epoll add for uart
            event.events = EPOLLIN ;
        	event.data.fd = p->uart_fd;
        	ret = epoll_ctl(p->epoll_fd, EPOLL_CTL_ADD,  p->uart_fd, &event);
        	timeout = p->uart_timeout;
        	p->isLoop = 1;
        }
        else {
        	printf("error: can't uart[%d]\n", p->id);
        	p->isLoop = 0;
        }


    	while(p->isLoop) {
    		ret = epoll_wait(p->epoll_fd, events, SIZE_EPOLL, -1);
    		if (ret < 0 ) {
    			DBG("epoll wait:ret=%d\n", ret);
    			continue;
    		}
    		for(int i=0;i<ret;i++) {
//    			DBG("poll :i=%d, fd=%d\n", i, events[i].data.fd);
    			if (events[i].data.fd == p->uart_fd) {
    				len = uart_read(p->uart_fd, uart_buffer, SIZE_BUF_U);
    				if (len > 0) {
    					sending(p, uart_buffer, len);
    				}
    				else {
    					timeout--;
    					if (timeout <= 0) {
    						sending(p, uart_buffer, len);
    						timeout = p->uart_timeout;
    					}
    					else {
    						usleep(1000);
    					}
    				}
    			}
    			else if (events[i].data.fd == p->client_fd) {
    				lenS = recv(p->client_fd, socket_buffer, SIZE_BUF_S, 0);
    				if (lenS > 0) {
//    					DBG("socket:fd=%d,l=%d\n",p->client_fd, len);
    					n = write(p->uart_fd, socket_buffer, lenS);
    				}
    				else {
    					DBG("socket:len=%d, close client fd=%d\n", len, p->client_fd);
    					p->isLoop = 0;
    				}
    			}
    		}
    	}

    	close(p->client_fd);
    	// epoll delete for socket
    	epoll_ctl(p->epoll_fd, EPOLL_CTL_DEL, p->client_fd, NULL);
    	p->client_fd = -1;


		uart_close(p);
		// epoll delete for uart
		epoll_ctl(p->epoll_fd, EPOLL_CTL_DEL, p->uart_fd, NULL);


    	DBG("thread loop exit: id=%d\n", p->id);
    }

	DBG("-----task exit----\n");


	pthread_exit(NULL);
}


int task_init()
{
	int ret = 0;
	for( int i=0;i<SIZE_TASK;i++) {
		ret = task_create(&task[i]);
		if ( ret != 0 ) {
			DBG("error: fail tast_start[%d]=%d\n", i, ret);
			return -1;
		}
	}

	return 0;
}


int task_end()
{
	int ret = 0;
	for( int i=0;i<SIZE_TASK;i++) {

		task[i].isLoop = 0;
		task[i].isRun = 0;

		ret = task_join(&task[i]);
		if ( ret != 0 ) {
			DBG("error: fail tast_join[%d]=%d\n", i, ret);
			return -1;
		}
	}

	return 0;
}


int task_create(sConfig *pTask)
{
	if(pthread_create( &(pTask->threadId), NULL, task_worker, pTask) != 0) {
		DBG("thread create error\n");
		return -1;
	}

	return 0;
}


int task_join(sConfig *pTask)
{
	DBG("id=%d, threadId=%d\n", pTask->id, pTask->threadId);

	pthread_join(pTask->threadId, NULL); //1번 스레드 자원 회수

	return 0;
}

