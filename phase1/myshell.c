#include "myshell.h"

/*******************************************

주어진 shellex.c 파일을 참고하여 프로그램을 
작성했다.

입력한 명령어는 fork()를 통해 자식에서 실행
부모 프로세스는 자식이 종료되는 걸
Sigsuspend를 통해 기다린다. 

Sigchld가 들어오면 자식을 reap하고 종료된다.

cd와 exit은 built-in으로 작성했다.

*******************************************/

#define MAXARGS 128
// command parsing
int myshell_parseinput(char*buf, char**argv)
{
	char*delim;
	int argc;
	int flag=0;

	// check if background
	if(buf[strlen(buf)-2]=='&')
	{
		flag=1;
		buf[strlen(buf)-2]=' ';
	}

	buf[strlen(buf)-1]=' ';
	while(*buf && (*buf==' ' ))
		buf++;

	// build the argv  list
	argc = 0;
	while((delim=strchr(buf,' '))){
		argv[argc++] = buf;
		*delim='\0';
		buf = delim+1;
		while(*buf&&(*buf==' '))
			buf++;
	}
	argv[argc]=NULL;

	if(argc==0)
		return -1;
	return flag;
}

// command execute, call execvp
void myshell_execute(char*cmd)
{
	char *argv[MAXARGS];
	char buf[MAXLINE];
	int g_type; // foreground or background

	// signal handling
	sigset_t prev, mask;
	Signal(SIGCHLD, sigchild_handler);
	Signal(SIGINT, sigint_handler);

	strcpy(buf, cmd);
	g_type = myshell_parseinput(buf, argv);

	//empty line
	if(g_type==-1)
		return;
	//built in command
	if(builtin_command(argv))
		return;

	switch(Fork()){
		case -1:
			unix_error("fork error");
			exit(1);
		case 0:
			//child process
			Execvp(*argv,argv);
			exit(1);
	}
	// parent process
	pid =0;
	while(!pid)
		Sigsuspend(&prev);
}
int builtin_command(char**argv)
{
	
	if(!strcmp(argv[0],"cd")){
		if(chdir(argv[1])==-1){
			unix_error("change directory fail");
			exit(0);
		}
		return 1;
	}
	if(!strcmp(argv[0],"exit")){
		exit(0);
	}
	
	if(!strcmp(argv[0],"&"))
		return 1;
	return 0;
}
int main(void)
{
	char*dummy;
	char cmd[MAXLINE];
	while(1){

		// get command
		printf("%s> ", "CSE4100-SP-P4");
		dummy=fgets(cmd,MAXLINE,stdin);
		// execute command
		if(feof(stdin))
			exit(0);
		myshell_execute(cmd);
	}
}
