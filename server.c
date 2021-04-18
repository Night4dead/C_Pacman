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
#include <ctype.h>


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



int testExit(char buffer[]){
    return strcmp(buffer,EXIT)==0;
}


void print_help(){
    printf("\n"
           " usage : ./server nb_clients [OPTIONS]\n"
           " Lance le serveur sur le port 6000 a l'adresse IP 0.0.0.0, pour le nombre de clients autorisés (nb_clients) \n"
           "\noptions : \n"
           "    -d,  --debug    Affiche les positions non-valides trouvés aléatoirement par les fantomes,\n"
           "                    ainsi que les grilles après chaque coup de l'ordinateur\n"
           "    --help          affiche la page d'aide\n");
}


void erreur(const char *message) {
    printf("Erreur durant : %s\n", message);
    print_help();
    exit(EXIT_FAILURE);
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

void test_debug(int *a,const char *argv){
    if(strcmp(argv,"-d")==0|| strcmp(argv,"--debug")==0){
        *a=1;
    }
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
    int debug=0;
    int nb_listen;
    int lenAd;

    //var initialisation
    //test argc/argv:

    if(argc<2){
        erreur("init, nombre de variables insuffisants");
        exit(EXIT_FAILURE);
    }

    if(strcmp(argv[1],"--help")==0){
        print_help();
        exit(EXIT_SUCCESS);
    }

    if((nb_listen=atoi(argv[1]))==0){
        erreur("init, premier argument n'est pas le nombre de clients autorisés");
        exit(EXIT_FAILURE);
    }

    if(argc>2){
        test_debug(&debug,argv[2]);
    }

    //server setup

    fdSocketServer= createServerSocket();


    lenAd = sizeof(struct sockaddr_in);
    memset(&adServer,0x00,lenAd);
    adServer.sin_family = PF_INET;
    adServer.sin_addr.s_addr= htonl(INADDR_ANY);
    adServer.sin_port = htons(PORT);

    bindServer(fdSocketServer,(struct sockaddr*)&adServer,lenAd);

    listenServer(fdSocketServer,nb_listen);

    printf("Serveur démarré, ip : %s, listening\n", inet_ntoa(adServer.sin_addr));

    while(1){
        fdSocketClient = acceptConnection(fdSocketServer,(struct sockaddr*)&adClient,&lenAd);

        int pid = fork();
        if(pid==0){
            printf("Client connecté !");

            while(testExit(buffer)!=-1){

            }

            exit(EXIT_SUCCESS);
        }
    }

    close(fdSocketServer);
    close(fdSocketClient);

    return 0;
}