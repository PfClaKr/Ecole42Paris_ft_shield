#include "server.h"

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

int is_shell_running(int fd, t_clients *clients)
{
	int index = get_client_index(fd, clients);
	if (index == -1)
		return false;
	if (clients->active[index] == 1)
		return true;
	return false;
}

int get_index_shellfd(int fd, t_clients *clients)
{
	for (int i = 0; i < MAX_CLIENT; i++)
	{
		if (clients->from_shell[i] == fd)
			return i;
	}
	return -1;
}

bool is_shell(int fd, t_clients *clients)
{
	int index = get_index_shellfd(fd, clients);
	if (index == -1)
		return false;
	if (clients->active[index] == 1)
		return true;
	return false;
}

void init_clients(t_clients *clients)
{
	clients->num = 0;
	clients->new = -1;
	clients->free = 0;
	memset(clients->list, 0, sizeof(clients->list));
	memset(clients->auth, 0, sizeof(clients->auth));
	memset(clients->to_shell, 0, sizeof(clients->to_shell));
	memset(clients->from_shell, 0, sizeof(clients->from_shell));
	memset(clients->shell_pid, 0, sizeof(clients->shell_pid));
	memset(clients->active, 0, sizeof(clients->active));
}

int add_client(int epollfd, int sockfd, t_clients *clients)
{
	int fd = accept(sockfd, NULL, NULL);
	if (fd < 0)
	{
		// printf("accept failed\n");
		return -1;
	}
	int index = get_client_index(fd, clients);
	// printf("clients num: %d, index %d\n", clients->num, index);
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
			// printf("epoll_ctl failed\n");
			return -1;
		}
		if (index == -1)
		{
			clients->list[clients->free] = fd;
			clients->auth[clients->free] = 0;
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
	{
		// printf("remove_client: client not found\n");
		return;
	}
	// printf("closing fd: %d", clients->list[index]);
	epoll_ctl(epollfd, EPOLL_CTL_DEL, clients->list[index], NULL);
	epoll_ctl(epollfd, EPOLL_CTL_DEL, clients->from_shell[index], NULL);
	shutdown(clients->list[index], SHUT_WR);
	close(clients->list[index]);
	close(clients->from_shell[index]);
	clients->list[index] = 0;
	clients->auth[index] = 0;
	clients->active[index] = 0;
	clients->shell_pid[index] = 0;
	clients->to_shell[index] = 0;
	clients->from_shell[index] = 0;
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
		process_input(buffer);			   // if it ends with newline
		buffer[sizeof(buffer) - 1] = '\0'; // guarantee null termination
		// printf("check_keycode: buffer: %s\n", buffer);
		if (strcmp(base64_encode(buffer, strlen(buffer), buffer), PASSWORD) == 0)
			return 1;
		else
			return 0;
	}
	else if (len == 0)
	{
		// printf("check_keycode: client disconnect\n");
		remove_client(fd, epollfd, clients);
		return -2;
	}
	else if (len < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			// no data yet - non blocking
			// printf("check_keycode: no data (non blocking)\n");
			return 0;
		}
		else
		{
			// printf("check_keycode: read error\n");
			remove_client(fd, epollfd, clients);
			return -3;
		}
	}
	return 0;
}

void server()
{
	struct sockaddr_in server_addr;
	int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (sockfd < 0)
	{
		// printf("sockfd invalid\n");
		return;
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(4242);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	int opt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		// printf("bind invalid\n");
		return;
	}
	listen(sockfd, SOCK_MAX);
	int epollfd = epoll_create1(EPOLL_CLOEXEC);
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = sockfd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev) == -1)
	{
		// printf("epollfd invalid\n");
		return;
	}
	struct epoll_event events[SOCK_MAX];
	t_clients clients;
	init_clients(&clients);
	while (1)
	{
		// printf("epoll_wait block\n");
		int nfds = epoll_wait(epollfd, events, SOCK_MAX, -1);
		// printf("epoll_wait block release\n");
		for (int i = 0; i < nfds; i++)
		{
			if (events[i].events & EPOLLIN)
			{
				int event_fd = events[i].data.fd;
				// printf("fd: %d\n", event_fd);
				if (event_fd == sockfd)
				{
					int cl_state = add_client(epollfd, sockfd, &clients);
					if (cl_state < 0)
					{
						// printf("socket error\n");
						return;
					}
					if (cl_state == 0)
					{
						// printf("client added successfully\n");
						// printf("1 Keycode asked\n");
						write(clients.list[clients.new], "Keycode: ", 9);
					}
					if (cl_state == 1)
					{
						// printf("max client reached. rejecting\n");
						;
					}
				}
				else
				{
					// printf("1 list: %d %d %d\n", clients.list[0], clients.list[1], clients.list[2]);
					// printf("1 auth: %d %d %d\n", clients.auth[0], clients.auth[1], clients.auth[2]);
					if (is_auth(event_fd, &clients) == 1)
					{
						if (is_shell_running(event_fd, &clients) == 1)
						{
							// printf("shell running\n");
							int index = get_client_index(event_fd, &clients);
							int to_shell = clients.to_shell[index];
							char buffer[1024];
							bzero(buffer, sizeof(buffer));
							int size = read(event_fd, buffer, sizeof(buffer));
							buffer[size] = '\0';
							// printf("is_shell_runing_buffer %d: %s\n", size, buffer);
							if (size <= 0)
							{
								// printf("is_shell_running remove client\n");
								kill(clients.shell_pid[index], SIGKILL);
								remove_client(event_fd, epollfd, &clients);
							}
							else
							{
								// printf("is_shell_running write to shell\n");
								write(to_shell, buffer, size);
								usleep(1000);
								if (waitpid(clients.shell_pid[index], NULL, WNOHANG))
								{
									// printf("shell pid dead\n");
									kill(clients.shell_pid[index], SIGKILL);
									remove_client(event_fd, epollfd, &clients);
								}
							}
							continue;
						}
						else
							pre_shell(event_fd, epollfd, &clients);
					}
					else if (is_shell(event_fd, &clients) == 1)
					{
						int index = get_index_shellfd(event_fd, &clients);
						if (index == -1)
							continue;
						char buffer[1024];
						bzero(buffer, sizeof(buffer));
						int size = read(event_fd, buffer, sizeof(buffer));
						buffer[size] = '\0';
						if (size > 0)
							write(clients.list[index], buffer, size);
						continue;
					}
					else
					{
						if (check_keycode(event_fd, epollfd, &clients) == 1)
						{
							// printf("login\n");
							if (auth(event_fd, &clients) == -1)
							{
								// printf("crit: could not find client\n");
								remove_client(event_fd, epollfd, &clients);
								continue;
							}
							// printf("2 list: %d %d %d\n", clients.list[0], clients.list[1], clients.list[2]);
							// printf("2 auth: %d %d %d\n", clients.auth[0], clients.auth[1], clients.auth[2]);
							write(event_fd, "$> ", 3);
						}
						else
						{
							write(event_fd, "Keycode: ", 9);
						}
					}
				}
			}
			// printf("\n");
		}
	}
}
