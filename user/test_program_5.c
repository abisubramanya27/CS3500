#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/processinfo.h"

int
main(int argc, char** argv) {
    
    if (argc != 1) {
        fprintf(2, "Usage: %s\n", argv[0]);
        exit(1);
    }
    
    struct processinfo pi;
    if (get_process_info(&pi) < 0) {
      fprintf(2, "%s: get_process_info failed\n", argv[0]);
      exit(1);
    }

    printf("Process ID -> %d\n", pi.pid);
    printf("Process Name -> %s\n", pi.name);
    printf("Memory Size -> %d Bytes\n", pi.sz);
  
    exit(0);
}
