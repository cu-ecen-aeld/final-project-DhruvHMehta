CROSS_COMPILE=

ifeq ($(CC),)
	CC=$(CROSS_COMPILE)gcc
endif

ifeq ($(CFLAGS),)
	CFLAGS= -g -Wall -Werror
endif

ifeq ($(LDFLAGS),)
	LDFLAGS= -lgpiod
endif

all: sensors
default: sensors

sensors : sensors.o
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -o sensors sensors.o $(LDFLAGS)

sensors.o : sensors.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -c sensors.c $(LDFLAGS)

clean :
	rm -f sensors *.o
