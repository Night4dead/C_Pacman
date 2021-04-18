//
// Created by night on 4/18/21.
//

//imports
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


#define PORT 6000
#define MAX_BUFFER 1000

const char *EXIT = "exit";

//grid structure, tracks the game;
struct Grid {
    char **grid;
    int **ghosts;
    int **points;
    int **player;
};

//functions

void erreur(const char *message) {
    printf("Erreur durant : %s\n", message);
    exit(EXIT_FAILURE);
}


int testExit(char buffer[]){
    return strcmp(buffer,EXIT)==0;
}

int createServerSocket(){
    int serverSocket;

    serverSocket=socket(PF_INET, SOCK_STREAM,0);

    if (serverSocket<0){
        erreur("création de socket");
        exit(EXIT_FAILURE);
    }

    return serverSocket;
}

void bindServer(int serverSocket,struct sockaddr *adServ, int len){
    if(bind(serverSocket,(struct sockaddr*)adServ, len) ==-1){
        erreur("nommage");
        exit(EXIT_FAILURE);
    }
}

void listenServer(int serverSocket,int n){
    if(listen(serverSocket, n) == -1){
        erreur("listen");
        exit(EXIT_FAILURE);
    }
}

int acceptConnection(int server, struct sockaddr* adCl, int *len){
    int client;

    client = accept(server,adCl,&len);

    if(client ==-1){
        erreur("accept");
        exit(EXIT_FAILURE);
    }

    return client;
}

//main
int main(int argc, char const *argv[]){
    //variables
    int fdSocketServer;
    int fdSocketClient;
    struct sockaddr_in adServer;
    struct sockaddr_in adClient;
    char buffer[MAX_BUFFER];
    int nbReceived;
    int nb_listen;
    int lenAd;

    //var initialisation



    //server setup

    fdSocketServer= createServerSocket();


    lenAd = sizeof(struct sockaddr_in);
    memset(&adServer,0x00,lenAd);
    adServer.sin_family = PF_INET;
    adServer.sin_addr.s_addr= htonl(INADDR_ANY);
    adServer.sin_port = htons(PORT);

    bindServer(fdSocketServer,(struct sockaddr*)&adServer,lenAd);

    listenServer(fdSocketServer,nb_listen);

    while(1){
        fdSocketClient = acceptConnection(fdSocketServer,(struct sockaddr*)&adClient,&lenAd);

        int pid = fork();
        if(pid==0){
            printf("Client connecté !");

            while(testExit(buffer)!=-1){

            }
        }
    }

    close(fdSocketServer);
    close(fdSocketClient);

    return 0;
}