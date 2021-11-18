#include <mqueue.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct mq_attr sendmq;

int main()
{
    sendmq.mq_maxmsg = 10;
    sendmq.mq_msgsize = sizeof(int);

    mqd_t mq_receive_desc;
    int mq_receive_len;
    char buffer[sizeof(int)];
    int rx_data = 0;
    unsigned int rx_prio;

    mq_receive_desc = mq_open("/sendmq", O_RDWR, S_IRWXU, &sendmq);

    if(mq_receive_desc < 0)
    {
        perror("Receiver MQ failed to open");
        exit(-1);
    }

    for(int i = 0; i < 10; i++)
    {
        mq_receive_len = mq_receive(mq_receive_desc, buffer, sizeof(int), &rx_prio);
        if(mq_receive_len < 0)
            perror("Did not send any data");

        memcpy(&rx_data, buffer, sizeof(int));

        printf("rx_data = %d\n", rx_data);

        sleep(1);
    }

    mq_close(mq_receive_desc);
}
