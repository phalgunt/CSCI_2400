//
// tsh - A tiny shell program with job control
//
// <Phalgun Taman ID: Phalgun4 >
//

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string>

#include "globals.h"
#include "jobs.h"
#include "helper-routines.h"

//
// Needed global variable definitions
//

static char prompt[] = "tsh> ";
int verbose = 0;

//
// You need to implement the functions eval, builtin_cmd, do_bgfg,
// waitfg, sigchld_handler, sigstp_handler, sigint_handler
//
// The code below provides the "prototypes" for those functions
// so that earlier code can refer to them. You need to fill in the
// function bodies below.
//

void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

//
// main - The shell's main routine
//
int main(int argc, char *argv[])
{
  int emit_prompt = 1; // emit prompt (default)

  //
  // Redirect stderr to stdout (so that driver will get all output
  // on the pipe connected to stdout)
  //
  dup2(1, 2);

  /* Parse the command line */
  char c;
  while ((c = getopt(argc, argv, "hvp")) != EOF) {
    switch (c) {
    case 'h':             // print help message
      usage();
      break;
    case 'v':             // emit additional diagnostic info
      verbose = 1;
      break;
    case 'p':             // don't print a prompt
      emit_prompt = 0;  // handy for automatic testing
      break;
    default:
      usage();
    }
  }

  //
  // Install the signal handlers
  //

  //
  // These are the ones you will need to implement
  //
  Signal(SIGINT,  sigint_handler);   // ctrl-c
    Signal(SIGTSTP, sigtstp_handler);  // ctrl-z
    Signal(SIGCHLD, sigchld_handler);  // Terminated or stopped child

    //
    // This one provides a clean way to kill the shell
    //
    Signal(SIGQUIT, sigquit_handler);

    //
    // Initialize the job list
    //
    initjobs(jobs);

    //
    // Execute the shell's read/eval loop
    //
    for(;;) {
      //
      // Read command line
      //
      if (emit_prompt) {
        printf("%s", prompt);
        fflush(stdout);
      }

      char cmdline[MAXLINE];

      if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin)) {
        app_error("fgets error");
      }
      //
      // End of file? (did user type ctrl-d?)
      //
      if (feof(stdin)) {
        fflush(stdout);
        exit(0);
      }

      //
      // Evaluate command line
      //
      eval(cmdline);
      fflush(stdout);
      fflush(stdout);
    }

    exit(0); //control never reaches here
  }

  /////////////////////////////////////////////////////////////////////////////
  //
  // eval - Evaluate the command line that the user has just typed in
  //
  // If the user has requested a built-in command (quit, jobs, bg or fg)
  // then execute it immediately. Otherwise, fork a child process and
  // run the job in the context of the child. If the job is running in
  // the foreground, wait for it to terminate and then return.  Note:
  // each child process must have a unique process group ID so that our
  // background children don't receive SIGINT (SIGTSTP) from the kernel
  // when we type ctrl-c (ctrl-z) at the keyboard.
  //


/*
void eval(char *cmdline) 
{
  /* Parse command line */
  //
  // The 'argv' vector is filled in by the parseline
  // routine below. It provides the arguments needed
  // for the execve() routine, which you'll need to
  // use below to launch a process.
//   //
//   char *argv[MAXARGS];

  //
  // The 'bg' variable is TRUE if the job should run
  // in background mode or FALSE if it should run in FG
  //
 // int bg = parseline(cmdline, argv); 
 // if (argv[0] == NULL)  
 //   return; 

 // return;
//}
//*/
  void eval(char *cmdline)
  {
  
  	char *argv[MAXARGS];
      
    pid_t pid;
      
  	struct job_t *job;  
      
  	int bg = parseline(cmdline, argv);
    
    sigset_t st;
      
    sigemptyset(&st);
      
    sigaddset(&st, SIGCHLD);  
      
    if (argv[0] == NULL)
    {
  	 	return;   /* ignore empty lines */
  	} 
      
  	if (!builtin_cmd(argv))
    {
        sigprocmask(SIG_BLOCK, &st, NULL);
  		pid = fork();
        setpgid(0,0);
  		if(pid == 0)
        { 	
            sigprocmask(SIG_UNBLOCK, &st, NULL);
            
  			if(execvp(argv[0], argv) < 0)
            {
                printf("%s: Command not found\n", argv[0]);
                exit(0);
            }
  			
  		}
        
  			if(!bg)
            { 
  				addjob(jobs, pid, FG, cmdline);	
                sigprocmask(SIG_UNBLOCK, &st, NULL);
  				waitfg(pid); 
            }    
  			else
            {
  				addjob(jobs, pid, BG, cmdline); 
                sigprocmask(SIG_UNBLOCK, &st, NULL);
  				job = getjobpid(jobs, pid);	
  				printf("[%d] (%d) %s", job->jid, pid, cmdline);
  			}

  		}
       return;
  	}

   int builtin_cmd(char **argv)
    {
      string cmd(argv[0]);

      if (cmd == "quit") 
      {
        exit(0);
      }

      else if (cmd == "&") 
      { 
        return 1;
      }

      else if (cmd == "bg" || cmd == "fg") 
      {
        do_bgfg(argv);
        return 1;
      }
     
      else if (cmd == "jobs") 
      {
        listjobs(jobs);
        return 1;
      }
  return 0;     
}	
 
  void do_bgfg(char **argv)
  {
    struct job_t *jobp=NULL;

    if (argv[1] == NULL) 
    {
      printf("%s command requires PID or %%jobid argument\n", argv[0]);
      return;
    }

    if (isdigit(argv[1][0])) 
    {
      pid_t pid = atoi(argv[1]);
      if (!(jobp = getjobpid(jobs, pid))) 
      {
        printf("(%d): No such process\n", pid);
        return;
      }
    }
    else if (argv[1][0] == '%') 
    {
      int jid = atoi(&argv[1][1]);
      if (!(jobp = getjobjid(jobs, jid))) 
      {
        printf("%s: No such job\n", argv[1]);
        return;
      }
    }
    else 
    {
      printf("%s: argument must be a PID or %%jobid\n", argv[0]);
      return;
    }

    //finished:
    string cmd(argv[0]);
      
    if(cmd=="bg")  
    {
        jobp->state = BG; 
        kill(-jobp->pid, SIGCONT); 
        printf("[%d] (%d) %s", jobp->jid, jobp->pid, jobp->cmdline);
    }
    else if(cmd=="fg")  
    {
        jobp->state = FG; 
        kill(-jobp->pid, SIGCONT); 
        waitfg(jobp->pid); 
    }
    return;
  }


void waitfg(pid_t pid)
{
  while(pid == fgpid(jobs))
  { 
      sleep(1);
  }
    return;
}

/*
WIFEXITED(status). Returns true if the child terminated normally, via a call to exit or a return.

WEXITSTATUS(status). Returns the exit status of a normally terminated child. This status is only defined if WIFEXITED() returned true.

WIFSIGNALED(status). Returns true if the child process terminated because of a signal that was not caught.

WTERMSIG(status). Returns the number of the signal that caused the child process to terminate. This status is only defined if WIFSIGNALED() returned true.

WIFSTOPPED(status). Returns true if the child that caused the return is currently stopped.

WSTOPSIG(status). Returns the number of the signal that caused the child to stop. This status is only defined if WIFSTOPPED() returned true.

WIFCONTINUED(status). Returns true if the child process was restarted by receipt of a SIGCONT signal.
*/

void sigchld_handler(int sig) 
  {
  	int status; 
  	pid_t pid;	
  	while((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) //Return immediately, with a return value of 0, if none of the children in the wait set has stopped or terminated, or with a return value equal to the PID of one of the stopped or terminated children.
    {
        if(WIFEXITED(status))
        {
   			deletejob(jobs, pid);
   		}
  		else if(WIFSTOPPED(status))
        { 
  			struct job_t *job = getjobpid(jobs, pid); 
    		job->state = ST;
  			printf("Job [%d] (%d) stopped by signal 20\n", job->jid, pid);
  			return;
  		}
  		else if(WIFSIGNALED(status))
        { 
  			struct job_t *job = getjobpid(jobs, pid);
  			printf("Job [%d] (%d) terminated by signal 2\n", job->jid, pid);
  			deletejob(jobs, pid);
  		}
   	}
  	return;

  }

void sigint_handler(int sig)
  {
  pid_t pid = fgpid(jobs);
  if (pid > 0)
  { 
      kill(-pid, sig); 
  }
  return;
  }

void sigtstp_handler(int sig)
  {
    pid_t pid = fgpid(jobs);
    if(pid > 0)
    {
        kill(-pid, sig);
    }
    return;
  }

/*********************
 * End signal handlers
 *********************/
