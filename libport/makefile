include ../common.mak

NAME := port

LIBNAME := $(LIBDIR)/lib$(NAME).a

OBJS := queue.o port_linux.o

all: $(LIBNAME)


$(LIBNAME): $(OBJS)
	$(AR) rc $(LIBNAME) $(OBJS)

clean:
	rm -f $(OBJS) $(LIBNAME)