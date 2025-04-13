#ifndef SHELL_H
#define SHELL_H

#include "server.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct s_server t_server;

void	little_shell(int fd, t_server *server);
void 	shell(int fd, t_server *server);

#endif