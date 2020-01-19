#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

#define MSGSIZE 		256
#define abNormTermination 	0x0B

/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: asn1.cpp -	A simple program that uses fork, pipe and signals to read and process characters 
--				entered on a terminal keyboard.
--
--
-- PROGRAM: Basic Linux Application
--
-- FUNCTIONS:
--		int nameAddr(TCHAR* ip, HWND textHwnd)
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
void inputProcess(int pipeIO[], int pipeIT[], pid_t pIOid, pid_t pITid);
void outputProcess(int pipeIO[],int pipeOT[]);
void translateProcess(int pipeIT[],int pipeOT[]);
void fatal (char *);
void sig_usr(int signo);
void readUsual(int sig);

static int normalTermination = 0;




