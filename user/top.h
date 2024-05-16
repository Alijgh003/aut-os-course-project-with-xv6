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