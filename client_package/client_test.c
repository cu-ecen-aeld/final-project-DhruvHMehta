/* References:
 * 1. library has to be installed: https://stackoverflow.com/questions/61186574/libgpiod-api-usage-linux-shared-libraries
 * 2. client code reference: https://www.geeksforgeeks.org/socket-programming-cc/
 */

#include <stdio.h>
#include <unistd.h>
#include <gpiod.h>
#include <stdint.h>
#include <string.h>

#define MSEC_TO_SEC				(1000)
#define USEC_TO_MSEC			(1000)

#define SLEEP_TIME				(5*(USEC_TO_MSEC)*(MSEC_TO_SEC))
#define GPIO_TOGGLE_PIN			(25)
#define DEFAULT_LED_STATUS		(1)

#define LED_TOGGLE_TIME			(300*(USEC_TO_MSEC))

//gpio related initialization
struct gpiod_chip *gpio_driver_fd;
struct gpiod_line *gpio_LED_line_selected;
uint8_t LED_status = 0;
int gpio_init();

//client related initialization
void client_init();
void data_reception_indication();

//data processing operations
int data_processing(const char arr[]);

int main()
{
    //gpio_init();

    //client_init();
	
	data_processing("DIST20\nTEMP34.50\n");
	
    while(1)
    {
    	data_processing("DIST20\nTEMP34.50\n");
    	data_reception_indication();
    	usleep(SLEEP_TIME);
    }
}





//char *hello = "DIST20\nTEMP34.50\n";
int data_processing(const char rx_str[])
{
	bool l_status = false;
	int dist_value = 0;
	float temperature_value = 0;
	char dest_str[10] = {0};
	char temp_str[10] = {0};
	char detect_str[50] = {0};
	strncpy(detect_str,rx_str,strlen(rx_str));

	// string identification process
	if(0 == (strncmp(detect_str ,"TERMINATE",9)))
	{
		l_status = true;
		return l_status;
	}

	if(0 == (strncmp(detect_str ,"DIST",4)))
	{
		strncpy(temp_str,detect_str,6);
		strncpy(dest_str,temp_str+4,2);
		dist_value = atoi(dest_str);
		strncpy(temp_str,"",strlen(temp_str));
	}

	printf("detect string is %s\n",detect_str);

	if(0 == (strncmp(detect_str+7 ,"TEMP",4)))
	{
		strncpy(temp_str,detect_str+7,9);
		strncpy(dest_str,temp_str+4,5);
		temperature_value = atof(dest_str);
		strncpy(temp_str,"",strlen(temp_str));
	}

	printf("dist_value = %d , Temp_val is %f\n",dist_value,temperature_value);

	//data process
	if((dist_value < 15) && (dist_value > 3))
	{
	printf("Display the value on the terminal\n");
	}

	return l_status;
}


void client_init()
{

}

void data_reception_indication()
{
	LED_status ^= 1;
	//gpiod_line_set_value(gpio_LED_line_selected,LED_status);
	printf("LED_STATUS = %d\n",LED_status);
	usleep(LED_TOGGLE_TIME);
	LED_status ^= 1;
	printf("LED_STATUS = %d\n",LED_status);	
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
