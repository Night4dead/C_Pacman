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
#include <time.h>
//define
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
//end define
//const and global
const char *EXIT = "exit";
const char *WELCOME = "Bienvenu sur le serveur pacman !\n";
const char *LOSE_GAME = "Vous avez perdu la partie!\n";
const char *WIN_GAME = "Vous avez gagné la partie! Félicitations !\n";
int GAME_MODE=0;
int DEBUG=0;
int CLIENT;
int NUM_CLIENT;
time_t START;
//end const and global

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
struct Timer {
    int minutes;
    int seconds;
};
struct Timer timer;
//end structs

//helpers
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
void clearPointers(struct Grid * grid){
    free((*grid).player);
    for (int i = 0; i < 6; ++i) {
        free((*grid).ghosts[i]);
    }
    free((*grid).ghosts);
    for (int i = 0; i < 70; ++i) {
        free((*grid).points[i]);
    }
    free((*grid).points);
    for (int i = 0; i < LG_GRID_HEIGHT; ++i) {
        free((*grid).grid[i]);
    }
    free((*grid).grid);
}
void cleanBuffer(int *nb,char* s){
    if(*nb > 0 ){
        s[*nb]=0;
        *nb=0;
    }
}

void cleanStr(char* buf){
    buf[0]='\0';
}
//end helpers

//start grid helpers
void gameTime(char * buffer){
    int seconds_from_start;
    time_t now = time(NULL);
    seconds_from_start=now-START;
    START = now;
    timer.seconds-=seconds_from_start;
    if(timer.seconds<0){
        timer.seconds=60+timer.seconds;
        timer.minutes-=1;
    }
    sprintf(buffer," %02d:%02d",timer.minutes,timer.seconds);
}
void toStringGrid(struct Grid grid,char *buffer){
    strcpy(buffer,"");
    char temp_time[5];
    strcpy(temp_time,"");
    sprintf(buffer,"0 Score : %d  Timer : ",grid.score);
    gameTime(temp_time);
    strcat(buffer,temp_time);
    strcat(buffer,"  |");
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
    strcat(buffer,"&");
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
void sendGrid(struct Grid grid){
    char gridtostr[MAX_BUFFER];// = malloc(sizeof(char) * (grid.grid_width + 1) * grid.grid_height);
    toStringGrid(grid, gridtostr);
    int grid_send = send(CLIENT, gridtostr, (long) strlen(gridtostr), 0);
    if (grid_send < 0) {
        printf("Erreur d'envois\n");
        exit(EXIT_FAILURE);
    }
    strcpy(gridtostr,"");
}
void updateGrid(struct Grid *grid){
    for (int i = 0; i < (*grid).grid_height; i++) {
        for (int j = 0; j < (*grid).grid_width; j++) {
            (*grid).grid[i][j]=0;
        }
    }
    (*grid).grid[(*grid).player[1]][(*grid).player[0]]=3;
    for (int i = 0; i < (*grid).nb_points; i++) {
        (*grid).grid[(*grid).points[i][1]][(*grid).points[i][0]]=(*grid).points[i][2];
    }
    for (int i = 0; i < (*grid).nb_ghosts; i++) {
        (*grid).grid[(*grid).ghosts[i][1]][(*grid).ghosts[i][0]]=4;
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
//end grid helpers


// start of end of game handling
void calcBonus(int score, char * buffer){
    sprintf(buffer,"%d ( %d + time bonus : %d)\n",score+(timer.minutes*1000)+(timer.seconds*10),score,(timer.minutes*1000)+(timer.seconds*10));
}
void gameEnd(const char * buf, int score, int nb_points){
    char temp_score[20],str[500],final[500];
    //printf("\n values before clean : \n temp_score : %s \n str : %s \n final : %s\n",temp_score,str,final);
    cleanStr(temp_score);
    cleanStr(final);
    cleanStr(str);
    //printf("\n values after clean : \n temp_score : %s \n str : %s \n final : %s\n",temp_score,str,final);
    if(score>0 && nb_points==0){
        calcBonus(score,temp_score);
    } else{
        sprintf(temp_score," %d",score);
    }
    strcpy(final,buf);
    strcat(final,"\n Score final : ");
    strcat(final,temp_score);
    int losenotif = send(CLIENT,final, strlen(final),0);
    if(losenotif<0){
        printf("Erreur d'envois\n");
        exit(EXIT_FAILURE);
    }

}
void end(struct Grid grid, const char* endres){
    char temp[MAX_BUFFER], str[MAX_BUFFER];
    toStringGrid(grid,str);
    memmove(str,str+1, strlen(str));
    strcat(temp,"\n  ");
    strcat(temp,endres);
    strcat(temp,"  \n Tour final");
    strcat(temp,str);
    strcat(temp,"\n    ");
    gameEnd(temp,grid.score,grid.nb_points);
    cleanStr(temp);
    cleanStr(str);
}
void gameWin(struct Grid grid){
    end(grid,WIN_GAME);
    if(DEBUG==1){
        printf("Client n*%d a gagné\n",NUM_CLIENT);
    }
}
void gameOver(struct Grid grid){
    end(grid,LOSE_GAME);
    if(DEBUG==1){
        printf("Client n*%d a perdu\n",NUM_CLIENT);
    }
}
//end of end of game handling

//start gameloop
    //start of gameplay functions
        //start player moves
void deleteArrayElement(int** array, int* size, int pos){
    int tempx = array[pos][0];
    int tempy = array[pos][1];
    int tempv = array[pos][2];
    for (int i = pos; i < *size-1; ++i) {
        array[i][0] = array[i + 1][0];
        array[i][1] = array[i + 1][1];
        array[i][2] = array[i + 1][2];
    }
    array[*size-1][0]=tempx;
    array[*size-1][1]=tempy;
    array[*size-1][2]=tempv;
    *size-=1;
}
int allowedMove(int dest, int max){
    return (dest<max && dest>=0);
}
void getPointTest(int ** grid,int** points, int* nb_points, int player_x, int player_y,int *score) {
    for (int i = 0; i < *nb_points; ++i) {
        if (points[i][0] == player_x && points[i][1] == player_y) {
            if (grid[player_y][player_x] == 2) {
                *score += 200;
            } else {
                *score += 100;
            }
            deleteArrayElement(points, nb_points, i);
            break;

        }
    }
}
int getGhostTest(int** ghosts, int* nb_ghosts, int player_x, int player_y, int score){
    for (int i = 0; i < *nb_ghosts; ++i) {
        if(ghosts[i][0]==player_x && ghosts[i][1]==player_y){
            return 1;
        }
    }
    return 0;
}
int doMove(struct Grid* grid,int posx, int posy){
    int res=0;
    if(allowedMove(posy,(*grid).grid_height)==1 && allowedMove(posx,(*grid).grid_width)){
        if(getGhostTest((*grid).ghosts,&((*grid).nb_ghosts),posx,posy,(*grid).score)!=0){
            res=1;
        }
        getPointTest((*grid).grid,(*grid).points,&((*grid).nb_points),posx,posy,&((*grid).score));
        (*grid).player[0]=posx;
        (*grid).player[1]=posy;
        updateGrid(grid);
        if((*grid).nb_points==0){
            res=2;
        }
        if(res==1){
            gameOver(*grid);
        }
        if(res==2){
            gameWin(*grid);
        }
        return res;
    }
    if(DEBUG==1){
        printf("destination non valide!\n");
    }
}
int readMove(char* buf, struct Grid* grid){
    if (strcmp(buf,"forw")==0){
        if(DEBUG==1){
            printf("%s\n",buf);
        }
        return doMove(grid,(*grid).player[0],(*grid).player[1]-1);
    }
    if (strcmp(buf,"back")==0){
        if(DEBUG==1){
            printf("%s\n",buf);
        }
        return doMove(grid,(*grid).player[0],(*grid).player[1]+1);
    }
    if (strcmp(buf,"left")==0){
        if(DEBUG==1){
            printf("%s\n",buf);
        }
        return doMove(grid,(*grid).player[0]-1,(*grid).player[1]);
    }
    if (strcmp(buf,"rght")==0){
        if(DEBUG==1){
            printf("%s\n",buf);
        }
        return doMove(grid,(*grid).player[0]+1,(*grid).player[1]);
    }
    else if(testExit(buf)==1){
        printf("\nLe client %d a quitté ",NUM_CLIENT);
        exit(EXIT_SUCCESS);
    }
}
            //end player moves

            //start computer moves
int doCompMove(struct Grid *grid, int pos_x, int pos_y,int pos){
    if((pos_x>=0 && pos_x<(*grid).grid_width)&&(pos_y>=0 && pos_y<(*grid).grid_height)){
        (*grid).ghosts[pos][0]=pos_x;
        (*grid).ghosts[pos][1]=pos_y;
    }
    updateGrid(grid);
    if(DEBUG==1){
        printf("\nClient n*%d : moves computer",NUM_CLIENT);
        display(*grid);
    }
    if(pos_x==(*grid).player[0] && pos_y==(*grid).player[1]){
        return 1;
    }
    return 0;
}
int doAICompMove(struct Grid *grid,int pos){
    int new_posx, new_posy;
    new_posx = (*grid).player[0]-(*grid).ghosts[pos][0];
    new_posy = (*grid).player[1]-(*grid).ghosts[pos][1];
    (*grid).ghosts[pos][0] += (abs(new_posx)>=abs(new_posy) ? -(-new_posx/ abs(new_posx)) : 0);
    (*grid).ghosts[pos][1] += (abs(new_posx)>=abs(new_posy) ? 0 : -(-new_posy/ abs(new_posy)));
    updateGrid(grid);
    if(DEBUG==1){
        printf("\nClient n*%d : moves AI computer",NUM_CLIENT);
        display(*grid);
    }
    if((*grid).ghosts[pos][0]==(*grid).player[0] && (*grid).ghosts[pos][1]==(*grid).player[1]){
        return 1;
    }
    return 0;
}
int computerMove(struct Grid *grid){
    int res, test_dir,test_ai;
    for (int i = 0; i < (*grid).nb_ghosts; ++i) {
        test_ai = rand()%3;
        //TO-DO REMOVE 5 AND PUT BACK 0
        if(test_ai==5){
            res= doAICompMove(grid,i);
        } else {
            test_dir=rand()%4;
            switch (test_dir) {
                case 0:
                    res = doCompMove(grid,(*grid).ghosts[i][0]-1,(*grid).ghosts[i][1],i);
                    break;
                case 1:
                    res = doCompMove(grid,(*grid).ghosts[i][0]+1,(*grid).ghosts[i][1],i);
                    break;
                case 2:
                    res = doCompMove(grid,(*grid).ghosts[i][0],(*grid).ghosts[i][1]-1,i);
                    break;
                case 3:
                    res = doCompMove(grid,(*grid).ghosts[i][0],(*grid).ghosts[i][1]+1,i);
                    break;
            }
        }
        if(res==1){
            gameOver(*grid);
            return res;
        }
    }
    return res;
}
            //end computer moves
int game(struct Grid *grid){
    int res, res_comp;
    char buffer[MAX_BUFFER];
    strcpy(buffer,"");
    int nbReceived= recv(CLIENT,buffer,4,0);
    if(nbReceived<0){
        printf("Erreur de réception\n");
    } else {
        cleanBuffer(&nbReceived,buffer);
        if(DEBUG==1){
            printf("\nClient n*%d : buffer = %s",NUM_CLIENT,buffer);
        }
        res = readMove(buffer,grid);
        if(DEBUG==1){
            printf("\nClient n*%d",NUM_CLIENT);
            display(*grid);
        }
        if(res==2||res==1) {
            return res;
        }
        res_comp=computerMove(grid);
        if(res_comp==1) return res_comp;
        sendGrid(*grid);
        return res;
    }
}
void gameAction(struct Grid * grid,int * iter){
    char buffer[MAX_BUFFER];
    int gamelose = game(grid);
    if (gamelose == 1|| gamelose==2) {
        //restart game
        int nbReceived= recv(CLIENT,buffer,MAX_BUFFER,0);
        if(nbReceived<0){
            printf("Erreur de réception\n");
        } else {
            cleanBuffer(&nbReceived,buffer);
            if(strcmp(buffer,"replay")==0){
                clearPointers(grid);
                *iter=-1;
            } else{
                if(testExit(buffer)==1){
                    printf("\nLe client %d a quitté ",NUM_CLIENT);
                    exit(EXIT_SUCCESS);
                }
            }
        }
    }
}
    //end of gameplay functions

    //start initGame settings and variables
void initGrid(struct Grid* grid){
    (*grid).ghosts = malloc(sizeof(int*)*6);
    for(int i=0;i<6;i++){
        (*grid).ghosts[i]= malloc(sizeof(int)*2);
    }
    (*grid).points = malloc(sizeof(int*)*70);
    for (int i=0; i<70;i++){
        (*grid).points[i] = malloc(sizeof(int)*3);
    }
    (*grid).grid = malloc(sizeof(int*)*LG_GRID_HEIGHT);
    for (int i = 0; i < LG_GRID_HEIGHT; i++) {
        (*grid).grid[i]= malloc(sizeof(int)*LG_GRID_WIDTH);
    }
    (*grid).player= malloc(sizeof(int)*2);
}
struct Grid genGrid(int width, int height, int nb_points, int nb_ghosts){
    struct Grid grid;
    initGrid(&grid);
    //points initialization;
    int i=0;
    int x=0,y=0;
    grid.grid_height=height;
    grid.grid_width=width;
    grid.nb_points=nb_points;
    grid.nb_ghosts=nb_ghosts;
    do {
        x=rand()%width;
        y=rand()%height;
        if(present(grid.points,grid.nb_points,x,y)==0 && (x != grid.grid_width/2 || y != grid.grid_height/2)){
            grid.points[i][0]=x;
            grid.points[i][1]=y;
            grid.points[i][2]=(i%2==0? 1 : (i%3==0 ? 2 : 1));
            i++;
        }
    }while(i<grid.nb_points);
    //ghosts initialization;
    i=0;
    do{
        int x_g = rand()%grid.grid_width;
        int y_g = rand()%grid.grid_height;
        if(present(grid.points,grid.nb_points,x_g,y_g)==0 && present(grid.ghosts,grid.nb_ghosts,x_g,y_g)==0 && (x_g != grid.grid_width/2 || y_g != grid.grid_height/2)){
            grid.ghosts[i][0]=x_g;
            grid.ghosts[i][1]=y_g;
            i++;
        }
    } while(i<grid.nb_ghosts);
    //player initialization;
    grid.player[0]=grid.grid_width/2;
    grid.player[1]=grid.grid_height/2;
    //main grid initialization;
    for (int i = 0; i < grid.grid_height; i++) {
        for (int j = 0; j < grid.grid_width; j++) {
            grid.grid[i][j]=0;
        }
    }
    //placing elements in the main grid;
    updateGrid(&grid);
    grid.score=0;
    return grid;
}
struct Grid initSize(char* buf){
    struct Grid grid;
    if(strcmp(buf,"sm")==0){
        grid=genGrid(SM_GRID_WIDTH,SM_GRID_HEIGHT,30,2);
        GAME_MODE=1;
        timer.minutes=2;
        timer.seconds=0;
    }
    if(strcmp(buf,"md")==0){
        grid=genGrid(MD_GRID_WIDTH,MD_GRID_HEIGHT,50,4);
        GAME_MODE=2;
        timer.minutes=3;
        timer.seconds=0;
    }
    if(strcmp(buf,"lg")==0){
        grid=genGrid(LG_GRID_WIDTH,LG_GRID_HEIGHT,70,6);
        GAME_MODE=3;
        timer.minutes=4;
        timer.seconds=0;
    }
    return grid;
}
void gameGreeting(){
    int msg = send(CLIENT, WELCOME, strlen(WELCOME), 0);
    if (msg < 0) {
        printf("Erreur d'envois\n");
        exit(EXIT_FAILURE);
    }
}
struct Grid initGame(){
    struct Grid grid;
    char buffer[MAX_BUFFER];
    int nbReceived = recv(CLIENT,buffer,MAX_BUFFER,0);
    if(nbReceived<0){
        printf("Erreur de réception\n");
    } else {
        cleanBuffer(&nbReceived,buffer);
        if(DEBUG==1){
            printf("Client n*%d : %s\n",NUM_CLIENT,buffer);
        }
        grid=initSize(buffer);
        strcpy(buffer,"");
        if(DEBUG==1){
            printf("\nClient n*%d",NUM_CLIENT);
            display(grid);
        }
    }
    return grid;
}
struct Grid gameSetup(){
    struct Grid grid;
    gameGreeting();
    grid = initGame();
    START=time(NULL);
    sendGrid(grid);
    return grid;
}
    //end initGame
void gameLoop(){
    struct Grid grid;
    initGrid(&grid);
    int iter = 0;
    //start of config for game
    while (1) {
        if (iter == 0) {
            grid=gameSetup();
        } else {
            gameAction(&grid,&iter);
        }
        //add test for start of game
        iter++;
    }
}
//end game loop

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
void serverSetup(struct sockaddr_in* adserv,int * len, int* sockServ){
    *sockServ= createServerSocket();
    *len = sizeof(struct sockaddr_in);
    memset(adserv,0x00,*len);
    (*adserv).sin_family = PF_INET;
    (*adserv).sin_addr.s_addr= htonl(INADDR_ANY);
    (*adserv).sin_port = htons(PORT);
    bindServer(*sockServ,(struct sockaddr*)adserv,*len);
}
//end server connection

//app init passed values
void test_debug(int *a,const char *argv){
    if(strcmp(argv,"-d")==0|| strcmp(argv,"--debug")==0){
        *a=1;
    }
}
void initApp(int argc,const char**argv,int* nb_listen){
    if(argc<2){
        erreur("init, nombre de variables insuffisants");
        print_help();
        exit(EXIT_FAILURE);
    }
    if(strcmp(argv[1],"--help")==0){
        print_help();
        exit(EXIT_SUCCESS);
    }
    if((*nb_listen=atoi(argv[1]))==0){
        erreur("init, premier argument n'est pas le nombre de clients autorisés");
        print_help();
        exit(EXIT_FAILURE);
    }
    if(argc>2){
        test_debug(&DEBUG,argv[2]);
    }
}
//end app init

//main
int main(int argc, char const *argv[]){
    //variables
    srand(time(NULL));
    //sockets
    int fdSocketServer;
    int fdSocketClient;
    struct sockaddr_in adServer;
    struct sockaddr_in adClient;
    //var for gamemode choice
    int num_client=0;
    int nb_listen;
    int lenAd;
    //var initialisation
    //test argc/argv:
    initApp(argc,argv,&nb_listen);
    //server setup
    serverSetup(&adServer,&lenAd,&fdSocketServer);
    listenServer(fdSocketServer,nb_listen);
    printf("Serveur démarré, ip : %s, listening\n", inet_ntoa(adServer.sin_addr));
    while(1) {
        fdSocketClient = acceptConnection(fdSocketServer, (struct sockaddr *) &adClient, &lenAd);
        printf("Client n*%d connecté ! at : %s\n", num_client+1, inet_ntoa(adClient.sin_addr));
        num_client++;
        int pid = fork();
        if (pid == 0) {
            CLIENT = fdSocketClient;
            NUM_CLIENT = num_client;
            gameLoop();
            close(fdSocketClient);
            printf("Client %d a quitté le server\n", num_client);
            exit(EXIT_SUCCESS);
        }
    }
    close(fdSocketServer);
    return 0;
}