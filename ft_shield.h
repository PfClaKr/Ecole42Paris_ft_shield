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
# define EXE_PATH "/home/s/sh/a.out"
# define SYSTEMD_PATH "/etc/systemd/system/"


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

# define SYSVINIT_CONFIG ""


#endif
