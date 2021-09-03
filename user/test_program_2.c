#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char** argv) {
    
    if (echo_kernel(argc-1, argv+1) < 0) {
      fprintf(2, "%s: echo_kernel failed\n", argv[0]);
      exit(1);
    }
  
    exit(0);
}
