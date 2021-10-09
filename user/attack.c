#include "kernel/param.h"
#include "kernel/types.h"
#include "user/user.h"

void main(void) {
  int pid = fork();
  if (pid > 0) {
    wait(0);
    // printf("\nFROM PARENT :\n");
    if (pcbread() < 0) {
      printf("error: during pcbread system call\n");
    }
  }
  else {
    // printf("\nFROM CHILD :\n");
    // if (pcbread() < 0) {
    //   printf("error: during pcbread system call\n");
    // }
    //
    // few additional system calls for Lab 4 section 3 qn4
    sleep(5);
    printf("S");
  }

  exit(0);
}
