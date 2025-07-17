NAME = ft_nm

CC = gcc

CFLAGS = -Wall -Wextra -Werror Iincl -g3

RM = rm -f

LIB = ar rcs

SRCS = 

OBJTS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJTS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJTS)

clean:
	$(RM) $(OBJTS)

fclean: clean
	$(RM) $(NAME)
	$(RM) $(OBJTS)

re: all clean

.PHONY: all clean fclean re