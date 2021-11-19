// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#define PORT 8080

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
int string_operations(const char arr[]);

int main(int argc, char const *argv[])
{
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
   
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
       
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
   
   bool status = false; 
   while(1)
   {
    	valread = read( sock , buffer, 1024);
	printf("Message: %s",buffer);
	status = string_operations(buffer);
	if(status == true)
	{
		exit(1);
	}
    }

    //printf("%s\n",buffer );
    return 0;
}

//    char *hello = "DIST20\nTEMP34.50\n";
int string_operations(const char rx_str[])
{
	bool l_status = false;
	int dist_value = 0;
	float temperature_value = 0;
	char dest_str[10] = {0};
	char temp_str[10] = {0};
	char dummy_str[10] = {0};
	char detect_str[50] = {0};
	strncpy(detect_str,rx_str,strlen(rx_str));

	//string seperation based upon null and \n
	//logic to be added
	//
	//
	//
	//copy the string to detect_str

	// string identification process
	if(0 == (strncmp(detect_str ,"TERMINATE",9)))
	{
		l_status = true;
		return l_status;
	}
#if 1
	if(0 == (strncmp(detect_str ,"DIST",4)))
	{
		strncpy(temp_str,detect_str,6);
		strncpy(dest_str,temp_str+4,2);
		dist_value = atoi(dest_str);
		strncpy(temp_str,"",strlen(temp_str));
	}
#endif
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
	if((dist_value < 25) && (dist_value > 3))
	{
		printf("Display the value on the terminal\n");
	}

	return l_status;
}
