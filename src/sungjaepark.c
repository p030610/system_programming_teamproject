// 시프실 팀플용 프로젝트 코드 - 박성재
// 담당 모듈 : 

// 펠티어	
// 펌프	
// 차양막(dc모터)	
// lcd	



#define PWM_EXPORT "/sys/class/pwm/pwmchip0/export"
#define PWM_ENABLE "/sys/class/pwm/pwmchip0/pwm0/enable"
#define PWM_PERIOD "/sys/class/pwm/pwmchip0/pwm0/period"
#define PWM_DUTY_CYCLE "/sys/class/pwm/pwmchip0/pwm0/duty_cycle"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <wiringPi.h>


#define IN 0
#define OUT 1
#define PWM 0

#define LOW 0
#define HIGH 1
#define VALUE_MAX 256
#define DIRECTION_MAX 256

#define BUFFER_MAX 256
#define SERVO_MIN 1000000    // 서보 모터의 최소 위치에 해당하는 duty cycle (예: 1ms)
#define SERVO_MAX 2000000    // 서보 모터의 최대 위치에 해당하는 duty cycle (예: 2ms)
#define SERVO_PERIOD 20000000 // 서보 모터의 주기 (예: 20ms)
#define WIPER_CYCLE 5        // 와이퍼 동작 횟수

static void PWMWrite(const char *file, int value) {
    char buffer[BUFFER_MAX];
    int fd, bytes_written;

    fd = open(file, O_WRONLY);
    if (fd < 0) {
        fprintf(stderr, "Failed to open %s\n", file);
        return;
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", value);
    write(fd, buffer, bytes_written);
    close(fd);
}

static int GPIOExport(int pin) {
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open export for writing!\n");
        return (-1);
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);
    return (0);
}

static int GPIODirection(int pin, int dir) {
    static const char s_directions_str[] = "in\0out";

    char path[DIRECTION_MAX];
    int fd;

    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open gpio direction for writing!\n");
        return (-1);
    }

    if (-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) {
        fprintf(stderr, "Failed to set direction!\n");
        return (-1);
    }
}

static int GPIOWrite(int pin, int value) {
    static const char s_values_str[] = "01";

    char path[VALUE_MAX];
    int fd;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open gpio value for writing!\n");
        return (-1);
    }

    if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
        fprintf(stderr, "Failed to write value!\n");
        return (-1);
    }

    close(fd);
    return (0);
}

static int GPIOUnexport(int pin) {
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open unexport for writing!\n");
        return (-1);
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);
    return (0);
}


int peltier()//추가하드웨어필요
{
    return 0;
}
int pump()//추가하드웨어필요
{    

    return 0;
}
int lcd()
{
    return 0;
}
int led()
{
    return 0;
}
int main()
{

  if(wiringPiSetupPhys() == -1)
      return -1;

  pinMode(11, 1);
  pinMode(12, 1);
  pinMode(15, 1);
  pinMode(16, 1);

  digitalWrite(11, HIGH);
  digitalWrite(12, LOW);
  digitalWrite(15, HIGH);
  digitalWrite(16, LOW);


  return 0;
}
