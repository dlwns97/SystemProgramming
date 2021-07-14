#include "myshell.h"

// support pipelined command

int Pipe(int*fd)
{
	if(pipe(fd)==-1)
		unix_error("pipe error");
}
void seperate_pipe(char*cmd1[], char*cmd2[], char*cmd[])
{
	// read cmd and seperate from first '|'
	char*delim;
	int argc1=0,argc2=0;
	int idx=0;
	int idx2=0;

	//cmd1
	while(cmd[idx]!=NULL&&strcmp(cmd[idx],"|")!=0){
		cmd1[idx]=cmd[idx];
		idx++;
	}
	if(cmd[idx]!=NULL){
		idx+=1;
		while(cmd[idx]!=NULL){
			cmd2[idx2]=cmd[idx];
			idx2++;
			idx++;
		}
		cmd2[idx2]='\0';
	}
	cmd2[idx2]='\0';

}
void pipe_execute(char*cmd1[], char*cmd2[])
{
	int fd[2];
	char * p_cmd1[MAXARGS]= { 0 };
	char * p_cmd2[MAXARGS]= { 0 };

	sigset_t prev;
	Signal(SIGCHLD, sigchild_handler);

	Pipe(fd);

	// make child proc for pipe
	if(Fork()==0){ 
	//child
		dup2(fd[1],1);
		close(fd[0]);
		close(fd[1]);
		Execvp(cmd1[0],cmd1);
		exit(1);
	}
	// parent
	dup2(fd[0],0);
	close(fd[0]);
	close(fd[1]);
		
	seperate_pipe(p_cmd1,p_cmd2,cmd2);

	// check if more pipe exists
	if(p_cmd2[0]==0){
		// no more pipe
		Execvp(cmd2[0],cmd2);
	}
	// more pipe
	// recursive call
	else{
		pipe_execute(p_cmd1,p_cmd2);
	}
	pid=0;
	while(!pid)
		Sigsuspend(&prev);
}

// command parsing
int myshell_parseinput(char*buf, char**argv)
{
	char*delim;
	int argc;
	int idx=0;
	int flag=0;

	//check if background
	if(buf[strlen(buf)-2]=='&'){
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
	int g_type, p_type=0; // foreground or background, is pipe or not
	int idx=0;
	sigset_t prev;
	Signal(SIGCHLD, sigchild_handler);
	Signal(SIGINT, sigint_handler);

	strcpy(buf, cmd);

	//check if pipe
	while(cmd[idx]!='\0'){
		if(cmd[idx]=='|'){
			p_type=1;
			break;
		}
		idx++;
	}


	g_type = myshell_parseinput(buf, argv);

	//empty line
	if(g_type==-1)
		return;
	if(builtin_command(argv))
		return;

	switch(Fork()){
		case 0:
			//child process
			if(p_type==1){
				//pipelined command
				char* p_cmd[MAXARGS] = { 0 };
				char* p_cmd2[MAXARGS] = { 0 };
				seperate_pipe(p_cmd, p_cmd2, argv);
				pipe_execute(p_cmd,p_cmd2);
				dup2(STDOUT_FILENO,1);
			}
			// if not pipelined not builtin_command
			Execvp(*argv,argv);
			exit(1);
	}
	// parent process
	pid = 0;
	while(!pid)
		Sigsuspend(&prev);
}
int builtin_command(char**argv)
{
	
	if(!strcmp(argv[0],"cd")){
		if(chdir(argv[1])==-1){
			Sio_puts("change directory fail\n");
		}
		return 1;
	}
	if(!strcmp(argv[0],"exit"))
		exit(0);
	
	if(!strcmp(argv[0],"&"))
		return 1;
	return 0;
}
int main(void)
{
	char *dummy;
	char cmd[MAXLINE];
	while(1){

		// get command
		printf("%s> ","CSE4100-SP-P4");
		dummy=fgets(cmd,MAXLINE,stdin);
		// execute command
		if(feof(stdin))
			exit(0);
		myshell_execute(cmd);
	}
}
