#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define FIFO_SEND   "senderfifo"

int main()
{
    int rv = mkfifo(FIFO_SEND, 0666);
    int data;

    if(rv)
    {
        printf("mkfifo failed, %s", strerror(errno));
        exit(-1);
    }

    printf("Waiting for reader\n");
    int fd = open(FIFO_SEND, O_WRONLY);
    printf("Reader connected, sending...");
    srand(time(0));
    
    for(int i = 0; i < 10; i++)
    {
        data = rand() % 255;
        rv = write(fd, &data, sizeof(int));
        if(rv != sizeof(int))
            printf("Not all data was written\n");

        else printf("Sent %d\n", data);
        
        sleep(1);
    }

    return 0;

}
