//
// Created by night on 4/18/21.
//
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


#define SM_GRID_WIDTH 10
#define SM_GRID_HEIGHT 6
#define MD_GRID_WIDTH 13
#define MD_GRID_HEIGHT 9
#define LG_GRID_WIDTH 16
#define LG_GRID_HEIGHT 12
#define TAILLE 10



#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define PORT 6000
#define MAX_BUFFER 1000

const char *EXIT = "exit";
int NB_GHOSTS;


void cleanBuffer(int *nb,char s[]){
    if(*nb > 0 ){
        s[*nb]=0;
        *nb=0;
    }
}

int testExit(char buffer[]){
    return strcmp(buffer,EXIT)==0;
}

void lireMessager(char buffer[],char text[]){
    printf("%s",text);
    fgets(buffer,MAX_BUFFER,stdin);
    strtok(buffer,"\n");
}


void initSize(char* size){
    char* sm_grid = "1";
    char* md_grid = "2";
    char* lg_grid = "3";
    int valid_input=0;
    while(valid_input==0){
        lireMessager(size,"Veuillez choisir une difficulté ("ANSI_COLOR_GREEN"'1': facile"ANSI_COLOR_RESET", "ANSI_COLOR_CYAN"'2': normale"ANSI_COLOR_RESET", "ANSI_COLOR_MAGENTA"'3': difficile"ANSI_COLOR_RESET") :");
        if(strcmp(sm_grid, size) == 0){
            valid_input = 1;
            NB_GHOSTS=2;
            strcpy(size,"sm");
        }
        else if(strcmp(md_grid, size) == 0){
            valid_input = 1;
            NB_GHOSTS=4;
            strcpy(size,"md");
        }
        else if(strcmp(lg_grid, size) == 0){
            valid_input = 1;
            NB_GHOSTS=6;
            strcpy(size,"lg");
        }
        else{
            printf("Choix impossible, veuillez recommencer svp\n");
        }
    }
}


int sendServ(int serv, char* buffer){
    int msg=send(serv,buffer, strlen(buffer),0);
    if(msg<0){
        printf("Erreur d'envois\n");
        exit(EXIT_FAILURE);
    }
    return msg;
}

void initGameSettings(int serv){
    char sizeGrid[TAILLE];
    char nb_ghosts[TAILLE];
    initSize(sizeGrid);
    sendServ(serv,sizeGrid);
    sprintf(nb_ghosts,"%d",NB_GHOSTS);
}

void doMove(int serv){
    char buf[10];
    int valid=0;
    while(valid==0){
        lireMessager(buf,"Effectuez un déplacement valide (z/w | q/a | s | d |"ANSI_COLOR_MAGENTA" exit"ANSI_COLOR_RESET") : ");
        if(strcmp(buf,"w")==0|| strcmp(buf,"z")==0){
            sendServ(serv,"forw");
            valid=1;
        }
        else if(strcmp(buf,"a")==0|| strcmp(buf,"q")==0){
            sendServ(serv,"left");
            valid=1;
        }
        else if(strcmp(buf,"s")==0){
            sendServ(serv,"back");
            valid=1;
        }
        else if(strcmp(buf,"d")==0){
            sendServ(serv,"rght");
            valid=1;
        }
        else if(testExit(buf)==1){
            sendServ(serv,"exit");
            exit(EXIT_SUCCESS);
        }else {
            printf(" déplacement invalide\n");
        }
    }
    strcpy(buf,"");
}

void displayGridStr(char* buf, int tour){
    int grid=0;
    int i=0;
    if(buf[0]=='0'){
        i=1;
        printf("\nTour du joueur: %d   ",tour);
    }
    for (i; i < strlen(buf); ++i) {
        if(buf[i]=='|'){
            grid=1;
        }
        if(buf[i]=='\n'){
            grid=0;
        }
        if(grid==1){
            switch (buf[i]) {
                case '0':
                    printf("   |");
                    break;
                case '3':
                    printf(ANSI_COLOR_YELLOW" %c" ANSI_COLOR_RESET " |",'C');
                    break;
                case '4':
                    printf(ANSI_COLOR_RED" %c" ANSI_COLOR_RESET " |",'A');
                    break;
                case '2':
                    printf(" %c |",'@');
                    break;
                case '1':
                    printf(" %c |",'*');
                    break;
                case '|':
                    printf("\n|");
                    break;
                default:
                    printf("%c",buf[i]);
                    break;
            }
        } else {
            printf("%c",buf[i]);
        }

    }
    printf("\n");
}

int gameEnd(char *buf, int client){
    char play[4];
    displayGridStr(buf,-1);
    do{
        strcpy(play,"");
        lireMessager(play,"\nVoulez-vous refaire une partie ? ("ANSI_COLOR_GREEN"oui"ANSI_COLOR_RESET" | "ANSI_COLOR_RED"non"ANSI_COLOR_RESET") ");
    }while(strcmp(play,"oui")==1 && strcmp(play,"non")==1);
    if(strcmp(play,"non")==0){
        sendServ(client,EXIT);
        exit(EXIT_FAILURE);
    }
    sendServ(client,"replay");
    return 0;
}


int main(){
    char buffer[MAX_BUFFER];
    char gridstr[MAX_BUFFER];
    int** grid;
    struct sockaddr_in adServer;
    int nbReceived;
    int fdSocketClient= socket(PF_INET,SOCK_STREAM,0);
    adServer.sin_family=PF_INET;
    inet_aton("0.0.0.0",&adServer.sin_addr);
    adServer.sin_port= htons(PORT);
    if(connect(fdSocketClient,(struct sockaddr*)&adServer,sizeof(adServer))==-1){
        printf("Erreur de connect \n");
        exit(EXIT_FAILURE);
    }

    printf("Connexion ok !\n");
    int iter=0;
    while(testExit(buffer)!=1){
        if(iter==0){
            nbReceived=recv(fdSocketClient,buffer,MAX_BUFFER,0);
            if(nbReceived<0){
                printf("Erreur de réception\n");
                exit(EXIT_FAILURE);
            }
            cleanBuffer(&nbReceived,buffer);
            printf("message server : %s\n",buffer);
            initGameSettings(fdSocketClient);
        } else {

            nbReceived= recv(fdSocketClient,gridstr,MAX_BUFFER,0);
            if(nbReceived<0){
                printf("Erreur de réception\n");
                exit(EXIT_FAILURE);
            }
            cleanBuffer(&nbReceived,gridstr);
            if(gridstr[0]!='0'){
                iter = gameEnd(gridstr, fdSocketClient)-1;
                strcpy(buffer,"");
                strcpy(gridstr,"");
            } else{
                system("clear");
                displayGridStr(gridstr,iter);
                doMove(fdSocketClient);
            }
        }
        iter++;
    }
    close(fdSocketClient);
    return 0;
}