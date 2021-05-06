#include <string.h>
#include <stdio.h>

#define MAX_BUFFER 1000

void lireMessager(char buffer[],char text[]){
    printf("%s",text);
    fgets(buffer,4,stdin);
    strtok(buffer,"\n");
}

void calcBonus(int score, char * buffer){
    char tempbonus[5];
    sprintf(tempbonus,"%d",score+1000+10);
    strcat(buffer,tempbonus);
    strcat(buffer," ( ");
    sprintf(tempbonus,"%d",score);
    strcat(buffer,tempbonus);
    strcat(buffer," + time bonus : ");
    sprintf(tempbonus,"%d",1010);
    strcat(buffer,tempbonus);
    strcat(buffer," )\n");
    printf("\n buffer (temp_score) : %d; tempbonus: %d\n",*buffer,*tempbonus);
}


void gameEnd(const char * buf, int score, int nb_points){
    char res[MAX_BUFFER],temp_score[20], test[MAX_BUFFER];
    strcpy(res,buf);
    strcat(res,"\n Score final : ");
    strcpy(test,res);
    if(score>0 && nb_points==0){
        printf("\n%s %d\n",res, *res);
        calcBonus(score,temp_score);
        printf("\ntemp_score: %d; res: %d\n",*temp_score,*res);
    } else{
        //printf("\n%s\n",res);
        sprintf(temp_score," %d",score);
        //printf("\n%s\n",res);
    }
    printf("\n%s",res);
    strcat(test,temp_score);
    printf("\n%s",test);
}

int main(){
    gameEnd("This is the win", 3500,0);
    return 0;
}