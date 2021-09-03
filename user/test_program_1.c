#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char** argv) {
    
    if (argc != 2) {
        fprintf(2, "Usage: %s <string>\n", argv[0]);
        exit(1);
    }

    if (echo_simple(argv[1]) < 0) {
      fprintf(2, "%s: echo_simple failed\n", argv[0]);
      exit(1);
    }
  
    exit(0);
}
