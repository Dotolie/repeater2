#include <sys/epoll.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "debug.h"
#include "version.h"


#define FX(x)	(x * (-0.001f) + 1200)

sConfig task[MAX_TASK];
Word words[MAX_WORDS];


int config_init()
{
	for(int i=0;i<MAX_TASK;i++) {
		task[i].id = i;
		task[i].isRun = 0;
		task[i].isLoop = 0;
		task[i].uart_fd = -1;
		task[i].uart_rate = 921600;
		task[i].uart_timeout = FX(task[i].uart_rate);
		task[i].epoll_fd = -1;
		task[i].server_fd = -1;
		task[i].client_fd = -1;

		task[i].flip = 0;
		task[i].idx = 0;
	}

	return 0;
}


int config_end()
{
	return 0;
}


WordType config_classify(const char* word, char *num) {
    // 간단한 규칙으로 단어의 종류를 추정
	int len = strlen(word);
	int len_digit = 0;
	int len_alpha = 0;
	int len_etc = 0;
	int index = 0;


	for (int i=0;i<len;i++) {
		if (isdigit(word[i])) {
			len_digit++;
			num[index++] = word[i];
		}
		else if (isalpha(word[i])) {
			len_alpha++;

		}
		else {
			len_etc++;
		}
	}

	num[index] = 0;

	if ( len_alpha > 0 && len_digit > 0 ) {
		return WORD_CHNO;
	}
	else if ( len == len_digit ) {
		return WORD_VALUE;
	}
	else if( strncmp(word, "ver", 3) == 0 ) {
		return WORD_VERSION;
	}
	else if( strncmp(word, "hello", 5) == 0 ) {
		return WORD_GREETING;
	}
	else if( strncmp(word, "exit", 4) == 0 ) {
		return WORD_EXIT;
	}
	else if( strncmp(word, "?", 1) == 0 ) {
		return WORD_GET;
	}
	else if( strncmp(word, "\n", 1) == 0 ) {
		return WORD_ENTER;
	}
	else {
		return WORD_UNKNOWN;
	}

}



int config_parse(const char *text)
{
    char current_word[MAX_WORD_LENGTH];
	char number[16];
    int word_index = 0;
    int word_count = 0;

	WordType type;

    // 텍스트를 순차적으로 분석
    for (int i = 0; text[i] != '\0'; i++) {
		if ( text[i] == '=' || text[i] == ' ' || text[i] == ',') {
			current_word[word_index]='\0';

	        // 단어 종류를 분류하고 배열에 저장
	        type = config_classify(current_word, number);
	        strcpy(words[word_count].word, current_word);
	        words[word_count].type = type;
			words[word_count].num = atoi(number);
	        word_count++;
			word_index = 0;
		}
		else {
			if( text[i] >= 0x20 ) {
				current_word[word_index++] = text[i];
			}
		}
    }

    // 마지막 단어 처리
    if (word_index > 0) {
        current_word[word_index] = '\0';
        WordType type = config_classify(current_word, number);
        strcpy(words[word_count].word, current_word);
        words[word_count].type = type;
		words[word_count].num = atoi(number);
		word_count++;
    }

    // 결과 출력
//    printf("Words and their classifications:\n");
    for (int i = 0; i < word_count; i++) {
        printf("Word: %s, nu: %d, Type: ", words[i].word, words[i].num);
        switch (words[i].type) {
            case WORD_EXIT: printf("exit\n"); break;
            case WORD_CHNO: printf("channel\n"); break;
            case WORD_GREETING: printf("hello\n"); break;
            case WORD_GET: printf("getting\n"); break;
            case WORD_VERSION: printf("version\n"); break;
            case WORD_VALUE: printf("value\n"); break;
            default: printf("Unknown\n"); break;
        }
    }

	return word_count;
}

int config_work()
{
	int server_fd = -1;
	int client_fd = -1;
	int epoll_fd = -1;
	int option = 1;
	int isRun = 1;
	int isLoop = 1;
	int ret = 0;
	int len = 0;

	char outbuff[32] = {0,};
	int	chnum = 0;

	char socket_buffer[2048];
	struct sockaddr_in host_addr;
	struct sockaddr_in client_addr;

	struct epoll_event event;
	struct epoll_event events[2];

	socklen_t size = sizeof(struct sockaddr_in);

	epoll_fd = epoll_create(2);
	server_fd = socket(PF_INET,SOCK_STREAM,0);
	if (server_fd < 0) {
		printf("error:server socker error\n");
		return -1;
	}

	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

	memset(&(host_addr), 0, sizeof(struct sockaddr_in));
    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT_BASE + 100);
    host_addr.sin_addr.s_addr = 0;


    if (bind(server_fd, (struct sockaddr *)&(host_addr), sizeof(struct sockaddr)) < 0) {
    	printf("error: server bind error\n");
    	return -2;
    }


	isRun = 1;
    while(isRun) {
        listen(server_fd, 3);
        client_fd = accept(server_fd, (struct sockaddr *)&(client_addr), &size);

        // epoll add for socket
        event.events = EPOLLIN ;
    	event.data.fd = client_fd;
    	ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD,  client_fd, &event);
        DBG("client fd=%d, ip=%s, port=%d\n", client_fd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

//        send(client_fd, "connected\n", 10, 0);

        isLoop = 1;
    	while(isLoop) {
    		ret = epoll_wait(epoll_fd, events, 1, -1);

// 			DBG("poll :i=%d, fd=%d\n", i, events[i].data.fd);

    		if ( events[0].data.fd == client_fd ) {
    			len = recv(client_fd, socket_buffer, 2048, 0);
    			if (len > 0) {
    				socket_buffer[len] = 0;
    				ret = config_parse(socket_buffer);
    				for (int i=0;i<ret;i++) {
//    					printf("i=%d,type=%d,%s,ch=%d\n", i, words[i].type, words[i].word, words[i].num);
    					switch(words[i].type) {
							case WORD_EXIT:
								send(client_fd, "bye bye !\n", 10, 0);
								isLoop = 0;
								isRun = 0;
								ret = 0;	// for exit
								break;
							case WORD_GREETING:
								send(client_fd, "Hi", 2, 0);
								break;
							case WORD_CHNO:
								sprintf(outbuff,"%s=%d\n", words[i].word, task[words[i].num].uart_rate);
								chnum = words[i].num;
								break;
							case WORD_VALUE:
								task[chnum].uart_rate = words[i].num;
								task[chnum].uart_timeout = FX(words[i].num);
								DBG("set ch=ch%d rate=%d, timeout=%d\n", chnum, words[i].num, task[chnum].uart_timeout);
								break;
							case WORD_GET:
								send(client_fd, outbuff, strlen(outbuff), 0);
								break;
							case WORD_VERSION:
								sprintf(outbuff,"ver=%s\n", VER);
								break;
    					}
    				}
    				DBG("socket:fd=%d,l=%d,ret=%d,%s\n", client_fd, len, ret, socket_buffer);
    			}
    			else {
    				DBG("socket:len=%d, close client fd=%d\n", len, client_fd);
    				isLoop = 0;
    			}
    		}
    	}

    	// epoll delete for socket
    	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);

    	close(client_fd);
    	client_fd = -1;

    	DBG("thread loop exit\n");
    }

    return ret;
}
