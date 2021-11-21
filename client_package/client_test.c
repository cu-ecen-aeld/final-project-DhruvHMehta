/* References:
 * 1. library has to be installed: https://stackoverflow.com/questions/61186574/libgpiod-api-usage-linux-shared-libraries
 * 2. client code reference: https://www.geeksforgeeks.org/socket-programming-cc/
 */

#include <stdio.h>
#include <unistd.h>
#include <gpiod.h>
#include <stdint.h>
#include <string.h>

//client socket related extra additions
#include <sys/socket.h>
#include <arpa/inet.h>

//generic
#define MSEC_TO_SEC			(1000)
#define USEC_TO_MSEC			(1000)
//for gpio functionality
#define GPIO_CHIP_PATH			("/dev/gpiochip0")
#define SLEEP_TIME			(5*(USEC_TO_MSEC)*(MSEC_TO_SEC))
#define GPIO_TOGGLE_PIN			(25)
#define DEFAULT_LED_STATUS		(0)
//for client functionality
#define PORT_NO				(9000)
#define LED_TOGGLE_TIME			(250*(USEC_TO_MSEC))

#define GPIOSTATUS			(1)
#define DEBUG				(0)

//gpio related initialization
struct gpiod_chip *gpio_driver_fd;
struct gpiod_line *gpio_LED_line_selected;
uint8_t LED_status;
int gpio_init();

//client related initialization
bool client_init();
void data_reception_indication();

//data processing operations
int data_processing(const char arr[]);

//client global variables
int client_socket_fd;

int main(int argc, char *argv[])
{
	bool dataprocessing_status = false;
	bool client_init_status = false;
	char buffer[1024] = {0};
	char IP[20] = {0};

	if(argc != 2)
	{
		printf("Usage client_test [IP]\n");
		return -1;
	}

	memcpy(IP, argv[1], strlen(argv[1]));
	printf("Connecting to Server's IP %s\n", IP);

#if GPIOSTATUS
	gpio_init();
#endif

	client_init_status = client_init();
	if(client_init_status == false)
	{
		perror("client_init");  
		return -1;
	}

	LED_status = DEFAULT_LED_STATUS;
	
	while(1)
	{
		read(client_socket_fd,buffer,sizeof(buffer));
#if 1
		printf("buffer is %s\n",buffer);
#endif
		dataprocessing_status = data_processing(buffer);
		if(true == dataprocessing_status)
		{
			exit(1);
		}
	}
}

bool client_init()
{
	client_socket_fd = socket(AF_INET,SOCK_STREAM,0);
	if(client_socket_fd < 0)
	{
		perror("client_socket_fd");  
		return false; 
	}
	
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT_NO);
	
	int client_inet_pton_fd = inet_pton(AF_INET,"10.0.0.247",&server_address.sin_addr);
	if(client_inet_pton_fd <= 0)
	{
		perror("client_inet_pton_fd");  
		return false; 
	}
	
	int client_connect_fd = connect(client_socket_fd, (struct sockaddr *)&server_address,sizeof(server_address));
	if(client_connect_fd < 0)
	{
		perror("client_connect_fd");  
		return false; 
	}

	printf("client init completed\n");

	return true;
}

//char *hello = "DIST20\nTEMP34.50\n";
int data_processing(const char rx_str[])
{
	data_reception_indication();
	
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

	if(0 == (strncmp(detect_str+7 ,"TEMP",4)))
	{
		strncpy(temp_str,detect_str+7,9);
		strncpy(dest_str,temp_str+4,5);
		temperature_value = atof(dest_str);
		strncpy(temp_str,"",strlen(temp_str));
	}

	printf("distance value is = %d , Temperature value is %f\n",dist_value,temperature_value);
	
	//data process
	if((dist_value < 15) && (dist_value > 4))
	{
		if(temperature_value > 0)
		{
			printf("Distance is = %d , Temperature is %f\n",dist_value,temperature_value);
		}
	}

	return l_status;
}

void data_reception_indication()
{
	LED_status ^= 1;
#if GPIOSTATUS
	gpiod_line_set_value(gpio_LED_line_selected,LED_status);
#endif

#if DEBUG
	printf("LED_STATUS:B = %d\n",LED_status);
#endif
	usleep(LED_TOGGLE_TIME);
	
	LED_status ^= 1;
#if GPIOSTATUS
	gpiod_line_set_value(gpio_LED_line_selected,LED_status);
#endif	

#if DEBUG
	printf("LED_STATUS:A = %d\n",LED_status);
#endif	
}

int gpio_init()
{
	int rc = 0;
	int ret_status = 0;
	
	//get the file handler for gpio driver
	gpio_driver_fd = gpiod_chip_open(GPIO_CHIP_PATH);
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

	printf("GPIO init complete\n");

	return ret_status;
}
	
/* EOF */
