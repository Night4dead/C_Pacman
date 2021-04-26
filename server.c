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

#define SM_GRID_WIDTH 10
#define SM_GRID_HEIGHT 6
#define MD_GRID_WIDTH 13
#define MD_GRID_HEIGHT 9
#define LG_GRID_WIDTH 16
#define LG_GRID_HEIGHT 12

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
const char *WELCOME = "Bienvenu sur le serveur pacman !\n";
int DEBUG=0;

//grid structure, tracks the game;
struct Grid {
    int **grid;
    int grid_width;
    int grid_height;
    int **ghosts;
    int nb_ghosts;
    int **points;
    int nb_points;
    int *player;
    int score;
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
    exit(EXIT_FAILURE);
}


//server connection
int createServerSocket(){
    int serverSocket;

    serverSocket=socket(PF_INET, SOCK_STREAM,0);

    if (serverSocket<0){
        erreur("création de socket");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    return serverSocket;
}

void bindServer(int serverSocket,struct sockaddr *adServ, int len){
    if(bind(serverSocket,(struct sockaddr*)adServ, len) ==-1){
        erreur("nommage");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }
}

void listenServer(int serverSocket,int n){
    if(listen(serverSocket, n) == -1){
        erreur("listen");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }
}

int acceptConnection(int server, struct sockaddr* adCl, int *len){
    int client;

    client = accept(server,adCl,(unsigned int*)len);

    if(client ==-1){
        erreur("accept");
        close(server);
        exit(EXIT_FAILURE);
    }

    return client;
}
//end server connection


//input handling
void test_debug(int *a,const char *argv){
    if(strcmp(argv,"-d")==0|| strcmp(argv,"--debug")==0){
        *a=1;
    }
}

void cleanBuffer(int *nb,char s[]){
    if(*nb > 0 ){
        s[*nb]=0;
        *nb=0;
    }
}
//end input handling


//grid handling
void initGrid(struct Grid* grid, int h, int w, int n, int m){
    (*grid).ghosts = malloc(sizeof(int*)*n);
    for(int i=0;i<n;i++){
        (*grid).ghosts[i]= malloc(sizeof(int)*2);
    }

    (*grid).points = malloc(sizeof(int*)*m);
    for (int i=0; i<m;i++){
        (*grid).points[i] = malloc(sizeof(int)*2);
    }

    (*grid).player= malloc(sizeof(int)*2);

    (*grid).grid = malloc(sizeof(int*)*h);
    for (int i = 0; i < h; i++) {
        (*grid).grid[i]= malloc(sizeof(int)*w);
    }
}

int present(int** grid,int h, int posx,int posy){
    for (int i = 0; i < h; i++) {
        if(grid[i][0]==posx && grid[i][1]==posy){
            return 1;
        }
    }
    return 0;
}

void toStringGrid(struct Grid grid,char *buffer){
    strcat(buffer,"  Score : ");
    char temp_score[10];
    sprintf(temp_score,"%d",grid.score);
    strcat(buffer, temp_score);
    strcat(buffer,"|");
    for (int i = 0; i < grid.grid_height; ++i) {
        for (int j = 0; j < grid.grid_width; ++j) {
            char temp[5];
            sprintf(temp,"%d",grid.grid[i][j]);
            strcat(buffer,temp);
        }
        if(i<grid.grid_height-1){
            strcat(buffer,"|");
        }
    }
}

void genGrid(struct Grid *grid){
    initGrid(grid, (*grid).grid_height,(*grid).grid_width, (*grid).nb_ghosts, (*grid).nb_points);
    //points initialization;
    int i=0;
    int x=0,y=0;
    do {
        x=rand()%(*grid).grid_width;
        y=rand()%(*grid).grid_height;
        if(present((*grid).points,(*grid).nb_points,x,y)==0 && (x != (*grid).grid_width/2 || y != (*grid).grid_height)){
            (*grid).points[i][0]=x;
            (*grid).points[i][1]=y;
            i++;
        }
    }while(i<(*grid).nb_points);
    //ghosts initialization;
    int x_g=0,y_g=0;
    i=0;
    do{
        int x_g = rand()%(*grid).grid_width;
        int y_g = rand()%(*grid).grid_height;
        if(present((*grid).points,(*grid).nb_points,x_g,y_g)==0 && present((*grid).ghosts,(*grid).nb_ghosts,x_g,y_g)==0 && (x_g != (*grid).grid_width/2 || y_g != (*grid).grid_height)){
            (*grid).ghosts[i][0]=x_g;
            (*grid).ghosts[i][1]=y_g;
            i++;
        }
    } while(i<(*grid).nb_ghosts);
    //player initialization;
    (*grid).player[0]=(*grid).grid_width/2;
    (*grid).player[1]=(*grid).grid_height/2;
    //main grid initialization;
    for (int i = 0; i < (*grid).grid_height; i++) {
        for (int j = 0; j < (*grid).grid_width; j++) {
            (*grid).grid[i][j]=0;
        }
    }

    //placing elements in the main grid;

    //points;
    for (int i = 0; i < (*grid).nb_points; i++) {
        (*grid).grid[(*grid).points[i][1]][(*grid).points[i][0]]=(i%2==0? 1 : (i%3==0 ? 2 : 1));
    }
    //ghosts;
    for (int i = 0; i < (*grid).nb_ghosts; i++) {
        (*grid).grid[(*grid).ghosts[i][1]][(*grid).ghosts[i][0]]=4;
    }
    //player;
    (*grid).grid[(*grid).player[1]][(*grid).player[0]]=3;
    (*grid).score=0;
}

void initSize(char* buf, struct Grid* grid){
    if(strcmp(buf,"sm")==0){
        (*grid).grid_width = SM_GRID_WIDTH;
        (*grid).grid_height = SM_GRID_HEIGHT;
        (*grid).nb_points = 30;
    }
    if(strcmp(buf,"md")==0){
        (*grid).grid_width = MD_GRID_WIDTH;
        (*grid).grid_height = MD_GRID_HEIGHT;
        (*grid).nb_points = 50;
    }
    if(strcmp(buf,"lg")==0){
        (*grid).grid_width = LG_GRID_WIDTH;
        (*grid).grid_height = LG_GRID_HEIGHT;
        (*grid).nb_points = 70;
    }
}

void display(struct Grid grid){
    printf("\nTour du joueur : %d\t\t", 0);
    printf("Score : %d\n", grid.score);
    for(int i=0;i<grid.grid_height; i++) {
        for(int j=0;j<grid.grid_width;j++){
            switch (grid.grid[i][j]) {
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
//end grid handling
int allowedMove(int dest, int max){
    return (dest<max && dest>=0);
}

int getPointTest(int** points, int* nb_points, int player_x, int player_y){
    for (int i = 0; i < nb_points; ++i) {
        if(points[i][0]==player_x && points[i][1]==player_y){
            //delete the fucking points
            return 1;
        }
    }
    return 0;
}

int getGhostTest(struct Grid grid){
    for (int i = 0; i < grid.nb_ghosts; ++i) {
        if(grid.ghosts[i][0]==grid.player[1] && grid.ghosts[i][1]==grid.player[0]){
            //lose the game
            return 1;
        }
    }
    return 0;
}

void move(char* buf, struct Grid* grid,int client){
    if (strcmp(buf,"forw")==0){
        if(DEBUG==1){
            printf("%s\n",buf);
        }
        if(allowedMove((*grid).player[1]-1,(*grid).grid_height)==1){
            (*grid).player[1]-=1;
            if(getGhostTest(*grid)==0) (*grid).grid[(*grid).player[1]][(*grid).player[0]]=3;
            (*grid).grid[(*grid).player[1]+1][(*grid).player[0]]=0;
            if(getPointTest((*grid).points,(*grid).nb_points,(*grid).player[0],(*grid).player[1])==1) (*grid).score+=10;
        } else if(DEBUG==1){
            printf("destination non valide!\n");
        }
    } else if(strcmp(buf,"back")==0) {
        if(DEBUG==1){
            printf("%s\n",buf);
        }
        if(allowedMove((*grid).player[1]+1,(*grid).grid_height)==1){
            (*grid).player[1]+=1;
            if(getGhostTest(*grid)==0) (*grid).grid[(*grid).player[1]][(*grid).player[0]]=3;
            (*grid).grid[(*grid).player[1]-1][(*grid).player[0]]=0;
            if(getPointTest((*grid).points,(*grid).nb_points,(*grid).player[0],(*grid).player[1])==1) (*grid).score+=10;
        } else if(DEBUG==1){
            printf("destination non valide!\n");
        }
    } else if(strcmp(buf,"left")==0) {
        if(DEBUG==1){
            printf("%s\n",buf);
        }
        if(allowedMove((*grid).player[0]-1,(*grid).grid_width)==1){
            (*grid).player[0]-=1;
            if(getGhostTest(*grid)==0) (*grid).grid[(*grid).player[1]][(*grid).player[0]]=3;
            (*grid).grid[(*grid).player[1]][(*grid).player[0]+1]=0;
            if(getPointTest((*grid).points,(*grid).nb_points,(*grid).player[0],(*grid).player[1])==1) (*grid).score+=10;
        } else if(DEBUG==1){
            printf("destination non valide!\n");
        }
    } else if(strcmp(buf,"rght")==0) {
        if(DEBUG==1){
            printf("%s\n",buf);
        }
        if(allowedMove((*grid).player[0]+1,(*grid).grid_width)==1){
            (*grid).player[0]+=1;
            if(getGhostTest(*grid)==0) (*grid).grid[(*grid).player[1]][(*grid).player[0]]=3;
            (*grid).grid[(*grid).player[1]][(*grid).player[0]-1]=0;
            if(getPointTest((*grid).points,&((*grid).nb_points),(*grid).player[0],(*grid).player[1])==1) {
                (*grid).score+=10;

            }
        } else if(DEBUG==1){
            printf("destination non valide!\n");
        }
    } else if(testExit(buf)==1){
        printf("Le client %d a quitté\n",client);
        exit(EXIT_SUCCESS);
    }
}

void computerMove(){
    //to implement
}


//main
int main(int argc, char const *argv[]){
    //variables

    //Grid;
    struct Grid grid;

    //sockets
    int fdSocketServer;
    int fdSocketClient;
    struct sockaddr_in adServer;
    struct sockaddr_in adClient;

    //commands for moving pacman
    char buffer[MAX_BUFFER];

    //var for gamemode choice


    //var for size of grid
    int nbReceived;
    char* gridtostr;
    int grid_send;
    int num_client=0;
    int nb_listen;
    int lenAd;

    //var initialisation
    //test argc/argv:

    if(argc<2){
        erreur("init, nombre de variables insuffisants");
        print_help();
        exit(EXIT_FAILURE);
    }

    if(strcmp(argv[1],"--help")==0){
        print_help();
        exit(EXIT_SUCCESS);
    }

    if((nb_listen=atoi(argv[1]))==0){
        erreur("init, premier argument n'est pas le nombre de clients autorisés");
        print_help();
        exit(EXIT_FAILURE);
    }

    if(argc>2){
        test_debug(&DEBUG,argv[2]);
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
        printf("Client n*%d connecté ! at : %s\n",num_client,inet_ntoa(adClient.sin_addr));
        num_client++;
        int pid = fork();
        if(pid==0){
            int iter=0;
            int msg=send(fdSocketClient,WELCOME, strlen(WELCOME),0);
            if(msg<0){
                printf("Erreur d'envois\n");
                exit(EXIT_FAILURE);
            }

            //start of config for game
            while(testExit(buffer)!=1){
                printf("\n%d\n",iter);
                if(iter>=0 && iter<2){
                    nbReceived = recv(fdSocketClient,buffer,MAX_BUFFER,0);
                    if(nbReceived<0){
                        printf("Erreur de réception\n");
                    } else {
                        cleanBuffer(&nbReceived,buffer);
                        if (iter==0){
                            if(DEBUG==1){
                                printf("Client n*%d : %s\n",num_client,buffer);
                            }
                            initSize(buffer,&grid);
                        } else if(iter==1) {
                            if(DEBUG==1){
                                printf("Client n*%d : %s\n",num_client,buffer);
                            }
                            grid.nb_ghosts = atoi(buffer);
                            genGrid(&grid);
                            if(DEBUG==1){
                                printf("\nClient n*%d",num_client);
                                display(grid);

                            }
                            gridtostr = malloc(sizeof(char)*(grid.grid_width+1)*grid.grid_height);
                            toStringGrid(grid,gridtostr);
                            grid_send = send(fdSocketClient,gridtostr, (long)strlen(gridtostr),0);
                            if(grid_send<0){
                                printf("Erreur d'envois\n");
                                exit(EXIT_FAILURE);
                            }
                            strcpy(gridtostr,"");
                        }
                    }
                } else {
                    nbReceived= recv(fdSocketClient,buffer,MAX_BUFFER,0);
                    if(nbReceived<0){
                        printf("Erreur de réception\n");
                    } else {
                        cleanBuffer(&nbReceived,buffer);
                        move(buffer,&grid,num_client);
                        if(DEBUG==1){
                            printf("\nClient n*%d",num_client);
                            display(grid);

                        }
                        /*computerMove();
                        if(DEBUG==1){
                            printf("\nClient n*%d",num_client);
                            display(grid);
                        }*/
                        toStringGrid(grid,gridtostr);
                        grid_send = send(fdSocketClient,gridtostr, (long)strlen(gridtostr),0);
                        if(grid_send<0){
                            printf("Erreur d'envois\n");
                            exit(EXIT_FAILURE);
                        }
                        strcpy(gridtostr,"");
                    }
                }

                //add test for start of game

                iter++;
            }
            close(fdSocketClient);
            printf("Client %d a quitté le server\n",num_client);
            exit(EXIT_SUCCESS);
        }
    }

    close(fdSocketServer);


    return 0;
}