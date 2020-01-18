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
void inputProcess(int pipeIO[], int pipeIT[], pid_t pid);
void outputProcess(int p[]);
void translateProcess(int pipeIT[],int pipeOT[]);
void parent (int p[]);
void child (int p[]);
void fatal (char *);
void sig_usr(int signo);

static int normalTermination = 0;

void readUsual(int sig)
{
    if (sig == SIGUSR1)
    {
        normalTermination = 1;
    }
}

int main (void)
{
	int pipeIO[2];	//pipe for input and output process
	int pipeIT[2];  //pipe for input and translate process
	int pipeOT[2];	//pipe for output and translate process
	pid_t pOutputid, pTranslateid;

	signal(SIGUSR1,readUsual);

	/*----- Open the pipe -----------*/
	if (pipe(pipeIO) < 0 || pipe(pipeIT) < 0 || pipe(pipeOT) < 0 )
	{
		fatal ("pipe call");
		exit(1);
	}

	/*---- Set the O_NDELAY flag for pipeIO[0] -----------*/
	if (fcntl (pipeIO[0], F_SETFL, O_NDELAY) < 0)
	  	fatal ("fcntl call");


	/*-------- fork ---------------*/
	pOutputid = fork(); 
	switch(pOutputid)
	{
		case -1:        /* error */
			fatal ("fork call");
			break;

		case 0:        /* It's the child */
		  	outputProcess (pipeIO);
		default:       /* parent */
		  	
			pTranslateid = fork(); 
			switch(pTranslateid)
			{
				case -1:        /* error */
					fatal ("fork call");
					break;

				case 0:        /* It's the child */
				  	printf("translateProcess.\n");
				  	translateProcess (pipeIT, pipeOT);
				default:       /* parent */
					inputProcess (pipeIO, pipeIT, pOutputid);
			}
	}
	return 0;
}


void inputProcess (int pipeIO[2], int pipeIT[2], pid_t pid)
{
	char readInput;
	//char buf[MSGSIZE];
	char bufChar[2];
	int read = 1;
	//int i = 0;

	/* close the read descriptor */
	close (pipeIO[0]);  
	close (pipeIT[0]);   
	system("stty raw igncr -echo");

	while (read)
	{
		switch(readInput = getchar()){
			
			case 'E':
				//buf[i++] = endOfString;
				//write (pipeIO[1], buf, MSGSIZE);
				//write (pipeIT[1], buf, MSGSIZE);
				//i = 0;
				break;
			case 'T':
				//buf[i++] = endOfString;
				//write (pipeIO[1], buf, MSGSIZE);
				//i = 0;
				read = 0;
				kill(pid,SIGUSR1);
				break;
			case abNormTermination:
				kill(pid,SIGUSR1);
				kill(0,9);
				break;
			default:
				//buf[i++] = readInput;
				bufChar[0] = readInput;
				write (pipeIO[1], bufChar, 2);
				break;
		}

	}
	system("stty -raw -igncr echo");
	//printf("Terminate Parent Process.\n");	
	exit(0);
}

/*------ Child Code --------------------*/
void outputProcess (int p[2])
{
	int nread;
	char buf[MSGSIZE];

	close (p[1]);    /* close the write descriptor */
	printf ("Output: ");
	while(!normalTermination)
	{
		switch (nread = read(p[0], buf, MSGSIZE))
		{
		  case -1:
		  case 0:
		  	break;
		default:
			printf ("%s", buf);
			fflush(stdout);
		}
	}
	//printf("SIGUSR1 received.\n");
	//system("stty -raw -igncr echo");
}

void translateProcess(int pipeIT[2],int pipeOT[2])
{
	char buf[MSGSIZE];
	int nread;
	char bufTranslate[MSGSIZE];
	char letterA = 'a';
	/* close the write descriptor */
	close (pipeIT[1]); 
	close (pipeOT[1]);   

	while(!normalTermination)
	{
		switch (nread = read(pipeIT[0], buf, MSGSIZE))
		{
		  case -1:
		  case 0:
			break;
		  break;
		default:
			for(int j = 0;j != '\0'; j++){
				switch(buf[j]){
					case 'a':	
						bufTranslate[j] = letterA;
					case 'X':
						bufTranslate[strlen(bufTranslate)-1] = 0;
						break;
					case 'E':
						break;
					default:
						break;			
				}
			}
			printf ("MSG translate = %s", bufTranslate);
		}
	}
	//printf("SIGUSR1 received.\n");
	//system("stty -raw -igncr echo");
}

/*---------- Error function ------*/
void fatal (char *s)
{
	perror (s);    /* print error msg and die */
	exit(1);
}





