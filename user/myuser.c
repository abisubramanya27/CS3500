#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char** argv) {
    
    printf("arg0 is %s\n", argv[0]);
    for (int i = 1; i <= 3; i++) {
        char* tmp = "<empty>";
        if (argc > i) {
            tmp = argv[i];
        }
        printf("arg%d is %s\n", i, tmp);
    }

    exit(0);
}
