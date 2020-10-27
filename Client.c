#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <wiringPi.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void SetPinmode();
void ParameterCheck(int argc);
void ShowInformation();
void MainProgram(char *argv[]);
int ProgramExit(int program_exit);
int ChangeStatus(int status, char *argv[]);
void ReceiveInformation();
void SetSocket(char *argv[]);
void ConnectServer();


#define PORT 3490
#define MAXDATASIZE 100
#define VERSION 1.03

#define SWITCH_STATUS 0
#define SWITCH_EXIT 1

int sockfd, numbytes;
char bufRx[MAXDATASIZE];
char bufTx[MAXDATASIZE];
char bufTmp[MAXDATASIZE] = "";
struct sockaddr_in server_addr;

int main(int argc, char *argv[])
{
    SetPinmode();

    ParameterCheck(argc);

    ShowInformation();

    MainProgram(argv);

    return 0;
}

void SetPinmode() {
    if (wiringPiSetup() == -1) {
        exit(1);
    }

    pinMode(SWITCH_STATUS, 0);
    pinMode(SWITCH_EXIT, 0);
}


void ParameterCheck(int argc) {
    if (argc != 2) {
        fprintf(stderr,"usage: client IP_address_of_the_server\n");
        exit(1);
    }
}


void ShowInformation() {
    printf("##############################################################\n");
    printf("##                                                          ##\n");
    printf("##                welcome v%.2f SIPB_PROGRAM                ##\n", VERSION);
    printf("##                                                          ##\n");
    printf("##    You can get the information by pressing the button.   ##\n");
    printf("##                                                          ##\n");
    printf("##############################################################\n\n\n");
}

void MainProgram(char *argv[]) {
    int status = 0;
    int program_exit = 0;

    while(program_exit == 0) {
    	program_exit = ProgramExit(program_exit);
        status = ChangeStatus(status, argv);

        if(status == 1) {
            while(status == 1) {
                ReceiveInformation();
                status = ChangeStatus(status, argv);
            }
        }
    }
}

int ProgramExit(int program_exit) {
    if(digitalRead(SWITCH_EXIT) == 1) {
        printf("\n\nExit Program. Good bye^^\n\n\n");
        program_exit = 1;
    }

    delay(500);

    return program_exit;
}

int ChangeStatus(int status, char *argv[]) {
    if(digitalRead(SWITCH_STATUS) == 1) {
        if(status == 0) {
            printf("\n\nReceive information.\n\n");
            SetSocket(argv);
            ConnectServer();
            status = 1;
        } else {
            printf("\n\nStop receive information.\n\n");
            strcpy(bufTx, "D");
            if (send(sockfd, bufTx, strlen(bufTx)+1, 0) == -1)
                perror("send");
            close(sockfd);
            status = 0;
        }
        delay(300);
    }

    return status;
}

void ReceiveInformation() {
    strcpy(bufTx, "R");
    if (send(sockfd, bufTx, strlen(bufTx)+1, 0) == -1)
        perror("send");

    if ((numbytes=recv(sockfd, bufRx, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    if (strcmp(bufTmp, bufRx) != 0)
        printf("Current signal light status : %s\n",bufRx);

    strcpy(bufTmp, bufRx);

    delay(100);
}

void SetSocket(char *argv[]) {
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    memset(&(server_addr.sin_zero), '\0', 8);
}

void ConnectServer() {
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        perror("connect");
        exit(1);
    }
}


