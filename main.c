#include <string.h>
#include <stdio.h>

#define MAX_BUFFER 1000

void lireMessager(char buffer[],char text[]){
    printf("%s",text);
    fgets(buffer,4,stdin);
    strtok(buffer,"\n");
}

int main(){
    char play[4];
    lireMessager(play,"enter non ");
    printf("\n%s\n",play);
    int res = strcmp(play,"oui");
    printf("%d",res);
    return 0;
}