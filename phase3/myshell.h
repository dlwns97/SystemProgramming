#ifndef __MYSEHLL_H__
#define __MYSHELL_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>

#define MAXLINE 8192
#define MAXARGS 128
#define MAXPIPENUM 6
#define MAXJOBS 10

volatile sig_atomic_t pid;

//job structure && job functions
struct job_st{
	pid_t pid; //job pid
	int jobid;//job job id
	int jstate;// job state
};

#define NOTDEF 0
#define STP 1
#define FG 2
#define BG 3

struct job_st jobs[MAXJOBS];
int next_jobid = 1;

void job_init();
void job_free(struct job_st* jobs);
int job_add(struct job_st*jobs, pid_t tpid, int state);
int job_del(struct job_st*jobs, pid_t tpid);
pid_t job_fore(struct job_st*jobs);
struct job_st *get_jobpid(struct job_st*jobs, pid_t tpid);
struct job_st *get_jobid(struct job_st*jobs, int jobid);
int convert_pid(struct job_st*jobs, pid_t tpid);
void job_listing(struct job_st*jobs);
int get_maxjobid(struct job_st*jobs);
void wait_fg_Reap(pid_t tpid);


// user defined function
int myshell_parseinput(char* buf, char** argv);
void myshell_execute(char*cmd);
int builtin_command(char**argv);
void seperate_pipe(char*cmd1[], char*cmd2[], char*cmd[]);
void pipe_execute(char*cmd1[], char*cmd2[]);
int Pipe(int*fd);

// process control wrappers
pid_t Fork(void);
void Execvp(const char*filename, char* const argv[]);
pid_t Wait(int* status);
pid_t Waitpid(pid_t pid, int * iptr, int options);

// signal wrappers
typedef void handler_t;
handler_t *Signal(int signum, handler_t *handler);
void Sigprocmask(int how, const sigset_t*set, sigset_t*oldset);
void Sigemptyset(sigset_t *set);
void Sigfillset(sigset_t *set);
void Sigaddset(sigset_t *set, int signum);
void Sigdelset(sigset_t *set, int signum);
int Sigsuspend(const sigset_t*set);

// user defined error-handling functions
void unix_error(char*msg);
void sigint_handler(int sig);
void sigchild_handler(int sig);
void sigtstp_handler(int sig);

// Sio (Signal-safe I/O) routines
ssize_t sio_puts(char s[]);
ssize_t sio_putl(long v);
void sio_error(char s[]);

// Sio Wrappers
ssize_t Sio_puts(char s[]);
ssize_t Sio_putl(long v);
void Sio_error(char s[]);

// Unix I/O wrappers
int Dup2(int fd1, int fd2);
void Close(int fd);


// job related functions
void job_init()
{
	for(int i=0;i<MAXJOBS;i++){
		jobs[i].pid=0;
		jobs[i].jobid=0;
		jobs[i].jstate=NOTDEF;
	}
}
void job_free(struct job_st*job)
{
	// free contents of selected job
	job->pid=0;
	job->jobid=0;
	job->jstate=NOTDEF;
}
int job_add(struct job_st*jobs, pid_t tpid, int state)
{
	for(int i=0;i<MAXJOBS;i++){
		if(jobs[i].pid==0){
			// if no job in i index
			jobs[i].pid=tpid;
			jobs[i].jstate=state;
			jobs[i].jobid= next_jobid++;
			return 1;
		}
	}
	printf("Jobs oveflow\n");
	return 0;
}
int job_del(struct job_st*jobs, pid_t tpid)
{
	for(int i=0;i<MAXJOBS;i++){
		if(jobs[i].pid==tpid){
			job_free(&jobs[i]);
			next_jobid = get_maxjobid(jobs)+1;
			return 1;
		}
	}
	printf("No such job\n");
	return 0;
}
int get_maxjobid(struct job_st*jobs)
{
	int max=0;
	for(int i=0;i<MAXJOBS;i++){
		if(jobs[i].jobid>max){
			max = jobs[i].jobid;
		}
	}
	return max;
}
pid_t job_fore(struct job_st*jobs)
{
	for(int i=0;i<MAXJOBS;i++){
		if(jobs[i].jstate==FG){
			return jobs[i].pid;
		}
	}
	return 0;
}
struct job_st* get_jobpid(struct job_st*jobs, pid_t tpid)
{
	for(int i=0;i<MAXJOBS;i++){
		if(jobs[i].pid==tpid){
			return &jobs[i];
			}
	}
	return NULL;
}
struct job_st* get_jobid(struct job_st* jobs, int jobid)
{
	for(int i=0;i<MAXJOBS;i++){
		if(jobs[i].jobid==jobid){
			return &jobs[i];
		}
	}
	return NULL;
}
int convert_pid(struct job_st* jobs, pid_t tpid)
{
	for(int i=0;i<MAXJOBS;i++)
	{
		if(jobs[i].pid==tpid)
		{
			return jobs[i].jobid;
		}
	}
	return 0;
}
void job_listing(struct job_st*jobs)
{
	for(int i=0;i<MAXJOBS;i++){
		if(jobs[i].pid){
			printf("%2d %d | state: ",jobs[i].jobid, jobs[i].pid);
			if(jobs[i].jstate==0){
				printf("NOT DEFINED\n");
			}
			else if(jobs[i].jstate==1){
				printf("STOPPED\n");
			}
			else if(jobs[i].jstate==2){
				printf("FOREGROUND\n");
			}
			else if(jobs[i].jstate==3){
				printf("RUNNING BACKGROUND\n");
			}
		}
	}
}
void wait_fg_Reap(pid_t tpid)
{
	struct job_st*tmp = get_jobpid(jobs,tpid);
	if(!tmp)
		return;
	while(tmp->pid==tpid&&tmp->jstate==FG)
		sleep(1);
	return;
}
// error handling, signal set
void unix_error(char* msg)
{
	fprintf(stderr, "%s: %s\n", msg, strerror(errno));
	exit(0);
}
void sigint_handler(int sig)
{
	Sio_puts("\nPress ctrl-c?\n");
	sleep(2);
	exit(0);
}
void sigchild_handler(int sig)
{
	int status;
	pid_t child;

	while((child=waitpid(-1,&status, WNOHANG|WUNTRACED))>0){
		pid = child;
		if(WIFSIGNALED(status)){
			job_del(jobs,child);
			Sio_puts("Reap chlid by kill!\n");
		}
		else if(WIFEXITED(status)){
			job_del(jobs,child);
			Sio_puts("Reaped child process id: ");
			Sio_putl(child);
			Sio_puts("\n");
		}
		else if(WIFSTOPPED(status)){
			Sio_puts("Job ");
			Sio_putl(child);
			Sio_puts(" stopped. \n");
			struct job_st *tmp = get_jobpid(jobs,child);
			tmp->jstate=STP;
		}
	}
}
void sigtstp_handler(int sig)
{
	pid_t tpid;

	if((tpid=job_fore(jobs))>0)
	{
		kill(tpid,19);
	}
	
	return;
}
handler_t* Signal(int signum, handler_t* handler)
{
	struct sigaction action, old_action;

	action.sa_handler = handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = SA_RESTART;

	if (sigaction(signum, &action, &old_action) < 0)
		unix_error("Signal error");
	return (old_action.sa_handler);
}
void Sigprocmask(int how, const sigset_t* set, sigset_t* oldset)
{
	if (sigprocmask(how, set, oldset) < 0)
		unix_error("Sigprocmask error");
	return;
}
void Sigfillset(sigset_t*set)
{
	if(sigfillset(set)<0)
		unix_error("sigfillset error");
	return;
}
void Sigemptyset(sigset_t* set)
{
	if (sigemptyset(set) < 0)
		unix_error("Sigemptyset error");
	return;
}
void Sigaddset(sigset_t* set, int signum)
{
	if (sigaddset(set, signum) < 0)
		unix_error("Sigaddset error");
	return;
}
void Sigdelset(sigset_t*set, int signum)
{
	if(sigdelset(set,signum)<0)
		unix_error("Sigdelset error");
	return;
}
int Sigsuspend(const sigset_t* set)
{
	int rc = sigsuspend(set);
	if (errno != EINTR)
		unix_error("Sigsuspend error");
	return rc;
}

pid_t Wait(int* status)
{
	pid_t pid;

	if ((pid = wait(status)) < 0)
		unix_error("Wait error");
	return pid;
}

pid_t Waitpid(pid_t pid, int* iptr, int options)
{
	pid_t retpid;

	if ((retpid = waitpid(pid, iptr, options)) < 0)
		unix_error("Waitpid error");
	return(retpid);
}
//process control wrappers
pid_t Fork(void)
{
	pid_t pid;

	if ((pid = fork()) < 0)
		unix_error("Fork error");
	return pid;
}
void Execvp(const char* filename, char* const argv[])
{
	if (execvp(filename, argv) < 0)
		unix_error("Execvp error");
}
// Sio routines
static void sio_reverse(char s[])
{
	int c, i, j;

	for (i = 0, j = strlen(s) - 1; i < j; i++, j--)
	{
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}
static void sio_ltoa(long v, char s[], int b)
{
	int c, i = 0;
	do {
		s[i++] = ((c = (v % b)) < 10) ? c + '0' : c - 10 + 'a';
	} while ((v /= b) > 0);
	s[i] = '\0';
	sio_reverse(s);
}
static size_t sio_strlen(char s[])
{
	int i = 0;
	while (s[i] != '\0')
		++i;
	return i;
}
ssize_t sio_puts(char s[])
{
	return write(STDOUT_FILENO, s, sio_strlen(s));
}
ssize_t sio_putl(long v)
{
	char s[128];
	sio_ltoa(v, s, 10);
	return sio_puts(s);
}
void sio_error(char s[])
{
	sio_puts(s);
	_exit(1);
}
// Sio Wrappers
ssize_t Sio_puts(char s[])
{
	ssize_t n;
	if ((n = sio_puts(s)) < 0)
		sio_error("Sio_puts error");
	return n;
}
ssize_t Sio_putl(long v)
{
	ssize_t n;
	if ((n = sio_putl(v)) < 0)
		sio_error("Sio_putl error");
	return n;
}
void Sio_error(char s[])
{
	sio_error(s);
}

int Dup2(int fd1, int fd2)
{
	int rc;

	if((rc=dup2(fd1, fd2))<0)
		unix_error("Dup2 error");
	return rc;
}
void Close(int fd)
{
	int rc;
	if((rc=close(fd))<0)
		unix_error("Close error");
}
#endif
