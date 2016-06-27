/*
 * A test program for GeekOS user mode
 */

#include <conio.h>
#include <geekos/syscall.h>

int main(int argc, char** argv)
{

  int i,j;   


  for (i=0; i < 400; i++) {
    for(j=0;j<200000;j++);
    Print("!");
  }

return 0;
}
