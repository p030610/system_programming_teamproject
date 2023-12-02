#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <wiringPi.h>
#include <softPwm.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static const char *DEVICE = "/dev/spidev0.0";
static uint8_t MODE = 0;
static uint8_t BITS = 8;
static uint32_t CLOCK = 1000000;
static uint16_t DELAY = 5;

#define SERVO 1
#define SERVO_MIN 5
#define SERVO_MAX 25
#define WIPER_CYCLE 5

static int prepare(int fd) {
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

void servoControl(int pos){
    softPwmWrite(SERVO, pos);
}

int main() {
    int fd, adcValue;
    int pos = SERVO_MIN, cycleCount=0;  // 서보 모터 초기 위치
    int dir =1;

    // WiringPi 초기화
    if (wiringPiSetup() == -1) {
        return -1;
    }

    // SPI 디바이스 열기
    fd = open(DEVICE, O_RDWR);
    if (fd <= 0) {
        perror("Device open error");
        return -1;
    }

    // SPI 설정
    if (prepare(fd) == -1) {
        perror("Device prepare error");
        return -1;
    }

    // 서보 모터 설정
    softPwmCreate(SERVO, 0, 200);

   while (1) {
        // ADC 값 읽기
        adcValue = readadc(fd, 0);
        printf("ADC Value: %d\n", adcValue);

        // 비 감지 여부에 따른 서보 모터 제어
        if (adcValue < 900) {  // 비 감지
            for (int i = 0; i < WIPER_CYCLE; i++) {
                // 서보 모터를 최대 위치로 이동
                servoControl(SERVO_MAX);
                delay(500);

                // 서보 모터를 최소 위치로 이동
                servoControl(SERVO_MIN);
                delay(500);
            }
        }

        delay(1000);  // 다음 ADC 읽기까지 대기
    }

    return 0;
}