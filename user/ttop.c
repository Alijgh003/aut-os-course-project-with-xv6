#include "kernel/types.h"
#include  "user/user.h"
#include "top.h"

int 
main(int argc,char *argv[])
{
    int pid = -1;
    for(int i=0; i<10; i++){
        pid = fork();
        if(pid < 0){
            printf("fork was invoked unsuccessfully.");
            return -1;
        }else if(pid == 0){
            for (int j = 0 ; j< 1000000000;j++);
            int mpid = getpid();
            printf("process with pid=%d done.\n",mpid);
            break;
        }
    }
    if(pid > 0){
        pid = fork();
        if(pid < 0){
            printf("fork was invoked unsuccessfully.");
            return -1;
        }else if(pid == 0){
            while(1){
                printf("\n\n\n");
                print_top();
                sleep(20);
                printf("\n\n\n");
            }
        }else{
            for(int i=0;i<10;i++){
                wait(0);
            }
            printf("All children done.\n");
            kill(pid);
        }
    }
    exit(0);
}