#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#define PWM 0 // gpio 18
#define IN 0
#define OUT 1
#define LOW 0
#define HIGH 1
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0])) //SPI readadc
#define MAX_TIME 100
#define DHT_PIN_1 0 // GPIO pin 17
#define buzzer 27 // GPIO pin 27 


#define MAX_TIME 100

static const char *water = "/dev/spidev0.0"; //수위센서 //CH4
static const char *light = "/dev/spidev0.0"; // 조도 센서     //CH0
static const char *soil = "/dev/spidev0.0"; // 토양수분센서    //CH2
static uint8_t MODE = 0;
static uint8_t BITS = 8;
static uint32_t CLOCK = 1000000;
static uint16_t DELAY = 5;

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

static int prepare(int fd) {  //SPI 시작
  if (ioctl(fd, SPI_IOC_WR_MODE, &MODE) == -1) {
    perror("Can't set MODE");
    return -1;
  }

  if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &BITS) == -1) {
    perror("Can't set number of BITS");
    return -1;
  }

  if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &CLOCK) == -1) {
    perror("Can't set write CLOCK");
    return -1;
  }

  if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &CLOCK) == -1) {
    perror("Can't set read CLOCK");
    return -1;
  }

  return 0;
}

uint8_t control_bits_differential(uint8_t channel) {
  return (channel & 7) << 4;
}

uint8_t control_bits(uint8_t channel) {
  return 0x8 | control_bits_differential(channel);
}

int readadc(int fd, uint8_t channel) {
  uint8_t tx[] = {1, control_bits(channel), 0};
  uint8_t rx[3];

  struct spi_ioc_transfer tr = {
      .tx_buf = (unsigned long)tx,
      .rx_buf = (unsigned long)rx,
      .len = ARRAY_SIZE(tx),
      .delay_usecs = DELAY,
      .speed_hz = CLOCK,
      .bits_per_word = BITS,
  };

  if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) == 1) {
    perror("IO Error");
    abort();
  }

  return ((rx[1] << 8) & 0x300) | (rx[2] & 0xFF);
}


static int PWMExport(int pwmnum) {
#define BUFFER_MAX 3
  char buffer[BUFFER_MAX];
  int fd, byte;

  // TODO: Enter the export path.
  fd = open("/sys/class/pwm/pwmchip0/export", O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open export for export!\n");
    return (-1);
  }

  byte = snprintf(buffer, BUFFER_MAX, "%d", pwmnum);
  write(fd, buffer, byte);
  close(fd);

  sleep(1);

  return (0);
}

static int PWMEnable(int pwmnum) {
  static const char s_enable_str[] = "1";

  char path[DIRECTION_MAX];
  int fd;

  // TODO: Enter the enable path.
  snprintf(path, DIRECTION_MAX, "/sys/class/pwm/pwmchip0/pwm0/enable", pwmnum);
  fd = open(path, O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open in enable!\n");
    return -1;
  }

  write(fd, s_enable_str, strlen(s_enable_str));
  close(fd);

  return (0);
}

static int PWMWritePeriod(int pwmnum, int value) {
  char s_value_str[VALUE_MAX];
  char path[VALUE_MAX];
  int fd, byte;

  // TODO: Enter the period path.
  snprintf(path, VALUE_MAX, "/sys/class/pwm/pwmchip0/pwm0/period", pwmnum);
  fd = open(path, O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open in period!\n");
    return (-1);
  }
  byte = snprintf(s_value_str, VALUE_MAX, "%d", value);

  if (-1 == write(fd, s_value_str, byte)) {
    fprintf(stderr, "Failed to write value in period!\n");
    close(fd);
    return -1;
  }
  close(fd);

  return (0);
}

static int PWMWriteDutyCycle(int pwmnum, int value) {
  char s_value_str[VALUE_MAX];
  char path[VALUE_MAX];
  int fd, byte;

  // TODO: Enter the duty_cycle path.
  snprintf(path, VALUE_MAX, "/sys/class/pwm/pwmchip0/pwm0/duty_cycle", pwmnum);
  fd = open(path, O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open in duty cycle!\n");
    return (-1);
  }
  byte = snprintf(s_value_str, VALUE_MAX, "%d", value);

  if (-1 == write(fd, s_value_str, byte)) {
    fprintf(stderr, "Failed to write value in duty cycle!\n");
    close(fd);
    return -1;
  }
  close(fd);

  return (0);
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
    printf("Temp: %d°C    Humidity: %d%%\n", temperature1, humidity1);
    pthread_exit(0);
}


int main(int argc, char **argv) {
  int fd = open(light, O_RDWR);
  int fd1 = open(soil, O_RDWR);
  int fd2 = open(water,O_RDWR);
  int temperature1, humidity1;
  clock_t start_t, end_t;
  double time;

   PWMExport(PWM);
    PWMWritePeriod(PWM, 10000000);
    PWMWriteDutyCycle(PWM, 0);
    PWMEnable(PWM);
 
 
  if (fd <= 0) {
    perror("Device open error");
    return -1;
  }

  if (prepare(fd) == -1) {
    perror("Device prepare error");
    return -1;
  }
  if (fd1 <= 0) {
    perror("Device open error");
    return -1;
  }

  if (prepare(fd1) == -1) {
    perror("Device prepare error");
    return -1;
  }

  if (fd2 <= 0) {
    perror("Device open error");
    return -1;
  }

  if (prepare(fd2) == -1) {
    perror("Device prepare error");
    return -1;
  }

  while (1) {
    pthread_t p_thread1; // 온습도 센서 스레드
    int status_1; // 온습도 센서 스레드 종료시 반환
    pthread_create(&p_thread1, NULL, measure1, NULL);
    printf("조도 value: %d\n", readadc(fd, 0)); // 조도
    printf("토양수분 value: %d\n", readadc(fd1, 2)); // 수분
    printf("수위센서 value: %d\n", readadc(fd2, 4)); // 수위센서 
    
    
    PWMWriteDutyCycle(PWM, 100000);
    usleep(1000000);
  }
}

