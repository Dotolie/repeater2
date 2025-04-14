#ifndef	__CONFIG_H__
#define __CONFIG_H__

#include <pthread.h>
#include <arpa/inet.h>
#include <poll.h>

#define MAX_TASK	16
#define PORT_BASE	32000

#define MAX_WORD_LENGTH 100
#define MAX_WORDS 100


typedef enum {
	BAUD_9600,
	BAUD_19200,
	BAUD_115200,
	BAUD_921600,
} BAUD_RATES;

typedef enum {
	WORD_EXIT,
    WORD_CHNO,
    WORD_VALUE,
    WORD_VERSION,
    WORD_GET,
	WORD_ENTER,
    WORD_UNKNOWN
} WordType;


typedef struct {
    char word[MAX_WORD_LENGTH];
    int num;
    WordType type;
} Word;


typedef struct {
	int	id;
	int isRun;
	int isLoop;
	int uart_fd;
	int uart_rate;
	int epoll_fd;
	int	server_fd;
	int client_fd;

	struct sockaddr_in host_addr;
	struct sockaddr_in client_addr;

	pthread_t threadId;
} sConfig;


extern sConfig task[];



int config_init();
int config_end();
int config_work();


#endif //__CONFIG_H__
