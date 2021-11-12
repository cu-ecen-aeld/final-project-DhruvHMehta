#include <gpiod.h>
#include <time.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>

#define GPIOCHIP0   "/dev/gpiochip0"
#define TRIG        5
#define ECHO        6
#define V_SOUND     330

struct gpiod_chip *chip;
struct gpiod_line *trigger, *echo;
struct timespec start_time, end_time, wait_time;

int main()
{
    int rv;

    chip = gpiod_chip_open(GPIOCHIP0);
    if(chip == NULL)
        return -1;

   trigger = gpiod_chip_get_line(chip, TRIG);
   if(trigger == NULL)
   {
       syslog(LOG_CRIT, "Trig get line failed, errno = %s", strerror(errno));
       gpiod_chip_close(chip);
       return -1;
   }

   echo = gpiod_chip_get_line(chip, ECHO);
   if(echo == NULL)
   {
       syslog(LOG_CRIT, "echo get line failed, errno = %s", strerror(errno));
       gpiod_chip_close(chip);
       return -1;
   }
    
   rv = gpiod_line_request_output(trigger, "ultrasonic_trig", 0);
   if(rv != 0)
   {
       syslog(LOG_CRIT, "Trig request get output failed, errno = %s", strerror(errno));
       gpiod_chip_close(chip);
       return -1;
   }
   
   syslog(LOG_CRIT, "Requested 0 at trigger");
   
   rv = gpiod_line_set_value(trigger, 0);
   if(rv != 0)
       syslog(LOG_CRIT,"gpiod_line_set_value->1 failed");

   syslog(LOG_CRIT, "Set 0 at trigger");
    
 /*
   rv = gpiod_line_request_input(echo, "ultrasonic_echo");
   if(rv != 0)
   { 
       syslog(LOG_CRIT, "Echo request input failed, errno = %s", strerror(errno));
       gpiod_chip_close(chip);
       return -1;
   }
*/
   rv = gpiod_line_request_both_edges_events(echo, "ultrasonic_echo");
   if(rv != 0)
   {
       syslog(LOG_CRIT, "Request both edges failed, errno = %s", strerror(errno));
       gpiod_chip_close(chip);
       return -1;
   }

   syslog(LOG_CRIT,"Before sleep 1");
   sleep(1);
   syslog(LOG_CRIT,"After sleep 1");
    while(1)
    {
        rv = gpiod_line_set_value(trigger, 1);
        if(rv != 0)
            syslog(LOG_CRIT,"gpiod_line_set_value->1 failed, errno = %s", strerror(errno));

        usleep(10);

        rv = gpiod_line_set_value(trigger, 0);
        if(rv != 0)
            syslog(LOG_CRIT,"gpiod_line_set_value->0 failed, errno = %s", strerror(errno));

        wait_time.tv_sec  = 0;
        wait_time.tv_nsec = 50000000;

        rv = gpiod_line_event_wait(echo, NULL);
        if(rv != 1)
        {
            syslog(LOG_CRIT,"gpiod_line_event_wait, errno = %s", strerror(errno));
            gpiod_chip_close(chip);
            return -1;
        }

        rv = clock_gettime(CLOCK_MONOTONIC, &start_time);
        if(rv != 0)
            syslog(LOG_CRIT,"Something went wrong in clock_gettime\n");

        rv = gpiod_line_event_wait(echo, &wait_time);
        if(rv != 1)
        {
            syslog(LOG_CRIT,"gpiod_line_event_wait, errno = %s", strerror(errno));
            gpiod_chip_close(chip);
            return -1;
        }
        
        rv = clock_gettime(CLOCK_MONOTONIC, &end_time);
        if(rv != 0)
            syslog(LOG_CRIT,"Something went wrong in clock_gettime\n");

        syslog(LOG_CRIT, "Distance = %ld\n", V_SOUND*((end_time.tv_nsec)*1000000000));

        sleep(1);        
    }
// gpio_chip_close(chip);

}



