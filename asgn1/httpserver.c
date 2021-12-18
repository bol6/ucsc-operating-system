#include <sys/socket.h>
#include <sys/stat.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <fcntl.h>
#include <unistd.h> // write
#include <string.h> // memset
#include <stdlib.h> // atoi
#include <stdbool.h> // true, false
#include <ctype.h>
#include <errno.h>

#define BUFFER_SIZE 4096

//AUTHOR BLIU62 BO LIU
struct httpObject {
    /*
        Create some object 'struct' to keep track of all
        the components related to a HTTP message
        NOTE: There may be more member variables you would want to add
    */
    char method[5];         // PUT, HEAD, GET
    char filename[28];      // what is the file we are worried about
    char httpversion[9];    // HTTP/1.1
    ssize_t content_length; // example: 13
    int status_code;
    uint8_t buffer[BUFFER_SIZE];
};

/*
    \brief 1. Want to read in the HTTP message/ data coming in from socket
    \param client_sockd - socket file descriptor
    \param message - object we want to 'fill in' as we read in the HTTP message
*/
void read_http_response(ssize_t client_sockd, struct httpObject* message) {
   // printf("This function will take care of reading message\n");




      
     
       uint8_t buff[BUFFER_SIZE + 1];
        ssize_t bytes = recv(client_sockd, buff, BUFFER_SIZE, 0);
        buff[bytes] = 0; // null terminate
        
    
        char *token;
        char *token1;

          
       token=strtok(buff," ");

              
        strcpy(message->method,token);


         token=strtok(NULL,"  ");
         
          token=token +1;
          
          strcpy(message->filename,token);
     
        
          token=strtok(NULL,"r");
 
     
       token[strlen(token)-1]=0;
      

         if(strcmp(message->method,"PUT")==0){
          token=strtok(NULL," ");
        
            token=strtok(NULL," ");
           
            
             
            token=strtok(NULL," ");
            token=strtok(NULL," ");
             token=strtok(token,"E");
  
          message->content_length=atoi(token);
       

         }
        


    return;
}

/*
    \brief 2. Want to process the message we just recieved
*/
void process_request(ssize_t server_sockd,ssize_t client_sockd, struct httpObject* message) {
    printf("Process  Request\n");
                  
             int namelength=strlen( message->filename);
             
          if(strcmp(message->method,"PUT")!=0&&strcmp(message->method,"HEAD")!=0){
           
           
                        
           

           if(namelength>27){
             
                sprintf(message->buffer,"HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
                   send(client_sockd,message->buffer,strlen(message->buffer),0);
                        close(client_sockd);
           
           
           }
             }



           for (int i = 0; i < namelength; i++)
           {
              if(message->filename[i]>='a'&&message->filename[i]<='z'){
                  
                  
                  continue;

              }

              if(message->filename[i]>='A'&&message->filename[i]<='Z'){
                  
                  continue;
              }
               if(message->filename[i]>='0'&&message->filename[i]<='9'){
                 
                  continue;
              }

               if(message->filename[i]=='-'||message->filename[i]=='_'){
                  
                  continue;
              }

                     
                sprintf(message->buffer,"HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
                send(client_sockd,message->buffer,strlen(message->buffer),0);
                        close(client_sockd);
              
    
           } 
           
           
      

      
         ssize_t file_input;
         ssize_t count=0;
         char buff1[BUFFER_SIZE+1];
         //int store =st.st_size;
if(strcmp(message->method,"PUT")==0){


ssize_t file_descriptor =open(message->filename, O_WRONLY|O_CREAT|O_TRUNC,00700);

                   if(file_descriptor==-1){


if(errno==ENOENT){
                      sprintf(message->buffer,"HTTP/1.1 403 Forbideen\r\nContent-Length: 0\r\n\r\n");
                      send(client_sockd,message->buffer,strlen(message->buffer),0);
                     close(client_sockd);
                  } 
                     
                     else{
                   sprintf(message->buffer,"HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
                   send(client_sockd,message->buffer,strlen(message->buffer),0);
                     close(client_sockd);
                    

                 }


                   }



                 
           
           int counter=message->content_length;
        

           while(counter>0){
                  

                      file_input=read(client_sockd, buff1,1);
                   
                                             
                     write(file_descriptor, buff1,file_input);
                    
                  
                      counter=counter-1;
                       

           }
                
                    
                   if(file_input !=-1){

                       dprintf(client_sockd,"HTTP/1.1 201 Created\r\nContent-Length: 0\r\n\r\n");                      
                        close(client_sockd);
                       
                    
                }

                    close(file_descriptor);


}



         if(strcmp(message->method,"HEAD")==0){

            
      struct stat st;

              stat(message->filename,&st);
              int length=st.st_size;
               

               ssize_t file_descriptor =open(message->filename, O_RDONLY);
             
         if(file_descriptor==-1){
                 if(errno==EACCES){

                     sprintf(message->buffer,"HTTP/1.1 403 Forbideen\r\nContent-Length: %d\r\n\r\n",length);
                     send(client_sockd,message->buffer,strlen(message->buffer),0);
                     close(client_sockd);
                    
                 }
                  else if(errno==ENOENT){
                      sprintf(message->buffer,"HTTP/1.1 404 Not Found\r\nContent-Length: %d\r\n\r\n",length);
                      send(client_sockd,message->buffer,strlen(message->buffer),0);
                     close(client_sockd);
                  } 
                     
                     else{
                   sprintf(message->buffer,"HTTP/1.1 500 Internal Server Error\r\nContent-Length: %d\r\n\r\n",length);
                   send(client_sockd,message->buffer,strlen(message->buffer),0);
                     close(client_sockd);
                    

                 }

         }

            if(file_descriptor !=-1){
               sprintf(message->buffer,"HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n",length);
                   send(client_sockd,message->buffer,strlen(message->buffer),0);
                   close(client_sockd);
             }



           close(file_descriptor);



         }




     
    if(strcmp(message->method,"GET")==0){
              
               struct stat st;

              stat(message->filename,&st);
              int length=st.st_size;
               

               ssize_t file_descriptor =open(message->filename, O_RDONLY,S_IRWXG);

         if(file_descriptor==-1){
                 if(errno==EACCES){

                     sprintf(message->buffer,"HTTP/1.1 403 Forbideen\r\nContent-Length: %d\r\n\r\n",length);
                     send(client_sockd,message->buffer,strlen(message->buffer),0);
                     close(client_sockd);
                    
                 }else if(errno==ENOENT){
                   sprintf(message->buffer,"HTTP/1.1 404 Not Found\r\nContent-Length: %d\r\n\r\n",length);
                   send(client_sockd,message->buffer,strlen(message->buffer),0);
                     close(client_sockd);
                    

                 }
                 else
                 {
                    sprintf(message->buffer,"HTTP/1.1 500 Internal Server Error\r\nContent-Length: %d\r\n\r\n",length);
                   send(client_sockd,message->buffer,strlen(message->buffer),0);
                     close(client_sockd);
                 }
                 



         }
             if(file_descriptor !=-1){
                 sprintf(message->buffer,"HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n",length);
                 send(client_sockd,message->buffer,strlen(message->buffer),0);
             }

             file_input=read(file_descriptor, buff1, sizeof(buff1)) ;
             while(file_input >0){
                    write(client_sockd,buff1,file_input);                    
                   file_input=read(file_descriptor, buff1,sizeof(buff1)) ;


             }
  
               close(file_descriptor);
               close(client_sockd);

    
}

     






    return;
}

/*
    \brief 3. Construct some response based on the HTTP request you recieved
*/
void construct_http_response() {
    printf("Constructing Response\n");

    return;
}




int main(int argc, char** argv) {
    /*
        Create sockaddr_in with server information
    */
    char* port = argv[1];
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port));
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    socklen_t addrlen = sizeof(server_addr);

    /*
        Create server socket
    */
    int server_sockd = socket(AF_INET, SOCK_STREAM, 0);

    // Need to check if server_sockd < 0, meaning an error
    if (server_sockd < 0) {
        perror("socket");
    }

    /*
        Configure server socket
    */
    int enable = 1;

    /*
        This allows you to avoid: 'Bind: Address Already in Use' error
    */
    int ret = setsockopt(server_sockd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

    /*
        Bind server address to socket that is open
    */
    ret = bind(server_sockd, (struct sockaddr *) &server_addr, addrlen);

    /*
        Listen for incoming connections
    */
    ret = listen(server_sockd, 5); // 5 should be enough, if not use SOMAXCONN

    if (ret < 0) {
        return EXIT_FAILURE;
    }

    /*
        Connecting with a client
    */
    struct sockaddr client_addr;
    socklen_t client_addrlen;

    struct httpObject message;

    while (true) {
        printf("[+] server is waiting...\n");

        /*
         * 1. Accept Connection
         */
        int client_sockd = accept(server_sockd, &client_addr, &client_addrlen);
        // Remember errors happen

        /*
         * 2. Read HTTP Message
         */
        read_http_response(client_sockd, &message);

        /*
         * 3. Process Request
         */
        process_request(server_sockd, client_sockd, &message);

        /*
         * 4. Construct Response
         */
        construct_http_response();

        /*
         * 5. Send Response
         */
        printf("Response Sent\n");

        
       close(client_sockd);
    }

    return EXIT_SUCCESS;
}

