#include <kernel/types.h>
#include <user/user.h>

// User Program written for Lab Exam
int
main(int argc, char **argv)
{
  int K = 1; // Caesar Cipher Key to be set here

  if (setKey(K) < 0) {
    printf("Error while setting Caesar Cipher Key!! Exiting...");
    exit(0);
  }
  
  char *str = "GDKKN VNQKC!";
  printf("%s\n", str);
  
  if (setKey(0) < 0) {
    printf("Error while re-setting Caesar Cipher Key!! Exiting...");
  }

  exit(0);
}
