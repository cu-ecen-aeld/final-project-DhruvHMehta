/*****************************************************************
 *
 *  Sensors.c
 *  Brief: Combination of ultrasonic sensor and I2C temperature
 *         sensor to obtain distance from object and its temperature
 *
 *  References: https://olegkutkov.me/2017/08/10/mlx90614-raspberry/
 *
 *  Author: Dhruv Mehta
 *          Rishab Shah
 *****************************************************************/

#include <gpiod.h>
#include <time.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#include <mqueue.h>

/* Macros for Ultrasonic */
#define GPIOCHIP0   "/dev/gpiochip0"
#define TRIG        5
#define ECHO        6
#define V_SOUND     330

/* Macros for Temperature */
#define MLX90614_TA 			(0x06)
#define MLX90614_TOBJ1 			(0x07)
#define MLX90614_TOBJ2 			(0x08)
#define TEMPERATURE_TYPE 		(MLX90614_TOBJ1)

#define MLX90614_DEVICE_ADDRESS		(0x5A)
#define I2C_DEV_PATH 			("/dev/i2c-1")

#define SLEEP_DURATION 			(1000000)

/* Just in case if these were not defined */
#ifndef I2C_SMBUS_READ 
#define I2C_SMBUS_READ 1 
#endif 
#ifndef I2C_SMBUS_WRITE 
#define I2C_SMBUS_WRITE 0 
#endif

/* Ultrasonic variables for libgpiod pins and time */
struct gpiod_chip *chip;
struct gpiod_line *trigger, *echo, *u_power;
struct timespec start_time, end_time;

/* Temperature i2c_smbus_data struct */
typedef union i2c_smbus_data i2c_data;
int fdev;
struct i2c_smbus_ioctl_data sdat;
i2c_data data;

/* Message queue to send data to socketserver */
struct mq_attr sendmq;

int Ultrasonic_Init()
{
    /******************************************************
     *
     * Ultrasonic Configuration
     *
     *****************************************************/
    int rv;

    /* Open the gpiochip0 char device driver */
    chip = gpiod_chip_open(GPIOCHIP0);
    if(chip == NULL)
        return -1;

    /* Get a line for ultrasonic trigger pin */
   trigger = gpiod_chip_get_line(chip, TRIG);
   if(trigger == NULL)
   {
       syslog(LOG_CRIT, "Trig get line failed, errno = %s", strerror(errno));
       gpiod_chip_close(chip);
       return -1;
   }

   /* Get a line for ultrasonic echo pin */
   echo = gpiod_chip_get_line(chip, ECHO);
   if(echo == NULL)
   {
       syslog(LOG_CRIT, "echo get line failed, errno = %s", strerror(errno));
       gpiod_chip_close(chip);
       return -1;
   }
    
   /* Set the trigger pin to output */
   rv = gpiod_line_request_output(trigger, "ultrasonic_trig", 0);
   if(rv != 0)
   {
       syslog(LOG_CRIT, "Trig request get output failed, errno = %s", strerror(errno));
       gpiod_chip_close(chip);
       return -1;
   }
   
   syslog(LOG_CRIT, "Requested 0 at trigger");
  
   /* Set the ECHO pin to input */
   rv = gpiod_line_request_input(echo, "ultrasonic_echo");
   if(rv != 0)
   {
       syslog(LOG_CRIT, "echo request get input failed, errno = %s", strerror(errno));
       gpiod_chip_close(chip);
       return -1;
   }  

   /* Explicitly set the trigger pin low if it is not already */
   rv = gpiod_line_set_value(trigger, 0);
   if(rv != 0)
       syslog(LOG_CRIT,"gpiod_line_set_value->0 failed");

   syslog(LOG_CRIT, "Set 0 at trigger");
 
   /* Let pins stabilize */ 
   sleep(1);

   return 0;

}

int Temperature_Init()
{
    /*********************************************************
     *
     * I2C Temperature Configuration
     *
     ********************************************************/

    fdev = open(I2C_DEV_PATH, O_RDWR); // open i2c bus

    if (fdev < 0) {
        fprintf(stderr, "Failed to open I2C interface %s Error: %s\n", I2C_DEV_PATH, strerror(errno));
        return -1;
    }
    
    // set slave device address, default MLX is 0x5A
    unsigned char i2c_addr = MLX90614_DEVICE_ADDRESS;
    if (ioctl(fdev, I2C_SLAVE, i2c_addr) < 0) {
        fprintf(stderr, "Failed to select I2C slave device! Error: %s\n", strerror(errno));
        return -1;
    }

    // enable checksums control
    if (ioctl(fdev, I2C_PEC, 1) < 0) {
        fprintf(stderr, "Failed to enable SMBus packet error checking, error: %s\n", strerror(errno));
        return -1;
    }

    char command = TEMPERATURE_TYPE; 
    // build request structure
   sdat.read_write = I2C_SMBUS_READ,
   sdat.command = command,
   sdat.size = I2C_SMBUS_WORD_DATA,
   sdat.data = &data;
   
    return 0;
}

int main()
{
    int distance = 0;

    /* Initalize Ultrasonic and Temperature Sensors */
    if(Ultrasonic_Init() == -1)
        return -1;

    if(Temperature_Init() == -1)
        return -1;

    sendmq.mq_maxmsg = 10;
    sendmq.mq_msgsize = sizeof(int);

    mqd_t mq_send_desc;
    int mq_send_len;
    char buffer[sizeof(int)];

    mq_send_desc = mq_open("/sendmq", O_CREAT | O_RDWR, S_IRWXU, &sendmq);
    if(mq_send_desc < 0)
    {
        perror("Sender MQ failed");
        exit(-1);
    }

    while(1)
    {
        /******************
         * Ultrasonic Read
         *****************/

        /* Send a trigger pulse of 10uS to initate a distance read */
        int rv = gpiod_line_set_value(trigger, 1);
        if(rv != 0)
            syslog(LOG_CRIT,"gpiod_line_set_value->1 failed, errno = %s", strerror(errno));

        /* For some reason, this sleeps for 100uS
         * https://www.iot-programmer.com/index.php/books/22-raspberry-pi-and-the-iot-in-c/
         * chapters-raspberry-pi-and-the-iot-in-c/35-raspberry-pi-iot-in-c-introduction-to-the-gpio?start=3
         */
        usleep(10);

        /* End of trigger pulse */
        rv = gpiod_line_set_value(trigger, 0);
        if(rv != 0)
            syslog(LOG_CRIT,"gpiod_line_set_value->0 failed, errno = %s", strerror(errno));

        /* Wait for lines to stabilize for atleast 100uS */
        usleep(100);

        /* Spin here till the echo pin goes high */
        while((gpiod_line_get_value(echo)) != 1);
        
        /* Mark the time when the echo pin went high */
        rv = clock_gettime(CLOCK_MONOTONIC, &start_time);
        if(rv != 0)
            syslog(LOG_CRIT,"Something went wrong in clock_gettime\n");

        syslog(LOG_CRIT, "Start time = %ldsec and %ldnsec", start_time.tv_sec, start_time.tv_nsec);
        /* Wait for the echo pin to go low. Don't wait beyond 50mS as it is not possible beyond 3m */
        while((gpiod_line_get_value(echo)) != 0);

        /* Mark the time when the echo pin went low */
        rv = clock_gettime(CLOCK_MONOTONIC, &end_time);
        if(rv != 0)
            syslog(LOG_CRIT,"Something went wrong in clock_gettime\n");

        syslog(LOG_CRIT, "End time = %ldsec and %ldnsec", end_time.tv_sec, end_time.tv_nsec);
        
        /* Distance between object and sensor is calculated using d = v*(Tend - Tstart)/2
         * Convert nsec to s -> 10^-9 and convert m to cm 10^2 -> Final division -> 10^-7*/
        syslog(LOG_CRIT, "Time difference = %ld\n", end_time.tv_nsec - start_time.tv_nsec);
        
        distance = (V_SOUND*(end_time.tv_nsec - start_time.tv_nsec)/10000000)/2;
        syslog(LOG_CRIT, "Distance = %dcm\n", distance);
        printf("Distance = %dcm\n", distance);

        /*******************
         * Temperature Read
         *******************/

        // do actual request
	    if (ioctl(fdev, I2C_SMBUS, &sdat) < 0) {
       	fprintf(stderr, "Failed to perform I2C_SMBUS transaction, error: %s\n", strerror(errno));
        return -1;
    	}
	
	    // calculate temperature in Celsius by formula from datasheet
	    double temp = (double) data.word;
    	temp = (temp * 0.02)-0.01;
    	temp = temp - 273.15;

    	// print result
    	printf("Temperature value read from object = %04.2f\n", temp);

        /* Send data in message queue */
        memcpy(buffer, &distance, sizeof(int));
        mq_send_len = mq_send(mq_send_desc, buffer, sizeof(int), 1);
        if(mq_send_len < 0)
            perror("Did not send any data");

        /* Wait a second before next measurement */
        sleep(1);        
    }
// gpio_chip_close(chip);
return 0;
}



