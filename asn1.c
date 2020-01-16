#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MSGSIZE 16

char *msg1 = "Hello World #1";
char *msg2 = "Hello World #2";
char *msg3 = "Hello World #3";

/*------ Function prototypes ----------*/
void inputProcess(int p[]);
void outputProcess(int p[]);
void translateProcess(int p[]);
void parent (int p[]);
void child (int p[]);
void fatal (char *);

/*------- Message Strings -------------*/
char *msg1 = "Hello World";
char *msg2 = "Goodbye World";

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
	int nread;
	char buf[MSGSIZE];

	/* close the read descriptor */
	close (p[0]);    
	system("stty raw igncr -echo");

	for (;;)
	{
		switch (nread = write(p[1], buf, MSGSIZE))
		{
			case -1:
			case 0:
				printf ("(pipe empty)\n");
				sleep(1);
				break;
			default:
			  if (strcmp (buf, msg2) == 0)
			  {
				  printf ("End of Conversation\n");
				  exit(0);
			  }
			else
				printf ("MSG = %s\n", buf);
		}
	}
}

/*------ Child Code --------------------*/
void child (int p[2])
{
	int count;
	
	close (p[1]);    /* close the write descriptor */

	for (count = 0; count < 3; count ++)
	{
		read (p[0], msg1, MSGSIZE);
		sleep (3);
	}
	/*--- Send final message ------------*/
	write (p[1], msg2, MSGSIZE);
	system("stty -raw -igncr echo");
	exit(0);
}

/*---------- Error function ------*/
void fatal (char *s)
{
	perror (s);    /* print error msg and die */
	exit(1);
}




