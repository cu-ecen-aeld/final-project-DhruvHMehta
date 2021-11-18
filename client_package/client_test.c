/* References:
 * 1. library has to be installed: https://stackoverflow.com/questions/61186574/libgpiod-api-usage-linux-shared-libraries
 */

#include <stdio.h>
#include <unistd.h>
#include <gpiod.h>

#define SLEEP_TIME                      (1000000)

int main()
{       
    while(1)
    {
        printf("Hello World\n");
        usleep(SLEEP_TIME);
    }
}
