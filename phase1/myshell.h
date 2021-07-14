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

volatile sig_atomic_t pid;

// shell function
int myshell_parseinput(char* buf, char** argv);
void myshell_execute(char*cmd);
int builtin_command(char**argv);

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
void Sigaddset(sigset_t *set, int signum);
int Sigsuspend(const sigset_t*set);

// own error-handling functions
void unix_error(char*msg);
void sigint_handler(int sig);
void sigchild_handler(int sig);

// Sio (Signal-safe I/O) routines
ssize_t sio_puts(char s[]);
ssize_t sio_putl(long v);
void sio_error(char s[]);

// Sio Wrappers
ssize_t Sio_puts(char s[]);
ssize_t Sio_putl(long v);
void Sio_error(char s[]);


// error handling
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
	int olderrno = errno;
	if ((pid = wait(NULL)) < 0)
		Sio_error("wait error");
	errno = olderrno;
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
int Sigsuspend(const sigset_t* set)
{
	int rc = sigsuspend(set);
	if (errno != EINTR)
		unix_error("Sigsuspend error");
	return rc;
}
/* $begin wait */
pid_t Wait(int* status)
{
	pid_t pid;

	if ((pid = wait(status)) < 0)
		unix_error("Wait error");
	return pid;
}
/* $end wait */

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

#endif
