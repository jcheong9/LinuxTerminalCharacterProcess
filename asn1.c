#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

#define MSGSIZE 		256
#define abNormTermination 	0x0B
#define endOfString   		00

/*------ Function prototypes ----------*/
void inputProcess(int p[], pid_t pid, pid_t ppid);
void outputProcess(int p[]);
void translateProcess(int p[]);
void parent (int p[]);
void child (int p[]);
void fatal (char *);
void sig_usr(int signo);

static int received = 0;

void readUsual(int sig)
{
    if (sig == SIGUSR1)
    {
        received = 1;
    }
}

int main (void)
{
	int pfd[2];
	pid_t pid;
	pid_t ppid = getpid();

	signal(SIGUSR1,readUsual);

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
	pid = fork(); 
	switch(pid)
	{
		case -1:        /* error */
			fatal ("fork call");
		case 0:        /* It's the child */
		  	outputProcess (pfd);
		default:       /* parent */
		  	inputProcess (pfd, pid, ppid);
	}
}


void inputProcess (int p[2],pid_t pid,pid_t ppid)
{
	char readInput;
	char buf[MSGSIZE];

	int i = 0;

	close (p[0]);    /* close the read descriptor */
	system("stty raw igncr -echo");
	while (1)
	{
		switch(readInput = getchar()){
			
			case 'E':
				buf[i++] = endOfString;
				write (p[1], buf, MSGSIZE);
				i = 0;
				break;
			case 'T':
				buf[i++] = endOfString;
				write (p[1], buf, MSGSIZE);
				i = 0;
				kill(pid,SIGUSR1);
				sleep(1);
				kill(ppid, SIGKILL);
				break;
			case abNormTermination:
				system("stty -raw -igncr echo");
				kill(0,9);
				break;
			default:
				buf[i++] = readInput;
				break;

		}
		
	}
		
	exit(0);
}

/*------ Child Code --------------------*/
void outputProcess (int p[2])
{
	int nread;
	char buf[MSGSIZE];

	close (p[1]);    /* close the write descriptor */

	while(!received)
	{
		switch (nread = read(p[0], buf, MSGSIZE))
		{
		  case -1:
		  case 0:
			break;
		  break;
		default:
			printf ("MSG = %s\n", buf);
		}
	}
	printf("SIGUSR1 received.\n");
	system("stty -raw -igncr echo");
}

/*---------- Error function ------*/
void fatal (char *s)
{
	perror (s);    /* print error msg and die */
	exit(1);
}





