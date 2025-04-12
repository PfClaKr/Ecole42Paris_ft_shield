#ifndef SERVER_H
#define	SERVER_H

#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>

# define PORT_FORTYTWO 4242

typedef struct s_client
{
	int		fd;

}			t_client;

typedef struct s_server
{
	struct sockaddr_in	addr;
	int		port;
	int		sockfd;
	int		epollfd;
	int		max_clients;
}			t_server;

void	server();

#endif