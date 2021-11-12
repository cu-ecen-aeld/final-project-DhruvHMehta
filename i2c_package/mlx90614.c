/* References: https://olegkutkov.me/2017/08/10/mlx90614-raspberry/
 * Modified by : Rishab Shah
 */
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

/* Macros */
#define MLX90614_TA 			(0x06)
#define MLX90614_TOBJ1 			(0x07)
#define MLX90614_TOBJ2 			(0x08)
#define TEMPERATURE_TYPE 		(MLX90614_TOBJ1)

#define MLX90614_DEVICE_ADDRESS		(0x5A)
#define I2C_DEV_PATH 			("/dev/i2c-1")

/* Just in case if these were not defined */
#ifndef I2C_SMBUS_READ 
#define I2C_SMBUS_READ 1 
#endif 
#ifndef I2C_SMBUS_WRITE 
#define I2C_SMBUS_WRITE 0 
#endif

typedef union i2c_smbus_data i2c_data;

int main()
{
    int fdev = open(I2C_DEV_PATH, O_RDWR); // open i2c bus

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

    // trying to read something from the device using SMBus READ request
    i2c_data data;
    char command = TEMPERATURE_TYPE; 
    // build request structure
    struct i2c_smbus_ioctl_data sdat = {
        .read_write = I2C_SMBUS_READ,
        .command = command,
        .size = I2C_SMBUS_WORD_DATA,
        .data = &data
    };

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

    return 0;
}
