#ifndef FT_SHIELD_H
# define FT_SHIELD_H

# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <stdbool.h>
# include <fcntl.h>
# include <sys/stat.h>
# include <string.h>
# include <syslog.h>


// PATH CONFIGURATION

# define BIN_PATH "/bin/ft_shield"
# define SYSTEMD_PATH "/etc/systemd/system/"
# define SYSVINIT_PATH "/etc/init.d/"


// SYSTEMD CONFIGURATION

# define SYSTEMD_CONFIG "[Unit]\n\
Description=project ft_shield\n\
\n\
[Service]\n\
Type=simple\n\
User=root\n\
WorkingDirectory=/\n\
ExecStart=/bin/ft_shield\n\
#Restart=always\n\
StandardOutput=journal\n\
\n\
[Install]\n\
WantedBy=multi-user.target\n"


// SYSVINIT CONFIGURATION

# define SYSVINIT_CONFIG "#!/bin/sh\n\
### BEGIN INIT INFO\n\
# Provides:		ft_shield\n\
# Required-Start:	$remote_fs $syslog\n\
# Required-Stop:	$remote_fs $syslog\n\
# Default-Start:	2 3 4 5\n\
# Default-Stop:		0 1 6\n\
# Short-Description:	project ft_shield\n\
# Description:		implemented for sysvinit from  systemd\n\
### END INIT INFO\n\
\n\
DAEMON=\"/bin/ft_shield\"\n\
DAEMON_NAME=\"ft_shield\"\n\
PIDFILE=\"/var/run/$DAEMON_NAME.pid\"\n\
LOGFILE=\"/var/log/$DAEMON_NAME.log\"\n\
WORKDIR=\"/\"\n\
\n\
. /lib/lsb/init-functions\n\
\n\
start() {\n\
	echo \"Starting $DAEMON_NAME...\"\n\
	cd \"$WORKDIR\"\n\
	start-stop-daemon --start --background --make-pidfile --pidfile \"$PIDFILE\" \\\n\
		--exec \"$DAEMON\" --chuid root \\\n\
		>> \"$LOGFILE\" 2>&1\n\
	log_end_msg $?\n\
}\n\
\n\
stop() {\n\
	echo \"Stopping $DAEMON_NAME...\"\n\
	start-stop-daemon --stop --pidfile \"$PIDFILE\" --retry 5\n\
	log_end_msg $?\n\
}\n\
\n\
status() {\n\
	status_of_proc \"$DAEMON\" \"$DAEMON_NAME\" && exit 0 || exit $?\n\
}\n\
\n\
case \"$1\" in\n\
	start)\n\
		start\n\
		;;\n\
	stop)\n\
		stop\n\
		;;\n\
	restart)\n\
		stop\n\
		start\n\
		;;\n\
	status)\n\
		status\n\
		;;\n\
	*)\n\
		echo \"Usage: $0 {start|stop|restart|status}\"\n\
		exit 1\n\
		;;\n\
esac\n\
\n\
exit 0\n"


#endif
