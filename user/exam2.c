#include <kernel/types.h>
#include <user/user.h>

// User Program written for Lab Exam
int
main(int argc, char **argv)
{
  int K = 26; // Caesar Cipher Key to be set here
  if (setKey(K) < 0) {
    printf("Error while setting Caesar Cipher Key!! Exiting...");
    exit(0);
  }
  
  char *str = "CS3500!";
  for(int i = 0; i < strlen(str); i++) {
    int st = 0, end = 0, mod = 26;
    if(str[i] >= 'a' && str[i] <= 'z') st = 'a', end = 'z';
    else if(str[i] >= 'A' && str[i] <= 'Z') st = 'A', end = 'Z';
    else if(str[i] >= '0' && str[i] <= '9') st = '0', end = '9', mod = 10;
    else continue;

    for(int c = st; c <= end; c++) {
      if(((c - st + K)%mod + st) == str[i])  {
        str[i] = c;
        break;
      }
    }
  }
  printf("%s\n", str);
  
  if (setKey(0) < 0) {
    printf("Error while re-setting Caesar Cipher Key!! Exiting...");
    exit(0);
  }

  printf("String before encryption: %s\n", str);

  exit(0);
}
