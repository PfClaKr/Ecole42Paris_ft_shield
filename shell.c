#include "shell.h"
#include <stdio.h>
#include <errno.h>

int pre_shell(int fd, int epollfd, t_clients *clients)
{
	char buffer[1024];
	bzero(buffer, sizeof(buffer));
	int len = read(fd, buffer, sizeof(buffer) - 1);
	if (len > 0)
	{
		process_input(buffer);			   // if it ends with newline
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
			shell(fd, epollfd, clients);
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

void set_nonblocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
		perror("fcntl");
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
		perror("fcntl");
}

void shell(int fd, int epollfd, t_clients *clients)
{
	int index = get_client_index(fd, clients);
	if (index == -1)
		return;
	int to_shell[2], from_shell[2];
	pipe(to_shell);
	pipe(from_shell);

	pid_t pid = fork();
	if (pid == 0)
	{
		dup2(to_shell[0], STDIN_FILENO);
		dup2(from_shell[1], STDOUT_FILENO);
		dup2(from_shell[1], STDERR_FILENO);
		close(to_shell[1]);
		close(from_shell[0]);
		execve("/bin/sh", (char *[]){"/bin/sh", NULL}, NULL);
		// execl("/bin/bash", "bash", "-i", NULL);
		exit(1);
	}

	close(to_shell[0]);
	close(from_shell[1]);
	set_nonblocking(fd);
	set_nonblocking(from_shell[0]);
	set_nonblocking(to_shell[1]);

	clients->to_shell[index] = to_shell[1];
	clients->from_shell[index] = from_shell[0];
	clients->shell_pid[index] = pid;
	clients->active[index] = 1;

	struct epoll_event e = {.events = EPOLLIN, .data.fd = from_shell[0]};
	epoll_ctl(epollfd, EPOLL_CTL_ADD, from_shell[0], &e);
}
