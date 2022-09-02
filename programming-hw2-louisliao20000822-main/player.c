#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>


int my_guess(int player_id,int round){
    int guess;
    /* initialize random seed: */
    srand ((player_id + round) * 323);
    /* generate guess between 1 and 1000: */
    guess = rand() % 1001;
    return guess;
}

int main(int argc, char** argv)
{
	int n;
	char buf[1024];

	int player_id = atoi(argv[2]);


	for(int i = 1; i <= 10; i++) {
		int guess = my_guess(player_id,i);
        printf("%d %d\n", player_id, guess);
		fflush(stdout);
	    fsync (STDOUT_FILENO);

    }   
	exit(0);
}