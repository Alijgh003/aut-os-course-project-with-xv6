enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

struct proc_info {
    char name[16];
    int pid;
    int ppid;
    long time;
    double cpu;
    enum procstate state;
};

struct top {
  long uptime;
  int total_processes;
  int running_processes;
  int sleeping_processes;
  struct proc_info proc_list[64];
};

void print_top(void);


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


void
print_top()
{
    struct top *utop;
    utop = (struct top*) malloc((uint) sizeof (struct top));
    int r = top(utop);
    if(r == 0){
        printf("uptime: %d seconds\n",utop->uptime);
        printf("#total_processes: %d\n",utop->total_processes);
        printf("#running_processes: %d\n",utop->running_processes);
        printf("#sleeping_processes: %d\n",utop->sleeping_processes);
        printf("\n");
        printf("NAME\tPID\tPPID\tTIME\t%%CPU\tSTATUS\n");
        struct proc_info *pinfo = utop->proc_list;
        for(int i=0; i < utop->total_processes ;i++){
            printf("%s\t%d\t%d\t%d\t%f\t%s\n",pinfo->name,pinfo->pid,pinfo->ppid,pinfo->time,pinfo->cpu,getState(pinfo->state));
            pinfo++;
        }
        printf("\n");
    }
}

