#include "kernel/types.h"
#include  "user/user.h"
#include "user/top.h"

int 
main(int argc,char *argv[])
{
    printf("start\n");
    sbrk(89478485);
    int pid;
    pid = fork();

    if(pid == 0){
        for(int i=0;i<10;i++){
            printf("\033[2J\033[1;1H\n");
            print_top();
            sleep(10);
        }
        printf("\033[2J\033[1;1H\n");
    }else{
        wait(0);
        printf("done\n");
    }
    exit(0);
}