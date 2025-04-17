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

void shell(int fd, int epollfd, t_clients *clients)
{
	printf("bash\n");
	while (1)
	{
		write(fd, "bash>\n", 5);
		getchar();
	}
	// pid_t parent_pid;
	// pid_t child_pid;

	// parent_pid = getpid();
	// child_pid = fork();
	// if (child_pid == 0)
	// {
	// 	dup2(fd, STDIN_FILENO);
	// 	dup2(fd, STDOUT_FILENO);
	// 	dup2(fd, STDERR_FILENO);
	// 	chdir("/");
	// 	execl("/bin/bash", "bash", "-i", NULL);
	// }
	// else
	// {
	// 	int pid = waitpid(child_pid, NULL, WNOHANG);
	// 	server->request_shell[fd] = false;
	// 	epoll_ctl(server->epollfd, EPOLL_CTL_DEL, fd, NULL);
	// 	server->current_client--;
	// 	close(fd);
	// }
}
