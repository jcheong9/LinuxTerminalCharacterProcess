/*********************************************************************
* Simple program that creates two processes, both parent and child
* output their respective PIDs
**********************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

int main(void) 
{
	pid_t childpid = 0; 
   	int i, n;
	char ch = 'k';
	char buf[] = "";
 
    	system("stty raw igncr -echo");
   	n = 3;  
   	for (i = 1; i < n; i++)
      		if ((childpid = fork()) <= 0)
         		break;
	
	while(ch !='T'){
		scanf("%c",&ch);
		strncat(buf, &ch, 1); 
		
	} 
	
	printf("Appended String: %s\n", buf); 

   	fprintf(stderr, "i:%d  process ID:%ld  parent ID:%ld  child ID:%ld\n",
           i, (long)getpid(), (long)getppid(), (long)childpid);
	system("stty -raw -igncr echo");
   	return 0; 
}
