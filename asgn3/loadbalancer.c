#include<err.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/time.h>
#include<stdbool.h>
#include<getopt.h>
#include<pthread.h>

#define BUFFER_SIZE 4096

typedef struct thread_data{
 
    uint16_t connectport;
    uint16_t listenport;

}thread_data;
















/*
 * client_connect takes a port number and establishes a connection as a client.
 * connectport: port number of server to connect to
 * returns: valid socket if successful, -1 otherwise
 */
int client_connect(uint16_t connectport) {
    int connfd;
    struct sockaddr_in servaddr;

    connfd=socket(AF_INET,SOCK_STREAM,0);
    if (connfd < 0)
        return -1;
    memset(&servaddr, 0, sizeof servaddr);

    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(connectport);

    /* For this assignment the IP address can be fixed */
    inet_pton(AF_INET,"127.0.0.1",&(servaddr.sin_addr));
    //int num=connect(connfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
    //printf("num:%d\n",num);
    if(connect(connfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0)
        return -1;
  
    return connfd;
    
}

/*
 * server_listen takes a port number and creates a socket to listen on 
 * that port.
 * port: the port number to receive connections
 * returns: valid socket if successful, -1 otherwise
 */
int server_listen(int port) {
    int listenfd;
    int enable = 1;
    struct sockaddr_in servaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
        return -1;
    memset(&servaddr, 0, sizeof servaddr);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0)
        return -1;
    if (bind(listenfd, (struct sockaddr*) &servaddr, sizeof servaddr) < 0)
        return -1;
    if (listen(listenfd, 500) < 0)
        return -1;
    return listenfd;
}

/*
 * bridge_connections send up to 100 bytes from fromfd to tofd
 * fromfd, tofd: valid sockets
 * returns: number of bytes sent, 0 if connection closed, -1 on error
 */
int bridge_connections(int fromfd, int tofd) {
    char recvline[BUFFER_SIZE];

    int n;
    n = recv(fromfd, recvline, 100, 0);
    if (n < 0) {
        printf("connection error receiving\n");
        return -1;
    } else if (n == 0) {
        printf("receiving connection ended\n");
        return 0;
    }
    recvline[n] = '\0';
   
    n = send(tofd, recvline, n, 0);
  
    if (n < 0) {
        printf("connection error sending\n");
        return -1;
    } else if (n == 0) {
        printf("sending connection ended\n");
        return 0;
    }
    return n;
}

/*
 * bridge_loop forwards all messages between both sockets until the connection
 * is interrupted. It also prints a message if both channels are idle.
 * sockfd1, sockfd2: valid sockets
 */
void bridge_loop(int sockfd1, int sockfd2) {
    //sockfd1 = client
    //sockfd2 = server
    fd_set set;
    struct timeval timeout;

    int fromfd, tofd;
    while(1) {
        // set for select usage must be initialized before each select call
        // set manages which file descriptors are being watched
        FD_ZERO (&set);
        FD_SET (sockfd1, &set);
        FD_SET (sockfd2, &set);

        // same for timeout
        // max time waiting, 5 seconds, 0 microseconds
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        // select return the number of file descriptors ready for reading in set
        switch (select(FD_SETSIZE, &set, NULL, NULL, &timeout)) {
            case -1:
                printf("error during select, exiting\n");
                return;
            case 0:
                printf("both channels are idle, waiting again\n");
                continue;
            default:
                if (FD_ISSET(sockfd1, &set)) {
                    //printf("1\n");
                    fromfd = sockfd1;
                    tofd = sockfd2;
                } else if (FD_ISSET(sockfd2, &set)) {
                    //printf("2\n");
                    fromfd = sockfd2;
                    tofd = sockfd1;
                } else {
                    printf("this should be unreachable\n");
                    return;
                }
        }
        if (bridge_connections(fromfd, tofd) <= 0)
            return;
    }
}

int check_health(int sockfd){



   uint8_t buffer[4096];
    int status_code;
    int length;
    int error;
    int entries;

    int num_send = dprintf(sockfd, "GET /healthcheck HTTP/1.1\r\n\r\n");
    
    if(num_send<0){
       
        return -1;
    }

    read(sockfd, buffer, sizeof(buffer));
   

    sscanf(buffer, "HTTP/1.1 %d OK\r\nContent-Length: %d\r\n%d\n%d", &status_code, &length, &error, &entries);

    if(status_code != 200){
        return -1;
    }
    
    return entries;
}





int main(int argc,char **argv) {
    int connfd, listenfd, acceptfd;
    uint16_t connectport, listenport;
   
   struct  thread_data data;
  



  

    if (argc < 3) {
        printf("missing arguments: usage %s port_to_connect port_to_listen", argv[0]);
        return 1;
    }

     int portarray[100];
     int Nnumber=0;
     int Rnumber=0;
     int index=0;
     int i=1;


        while (i<argc){

        if(strcmp(argv[i],"-N")==0){

            Nnumber=atoi(argv[i+1]);
             i=i+1;
        }else if 
        
        
         (strcmp(argv[i],"-R")==0){

            Rnumber=atoi(argv[i+1]);
             i=i+1;
        
        }else if((strcmp(argv[i],"-R")!=0)&&(strcmp(argv[i],"-N")!=0)&&index==0){
                 
                 connectport = atoi(argv[i]);
                 index++;

         }else{
           
           portarray[index-1]=atoi(argv[i]);
            index++;


         }
         
        
         i=i+1;
        
       
        }
       

      index=index-1;
     
   
   

    if (Nnumber==0)
    {
       Nnumber=4;
    }
    
    if (Rnumber==0)
    {
       Rnumber=5;
    }





    
    
    data.listenport = connectport;
    data.connectport = portarray[0];
   


   int best=0;
   int bestport;
   int clinent;
   int indexport=0;
    while(true){




      


        
       
        for(int ii=0; i<index;ii++){
            
            
           clinent=client_connect(portarray[i]);
           if(clinent<0){
            

           continue;

           }else{

               if((check_health(clinent))<0){


                                    continue;

               }else{
                 if((check_health(clinent)<=best))

                     best=check_health(clinent);
                     indexport=ii;
                      data.connectport = portarray[ii];

            }
        }
        }







        
        if ((listenfd = server_listen(data.listenport)) < 0)
            err(1, "failed listening");
           
       
        if ((acceptfd = accept(listenfd, NULL, NULL)) < 0)
            err(1, "failed accepting");
            

        
        if ((connfd = client_connect(data.connectport)) < 0){
            char buff_error[BUFFER_SIZE];
            sprintf(buff_error,"HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
            send(acceptfd, buff_error, strlen(buff_error),0);
            if(indexport<index-1){
           data.connectport=portarray[indexport+1];

            }else{
                indexport=0;
data.connectport=portarray[indexport];


            }
            close(listenfd);
            continue;
        }

        
        bridge_loop(acceptfd, connfd);
        
        
        
        close(listenfd);
    }
}