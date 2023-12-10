#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
<<<<<<< HEAD
#include <wiringPi.h>
#include <wiringPiI2C.h>

#define I2C_ADDR 0x27
#define LCD_WIDTH 16

#define LCD_CHR 1
#define LCD_CMD 0

#define LCD_LINE_1 0x80
#define LCD_LINE_2 0xC0
#define LCD_LINE_3 0x94
#define LCD_LINE_4 0xD4

#define LCD_BACKLIGHT 0x08
#define ENABLE 0b00000100

#define E_PULSE 0.0005
#define E_DELAY 0.0005

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

#define buffersize 1000

int fd;

void lcd_init();
void lcd_byte(int bits, int mode);
void lcd_toggle_enable(int bits);
void lcd_string(const char *message, int line);

void lcd_init()
{
    lcd_byte(0x33, LCD_CMD);
    lcd_byte(0x32, LCD_CMD);
    lcd_byte(0x06, LCD_CMD);
    lcd_byte(0x0C, LCD_CMD);
    lcd_byte(0x28, LCD_CMD);
    lcd_byte(0x01, LCD_CMD);

    delay(E_DELAY * 1000);
}

void lcd_byte(int bits, int mode)
{
    int bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT;
    int bits_low = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT;

    wiringPiI2CWrite(fd, bits_high);
    lcd_toggle_enable(bits_high);

    wiringPiI2CWrite(fd, bits_low);
    lcd_toggle_enable(bits_low);
}

void lcd_toggle_enable(int bits)
{
    delay(E_DELAY);
    wiringPiI2CWrite(fd, (bits | ENABLE));
    delay(E_PULSE);
    wiringPiI2CWrite(fd, (bits & ~ENABLE));
    delay(E_DELAY);
}

void lcd_string(const char *message, int line)
{
    lcd_byte(line, LCD_CMD);

    int i;
    for (i = 0; i < LCD_WIDTH; i++)
    {
        lcd_byte(message[i], LCD_CHR);
    }
}

void error_handling(char *message) { // 모든 소켓 관련 에러 핸들링
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}

void* execute_outputs(void* arg)
{
    wiringPiSetupPhys();
    wiringPiSetup();

    int state;
    char * temp;
    char * hum;
    char * bright;
    char * groundhum;
    char * water;
    char * rain;
    int tempint = 0;
    int humint = 0;
    int brightint = 0;
    int groundhumint = 0;
    int waterint = 0;
    int rainint = 0;
=======


#define buffersize 1000

void error_handling(char *message) { // 모든 소켓 관련 에러 핸들링
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}


// // gogoclient 서버에 접속하는 client들이 스레드를 통해서 동시 출력이 가능할 수 있도록 하는 함수 gogo 
// void* gogoclient(void* arg) { // 여기서 인수 = client의 파일 스크립터를 가르키는 주소
//     int clnt_sock = *(int*)arg; // 그 주소를 int로 바꿈 dup2 , close int를 충족시켜주기 위함
//     int state;
//     char buffer[50];
//     pthread_t shell_thread; // shell함수로 출발하기 위함
//     pthread_create(&shell_thread, NULL, shell, NULL); // shell 함수를 스레드로 실행시킴

//     char msg[256] = {0};

//     read(clnt_sock, msg, sizeof(msg));
    
    
//     // 이시점에서 bash프로그램을 터미널에서 입출력을 담당할 수 있음


//     pthread_join(shell_thread, NULL); // 위 스레드가 종료될때까지 대기 -> 클라이언트가 종료할때까지

//     pthread_exit(&state);
//     close(clnt_sock); // 손님파일 디스크립터 초기화
  

//     return NULL;
// }

void* execute_outputs(void* arg)
{
    int state;
>>>>>>> 016028d67ade96de717970cd49b36d774ff27fc9
    int clnt_sock = *(int*)arg;
    pthread_t pi_thread;

    char msg[256] = {0};

<<<<<<< HEAD
    fd = wiringPiI2CSetup(I2C_ADDR);

    lcd_init();
=======
>>>>>>> 016028d67ade96de717970cd49b36d774ff27fc9

    while(1)
    {
        read(clnt_sock, msg, sizeof(msg));

<<<<<<< HEAD
        usleep(50000);

        char * tok;

        tok = strtok(msg, " ");

        int index = 0;

        while(tok != NULL)
        {
            if(index == 1)
            {
                temp = tok;
            }
            else if(index == 2)
            {
                hum = tok;
            }
            else if(index == 3)
            {
                bright = tok;
            }
            else if(index == 4)
            {
                groundhum = tok;
            }
            else if(index == 5)
            {
                water = tok;
            }
            else if(index == 6)
            {
                rain = tok;
            }
            index++;
            tok = strtok(NULL, " ");
        }

        if(strcmp(temp,"0") != 0)
        {
            tempint = atoi(temp);
        }
        if(strcmp(hum,"0") != 0)
        {
            humint = atoi(hum);
        }
        if(strcmp(bright,"0") != 0)
        {
            brightint = atoi(bright); 
        }
        if(strcmp(groundhum,"0") != 0)
        {
            groundhumint = atoi(groundhum); 
        }
        if(strcmp(water,"0") != 0)
        {
            waterint = atoi(water); 
        }
        if(strcmp(rain,"0") != 0)
        {
            rainint = atoi(rain); 
        }

        printf("temp : %d, hum :  %d, bright : %d, groundhum : %d, water : %d, rain : %d\n", tempint, humint, brightint, groundhumint, waterint, rainint);
        
        pinMode(11, 1);
        pinMode(12, 1);
        pinMode(15, 1);
        pinMode(16, 1);
        pinMode(21, 1);
        pinMode(22, 1);

        if((brightint > 600))//차양막 작동
        {
            printf("light blocker is working..\n");
            digitalWrite(11, LOW);
            digitalWrite(12, HIGH);
        }
        else
        {
            printf("light blocker stop\n");
            digitalWrite(11, HIGH);
            digitalWrite(12, LOW);
        }

        if((tempint < 30))//펠티어소자 출력
        {
            printf("peltier is working..\n");
            digitalWrite(15, HIGH);
            digitalWrite(16, LOW);
        }
        else
        {
            printf("peltier is not working..\n");
            digitalWrite(15, LOW);
            digitalWrite(16, LOW);
        }

        if((groundhumint > 900))//펌프 작동
        {
            printf("pump is working..\n");
            digitalWrite(21, HIGH);
            digitalWrite(22, LOW);
        }
        else
        {
            printf("pump is not working..\n");
            digitalWrite(21, LOW);
            digitalWrite(22, LOW);
        }

        lcd_string("10", LCD_LINE_1);
        lcd_string("20", LCD_LINE_2);
        delay(3000);

        printf("***************************************************************************\n");

    }
}


=======
        usleep(5000000);

        printf("%s\n", msg);
    }


}

>>>>>>> 016028d67ade96de717970cd49b36d774ff27fc9
int main(int argc, char* argv[]) {
    char buf[buffersize]; // while문 종료를 위한 buffer
    int serv_sock,clnt_sock = -1;          // 서버 소켓, accept를 통해 입력받을 client 소켓의 파일 디스크립터 선언
    int byte_received; 

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);      // 매개변수를 제대로 입력받지 못했다면 출력
    }

   
    serv_sock = socket(AF_INET, SOCK_STREAM, 0); // 서버 소켓 create
    if (serv_sock == -1) { // 반환 값이 -1이면 에러핸들링
        error_handling("socket() error");
    }

    
    struct sockaddr_in serv_addr;       // 소켓의 주소, 연결방법, 주소 사이즈 ,IP,포트 설정을 받는 구조체 선언
    memset(&serv_addr, 0, sizeof(serv_addr)); // 구조체 초기화
    serv_addr.sin_family = AF_INET; // ipv4로 지정
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 엔디안 처리를 해주며 이때 IP는 0.0.0.0 이 되어 사용가능한 모든 클라이언트가 연결할 수 있도록 설정
    serv_addr.sin_port = htons(atoi(argv[1])); // 입력한 포트를 저장, int형태로 변환 후 엔디안처리 

    
    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) { //위 설정한 주소들을 서버 소켓에 바인딩
        error_handling("bind() error");
    }

 
    if (listen(serv_sock, 5) == -1) { // client의 요청 대기
        error_handling("listen() error");
    }

   
    while (1) { // while문 실행
            
            clnt_sock = accept(serv_sock, NULL, NULL); // clnt과 serv_sock의 소켓연결 이때 clnt_sock의 주소와 크기는 따로 설정할 필요없음 
        if (clnt_sock == -1) {
            error_handling("accept() error");
        }

        pthread_t client_thread; // client와 연결 되었을때의 스레드 지정
        pthread_create(&client_thread, NULL, execute_outputs, (void*)&clnt_sock); 
        // pthread_join(&client_thread, NULL);

    }
    close(serv_sock); // 서버 종료

    return 0;
}
