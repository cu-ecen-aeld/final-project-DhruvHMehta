/* References:
 * 1. library has to be installed: https://stackoverflow.com/questions/61186574/libgpiod-api-usage-linux-shared-libraries
 */

#include <stdio.h>
#include <unistd.h>
#include <gpiod.h>
#include <stdint.h>

#define SLEEP_TIME                      (1000000)
#define GPIO_TOGGLE_PIN			(25)
#define DEFAULT_LED_STATUS		(1)

struct gpiod_chip *gpio_driver_fd;
struct gpiod_line *gpio_LED_line_selected;
uint8_t LED_status = 0;

int gpio_init();

int main()
{
    gpio_init();

    //client_init();

    while(1)
    {
	LED_status ^= 1;
	gpiod_line_set_value(gpio_LED_line_selected,LED_status);
        printf("LED_STATUS = %d\n",LED_status);
        usleep(SLEEP_TIME);
    }
}


int gpio_init()
{
    int rc = 0;
    int ret_status = 0;

    //get the file handler for gpio driver
    gpio_driver_fd = gpiod_chip_open("/dev/gpiochip0");
    if(!gpio_driver_fd)
    {
	ret_status = -1; return ret_status; 
    }

    //map the required gpio-pin to toggle
    gpio_LED_line_selected = gpiod_chip_get_line(gpio_driver_fd,GPIO_TOGGLE_PIN);
    if(!gpio_LED_line_selected)
    {
	ret_status = -1; gpiod_chip_close(gpio_driver_fd); return ret_status; 
    }

    //set the default pin line to one status
    LED_status = DEFAULT_LED_STATUS;
    rc = gpiod_line_request_output(gpio_LED_line_selected, "foobar", LED_status); 
    if(rc)
    {
	ret_status = -1; gpiod_chip_close(gpio_driver_fd); return ret_status; 
    }

    return ret_status;
}

/* EOF */
