#include <netinet/in.h>
#include <stdio.h>      /* for snprintf */
#include <stdlib.h>
#include "csapp.h"
#include "calc.h"
#include <pthread.h>

#define LINEBUF_SIZE 1024

struct ConnInfo {
	int clientfd;
	struct Calc *calc;
};

int chat_with_client(struct Calc *calc, int infd, int outfd) {
	rio_t in;
	char linebuf[LINEBUF_SIZE];

	/* wrap standard input (which is file descriptor 0) */
	rio_readinitb(&in, infd);

	/*
	 * Read lines of input, evaluate them as calculator expressions,
	 * and (if evaluation was successful) print the result of each
	 * expression.  Quit when "quit" command is received.
	 */
	int done = 0;
	while (!done) {
		ssize_t n = rio_readlineb(&in, linebuf, LINEBUF_SIZE);
		if (n <= 0) {
			/* error or end of input */
			done = 2;
		} else if (strcmp(linebuf, "quit\n") == 0 || strcmp(linebuf, "quit\r\n") == 0) {
			/* quit command */
			done = 1;
		} else if (strcmp(linebuf, "shutdown\n") == 0 || strcmp(linebuf, "shutdown\r\n") == 0){
			/* shutdown command */
			done = 2;
		} else {
			/* process input line */
			int result;
			if (calc_eval(calc, linebuf, &result) == 0) {
				/* expression couldn't be evaluated */
				rio_writen(outfd, "Error\n", 6);
			} else {
				/* output result */
				int len = snprintf(linebuf, LINEBUF_SIZE, "%d\n", result);
				if (len < LINEBUF_SIZE) {
					rio_writen(outfd, linebuf, len);
				}
			}
		}
	}

	return done;
}

void *worker(void *arg) {
	struct ConnInfo *info = arg;

	pthread_detach(pthread_self());

	chat_with_client(info->calc ,info->clientfd, info->clientfd);
	close(info->clientfd);
	free(info);

	return NULL;
}


int main(int argc, char **argv) {
	/* TODO: implement this program */
	if (argc != 2) {
    	printf("Usage: ./server <port>\n");
		return 1;
  	}
	  
	char* port = argv[1];
	int socket = Open_listenfd(port);
	struct Calc *calc = calc_create();

	while(1) {
		int client = Accept(socket, NULL, NULL);

		struct ConnInfo *info = malloc(sizeof(struct ConnInfo));
		info->clientfd = client;
		info->calc = calc;

		pthread_t thr_id;
		if (pthread_create(&thr_id, NULL, worker, info) != 0) {
			printf("pthread_create failed\n");
			return 1;
		}

	}

	calc_destroy(calc);
	return 0;
}
