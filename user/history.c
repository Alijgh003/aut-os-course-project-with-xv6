#include "kernel/types.h"
#include  "user/user.h"


int isNumber(char number[])
{
    int i = 0;
    if (number[0] == '-')
        i = 1;
    for (; number[i] != 0; i++)
    {
        if (number[i] > '9' || number[i] < '0')
            return 0;
    }
    return 1;
}

int 
main(int argc,char *argv[])
{
    if(argc < 2){
        printf("invalid arguments.\nhistory {historyIndex}\n");
    }else{
        if(isNumber(argv[1])){
            int x = atoi(argv[1]);
            history(x);
        }else{
            printf("history index must be an integer.\n");
        }
    }
    exit(0);
}