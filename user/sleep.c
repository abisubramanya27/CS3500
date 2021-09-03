#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char** argv)
{
   
    if (argc < 2) {
        printf("usage: sleep N_TICKS\n");
        exit(1);
    } 
    
    //No Error
    int num = atoi(argv[1]);
    sleep(num);
    printf("<from user:> Program exiting after sleeping successfully...\n");

    exit(0);
}
