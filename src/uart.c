/*
 * Copyright (c) 2012 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>


#include "config.h"
#include "uart.h"
#include "debug.h"




int set_baud_rate(int ch, int baud_rate)
{
	uint32_t base_addr = 0;
	uint32_t calc_baud = 0;

	base_addr = UART_BASE + (ch << 16);
	printf("UART CH%d : 0x%x\n",ch, base_addr);
	size_t PAGESIZE = getpagesize();
    off_t aligned_base_address = base_addr & ~(PAGESIZE - 1);

	int mem_fd = open("/dev/mem", O_RDWR|O_SYNC);
	if (mem_fd == -1) {
        printf("Failed to open /dev/mem\n");
        close(mem_fd);
        return -1;
    }
	char *ptr = (char *)mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, aligned_base_address);
	if (ptr == MAP_FAILED) {
        printf("Failed to mmap /dev/mem\n");
        close(mem_fd);
        return -1;
    }

	*((volatile uint32_t *)(ptr + 0x0c)) = 0x03;
	usleep(1000);
	*((volatile uint32_t *)(ptr + 0x0c)) = 0x00;

	calc_baud = 100000000 / (baud_rate * 16);
	printf("setting value = %d\n", calc_baud);
	*((volatile uint32_t *)(ptr + 0x10)) = calc_baud;


    close(mem_fd);

	return 0;
}


int uart_init()
{
	return 0;
}


int uart_end()
{
	return 0;
}


int uart_open(sConfig *pTask)
{
	int n = 0;
	char uart_device[UART_CHANNELS];
	char uart_buffers[BUFFER_SIZE];

    snprintf(uart_device, sizeof(uart_device), "%s%d", BASE_UART_DEVICE, pTask->id);

    set_baud_rate(pTask->id, pTask->uart_rate);//ch, baud_rate

    pTask->uart_fd = open(uart_device, O_RDWR);
    if (pTask->uart_fd < 0) {
        printf("drror : opening UART device id=%d\n", pTask->id);
        close(pTask->uart_fd);
        return -1;
    }

	return 0;
}


int uart_close(sConfig *pTask)
{
	printf("close uart : fd=%d, id=%d\n", pTask->uart_fd, pTask->id);

	if (pTask->uart_fd > -1) {
		close(pTask->uart_fd);
		pTask->uart_fd = -1;
	}

	return 0;
}



int uart_read(int fd, char *pBuf, int size)
{
	ssize_t byte_read = 0;

	byte_read = read(fd, pBuf, size);

//	DBG("uard_read:fd=%d, size=%d\n", fd, byte_read);

	return byte_read;
}


