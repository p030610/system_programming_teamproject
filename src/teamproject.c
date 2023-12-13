#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
void error_handling(char *message) { // 모든 소켓 관련 에러 핸들링
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static const char *DEVICE = "/dev/spidev0.0";
static const char *DEVICE1 = "/dev/spidev0.0";
static uint8_t MODE = 0;
static uint8_t BITS = 8;
static uint32_t CLOCK = 1000000;
static uint16_t DELAY = 5;

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


int main(int argc, char **argv) {
  char buffer[50];
  int clnt_sock;
  int ave_tem = 0;

   struct sockaddr_in serv_addr;

  if (argc != 3) {
        printf("Usage: %s <IP> <port>\n", argv[0]); // 매개변수를 제대로 입력받지 못했다면 출력
        
    }
   
    //소켓 생성
    clnt_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (clnt_sock == -1) error_handling("socket() error"); // clnt_sock의 파일 디스크립터 선언


    memset(&serv_addr, 0, sizeof(serv_addr));    // serv_addr 구조체 모두 초기화
    serv_addr.sin_family = AF_INET;             //IPv4로 설정
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]); //문자열로 입력된 IP를 바이너리 표현으로 변경
    serv_addr.sin_port = htons(atoi(argv[2])); // 문자열로 입력된 port를 int로 변경 후 호스트 바이트 순서에서 네트워크 바이트순서로 변경 - > 아키텍쳐마다 엔디안이 다를 수 있으므로


    // 서버에 연결
    if (connect(clnt_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) { // client 소켓을 지정한 서버소켓에 연결 시도
        error_handling("connect() error");
    }

    printf("Connection established\n"); // client가 server에 연결될 시 선언


  

    printf("Connection established\n"); // client가 server에 연결될 시 선언

  int fd = open(DEVICE, O_RDWR);
  if (fd <= 0) {
    perror("Device open error");
    return -1;
  }

  if (prepare(fd) == -1) {
    perror("Device prepare error");
    return -1;
  }

  int fd1 = open(DEVICE1, O_RDWR);
  if (fd <= 0) {
    perror("Device open error");
    return -1;
  }

  if (prepare(fd1) == -1) {
    perror("Device prepare error");
    return -1;
  }


  while (1) {
    ave_tem = (readadc(fd, 5) + readadc(fd1, 3))/2 ;
    printf("value: %d\n", ave_tem);
     sprintf(buffer, "w 0 0 %d 0 0 0 ", ave_tem);

    ssize_t sent_bytes = send(clnt_sock, buffer, strlen(buffer), 0);
        if (sent_bytes == -1) {
            error_handling("send() error");
        }  


   
    usleep(100000);
  }
}