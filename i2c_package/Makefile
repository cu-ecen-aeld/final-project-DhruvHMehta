CROSS_COMPILE=

ifeq ($(CC),)
	CC=$(CROSS_COMPILE)gcc
endif

ifeq ($(CFLAGS),)
	CFLAGS= -g -Wall -Werror
endif

all: mlx90614test
default: mlx90614test

mlx90614test : mlx90614.o
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -o mlx90614test mlx90614.o $(LDFLAGS)

mlx90614.o : mlx90614.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -c mlx90614.c $(LDFLAGS)

clean :
	rm -f *.o mlx90614test
