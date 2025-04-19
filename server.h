#ifndef SERVER_H
#define SERVER_H

#include "shell.h"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>

#define PORT_FORTYTWO 4242
#define PASSWORD "4242"
#define SOCK_MAX 32
#define MAX_CLIENT 3

typedef struct s_server
{
	struct sockaddr_in addr;
	// bool request_shell[1024];
	// bool connected[1024];
	int port;
	int sockfd;
	int epollfd;
	int max_clients;
	int current_client;
} t_server;

typedef struct s_clients
{
	int num;			  // currently connected clients
	int list[MAX_CLIENT]; // list of clients
	int auth[MAX_CLIENT]; // list of authorized clients
	int new;			  // new connected index
	int free;			  // last disconnected index
	int to_shell[MAX_CLIENT];
	int from_shell[MAX_CLIENT];
	int shell_pid[MAX_CLIENT];
	int active[MAX_CLIENT];
} t_clients;

void server();
int pre_shell(int fd, int epollfd, t_clients *clients);
void shell(int fd, int epollfd, t_clients *clients);
void process_input(char *str);
void remove_client(int fd, int epollfd, t_clients *clients);
int get_client_index(int fd, t_clients *clients);

#endif
