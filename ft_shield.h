#ifndef FT_SHIELD_H
# define FT_SHIELD_H

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <fcntl.h>
# include <string.h>
# include <syslog.h>
# include <sys/stat.h>

# define BIN_PATH "/bin/ft_shield"
# define EXE_PATH "/home/s/sh/a.out"

# define CONFIG "[Unit]\n\
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

#endif
