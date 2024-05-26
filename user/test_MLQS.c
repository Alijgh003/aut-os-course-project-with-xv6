#include "kernel/types.h"
#include  "user/user.h"
#include "top.h"

int 
main(int argc,char *argv[])
{
    printf("start\n");
    int pid = -1;
    for(int i=0; i<10; i++){
        pid = fork();
        if(pid < 0){
            printf("fork was invoked unsuccessfully.");
            return -1;
        }else if(pid == 0){
            for (int j = 0 ; j< 1000000000;j++);
            int mpid = getpid();
            printf("\n\nprocess with pid=%d done.\n\n",mpid);
            break;
        }
    }
    if(pid > 0){
        for(int i=0; i<10; i++){
            wait(0);
        }
        printf("finished\n");
    }
    exit(0);
}