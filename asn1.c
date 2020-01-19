#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

#define MSGSIZE 		256
#define abNormTermination 	0x0B

/*------ Function prototypes ----------*/
void inputProcess(int pipeIO[], int pipeIT[], pid_t pIOid, pid_t pITid);
void outputProcess(int pipeIO[],int pipeOT[]);
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
	
	system("stty raw igncr -echo"); //disable keyboard functionality
	
	/*----- Open the pipe -----------*/
	if (pipe(pipeIO) < 0 || pipe(pipeIT) < 0 || pipe(pipeOT) < 0 )
	{
		fatal ("pipe call");
		exit(1);
	}

	/*---- Set the O_NDELAY flag for pipeIO[0] -----------*/
	if (fcntl (pipeIO[0], F_SETFL, O_NDELAY) < 0)
	  	fatal ("fcntl call for input and output");
	if (fcntl (pipeIT[0], F_SETFL, O_NDELAY) < 0)
	  	fatal ("fcntl call for input and translate");


	/*-------- fork ---------------*/
	pOutputid = fork(); 
	switch(pOutputid)
	{
		case -1:        /* error */
			fatal ("fork call");
			break;

		case 0:        /* It's the child */
		  	outputProcess (pipeIO, pipeOT);
		default:       /* parent */
		  	
			pTranslateid = fork(); 
			switch(pTranslateid)
			{
				case -1:        /* error */
					fatal ("fork call");
					break;

				case 0:        /* It's the child */
				  	//printf("translateProcess.\n");
				  	translateProcess (pipeIT, pipeOT);
				default:       /* parent */
					inputProcess (pipeIO, pipeIT, pOutputid, pTranslateid);
			}
	}
	
	system("stty -raw -igncr echo"); //return keyboard state.
	return 0;
}


void inputProcess (int pipeIO[2], int pipeIT[2],pid_t pIOid, pid_t pITid)
{
	char readInput;
	char buf[MSGSIZE];
	char bufChar[1];
	int read = 1;
	int i = 0;

	/* close the read descriptor */
	close (pipeIO[0]);  
	close (pipeIT[0]);   
	

	while (read)
	{
		switch(readInput = getchar()){
			
			case 'E':
				bufChar[0] = readInput;
				buf[i++] = readInput;
				buf[i++] = '\0';
				i = 0;
				write (pipeIT[1], buf, MSGSIZE);
				write (pipeIO[1], bufChar, 1);
				break;
			case 'T':
				bufChar[0] = readInput;
				buf[i++] = readInput;
				buf[i++] = '\0';
				i = 0;
				write (pipeIO[1], bufChar, 1);
				write (pipeIT[1], buf, MSGSIZE);
				read = 0;
				kill(pIOid,SIGUSR1);
				kill(pITid,SIGUSR1);
				break;
			case abNormTermination:
				kill(pIOid,SIGUSR1);
				kill(pITid,SIGUSR1);
				kill(0,9);
				break;
			default:
				buf[i++] = readInput;
				bufChar[0] = readInput;
				write (pipeIO[1], bufChar, 1);
				break;
		}
		

	}
	sleep(1);

	//printf("Terminate Parent Process.\n");	
}

/*------ Child Code --------------------*/
void outputProcess(int pipeIO[],int pipeOT[])
{
	int nread;
	int pressedE = 0;
	char buf[MSGSIZE];
	/* close the write descriptor */
	close (pipeIO[1]); 
	/* close the write descriptor */   
	close (pipeOT[1]); 
	printf ("Output: ");
	while(!normalTermination)
	{
		if(!pressedE){
			switch (nread = read(pipeIO[0], buf, MSGSIZE))
			{
				case -1:
				case 0:
					break;
				default:
					switch(buf[0]){
					
						case 'E':
							printf ("%s\n\r", buf);
							fflush(stdout);
							
							pressedE = 1;
							break;
						default:
							printf ("%s", buf);
							fflush(stdout);
							break;

					}
					break;
			}
		}else{
			switch (nread = read(pipeOT[0], buf, MSGSIZE))
			{
				case -1:
				case 0:
					break;
				default:
					printf ("Translate: ");
					printf ("%s\n\r", buf);
					fflush(stdout);
					pressedE = 0;
					printf ("Output: ");
					break;
			}
		}
	}
}

void translateProcess(int pipeIT[2],int pipeOT[2])
{
	char buf[MSGSIZE];
	int nread;
	
	char bufTranslate[MSGSIZE];
	int i = 0;
	int j = 0;
	
	/* close the write descriptor */
	close (pipeIT[1]); 
	/* close the read descriptor */
	close (pipeOT[0]);   
	//printf("translateProcess.\n");
	while(!normalTermination)
	{
		switch (nread = read(pipeIT[0], buf, MSGSIZE))
		{
		  	case -1:
		  	case 0:
		  		
				break;
			default:
				
				for(i = 0, j = 0; buf[i]!='\0'; i++){	
					switch(buf[i]){
						case 'a':	
							
							bufTranslate[i++] = 'z';
						case 'X':
							j = (j > 0) ? j - 1 : 0;
							break;
						default:
							
							bufTranslate[j++] = buf[i];
							//printf ("%s", bufTranslate);
							//fflush(stdout);
							break;			

					}
	
					//printf ("MSG translate = %s", bufTranslate);
				}
				bufTranslate[j-1] = '\0';
				write (pipeOT[1], bufTranslate, MSGSIZE);
				
		}
	}

	//printf("SIGUSR1 translate received.\n");
	//system("stty -raw -igncr echo");
}

/*---------- Error function ------*/
void fatal (char *s)
{
	perror (s);    /* print error msg and die */
	exit(1);
}





