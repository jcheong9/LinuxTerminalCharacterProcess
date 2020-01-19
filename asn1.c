

/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: asn1.cpp -	A simple program that uses fork, pipe and signals to read and process characters 
--				entered on a terminal keyboard.
--
--
-- PROGRAM: Linux Process Application
--
-- FUNCTIONS:
--		int main (void)
--		void inputProcess (int pipeIO[2], int pipeIT[2],pid_t pIOid, pid_t pITid)
--		void outputProcess(int pipeIO[],int pipeOT[])
--		void translateProcess(int pipeIT[2],int pipeOT[2])
--		void fatal (char *s)
--		void readUsual(int sig)
--		
--
-- DATE: January 15, 2020
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jameson Cheong
--
-- PROGRAMMER: Jameson Cheong
--
-- NOTES:
-- This simple  program  will  get  the  canonical  name,
-- aliases,  and  '.'  separated  Internet IP addresses for a
-- given destination host using the window socket. 
-- 
----------------------------------------------------------------------------------------------------------------------*/
#include "asn1.h"

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
		  	outputProcess (pipeIO, pipeOT);  //output process is created
		default:       /* parent */
		  	
			pTranslateid = fork(); 
			switch(pTranslateid)
			{
				case -1:        /* error */
					fatal ("fork call");
					break;

				case 0:        /* It's the child */
				  	translateProcess (pipeIT, pipeOT);  //translate process is created
				default:       /* parent */
					inputProcess (pipeIO, pipeIT, pOutputid, pTranslateid);
			}
	}
	
	system("stty -raw -igncr echo"); //return keyboard state.
	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: inputProcess
--
-- DATE: January 15, 2020
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jameson Cheong
--
-- PROGRAMMER: Jameson Cheong
--
-- INTERFACE: void inputProcess (int pipeIO[2], int pipeIT[2],pid_t pIOid, pid_t pITid)
--
-- RETURNS: void
--
-- NOTES:
-- This function gets the keyboard inputs and reads the input for processing. If character 'E' is pressed, the character write
-- to output process and buffer write to translate process. If character 'T' is pressed, same as 'E' process with additional 
-- signal to kill output procss and translate process. If chacter '^k' is pressed, all processes are killed. The rest characters
-- write to output process.
--
----------------------------------------------------------------------------------------------------------------------*/
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
				write (pipeIO[1], bufChar, 1);
				write (pipeIT[1], buf, MSGSIZE);
				buf[i] = '\0';
				break;
			case 'T':
				bufChar[0] = readInput;
				buf[i++] = readInput;
				buf[i++] = '\0';
				i = 0;
				write (pipeIO[1], bufChar, 1);
				write (pipeIT[1], buf, MSGSIZE);
				read = 0;
				sleep(1);
				kill(pIOid,SIGUSR1);
				kill(pITid,SIGUSR1);
				break;
			case abNormTermination:
				kill(pIOid,SIGUSR1);
				kill(pITid,SIGUSR1);
				kill(0,9);
				break;
			default:
				bufChar[0] = readInput;
				buf[i++] = readInput;
				write (pipeIO[1], &readInput, 1);
				break;
		}
		

	}
	
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: outputProcess
--
-- DATE: January 15, 2020
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jameson Cheong
--
-- PROGRAMMER: Jameson Cheong
--
-- INTERFACE: void outputProcess(int pipeIO[],int pipeOT[])
--
-- RETURNS: void
--
-- NOTES:
-- This function gets characters from inputs process and gets buffer form translate process. If character 'E' or 'T' received 
-- from input process switch and process buffer from translate process. The rest of characters are printed on to the screen.
--
----------------------------------------------------------------------------------------------------------------------*/
void outputProcess(int pipeIO[],int pipeOT[])
{
	int nread;
	int pressedE = 0;
	int startOutputPrint = 1;
	char buf[MSGSIZE];
	/* close the write descriptor */
	close (pipeIO[1]); 
	/* close the write descriptor */   
	close (pipeOT[1]); 
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
							printf ("%c\n\r", buf[0]);
							fflush(stdout);		
							pressedE = 1;
							startOutputPrint = 1;
							break;
						case 'T':
							printf ("%c\n\r", buf[0]);
							fflush(stdout);
							pressedE = 1;
							startOutputPrint = 1;
							break;
						default:
							if(startOutputPrint){
								printf ("Output: ");
								startOutputPrint = 0;
							}
							printf ("%c", buf[0]);
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
					pressedE = 0;
					fflush(stdout);
					break;
			}
		}
	}
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: translateProcess
--
-- DATE: January 15, 2020
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jameson Cheong
--
-- PROGRAMMER: Jameson Cheong
--
-- INTERFACE: void translateProcess(int pipeIT[2],int pipeOT[2])
--
-- RETURNS: void
--
-- NOTES:
-- This function gets characters from inputs process and sent translated buffer to output process. If character 'a' 
-- received, the character is converted to 'z'. If character 'X' recieved, the last character is deleted. If character 'K'
-- received, bufTranslate index is reset to zero to line-kill. The rest of characters are append to bufTranslate.
--
----------------------------------------------------------------------------------------------------------------------*/
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
							bufTranslate[j++] = 'z';
							break;
						case 'X':
							j = (j > 0) ? j - 1 : 0;
							break;
						case 'K':
							j=0;
							break;
						default:
							bufTranslate[j++] = buf[i];
							break;			

					}

				}
				bufTranslate[j-1] = '\0';
				write (pipeOT[1], bufTranslate, MSGSIZE);
				fflush(stdout);	
		}
	}

}

/*---------- Error function ------*/
void fatal (char *s)
{
	perror (s);    /* print error msg and die */
	exit(1);
}


void readUsual(int sig)
{
    if (sig == SIGUSR1)
    {
        normalTermination = 1;
    }
}





