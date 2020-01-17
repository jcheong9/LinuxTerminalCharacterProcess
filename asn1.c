#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

#define MSGSIZE 256
#define abNormTermination 0x0B

/*------ Function prototypes ----------*/
void inputProcess(int p[]);
void outputProcess(int p[]);
void translateProcess(int p[]);
void parent (int p[]);
void child (int p[]);
void fatal (char *);



int main (void)
{
	int pfd[2];

	/*----- Open the pipe -----------*/
	if (pipe(pfd) < 0)
	{
		fatal ("pipe call");
		exit(1);
	}

	/*---- Set the O_NDELAY flag for p[0] -----------*/
	if (fcntl (pfd[0], F_SETFL, O_NDELAY) < 0)
	  	fatal ("fcntl call");

	/*-------- fork ---------------*/
	switch(fork())
	{
		case -1:        /* error */
			fatal ("fork call");
		case 0:        /* It's the child */
		  	child (pfd);
		default:       /* parent */
		  	parent (pfd);
	}
}

/*------- Parent Code -------*/
void parent (int p[2])
{
	char ch;
	char buf[] = "";
	char newLine = '\n';
	//char *contStr;

	close (p[0]);    /* close the read descriptor */
	system("stty raw igncr -echo");
	while (1)
	{
		if(scanf("%c",&ch) == 1){
			strncat(buf, &ch, 1); 
			if(ch == 'E'){
				strncat(buf, &newLine, 1); 
				write (p[1], buf, MSGSIZE);
				strcpy(buf,"");
			}
			if(ch == abNormTermination){
				kill(0,9);
			}
		}
		printf("Appended String: %s\n", buf); 
		
	}
	/*--- Send final message ------------*/
	//write (p[1], msg2, MSGSIZE);
		
	exit(0);
}

/*------ Child Code --------------------*/
void child (int p[2])
{
	int nread;
	char buf[MSGSIZE];

	close (p[1]);    /* close the write descriptor */

	while(1)
	{
		switch (nread = read(p[0], buf, MSGSIZE))
		{
		  case -1:
		  case 0:
			sleep(1);
		  break;
		default:
			printf ("MSG = %s\n", buf);
		}
	}
	system("stty -raw -igncr echo");
}

/*---------- Error function ------*/
void fatal (char *s)
{
	perror (s);    /* print error msg and die */
	exit(1);
}




