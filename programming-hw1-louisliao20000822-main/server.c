#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>

#define ERR_EXIT(a) do { perror(a); exit(1); } while(0)

typedef struct {
    char hostname[512];  // server's hostname
    unsigned short port;  // port to listen
    int listen_fd;  // fd to wait for a new connection
} server;

typedef struct {
    char host[512];  // client's host
    int conn_fd;  // fd to talk with client
    char buf[512];  // data sent by/to client
    size_t buf_len;  // bytes used by buf
    // you don't need to change this.
    int id;
    int wait_for_write;  // used by handle_read to know if the header is read or not.
} request;

server svr;  // server
request* requestP = NULL;  // point to a list of requests
int maxfd;  // size of open file descriptor table, size of request list

const char* accept_read_header = "ACCEPT_FROM_READ";
const char* accept_write_header = "ACCEPT_FROM_WRITE";

static void init_server(unsigned short port);
// initailize a server, exit for error

static void init_request(request* reqP);
// initailize a request instance

static void free_request(request* reqP);
// free resources used by a request instance

typedef struct {
    int id;          //902001-902020
    int AZ;          
    int BNT;         
    int Moderna;     
}registerRecord;

int check_valid_id(char *buf){

    if(strlen(buf)==6)
        return atoi(buf)-902000;
    else
        return 0;    
}

bool check_valid(int *arr){
    int check[4]={0};
    for(int i=0;i<3;i++){
        if(arr[i]>0&&arr[i]<4)
        {
            if(check[arr[i]]==0){
                check[arr[i]]=1;
                continue;
            }
            else
                return 0;
        }
        else
            return 0;}
        return 1;    
}

void string_prefer(int a,int b,int c,char *order){
    int pre[3]= {a,b,c};
    char name[3][512] = {"AZ","BNT","Moderna"};
    char Concat[512] = "\0";
    for(int j=1;j<=3;j++)
        for(int i=0;i<3;i++){
            if(j==3&&pre[i]==j){
                strcat(Concat,name[i]);
                strcat(Concat,".\n");
            }
            else if(pre[i]==j){
                strcat(Concat,name[i]);
                strcat(Concat," > ");
            }
        } 
    strcpy(order,Concat);  
    return;
}

int prefer_write(int id,char *pre,int fd){
    registerRecord reg;
    if(strlen(pre)!=5)
        return-1;
    if(pre[1]!=' '||pre[3]!=' ')
        return -1;
    int arr[3] = {0};
    sscanf(pre,"%d%d%d",&arr[0],&arr[1],&arr[2]);
    bool flag = check_valid(arr);
    if(flag == 0){
        return -1;
    }
    else{
        reg.AZ = arr[0];
        reg.BNT = arr[1];
        reg.Moderna = arr[2];
        reg.id =id+902000; 
        lseek(fd, sizeof(registerRecord) * (id-1), SEEK_SET);
        write(fd, &reg, sizeof(registerRecord));
        return 1;
    }
        
}

int handle_read(request* reqP) {
    int r;
    char buf[512];

    // Read in request from client
    r = read(reqP->conn_fd, buf, sizeof(buf));
    if (r < 0) return -1;
    if (r == 0) return 0;
    char* p1 = strstr(buf, "\015\012");
    int newline_len = 2;
    if (p1 == NULL) {
       p1 = strstr(buf, "\012");
        if (p1 == NULL) {
            ERR_EXIT("this really should not happen...");
        }
    }
    size_t len = p1 - buf + 1;
    memmove(reqP->buf, buf, len);
    reqP->buf[len - 1] = '\0';
    reqP->buf_len = len-1;
    return 1;
}

int main(int argc, char** argv) {
    
    // Parse args.
    if (argc != 2) {
        fprintf(stderr, "usage: %s [port]\n", argv[0]);
        exit(1);
    }
    registerRecord Record[20];
    struct sockaddr_in cliaddr;  // used by accept()
    int clilen;
    fd_set read_set,mas_set;
    int conn_fd;  // fd for a new connection with client
    int file_fd;  // fd for file that we open for reading
    char buf[512];
    int buf_len;
    bool read_lock[21]={0},write_lock[21]={0};
    struct timeval timeout;
    struct flock lock;

    file_fd=open("registerRecord",O_RDWR); //Open_File

    FD_ZERO(&read_set);
    FD_ZERO(&mas_set);
    // Initialize server
    init_server((unsigned short) atoi(argv[1]));
    // Loop for handling connections
    fprintf(stderr, "\nstarting on %.80s, port %d, fd %d, maxconn %d...\n", svr.hostname, svr.port, svr.listen_fd, maxfd);
    FD_SET(svr.listen_fd,&mas_set);
    listen(svr.listen_fd,10);

    while (1) {
        // TODO: Add IO multiplexing
        memcpy(&read_set, &mas_set, sizeof(mas_set));
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        if(select(maxfd,&read_set,NULL,NULL,&timeout)<=0)
        continue;

        for(int i=3;i<maxfd;i++){
        if(FD_ISSET(i,&read_set)){
        // Check new connection
        if(i==svr.listen_fd){
            clilen = sizeof(cliaddr);
            conn_fd = accept(svr.listen_fd, (struct sockaddr*)&cliaddr, (socklen_t*)&clilen);
            if (conn_fd < 0) {
                if (errno == EINTR || errno == EAGAIN) continue;  // try again
                if (errno == ENFILE) {
                    (void) fprintf(stderr, "out of file descriptor table ... (maxconn %d)\n", maxfd);
                    continue;
                }
                ERR_EXIT("accept");
            }
            requestP[conn_fd].conn_fd = conn_fd;
            FD_SET(conn_fd,&mas_set);
            strcpy(requestP[conn_fd].host, inet_ntoa(cliaddr.sin_addr));
            fprintf(stderr, "getting a new request... fd %d from %s\n", conn_fd, requestP[conn_fd].host);
            sprintf(buf,"Please enter your id (to check your preference order):\n");    
            write(requestP[conn_fd].conn_fd, buf, strlen(buf)); 
            continue;
            }
    // TODO: handle requests from clients
    else{
#ifdef READ_SERVER
        int id_val;
        registerRecord reg;      
        int ret = handle_read(&requestP[i]); // parse data from client to requestP[conn_fd].buf
        fprintf(stderr, "ret = %d\n", ret);
        if (ret < 0) {
             fprintf(stderr, "bad request from %s\n", requestP[i].host);
             continue;
        }
        id_val=check_valid_id(requestP[i].buf);
        requestP[i].id= id_val;
        if(atoi(requestP[i].buf)==0||id_val<1||id_val>20){
            sprintf(buf, "[Error] Operation failed. Please try again.\n");
            write(requestP[i].conn_fd, buf, strlen(buf));
            close(requestP[i].conn_fd);
            free_request(&requestP[i]); 
            FD_CLR(i,&mas_set);
            continue; 
            }
        lock.l_type = F_RDLCK;
        lock.l_start = sizeof(registerRecord) * (requestP[i].id - 1);
        lock.l_whence = SEEK_SET;
        lock.l_len = sizeof(registerRecord);    
        if(write_lock[requestP[i].id]==0&&fcntl(file_fd, F_SETLK, &lock) != -1){
            //no lock
            if(fcntl(file_fd, F_SETLK, &lock) != -1){
                read_lock[requestP[i].id]=true;
                lseek(file_fd, sizeof(registerRecord) * (requestP[i].id - 1), SEEK_SET);
                read(file_fd, &reg, sizeof(registerRecord));
                char order[512];
                string_prefer(reg.AZ,reg.BNT,reg.Moderna,order);
                sprintf(buf, "Your preference order is %s",order);
                write(requestP[i].conn_fd, buf, strlen(buf));

                lock.l_type = F_UNLCK;
                lock.l_start = sizeof(registerRecord) * (requestP[i].id - 1);
                lock.l_whence = SEEK_SET;
                lock.l_len = sizeof(registerRecord);
                fcntl(file_fd, F_SETLK, &lock);
                read_lock[requestP[i].id] = false;
            }
        }
        else{
            //locked
            sprintf(buf, "Locked.\n");
            write(requestP[i].conn_fd, buf, strlen(buf));
        }    
        close(requestP[i].conn_fd);
        free_request(&requestP[i]); 
        FD_CLR(i,&mas_set);

#else
        int id_val;
        int first,second,thrid;
        if(requestP[i].wait_for_write==0){
            int ret = handle_read(&requestP[i]); // parse data from client to requestP[conn_fd].buf
            if (ret < 0) {
                fprintf(stderr, "bad request from %s\n", requestP[i].host);
                continue;
            }
            id_val=check_valid_id(requestP[i].buf);
            if(atoi(requestP[i].buf)==0||id_val<1||id_val>20){
                sprintf(buf, "[Error] Operation failed. Please try again.\n");
                write(requestP[i].conn_fd, buf, strlen(buf));
                close(requestP[i].conn_fd);
                free_request(&requestP[i]); 
                FD_CLR(i,&mas_set);
                continue; 
            }
            requestP[i].id=id_val;
            registerRecord reg;

            lock.l_type = F_WRLCK;
            lock.l_start = sizeof(registerRecord) * (requestP[i].id - 1);
            lock.l_whence = SEEK_SET;
            lock.l_len = sizeof(registerRecord);
            lock.l_pid = getpid();         
 
            if(write_lock[requestP[i].id]==0&&read_lock[requestP[i].id]==0&&fcntl(file_fd, F_SETLK, &lock) != -1){
                    requestP[i].wait_for_write=1;
                    write_lock[requestP[i].id] = true;
                    lseek(file_fd, sizeof(reg) * (id_val-1), SEEK_SET);
                    read(file_fd, &reg, sizeof(registerRecord));
                    char order[512];
                    string_prefer(reg.AZ,reg.BNT,reg.Moderna,order);
                    sprintf(buf, "Your preference order is %s",order);
                    write(requestP[i].conn_fd, buf, strlen(buf));
                    sprintf(buf,"Please input your preference order respectively(AZ,BNT,Moderna):\n");
                   write(requestP[i].conn_fd, buf, strlen(buf));
                }
                else{
                    //locked
                    sprintf(buf, "Locked.\n");
                    write(requestP[i].conn_fd, buf, strlen(buf));
                    close(requestP[i].conn_fd);
                    free_request(&requestP[i]); 
                    FD_CLR(i,&mas_set);
                }
            }
        else{
            char pre_arr[512]="\0";
            int ret = handle_read(&requestP[i]); // parse data from client to requestP[conn_fd].buf
            fprintf(stderr, "ret = %d\n", ret);
            if (ret < 0) {
                fprintf(stderr, "bad request from %s\n", requestP[i].host);
                continue;
            }
            int cur_id=requestP[i].id;
            int flag = prefer_write(cur_id,requestP[i].buf,file_fd);
            if(flag==-1)
            sprintf(buf, "[Error] Operation failed. Please try again.\n");
            else{
            char order[512];
            registerRecord reg;
            lseek(file_fd, sizeof(reg) * (cur_id-1), SEEK_SET);
            read(file_fd, &reg, sizeof(registerRecord));
            string_prefer(reg.AZ,reg.BNT,reg.Moderna,order);
            sprintf(buf, "Preference order for %d modified successed, new preference order is %s\n",requestP[i].id+902000,order);}
            write(requestP[i].conn_fd, buf, strlen(buf));

            lock.l_type = F_UNLCK;
            lock.l_start = sizeof(registerRecord) * (requestP[i].id - 1);
            lock.l_whence = SEEK_SET;
            lock.l_len = sizeof(registerRecord);
            fcntl(file_fd, F_SETLK, &lock);
            write_lock[requestP[i].id] = false;

            close(requestP[i].conn_fd);
            free_request(&requestP[i]);  
            FD_CLR(i,&mas_set);
        }  
#endif

        }}}

    }
    close(file_fd);
    free(requestP);
    return 0;
}

// ======================================================================================================
// You don't need to know how the following codes are working
#include <fcntl.h>

static void init_request(request* reqP) {
    reqP->conn_fd = -1;
    reqP->buf_len = 0;
    reqP->id = 0;
    reqP->wait_for_write = 0;
}

static void free_request(request* reqP) {
    /*if (reqP->filename != NULL) {
        free(reqP->filename);
        reqP->filename = NULL;
    }*/
    init_request(reqP);
}

static void init_server(unsigned short port) {
    struct sockaddr_in servaddr;
    int tmp;

    gethostname(svr.hostname, sizeof(svr.hostname));
    svr.port = port;

    svr.listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (svr.listen_fd < 0) ERR_EXIT("socket");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    tmp = 1;
    if (setsockopt(svr.listen_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&tmp, sizeof(tmp)) < 0) {
        ERR_EXIT("setsockopt");
    }
    if (bind(svr.listen_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        ERR_EXIT("bind");
    }
    if (listen(svr.listen_fd, 1024) < 0) {
        ERR_EXIT("listen");
    }

    // Get file descripter table size and initialize request table
    maxfd = getdtablesize();
    requestP = (request*) malloc(sizeof(request) * maxfd);
    if (requestP == NULL) {
        ERR_EXIT("out of memory allocating all requests");
    }
    for (int i = 0; i < maxfd; i++) {
        init_request(&requestP[i]);
    }
    requestP[svr.listen_fd].conn_fd = svr.listen_fd;
    strcpy(requestP[svr.listen_fd].host, svr.hostname);

    return;
}
