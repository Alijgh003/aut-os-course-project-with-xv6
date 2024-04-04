#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int 
main(int argc,char *argv[])
{
    int year = getyear();
    printf("get year return value=%d\n",year);
    
    exit(0);
}