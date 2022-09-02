#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>

extern int timeslice, switchmode;

typedef struct TCB_NODE *TCB_ptr;
typedef struct TCB_NODE{
    jmp_buf  Environment;
    int      Thread_id;
    TCB_ptr  Next;
    TCB_ptr  Prev;
    int i, N;
    int w, x, y, z;
} TCB;

extern jmp_buf MAIN, SCHEDULER;
extern TCB_ptr Head;
extern TCB_ptr Current;
extern TCB_ptr Work;
extern sigset_t base_mask, waiting_mask, tstp_mask, alrm_mask;

void sighandler(int signo);
void scheduler();



#define ThreadCreate(function,thread_id,number)  		\
{                                                      \
	if(setjmp(MAIN)==0){									\
		function(thread_id,number);                  \
	}\
}



#define ThreadInit(thread_id,number)			\
{													\
	Current = malloc(sizeof(TCB));						\
	if(Head==NULL){                					\
		Head=Current;                 					\
		Head->N = number;							\
		Head->Thread_id=thread_id;					\
		Head->Next=Head->Prev=Head;					\
	}													\
	else{												\
		Current->N=number;								\
		Current->Thread_id=thread_id;     				\
		Work=Head->Prev;								\
		Head->Prev=Current;                             \
		Current->Next=Head;                             \
		Work->Next=Current;                             \
		Current->Prev=Work;                             \
	}    												\
	if(setjmp(Current->Environment)==0){				\
		longjmp(MAIN,1);								\
	}													\
}

#define ThreadExit()                                    \
{                                                       \
	longjmp(SCHEDULER, 2);                              \
}

#define ThreadYield()\
{                                                       \
	if(switchmode==1){										\
		sigpending(&waiting_mask);\
		if(setjmp(Current->Environment)==0){\
        if (sigismember(&waiting_mask, SIGTSTP)){\
				sigprocmask(SIG_UNBLOCK, &tstp_mask, NULL);\
		}\
		else if (sigismember(&waiting_mask, SIGALRM)){\
				sigprocmask(SIG_UNBLOCK, &alrm_mask, NULL);\
		}\
	}													\
	}\
	else{												\
		if(setjmp(Current->Environment)==0){\
			longjmp(SCHEDULER, 1);\
		}\
	}\
}
