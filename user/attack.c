#include "kernel/param.h"
#include "kernel/types.h"
#include "user/user.h"

void main(void) {
  if (pcbread() < 0) {
    printf("error: during pcbread system call\n");
  }

  exit(0);
}
