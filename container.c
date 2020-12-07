#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/wait.h>
#include <errno.h>

#define STACKSIZE (1024*1024)
static char child_stack[STACKSIZE];

struct clone_args {
        char **argv;
};

// child_exec is the func that will be executed as the result of clone
static int child_exec(void *stuff)
{
        struct clone_args *args = (struct clone_args *)stuff;
/*        
        // mount proc
        if (umount("/proc", 0) != 0) {
                fprintf(stderr, "failed unmount /proc %s\n",
                        strerror(errno));
                exit(-1);
        }
        if (mount("proc", "/proc", "proc", 0, "") != 0) {
                fprintf(stderr, "failed mount /proc %s\n",
                        strerror(errno));
                exit(-1);
        }
        
  */  
//cpu uncomment
// system("sysbench --test=cpu --cpu-max-prime=20000 run");
// Memory
//system("sysbench --test=memory --num-threads=140 --memory-total-size=10G run");
//file io
//system("sysbench --num-threads=16 --test=fileio --file-total-size=10G --file-test-mode=rndrw prepare");
//system(" sysbench --num-threads=16 --test=fileio --file-total-size=10G --file-test-mode=rndrw run");
//system("sysbench --num-threads=16 --test=fileio --file-total-size=10G --file-test-mode=rndrw cleanup ");
//threading
//system("sysbench --test=threads --thread-locks=10 --max-time=60 run");
        






 if (mount("none", "/mnt", "tmpfs", 0, "") != 0) {
                fprintf(stderr, "failed to mount mnt %s\n",
                        strerror(errno));
                exit(-1);
        }
    
        // sethostname
        const char * new_hostname = "my container";
        if (sethostname(new_hostname, strlen(new_hostname)) != 0) {
                fprintf(stderr, "failed to execvp argments %s\n",
                        strerror(errno));
                exit(-1);
        }
        
        if (execvp(args->argv[0], args->argv) != 0) {
                fprintf(stderr, "failed to execvp argments %s\n",
                        strerror(errno));
                exit(-1);
        }
        // we should never reach here!
        exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
        struct clone_args args;
        args.argv = &argv[1];

        int clone_flags = CLONE_NEWUSER|CLONE_NEWNET  |CLONE_NEWPID|CLONE_NEWNS | CLONE_NEWUTS| SIGCHLD; //all namespaces listed

        // the result of this call is that our child_exec will be run in another
        // process returning it's pid
        pid_t pid =
            clone(child_exec, child_stack + STACKSIZE, clone_flags, &args);
        if (pid < 0) {
                fprintf(stderr, "clone failed WTF!!!! %s\n", strerror(errno));
                exit(EXIT_FAILURE);
        }
        // lets wait on our child process here before we, the parent, exits
        if (waitpid(pid, NULL, 0) == -1) {
                fprintf(stderr, "failed to wait pid %d\n", pid);
                exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
}
