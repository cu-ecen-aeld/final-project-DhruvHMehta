#include <gpiod.h>
#include <time.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>

#define GPIOCHIP0   "/dev/gpiochip0"
#define TRIG        5
#define ECHO        6
#define U_POWER     13
#define V_SOUND     330

struct gpiod_chip *chip;
struct gpiod_line *trigger, *echo, *u_power;
struct timespec start_time, end_time, wait_time;

int main()
{
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
    
   /* Get a line for ultasonic power pin */
   u_power = gpiod_chip_get_line(chip, U_POWER);
   if(u_power == NULL)
   {
       syslog(LOG_CRIT, "u_power get line failed, errno = %s", strerror(errno));
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
   
   /* Set the power pin to output, initially powered off */
   rv = gpiod_line_request_output(u_power, "ultrasonic_power", 0);
   if(rv != 0)
   {
       syslog(LOG_CRIT, "u_power request get output failed, errno = %s", strerror(errno));
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
 
   /* Power on the ultrasonic sensor */
   rv = gpiod_line_set_value(u_power, 1);
   if(rv != 0)
       syslog(LOG_CRIT,"gpiod_line_set_value->1 failed");   

   /* Request gpio events for the echo pin for rising and falling edge
    * Time between the rising and falling edge (High time) is used to
    * calculate the distance from the sensor */
   /*rv = gpiod_line_request_both_edges_events(echo, "ultrasonic_echo");
   if(rv != 0)
   {
       syslog(LOG_CRIT, "Request both edges failed, errno = %s", strerror(errno));
       gpiod_chip_close(chip);
       return -1;
   }
*/
   // redundant
   syslog(LOG_CRIT,"Before sleep 1");
   sleep(1);
   syslog(LOG_CRIT,"After sleep 1");
    while(1)
    {
        /* Send a trigger pulse of 10uS to initate a distance read */
        rv = gpiod_line_set_value(trigger, 1);
        if(rv != 0)
            syslog(LOG_CRIT,"gpiod_line_set_value->1 failed, errno = %s", strerror(errno));

        /* For some reason, this sleeps for 100uS
         * https://www.iot-programmer.com/index.php/books/22-raspberry-pi-and-the-iot-in-c/
         * chapters-raspberry-pi-and-the-iot-in-c/35-raspberry-pi-iot-in-c-introduction-to-the-gpio?start=3
         *
         * Scheduler might be taking 74uS to switch contexts? */
        usleep(10);

        /* End of trigger pulse */
        rv = gpiod_line_set_value(trigger, 0);
        if(rv != 0)
            syslog(LOG_CRIT,"gpiod_line_set_value->0 failed, errno = %s", strerror(errno));

        /* Wait for lines to stabilize for atleast 100uS */
        usleep(100);
        /* Wait for 50mS, if the echo didn't arrive, the waves did not reflect back */
        wait_time.tv_sec  = 0;
        wait_time.tv_nsec = 50000000;

        /* Spin here till the echo pin goes high */
        while((gpiod_line_get_value(echo)) != 1)
            usleep(100);
        /*do
        {
            rv = gpiod_line_event_wait(echo, &wait_time);
        }
        while(rv <= 0);
*/
        /* Mark the time when the echo pin went high */
        rv = clock_gettime(CLOCK_MONOTONIC, &start_time);
        if(rv != 0)
            syslog(LOG_CRIT,"Something went wrong in clock_gettime\n");

        syslog(LOG_CRIT, "Start time = %ldsec and %ldnsec", start_time.tv_sec, start_time.tv_nsec);
        /* Wait for the echo pin to go low. Don't wait beyond 50mS as it is not possible beyond 3m */
        while((gpiod_line_get_value(echo)) != 0)
            usleep(100);

        /*rv = gpiod_line_event_wait(echo, &wait_time);
        if(rv != 1)
        {
            syslog(LOG_CRIT,"gpiod_line_event_wait, errno = %s", strerror(errno));
            gpiod_chip_close(chip);
            return -1;
        }
        */
        /* Mark the time when the echo pin went low */
        rv = clock_gettime(CLOCK_MONOTONIC, &end_time);
        if(rv != 0)
            syslog(LOG_CRIT,"Something went wrong in clock_gettime\n");

        syslog(LOG_CRIT, "End time = %ldsec and %ldnsec", end_time.tv_sec, end_time.tv_nsec);
        
        /* Distance between object and sensor is calculated using d = v*(Tend - Tstart)/2
         * Convert nsec to s -> 10^-9 and convert m to cm 10^2 -> Final division -> 10^-7*/
        syslog(LOG_CRIT, "Time difference = %ld\n", end_time.tv_nsec - start_time.tv_nsec);
        syslog(LOG_CRIT, "Distance = %ld\ncm", (V_SOUND*(end_time.tv_nsec - start_time.tv_nsec)/10000000)/2);

        /* Wait a second before next measurement */
        sleep(1);        
    }
// gpio_chip_close(chip);

}



