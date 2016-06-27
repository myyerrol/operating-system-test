/*
 * A test program for GeekOS user mode
 */

#include <conio.h>
#include <process.h>
#include <sched.h>
#include <sema.h>
#include <string.h>

int main(int argc, char** argv)
{

  int i,j;   


  for (i=0; i < 400; i++) {
    for(j=0;j<200000;j++);
    Print("0");
  }

return 0;
}
