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
    int rv, data;

    printf("Waiting for writer\n");
    int fd = open(FIFO_SEND, O_RDONLY);
    printf("Writer connected, receiving...");
    srand(time(0));
    
    for(int i = 0; i < 10; i++)
    {
        rv = read(fd, &data, sizeof(int));
        if(rv != sizeof(int))
            printf("Not all data was written\n");

        else printf("Read %d\n", data);
        
        sleep(1);
    }

    return 0;

}
