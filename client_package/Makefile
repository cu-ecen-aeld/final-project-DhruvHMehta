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

all: client_test
default: client_test

client_test : client_test.o
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -o client_test client_test.o $(LDFLAGS)

client_test.o : client_test.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -c client_test.c $(LDFLAGS)

clean :
	rm -f client_test *.o *.out

