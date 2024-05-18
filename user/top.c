#include "kernel/types.h"
#include "user/user.h"
#include "top.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

int 
main(int argc,char *argv[])
{
    while (1) {
        printf("\033[2J\033[1;1H\n");
        print_top();
        sleep(10);
    }
    exit(0);
}