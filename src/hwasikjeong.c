#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#define DHT_PIN_1 0 // GPIO pin 17
#define DHT_PIN_2 2 // GPIO pin 27

#define IN 0
#define OUT 1
#define LOW 0
#define HIGH 1
#define LED_PIN 21 // GPIO pin 21
#define PWM 0      // GPIO pin 18
#define VALUE_MAX 40
#define DIRECTION_MAX 128

static int GPIOExport(int pin)
{
#define BUFFER_MAX 3
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Failed to open export for writing!\n");
        return -1;
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);

    return 0;
}

static int GPIODirection(int pin, int dir)
{
    static const char s_directions_str[] = "in\0out";

    char path[DIRECTION_MAX];
    int fd;

    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(path, O_WRONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Failed to open gpio direction for writing\n");
        return -1;
    }

    if (write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3) == -1)
    {
        fprintf(stderr, "Failed to set direction!\n");
        return -1;
    }

    close(fd);
    return 0;
}

static int GPIOWrite(int pin, int value)
{
    static const char s_values_str[] = "01";

    char path[VALUE_MAX];
    int fd;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_WRONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Failed to open gpio value for writing!\n");
        return -1;
    }

    if (write(fd, &s_values_str[LOW == value ? 0 : 1], 1) != 1)
    {
        fprintf(stderr, "Failed to write value!\n");
        return -1;
    }

    close(fd);
    return 0;
}

static int GPIOUnexport(int pin)
{
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Failed to open unexport for writing!\n");
        return -1;
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);
    return 0;
}

// echo 0 > export
static int PWMExport(int pwmnum)
{
#define BUFFER_MAX 3
    char buffer[BUFFER_MAX];
    int fd, byte;

    fd = open("/sys/class/pwm/pwmchip0/export", O_WRONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Failed to open export for export!\n");
        return -1;
    }

    byte = snprintf(buffer, BUFFER_MAX, "%d", pwmnum);
    write(fd, buffer, byte);
    close(fd);

    sleep(1);

    return 0;
}

// echo 1 > pwm0/enable
static int PWMEnable(int pwmnum)
{
    static const char s_enable_str[] = "1";

    char path[DIRECTION_MAX];
    int fd;

    snprintf(path, DIRECTION_MAX, "/sys/class/pwm/pwmchip0/pwm%d/enable", pwmnum);
    fd = open(path, O_WRONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Failed to open in enable!\n");
        return -1;
    }

    write(fd, s_enable_str, strlen(s_enable_str));
    close(fd);

    return 0;
}

// echo 10000000 > pwm0/period
static int PWMWritePeriod(int pwmnum, int value)
{
    char s_value_str[VALUE_MAX];
    char path[VALUE_MAX];
    int fd, byte;

    snprintf(path, VALUE_MAX, "/sys/class/pwm/pwmchip0/pwm%d/period", pwmnum);
    fd = open(path, O_WRONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Failed to open in period!\n");
        return -1;
    }
    byte = snprintf(s_value_str, VALUE_MAX, "%d", value);

    if (write(fd, s_value_str, byte) == -1)
    {
        fprintf(stderr, "Failed to write value in period!\n");
        close(fd);
        return -1;
    }
    close(fd);

    return 0;
}

// echo 8000000 > pwm0/duty_cycle
static int PWMWriteDutyCycle(int pwmnum, int value)
{
    char s_value_str[VALUE_MAX];
    char path[VALUE_MAX];
    int fd, byte;

    snprintf(path, VALUE_MAX, "/sys/class/pwm/pwmchip0/pwm%d/duty_cycle", pwmnum);
    fd = open(path, O_WRONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Failed to open in duty cycle!\n");
        return -1;
    }
    byte = snprintf(s_value_str, VALUE_MAX, "%d", value);

    if (write(fd, s_value_str, byte) == -1)
    {
        fprintf(stderr, "Failed to write value in duty cycle!\n");
        close(fd);
        return -1;
    }
    close(fd);

    return 0;
}

int readDHT11(int *temperature, int *humidity, int DHT_PIN)
{
    if (wiringPiSetup() == -1)
        return -1;

    int dht_data[5] = {0, 0, 0, 0, 0};

    // Send start signal
    pinMode(DHT_PIN, OUTPUT);
    digitalWrite(DHT_PIN, LOW);
    delay(18);

    digitalWrite(DHT_PIN, HIGH);
    delayMicroseconds(40);

    pinMode(DHT_PIN, INPUT);

    // Read DHT response
    int counter = 0;
    while (digitalRead(DHT_PIN) == LOW)
    {
        delayMicroseconds(2);
        if (++counter == 1000)
            return -1;
    }

    counter = 0;
    while (digitalRead(DHT_PIN) == HIGH)
    {
        delayMicroseconds(2);
        if (++counter == 1000)
            return -1;
    }

    // Read data
    for (int i = 0; i < 5; ++i)
    {
        for (int j = 7; j >= 0; --j)
        {
            counter = 0;
            while (digitalRead(DHT_PIN) == LOW)
            {
                delayMicroseconds(2);
                if (++counter == 1000)
                    return -1;
            }

            counter = 0;
            while (digitalRead(DHT_PIN) == HIGH)
            {
                delayMicroseconds(2);
                if (++counter == 1000)
                    return -1;
            }

            if (counter > 30)
                dht_data[i] |= (1 << j);
        }
    }

    // Verify checksum
    if ((dht_data[0] + dht_data[1] + dht_data[2] + dht_data[3]) & 0xFF != dht_data[4])
        return -1;

    // Assign temperature and humidity values
    *humidity = dht_data[0];
    *temperature = dht_data[2];

    return 0;
}

int temperature1, humidity1;
void *measure1()
{
    readDHT11(&temperature1, &humidity1, DHT_PIN_1);
    // printf("Temp: %d°C    Humidity: %d%%\n", temperature1, humidity1);
    pthread_exit(0);
}

int temperature2, humidity2;
void *measure2()
{
    readDHT11(&temperature2, &humidity2, DHT_PIN_2);
    // printf("Temp: %d°C    Humidity: %d%%\n", temperature2, humidity2);
    pthread_exit(0);
}

int avg_temperature, avg_humidity;
int main()
{
    // int temperature1, humidity1;
    // int temperature2, humidity2;

    if (GPIOExport(LED_PIN) == -1)
    {
        return 1;
    }

    if (GPIODirection(LED_PIN, OUT) == -1)
    {
        return 2;
    }

    PWMExport(PWM);
    PWMWritePeriod(PWM, 10000000);
    PWMWriteDutyCycle(PWM, 0);
    PWMEnable(PWM);

    while (1)
    {
        // if (readDHT11(&temperature, &humidity) == 0) {
        //     printf("Temp: %d°C    Humidity: %d%%\n", temperature, humidity);
        // } else {
        //     fprintf(stderr, "Failed to read from DHT sensor\n");
        // }

        // readDHT11(&temperature1, &humidity1, DHT_PIN_1);
        // readDHT11(&temperature2, &humidity2, DHT_PIN_2);
        // printf("Temp: %d°C    Humidity: %d%%\n", temperature1, humidity1);
        // printf("Temp: %d°C    Humidity: %d%%\n", temperature2, humidity2);
        // delay(2000); // 2-second delay

        pthread_t p_thread1, p_thread2;
        int status_1, status_2;
        pthread_create(&p_thread1, NULL, measure1, NULL);
        pthread_create(&p_thread2, NULL, measure2, NULL);
        delay(1000); // 2-second delay

        avg_temperature = (temperature1 + temperature2) / 2;
        avg_humidity = (humidity1 + humidity2) / 2;
        printf("Temp: %d°C    Humidity: %d%%\n", avg_temperature, avg_humidity);

        if (avg_temperature >= 10)
        {
            GPIOWrite(LED_PIN, 0);
            PWMWriteDutyCycle(PWM, avg_temperature * 200000);
        }
        else
        {
            GPIOWrite(LED_PIN, 1);
        }
    }

    return 0;
}
