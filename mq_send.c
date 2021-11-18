#include <mqueue.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

struct mq_attr sendmq;

int main()
{
    sendmq.mq_maxmsg = 10;
    sendmq.mq_msgsize = sizeof(int);

    mqd_t mq_send_desc;
    int mq_send_len;
    char buffer[sizeof(int)];
    int dummy_data = 0;

    mq_send_desc = mq_open("/sendmq", O_CREAT | O_RDWR, S_IRWXU, &sendmq);

    if(mq_send_desc < 0)
    {
        perror("Sender MQ failed to open");
        exit(-1);
    }

    srand(time(0));
    for(int i = 0; i < 10; i++)
    {
        dummy_data = rand() % 255;
        memcpy(buffer, &dummy_data, sizeof(int));
        
        mq_send_len = mq_send(mq_send_desc, buffer, sizeof(int), 1);
        if(mq_send_len < 0)
            perror("Did not send any data");

        printf("Tx_data = %d\n", dummy_data);
        sleep(1);
    }

    mq_close(mq_send_desc);
}
