#include "threadtools.h"

void sighandler(int signo){
    if(signo==SIGTSTP){
        printf("TSTP signal caught!\n");
    }
    else if(signo==SIGALRM){
        printf("ALRM signal caught!\n");
        alarm(timeslice);
    }
    sigprocmask(SIG_SETMASK, &base_mask, NULL);
    longjmp(SCHEDULER,1);      
}


void scheduler(){
    Current = Head;
    while (1){
        int stat_jmp=setjmp(SCHEDULER);
        if(stat_jmp==0){
            longjmp(Current->Environment,1);
        }
        else if(stat_jmp == 1){
            Current=Current->Next;
        }
        else if(stat_jmp==2){
            Work=Current;
            Current=Current->Next;
            if (Current==Work){
                break;
            }
            else{
                Current->Prev=Work->Prev;
                Work->Prev->Next=Current;
            }
        }
    }
    longjmp(MAIN,1);
}