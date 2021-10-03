#include "kernel/param.h"
#include "kernel/types.h"
#include "user/user.h"

void main(void) {
  unsigned int i = 0x00646c72;
  printf("H%x Wo%s\n", 57616, &i);
  
  printf("x=%d y=%d\n", 3);
  exit(0);
}
