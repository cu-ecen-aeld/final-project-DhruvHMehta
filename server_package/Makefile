CROSS_COMPILE=

ifeq ($(CC),)
	CC=$(CROSS_COMPILE)gcc
endif

ifeq ($(CFLAGS),)
	CFLAGS= -g -Wall -Werror
endif

ifeq ($(LDFLAGS),)
	LDFLAGS= -lgpiod -lrt
endif

all: sensors socketserver
default: sensors socketserver

sensors : sensors.o
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -o sensors sensors.o $(LDFLAGS)

socketserver : server.o
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -o socketserver server.o $(LDFLAGS)

sensors.o : sensors.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -c sensors.c $(LDFLAGS)

server.o : server.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -c server.c $(LDFLAGS)

clean :
	rm -f sensors socketserver *.o
