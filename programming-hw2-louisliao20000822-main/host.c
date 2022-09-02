#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

int id;
int depth;
int luck_num;




int cau_diff(int a)
{
	if(a>=luck_num)
		return a-luck_num;
	else
		return luck_num-a;
}

int main(int argc, char *argv[]){
 
    char buf[512];
    
	int status;
	int c;
	while((c=getopt(argc,argv,"l:m:d:"))!= -1){
		switch(c){
			case 'l':
				luck_num = atoi(optarg);
				break;
			case 'm':
				id = atoi(optarg);
				break;
			case 'd':
				depth = atoi(optarg);
				break;
		}
	}

    if(depth == 0){ 
        int player[8];
        sprintf(buf, "fifo_%d.tmp",id );
        FILE *readFIFO = fopen(buf, "r");
        FILE *writeFIFO = fopen("fifo_0.tmp", "w");
        

            pid_t child_left_pid, child_right_pid;
            int pipe_read1[2], pipe_read2[2];
            int pipe_write1[2], pipe_write2[2];
            pipe(pipe_write1);
            pipe(pipe_write2);
            pipe(pipe_read1);
            pipe(pipe_read2);


            if( (child_left_pid=fork()) && (child_right_pid=fork()) < 0)
            fprintf(stderr,"error fork\n");
            if(child_left_pid==0){ 
                dup2(pipe_write1[0], STDIN_FILENO);
                close(pipe_write1[1]);
                dup2(pipe_read1[1], STDOUT_FILENO);
                close(pipe_read1[0]);
                
                close(pipe_write2[0]); 
                close(pipe_write2[1]);
                close(pipe_read2[0]);
                close(pipe_read2[1]);
                char buf_id[512],buf_luck_num[512];
                sprintf(buf_id,"%d",id);
				sprintf(buf_luck_num,"%d",luck_num);
                execlp("./host","./host","-m",buf_id,"-d","1","-l",buf_luck_num,NULL);
            }
            else if(child_right_pid==0){ 
                dup2(pipe_write2[0], STDIN_FILENO);
                dup2(pipe_read2[1], STDOUT_FILENO);
                close(pipe_write2[1]);
                close(pipe_read2[0]);

                close(pipe_write1[0]); 
                close(pipe_write1[1]);
                close(pipe_read1[0]);
                close(pipe_read1[1]);

                char buf_id[512],buf_luck_num[512];
                sprintf(buf_id,"%d",id);
				sprintf(buf_luck_num,"%d",luck_num);
                execlp("./host","./host","-m",buf_id,"-d","1","-l",buf_luck_num,NULL);
            }
            else{ 

            
                close(pipe_write1[0]);
                close(pipe_write2[0]);
                close(pipe_read1[1]);
                close(pipe_read2[1]);
                FILE *fpwrite1 = fdopen(pipe_write1[1], "w");
                FILE *fpwrite2 = fdopen(pipe_write2[1], "w");
                FILE *fpread1 = fdopen(pipe_read1[0], "r");
                FILE *fpread2 = fdopen(pipe_read2[0], "r");

                fscanf(readFIFO,"%d %d %d %d %d %d %d %d", &player[0], &player[1], &player[2], &player[3],&player[4],	
			    &player[5], &player[6], &player[7]);

                while(player[0]!=-1){

                    fprintf(fpwrite1, "%d %d %d %d\n", player[0], player[1], player[2], player[3]);
                    fflush(fpwrite1);
                    fprintf(fpwrite2, "%d %d %d %d\n", player[4], player[5], player[6], player[7]);
                    fflush(fpwrite2);


                    int player_score[13]={0};
                    int player_id[2]={0};
                    int player_guess[2] ={0};

                    for(int i=0;i<10;i++){

                        fscanf(fpread1, "%d %d", &player_id[0], &player_guess[0]);
                        fscanf(fpread2, "%d %d", &player_id[1], &player_guess[1]);

                        if(cau_diff(player_guess[0]) < cau_diff(player_guess[1])){
                                player_score[player_id[0]]+=10;
                        }
                        else if(cau_diff(player_guess[0]) > cau_diff(player_guess[1])){
                                player_score[player_id[1]]+=10;
                        }
                        else{
                            if(player_id[0]<player_id[1]){
                                player_score[player_id[0]]+=10;
                            }
                            else{
                                player_score[player_id[1]]+=10;
                            }
                        }
                    }
                    

                    sprintf(buf,"%d\n",id);	
                    fprintf(writeFIFO,buf,strlen(buf));
                    for(int i=0;i<8;i++){
                    sprintf(buf,"%d %d\n",player[i],player_score[player[i]]);
                    fprintf(writeFIFO,buf,strlen(buf));
                    }
                    fflush(writeFIFO);

                    fscanf(readFIFO,"%d %d %d %d %d %d %d %d", &player[0], &player[1], &player[2], &player[3],&player[4],	
                    &player[5], &player[6], &player[7]);
            }
            fprintf(fpwrite1, "%d %d %d %d\n", player[0], player[1], player[2], player[3]);
            fflush(fpwrite1);
            fprintf(fpwrite2, "%d %d %d %d\n", player[4], player[5], player[6], player[7]);
            fflush(fpwrite2);
            wait(NULL); 
            wait(NULL);
            fclose(fpwrite1);
            fclose(fpwrite2);
            fclose(fpread1);
            fclose(fpread2);
            
        }
        exit(0);
    }
    else if(depth == 1){ 
        int player[4];

        for(int i=0;i<4;i++) 
            scanf("%d", &player[i]);
        
        pid_t leaf_left_pid, leaf_right_pid;
        int pipe_read1[2], pipe_read2[2];
        int pipe_write1[2], pipe_write2[2];

        pipe(pipe_write1); 
        pipe(pipe_write2);
        pipe(pipe_read1);
        pipe(pipe_read2);

        if( (leaf_left_pid=fork()) && (leaf_right_pid=fork()) < 0 )
        fprintf(stderr,"in child fork error\n");
        if(leaf_left_pid==0){  
            dup2(pipe_write1[0], STDIN_FILENO);
            dup2(pipe_read1[1], STDOUT_FILENO);            
            close(pipe_write1[1]);
            close(pipe_read1[0]);

            close(pipe_write2[0]); 
            close(pipe_write2[1]);
            close(pipe_read2[0]);
            close(pipe_read2[1]);

			char buf_id[512],buf_luck_num[512];
            sprintf(buf_id,"%d",id);
			sprintf(buf_luck_num,"%d",luck_num);
            execlp("./host","./host","-m",buf_id,"-d","2","-l",buf_luck_num,NULL);
        }
        else if(leaf_right_pid==0){  
            dup2(pipe_write2[0], STDIN_FILENO);
            dup2(pipe_read2[1], STDOUT_FILENO);            
            close(pipe_write2[1]);
            close(pipe_read2[0]);

            close(pipe_write1[0]); 
            close(pipe_write1[1]);
            close(pipe_read1[0]);
            close(pipe_read1[1]);
			char buf_id[512],buf_luck_num[512];
			sprintf(buf_id,"%d",id);
			sprintf(buf_luck_num,"%d",luck_num);
            execlp("./host","./host","-m",buf_id,"-d","2","-l",buf_luck_num,NULL);
        }
        else{  

            close(pipe_write1[0]);
            close(pipe_write2[0]);
            close(pipe_read1[1]);
            close(pipe_read2[1]);
            FILE *fpwrite1 = fdopen(pipe_write1[1], "w");
            FILE *fpwrite2 = fdopen(pipe_write2[1], "w");
            FILE *fpread1 = fdopen(pipe_read1[0], "r");
            FILE *fpread2 = fdopen(pipe_read2[0], "r");

            while(player[0]!= -1) {
                fprintf(fpwrite1, "%d %d\n", player[0], player[1]);
                fflush(fpwrite1);

                fprintf(fpwrite2, "%d %d\n", player[2], player[3]);
                fflush(fpwrite2);


                int player_id[2]={0};
                int player_guess[2]={0};
                int winner;

                for(int i=0;i<10;i++){

                fscanf(fpread1, "%d %d", &player_id[0], &player_guess[0]);
                fscanf(fpread2, "%d %d", &player_id[1], &player_guess[1]);
                
                
                if(cau_diff(player_guess[0]) < cau_diff(player_guess[1])){
                        winner = 0;
                }
                else if(cau_diff(player_guess[0]) > cau_diff(player_guess[1])){
                        winner =1;
                }
                else{
                    if(player_id[0]<player_id[1]){
                        winner =0;
                    }
                    else{
                        winner =1;
                    }
                }
                printf("%d %d\n",player_id[winner],player_guess[winner]);
                fflush(stdout);

                }

                for(int i=0;i<4;i++) 
                    scanf("%d", &player[i]);

            }

            fprintf(fpwrite1, "%d %d\n", player[0], player[1]);
            fflush(fpwrite1);

            fprintf(fpwrite2, "%d %d\n", player[2], player[3]);
            fflush(fpwrite2);

            wait(NULL);
            wait(NULL);
            fclose(fpwrite1);
            fclose(fpwrite2);
            fclose(fpread1);
            fclose(fpread2);

        }
        exit(0);
    }
    else if(depth == 2){ 
        int player[2];

        for(int i=0;i<2;i++) 
            scanf("%d", &player[i]);
        
        
        
        while(player[0]!=-1){

            pid_t  player_left_pid, player_right_pid;
            int pipe_read1[2], pipe_read2[2];
            pipe(pipe_read1);
            pipe(pipe_read2);

            if( (player_left_pid=fork()) && (player_right_pid=fork()) < 0 )
            fprintf(stderr,"in leaf fork error\n"); 
            if(player_left_pid==0){ 
                dup2(pipe_read1[1], STDOUT_FILENO);
                close(pipe_read1[0]);

                close(pipe_read2[0]);
                close(pipe_read2[1]);

                sprintf(buf, "%d", player[0]);
                execlp("./player", "./player", "-n",buf, NULL);
            }
            else if(player_right_pid==0){ 
                dup2(pipe_read2[1], STDOUT_FILENO);
                close(pipe_read2[0]);

                close(pipe_read1[0]);
                close(pipe_read1[1]);

                sprintf(buf, "%d", player[1]);
                execlp("./player", "./player", "-n",buf, NULL);
            }
            else{ 
                
                close(pipe_read1[1]);
                close(pipe_read2[1]);

                FILE *fpread1 = fdopen(pipe_read1[0], "r");
                FILE *fpread2 = fdopen(pipe_read2[0], "r");

                for(int i=0;i<10;i++){
                    int player_id[2];
                    int player_guess[2];
                    int winner;

                    fscanf(fpread1, "%d %d", &player_id[0], &player_guess[0]);
                    fscanf(fpread2, "%d %d", &player_id[1], &player_guess[1]);
                    if(cau_diff(player_guess[0]) < cau_diff(player_guess[1])){
                            winner = 0;
                    }
                    else if(cau_diff(player_guess[0]) > cau_diff(player_guess[1])){
                            winner =1;
                    }
                    else{
                        if(player_id[0]<player_id[1]){
                            winner =0;
                        }
                        else{
                            winner =1;
                        }
                    }

                    printf("%d %d\n",player_id[winner],player_guess[winner]);
                    fflush(stdout);
                    fsync(STDOUT_FILENO);

                }



                fclose(fpread1);
                fclose(fpread2);

                wait(NULL);
                wait(NULL);

                for(int i=0;i<2;i++) 
                    scanf("%d", &player[i]);
            }
        }
     exit(0);
    }
    return 0;
}
