CROSS_COMPILE=

ifeq ($(CC),)
	CC=$(CROSS_COMPILE)gcc
endif

ifeq ($(CFLAGS),)
	CFLAGS= -g -Wall -Werror
endif

ifeq ($(LDFLAGS),)
	LDFLAGS= -lrt
endif

all: mq_send mq_receive npipe_send npipe_receive
default: mq_send mq_receive npipe_send npipe_receive

mq_send : mq_send.o
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -o mq_send mq_send.o $(LDFLAGS)

mq_receive : mq_receive.o
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -o mq_receive mq_receive.o $(LDFLAGS)

npipe_send : npipe_send.o
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -o npipe_send npipe_send.o $(LDFLAGS)

npipe_receive : npipe_receive.o
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -o npipe_receive npipe_receive.o $(LDFLAGS)

mq_send.o : mq_send.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -c mq_send.c $(LDFLAGS)

mq_receive.o : mq_receive.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -c mq_receive.c $(LDFLAGS)

npipe_send.o : npipe_send.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -c npipe_send.c $(LDFLAGS)

npipe_receive.o : npipe_receive.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -c npipe_receive.c $(LDFLAGS)

clean :
	rm -f mq_send mq_receive npipe_send npipe_receive *.o
