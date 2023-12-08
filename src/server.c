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
    int clnt_sock = *(int*)arg;
    pthread_t pi_thread;

    char msg[256] = {0};


    while(1)
    {
        read(clnt_sock, msg, sizeof(msg));

        usleep(5000000);

        printf("%s\n", msg);
    }


}

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