#include <stdio.h>
#include <string.h>

int main()
{
  char *destip = strdup("10.10.10.1/24");
 
  char *found = strsep(&destip,"/");

  printf("string1:%s\n", found);
  
  found = strsep(&destip,"/");

  printf("string2:%s\n", found);


}
