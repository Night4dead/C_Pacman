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
int GRID_WIDTH;
int GRID_HEIGHT;
int score;

void display(int** grid){
    printf("\nTour du joueur : %d\t\t", 0);
    printf("Score : %d\n", 0);
    for(int i=0;i<GRID_HEIGHT; i++) {
        for(int j=0;j<GRID_WIDTH;j++){
            switch (grid[i][j]) {
                case 0:
                    printf("   |");
                    break;
                case 3:
                    printf(ANSI_COLOR_YELLOW" %c" ANSI_COLOR_RESET " |",'C');
                    break;
                case 4:
                    printf(ANSI_COLOR_RED" %c" ANSI_COLOR_RESET " |",'A');
                    break;
                case 2:
                    printf(" %c |",'@');
                    break;
                case 1:
                    printf(" %c |",'*');
                    break;
            }
        }
        printf("\n");
    }
}

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
    char* sm_grid = "sm";
    char* md_grid = "md";
    char* lg_grid = "lg";
    int valid_input=0;
    while(valid_input==0){
        lireMessager(size,"Veuillez choisir une taille ('sm': petite, 'md': moyenne, 'lg': grande) :");
        if(strcmp(sm_grid, size) == 0){
            valid_input = 1;
            GRID_HEIGHT=SM_GRID_HEIGHT;
            GRID_WIDTH=SM_GRID_WIDTH;
        }
        else if(strcmp(md_grid, size) == 0){
            valid_input = 1;
            GRID_HEIGHT=MD_GRID_HEIGHT;
            GRID_WIDTH=MD_GRID_WIDTH;
        }
        else if(strcmp(lg_grid, size) == 0){
            valid_input = 1;
            GRID_HEIGHT=LG_GRID_HEIGHT;
            GRID_WIDTH=LG_GRID_WIDTH;
        }
        else{
            printf("Choix impossible, veuillez recommencer svp\n");
        }
    }
}

void initGhosts(char* nb_ghosts){
    int valid_input =0;
    while(valid_input==0){
        lireMessager(nb_ghosts,"Veuillez entrer un nombre de fantomes (entre 2 et 6) : ");
        if(atoi(nb_ghosts)>=2 && atoi(nb_ghosts)<=6){
            valid_input=1;
            NB_GHOSTS= atoi(nb_ghosts);
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
    initGhosts(nb_ghosts);
    sendServ(serv,nb_ghosts);
}

void initGrid(int **grid){
    grid= malloc(sizeof(int*)*GRID_HEIGHT);
    for (int i = 0; i < GRID_HEIGHT; ++i) {
        grid[i]= malloc(sizeof(int)*GRID_WIDTH);
    }
}

void doMove(int serv,char* buf){
    int valid=0;
    while(valid==0){
        lireMessager(buf,"Effectuez un déplacement valide (z/w | q/a | s | d | exit) : ");
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
    printf("\ntest done\n");
}

void displayGridStr(char* buf, int tour){
    printf("\nTour du joueur: %d   ",tour);
    for (int i = 0; i < strlen(buf); ++i) {
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
    }
    printf("\n");
}

int main(){
    char buffer[MAX_BUFFER];
    char move[10];
    char* gridstr;
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
            initGrid(grid);
            gridstr = malloc(sizeof(char)*GRID_HEIGHT*GRID_WIDTH);
            score = 0;
        } else {
            nbReceived= recv(fdSocketClient,gridstr,MAX_BUFFER,0);
            if(nbReceived<0){
                printf("Erreur de réception\n");
                exit(EXIT_FAILURE);
            }
            cleanBuffer(&nbReceived,gridstr);
            displayGridStr(gridstr,iter);
            doMove(fdSocketClient,move);
        }
        iter++;
    }
    close(fdSocketClient);

    return 0;
}