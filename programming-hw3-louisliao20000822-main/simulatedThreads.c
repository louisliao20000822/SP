#include "threadtools.h"


void MountainClimbing(int thread_id, int number){

	ThreadInit(thread_id, number);
	
	for (Current->i = 1; Current->i <= Current->N; Current->i++){
		sleep(1);
		if (Current->i == 1){
			Current->x = 1;
			printf("Mountain Climbing: %d\n", 1);}
		else if(Current->i  ==2){
			Current->y =2;
			printf("Mountain Climbing: %d\n", 2);}	
		else{
			Current->w = Current->x + Current->y;
			Current->x = Current->y;
			Current->y = Current->w;
			printf("Mountain Climbing: %d\n", Current->w);
	}
	        ThreadYield();
	}
	ThreadExit();
}


void ReduceInteger(int thread_id, int number){

	ThreadInit(thread_id,number);

	Current->i = 0;
	if(Current->N==1){
	sleep(1);	
	printf("Reduce Integer: %d\n", 0);		
	ThreadYield();
	} 	
    while (Current->N > 1){
		sleep(1);
        if(Current->N % 2 == 0)         
            Current->N  = Current->N/2; 
        else if(Current->N  == 3 || Current->N % 4 == 1){ 
            Current->N  = Current->N  - 1;
		}	
        else{                      
            Current->N  = Current->N  + 1;
		}	
        Current->i++;
		printf("Reduce Integer: %d\n", Current->i);
		
		ThreadYield();
		}
	ThreadExit();	

}

// Operation Count
// You are required to solve this function via iterative method, instead of recursion.
void OperationCount(int thread_id, int number){
	/* Please fill this code section. */
	ThreadInit(thread_id,number);

	if(Current->N==1){
		sleep(1);
		printf("Operation Count: %d\n", 0);
		ThreadYield();
	}
	for (Current->i = 0; Current->i < Current->N/2; Current->i++)
	{
		sleep(1);
		Current->x += Current->N-(2*Current->i + 1);
		printf("Operation Count: %d\n", Current->x);
		ThreadYield();		
	}
	ThreadExit();	
	
}

