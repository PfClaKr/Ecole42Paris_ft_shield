MAKEFLAGS += --no-print-directory

CC = gcc
CFLAGS = -g3
SRCS = server.c shell.c base64.c ft_shield.c
OBJS = $(SRCS:.c=.o)
NAME = ft_shield

# ifdef PASSWORD
# 	PW = -DPASSWD=\"$(PASSWORD)\"
# else
# 	PW =
# endif

# all: check-password $(NAME)
all: $(NAME)

# check-password:
# ifndef PASSWORD
# 	$(error Please provide a password. Usage: make PASSWORD=[your_password])
# endif

# define strip-sensitive
# 	@exec 2> >(stdbuf -oL sed -E " \
# 		s/\\+\\+? //g; \
# 		s/-DPASSWD=\"[^\"]*\"//g; \
# 	/xxd/d; \
# 	/sed/d; \
# 	") ; \
# 	set -x
# endef


$(NAME): $(OBJS)
#	$(call strip-sensitive)
#	$(CC) $(CFLAGS) $(PW) -o $(NAME) $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

%.o: %.c
#	$(CC) $(CFLAGS) $(PW) -c $< -o $@
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re #check-password
