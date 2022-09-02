#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>

typedef struct{
	int start;
	int end;
    int how;
	int **before_arr;
	int **after_arr;
}Argu;

int q_num;
int row,col,epoch;



void copy_arr(int **to,int **from){
    for(int i=0;i<row;i++){
        for(int j=0;j<col;j++){
            to[i][j] = from[i][j];
        }
    }
    return;
}

void cell_rule(int r,int c,Argu *ptr){
    int count=0;
    //print_arr(ptr->before_arr);
    for(int i =r-1;i<=r+1;i++){
		for(int j =c-1;j<=c+1;j++){
			if(i >= 0 && i < row && j>=0 &&j < col){
				if(ptr->before_arr[i][j] == 1){
					count++;
				}
			}
		}
    }            
    if(ptr->before_arr[r][c]==1)
        count--;            
    //printf("count : %d r: %d c: %d\n",count,r,c);
    if(count==3&&ptr->before_arr[r][c]==0){
        ptr->after_arr[r][c]=1;
    }
    else if(count<2&&ptr->before_arr[r][c]==1){
        ptr->after_arr[r][c]=0;
    }
    else if(count>3&&ptr->before_arr[r][c]==1)
    {
        ptr->after_arr[r][c]=0;
    }
    else if((count==2||count==3)&&ptr->before_arr[r][c]==1){
        ptr->after_arr[r][c]=1;
    }
    return;
}

void *run_epoch(void *argument){
    Argu *ptr = argument;
    //print_arr(ptr->before_arr);
    if(ptr->how==0){
    for(int i=ptr->start;i<=ptr->end;i++){
        for(int j=0;j<col;j++){
            cell_rule(i,j,ptr);
        }
    }
    }        
    else if(ptr->how==1){
        for(int i=ptr->start;i<=ptr->end;i++){
            for(int j=0;j<row;j++){
                cell_rule(j,i,ptr);
            }
        }  
    }
}


int main(int argc, char **argv){
    if(argc != 5){
		fprintf(stderr, "incorrect input\n");
		exit(1);
	}
    FILE *fp = fopen(argv[3],"r");
    FILE *fw = fopen(argv[4],"w");
    char c;
    int q_num=atoi(argv[2]);
	pthread_t tid[q_num];
    Argu argu[q_num];
    int ret = fscanf(fp,"%d %d %d\n",&row,&col,&epoch);
    //printf("%d %d %d\n",row,col,epoch);

    int **init_before_arr;
    int **init_after_arr;

    init_before_arr = (int**)malloc(row * sizeof(int*));
    init_after_arr = (int**)malloc(row * sizeof(int*));
    for (int i = 0; i < row; i++){
        init_before_arr[i] = (int*)malloc(col * sizeof(int));
        init_after_arr[i] = (int*)malloc(col * sizeof(int));
    }


    for(int i=0;i<row;i++){
        for(int j=0;j<col;j++){
            ret =fscanf(fp,"%c",&c);
            if(c=='O')
                init_before_arr[i][j]=1;
            else
                init_before_arr[i][j]=0;
            init_after_arr[i][j]=0;    
        }
        ret = fscanf(fp,"%c",&c);  
    } 
    if(strcmp(argv[1],"-t")==0){
        int row_or_col=0,bigger=0;
        if(row>=col){
            bigger=row;
            row_or_col=0;
        }
        else{
            bigger=col;
            row_or_col=1;
        }
        for(int i=0;i<q_num;i++){
            argu[i].how=row_or_col;
            argu[i].before_arr = init_before_arr;
            argu[i].after_arr = init_after_arr;
            argu[i].start = i * (bigger/q_num);
            if(i == q_num-1)
                argu[i].end = bigger-1;
            else
                argu[i].end = (i+1) * (bigger/q_num) -1;    
        }    
        for(int e=0;e<epoch;e++){
            for(int i=0;i<q_num;i++){
            pthread_create(&tid[i],NULL,run_epoch,&argu[i]);
            }
        for(int i = 0;i<q_num;i++){
			pthread_join(tid[i], NULL);
		}
        copy_arr(init_before_arr,init_after_arr);
        }
    }
    for(int i=0;i<row;i++){
        for(int j=0;j<col;j++){
            if(init_before_arr[i][j]==1)
                fprintf(fw,"O");
            else
                fprintf(fw,".");    
        }
        if(i < row-1){
			fprintf(fw, "\n");
		}
    }
    fclose(fp);
    fclose(fw);
}    