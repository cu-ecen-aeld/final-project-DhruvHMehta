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

all: gpiotest
default: gpiotest

gpiotest : gpio.o
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -o gpiotest gpio.o $(LDFLAGS)

gpio.o : gpio.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -c gpio.c $(LDFLAGS)

clean :
	rm -f gpiotest *.o
