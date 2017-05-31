CC := gcc
C++ := g++
LD := gcc
AR := ar

LIBDIR := -L../../lib
LIBFLAGS :=  -lpthread -lport
CFLAGS := -I../../include/port

%.o: %.cpp
	@echo build $<
	$(C++) -c $(CFLAGS) -o $@ $<

%.o: %.c
	@echo build $<
	$(CC) -c $(CFLAGS) -o $@ $<

NAME := port
OBJS := testport.o

all: $(NAME)

$(NAME): $(OBJS)
	@echo build port
	$(CC) -lstdc++ $(LIBDIR) $(CFLAGS) -o $@ $(OBJS) $(LIBFLAGS)

clean:
	@echo delete...
	rm -f $(OBJS) $(NAME)
