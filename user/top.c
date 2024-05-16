#include "kernel/types.h"
#include  "user/user.h"
#include "user/top.h"


const char* getState(enum procstate state){
    switch (state)
    {
    case UNUSED:
        return "UNUSED";
        break;
    case USED:
        return "USED";
        break;
    case SLEEPING:
        return "SLEEPING";
        break;
    case RUNNABLE:
        return "RUNNABLE";
        break;
    case RUNNING:
        return "RUNNING";
        break;
    case ZOMBIE:
        return "ZOMBIE";
        break;
    default:
        return "";
        break;
    }
}

int 
main(int argc,char *argv[])
{

    struct top *utop;
    utop = (struct top*) malloc((uint) sizeof (struct top));
    int r = top(utop);
    if(r == 0){
        printf("uptime: %d seconds\n",utop->uptime);
        printf("#total_processes: %d\n",utop->total_processes);
        printf("#running_processes: %d\n",utop->running_processes);
        printf("#sleeping_processes: %d\n",utop->sleeping_processes);
        printf("NAME\tPID\tPARENT PID\tTIME\t%%CPU\tSTATUS\n");
        struct proc_info *pinfo = utop->proc_list;
        for(int i=0; i < utop->total_processes ;i++){
            printf("%s\t%d\t%d\t\t%d\t%d\t%s\n",pinfo->name,pinfo->pid,pinfo->ppid,pinfo->time,pinfo->cpu,getState(pinfo->state));
            pinfo++;
        }
    }
    exit(0);
}