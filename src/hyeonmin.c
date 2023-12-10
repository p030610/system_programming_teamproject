#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

// SPI 설정
static const char *DEVICE = "/dev/spidev0.0";
static uint8_t MODE = 0;
static uint8_t BITS = 8;
static uint32_t CLOCK = 1000000;
static uint16_t DELAY = 5;

// PWM 설정
#define PWM_EXPORT "/sys/class/pwm/pwmchip0/export"
#define PWM_ENABLE_0 "/sys/class/pwm/pwmchip0/pwm0/enable"
#define PWM_PERIOD_0 "/sys/class/pwm/pwmchip0/pwm0/period"
#define PWM_DUTY_CYCLE_0 "/sys/class/pwm/pwmchip0/pwm0/duty_cycle"
#define PWM_ENABLE_1 "/sys/class/pwm/pwmchip0/pwm1/enable"
#define PWM_PERIOD_1 "/sys/class/pwm/pwmchip0/pwm1/period"
#define PWM_DUTY_CYCLE_1 "/sys/class/pwm/pwmchip0/pwm1/duty_cycle"

#define BUFFER_MAX 256
#define SERVO_MIN 1000000    // 서보 모터의 최소 위치에 해당하는 duty cycle (예: 1ms)
#define SERVO_MAX 2000000    // 서보 모터의 최대 위치에 해당하는 duty cycle (예: 2ms)
#define SERVO_PERIOD 20000000 // 서보 모터의 주기 (예: 20ms)
#define WIPER_CYCLE 5        // 와이퍼 동작 횟수

// PWM 관련 함수 정의
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

static void servoControl(int pos) {
    PWMWrite(PWM_DUTY_CYCLE_0, pos);
    PWMWrite(PWM_DUTY_CYCLE_1, pos);
}

// 여기에 나머지 SPI 관련 함수 정의 (prepare, control_bits, readadc)
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

int main(int argc, char *argv[]) {
    // 명령줄 인수 확인
     if (argc != 3) {
        printf("Usage: %s <Server IP> <Port>\n", argv[0]);
        exit(1);
    }

    char* server_ip = argv[1];
    int server_port = atoi(argv[2]);
    int sock;
    struct sockaddr_in serv_addr;

    // 네트워크 소켓 생성 및 서버 연결
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket() error");
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(server_ip);
    serv_addr.sin_port = htons(server_port);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("connect() error");
        close(sock);
        return -1;
    }
    
    int fd, adcValue;

    // SPI 디바이스 열기 및 설정
    fd = open(DEVICE, O_RDWR);
    if (fd <= 0) {
        perror("Device open error");
        return -1;
    }
    if (prepare(fd) == -1) {
        perror("Device prepare error");
        return -1;
    }

    // PWM 설정
    PWMWrite(PWM_EXPORT, 0);     // PWM export
    PWMWrite(PWM_PERIOD_0, SERVO_PERIOD); // PWM 주기 설정
    PWMWrite(PWM_ENABLE_0, 1);     // PWM 활성화
    PWMWrite(PWM_PERIOD_1, SERVO_PERIOD); // PWM 주기 설정
    PWMWrite(PWM_ENABLE_1, 1);     // PWM 활성화

    while (1) {
        // ADC 값 읽기
        adcValue = readadc(fd, 0);
        printf("ADC Value: %d\n", adcValue);

        // 비 감지 여부에 따른 서보 모터 제어
        if (adcValue < 700 && adcValue != 0) {  // 비 감지
          char buffer[BUFFER_MAX];
          snprintf(buffer, sizeof(buffer), "w 0 0 0 0 0 %d ", adcValue);
          // 서버에 ADC값 전송
          if(write(sock, buffer, strlen(buffer)) == -1) {
            perror("write() error");
            break;
          }
          for (int i = 0; i < WIPER_CYCLE; i++) {
                // 서보 모터를 최대 위치로 이동
                servoControl(SERVO_MAX);
                usleep(500000);

                // 서보 모터를 최소 위치로 이동
                servoControl(SERVO_MIN);
                usleep(500000);
          }
        }

        usleep(1000000);  // 다음 ADC 읽기까지 대기
    }

    // 종료 처리
    close(sock);
    PWMWrite(PWM_ENABLE_0, 0);
    PWMWrite(PWM_ENABLE_1, 0);
    close(fd);

    return 0;
}

