#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <wiringPi.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void SetPinmode();
void SetSocket();
void Accept();
void SignalLight();
void Red_Light();
void Green_Light();
void ShowSegment(int count);
void ReceiveInformation(int count, char tmp[]);
void SendInformation(int count, char tmp[]);

#define MYPORT 3490
#define BACKLOG 10
#define MAXDATASIZE 128

#define LED_RED 23
#define LED_GREEN 25
#define LEDa 2
#define LEDb 3
#define LEDc 4
#define LEDd 5
#define LEDe 6
#define LEDf 7
#define LEDg 21
#define LEDdp 22

int sockfd, new_fd;
struct sockaddr_in server_addr;
struct sockaddr_in client_addr;
socklen_t sin_size;

char bufRx[MAXDATASIZE];
char bufTx[MAXDATASIZE];
char tmp[MAXDATASIZE];
int numbytes;

int seg[10][8] = {{0, 0, 0, 0, 0, 0, 1, 1},
                  {1, 0, 0, 1, 1, 1, 1, 1},
                  {0, 0, 1, 0, 0, 1, 0, 1},
                  {0, 0, 0, 0, 1, 1, 0, 1},
                  {1, 0, 0, 1, 1, 0, 0, 1},
                  {0, 1, 0, 0, 1, 0, 0, 1},
                  {0, 1, 0, 0, 0, 0, 0, 1},
                  {0, 0, 0, 1, 1, 0, 1, 1},
                  {0, 0, 0, 0, 0, 0, 0, 1},
                  {0, 0, 0, 0, 1, 0, 0, 1}};

int main(void) {
    SetPinmode();

    SetSocket();

    Accept();

    SignalLight();

    return 0;
}

void SetPinmode() {
    if (wiringPiSetup() == -1) {
        exit(1);
    }

    pinMode(LED_RED, 1);
    pinMode(LED_GREEN, 1);
    pinMode(LEDa, 1);
    pinMode(LEDb, 1);
    pinMode(LEDc, 1);
    pinMode(LEDd, 1);
    pinMode(LEDe, 1);
    pinMode(LEDf, 1);
    pinMode(LEDg, 1);
    pinMode(LEDdp, 1);
}

void SetSocket() {
    int yes=1;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(MYPORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(server_addr.sin_zero), '\0', 8);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sin_size = sizeof(struct sockaddr_in);
}

void Accept() {
    while(1) {
        if ((new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size)) == -1) {
            perror("accept");
            continue;
        }
        break;
    }
}

void SignalLight() {
    int red_light_time = 0;
    int green_light_time = 0;

    printf("\n\nTurn on the Signal Light!!\n\n");

    while(1) {
        Red_Light();
        Green_Light();
    }
}

void Red_Light() {
    digitalWrite(LED_RED, 1);

    for(int i=9; i>=0; i--) {
        printf("Red Light %dsecond\n", i);
        ShowSegment(i);
        ReceiveInformation(i, "RED");
        delay(1000);
    }

    digitalWrite(LED_RED, 0);
}

void Green_Light() {
    digitalWrite(LED_GREEN, 1);

    for(int j=9; j>=0; j--) {
        printf("Green Light %dsecond\n", j);
        ShowSegment(j);
        ReceiveInformation(j, "GREEN");
        delay(1000);
    }

    digitalWrite(LED_GREEN, 0);
}

void ShowSegment(int count) {
    digitalWrite(LEDa, seg[count][0]);
    digitalWrite(LEDb, seg[count][1]);
    digitalWrite(LEDc, seg[count][2]);
    digitalWrite(LEDd, seg[count][3]);
    digitalWrite(LEDe, seg[count][4]);
    digitalWrite(LEDf, seg[count][5]);
    digitalWrite(LEDg, seg[count][6]);
    digitalWrite(LEDdp, seg[count][7]);
}

void ReceiveInformation(int count, char tmp[]) {
    if ((numbytes=recv(new_fd, bufRx, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    if (bufRx[0] == 'R') {
        SendInformation(count, tmp);
    } else if (bufRx[0] == 'D') {
        close(new_fd);
        printf("\n\nStop the Signal Light!!\n\n");
        Accept();
    }
}

void SendInformation(int count, char tmp[]) {
    sprintf(bufTx, "%s %dsecond", tmp, count);

    if (send(new_fd, bufTx, strlen(bufTx), 0) == -1)
        perror("send");
}
