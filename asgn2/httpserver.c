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
#include <pthread.h>
#include<netdb.h>
#include<getopt.h>
#include<unistd.h>



#define BUFFER_SIZE 4096
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond=PTHREAD_COND_INITIALIZER;
int indicateoffset =0;
int error =0;
int process =0;
char protocal[9];
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


struct logfile {
    char *logdata;
};




//this queue structure is from my past course 
typedef struct  NodeObj
{
    ssize_t data;
    struct NodeObj* next;
    
    
} NodeObj;

typedef NodeObj* Node;

typedef struct  QueueObj
{
  Node front;
  Node back;
  int length;  
} QueueObj;

typedef QueueObj* Queue;

Queue newQueue(void){
       Queue Q=malloc(sizeof(QueueObj));
       Q->front=Q->back=NULL;
       Q->length=0;
       return(Q);


}

Node newNode(ssize_t client_sockd){

     Node N =malloc(sizeof(NodeObj));
     N->data=client_sockd;
     N->next=NULL;
     return(N);

}

void Enqueue(Queue Q, ssize_t client_sockd){

          Node N= newNode(client_sockd);
         if (Q->length==0)
         {
             Q->front=Q->back=N;
         }else
         {
             Q->back->next=N;
             Q->back=N;
         }
         Q->length++;
         


}



void freeNode(Node* pN){
    if(pN!=NULL&&*pN!=NULL){
        free(*pN);
        *pN=NULL;
    }
}



void Dequeue(Queue Q){
Node N=NULL;

 N=Q->front;
 if(Q->length>1){
     Q->front=Q->front->next;
 }else{

       Q->front=Q->back=NULL;

 }


    Q->length--;
    freeNode(&N);


}




 Queue myque;




void* workerThread(void *arugments){

       struct logfile *args=(struct logfile *) arugments;
      
       

    char *logfilename=args ->logdata;
    

   int client_sockd=0;
   struct httpObject message;
 while(true){

 pthread_mutex_lock(&mutex);
 while (myque->length==0)
 {
      pthread_cond_wait(&cond,&mutex);
 }
 

     client_sockd=myque->front->data;
     
     Dequeue(myque);
     
     pthread_mutex_unlock(&mutex);

   read_http_response(client_sockd,&message);
    


   process_request(client_sockd,&message, args);
     
    
 }

}

    int countdigit(int n){
         int count=0;
           if(n==0){
               return 1;
           }

         while(n!=0){
             n=n/10;
             ++count;
         }
         return count;      

    }


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
                         
        sscanf(buff, "%s %s %s", message->method, message->filename, protocal);
       
         
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
void process_request(ssize_t client_sockd, struct httpObject* message,struct logfile* log) {
    printf("Process  Request\n");

    

             int namelength=strlen( message->filename);
             ssize_t logdescriptor;
             char *logfile=log->logdata;
             off_t offset;
             char Buferror[1024];
            int length= 37+namelength+9;



             if (logfile!=NULL&&indicateoffset==0){
        logdescriptor=open(logfile,O_CREAT|O_TRUNC|O_WRONLY,00700);

     }
    



           if (logfile!=NULL&&indicateoffset>0){
        logdescriptor=open(logfile,O_CREAT|O_WRONLY,00700);

        }






             
        if(strcmp(message->method,"GET")!=0&&strcmp(message->filename,"healthcheck")==0){
                     error=error+1;
                           
                sprintf(message->buffer,"HTTP/1.1 403 Forbideen\r\nContent-Length: %d\r\n\r\n",0);
                if (indicateoffset==0)
                {
                      sprintf(Buferror,"FAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,403);
                }else
                {
                     sprintf(Buferror,"\nFAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,403);
                }         
                
                 if(logfile!=NULL){

                    if(indicateoffset==0){
                   pthread_mutex_lock(&mutex);
                   offset=indicateoffset;
                   indicateoffset+=length;
                   pthread_mutex_unlock(&mutex);

                   }else
                   {
                  pthread_mutex_lock(&mutex);
                   offset=indicateoffset;
                   length=length+1;
                   indicateoffset+=length;
                   pthread_mutex_unlock(&mutex);
                    
                   }


                     pwrite(logdescriptor,Buferror,length,offset);
                     send(client_sockd,message->buffer,strlen(message->buffer),0);
                     close(client_sockd);
                     close(logdescriptor);
                    

                 }else{

                    send(client_sockd,message->buffer,strlen(message->buffer),0);
                    close(client_sockd);
                 


                 } 
              

                       
         }









 if(strcmp(protocal,"HTTP/1.1")!=0){
                      
         
                error=error+1; 
                sprintf(message->buffer,"HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
                if (indicateoffset==0)
                {
                      sprintf(Buferror,"FAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,400);
                      //printf("%s",Buferror);
                }else
                {   
                     sprintf(Buferror,"\nFAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,400);
                     // printf("%s",Buferror);
                }  
                    if(logfile!=NULL){

                     pwrite(logdescriptor,Buferror,length,offset);
                     send(client_sockd,message->buffer,strlen(message->buffer),0);
                     close(client_sockd);
                     close(logdescriptor);
                     

                 }else{

                    send(client_sockd,message->buffer,strlen(message->buffer),0);
                    close(client_sockd);
                  


                 }                 

           }






             
          if(strcmp(message->method,"PUT")!=0&&strcmp(message->method,"HEAD")!=0){
           
          
           if(namelength>27){
             
                error=error+1;
                sprintf(message->buffer,"HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
                if (indicateoffset==0)
                {
                      sprintf(Buferror,"FAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,400);
                     
                }else
                {   
                     sprintf(Buferror,"\nFAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,400);
                    
                }         
                    if(logfile!=NULL){

                    if(indicateoffset==0){
                   pthread_mutex_lock(&mutex);
                   offset=indicateoffset;
                   indicateoffset+=length;
                   pthread_mutex_unlock(&mutex);

                   }else
                   {
                  pthread_mutex_lock(&mutex);
                   offset=indicateoffset;
                   length=length+1;
                   indicateoffset+=length;
                   pthread_mutex_unlock(&mutex);
                    
                   }


                     pwrite(logdescriptor,Buferror,length,offset);
                     send(client_sockd,message->buffer,strlen(message->buffer),0);
                     close(client_sockd);
                     close(logdescriptor);
                    

                 }else{

                    send(client_sockd,message->buffer,strlen(message->buffer),0);
                    close(client_sockd);
                  


                 }          

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

                     error=error+1;
               
             
                sprintf(message->buffer,"HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
                if (indicateoffset==0)
                {
                      sprintf(Buferror,"FAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,400);
                }else
                {
                     sprintf(Buferror,"\nFAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,400);
                }

                
                   if(logfile!=NULL){

                    if(indicateoffset==0){
                   pthread_mutex_lock(&mutex);
                   offset=indicateoffset;
                   indicateoffset+=length;
                   pthread_mutex_unlock(&mutex);

                   }else
                   {
                  pthread_mutex_lock(&mutex);
                   offset=indicateoffset;
                   length=length+1;
                   indicateoffset+=length;
                   pthread_mutex_unlock(&mutex);
                    
                   }


                     pwrite(logdescriptor,Buferror,length,offset);
                     send(client_sockd,message->buffer,strlen(message->buffer),0);
                     close(client_sockd);
                     close(logdescriptor);
                     

                 }else{

                    send(client_sockd,message->buffer,strlen(message->buffer),0);
                    close(client_sockd);
                  


                 }          
                   
           } 
           
          
      
    
         ssize_t file_input;
         ssize_t count=0;
         char buff1[BUFFER_SIZE+1];
        
         
if(strcmp(message->method,"PUT")==0){
             


ssize_t file_descriptor =open(message->filename, O_WRONLY|O_CREAT|O_TRUNC,00700);

                   if(file_descriptor==-1){

                          error=error+1;
                   
                   
                   if(errno==ENOENT){

        
             
                sprintf(message->buffer,"HTTP/1.1 403 Forbideen\r\nContent-Length: 0\r\n\r\n");
                if (indicateoffset==0)
                {
                      sprintf(Buferror,"FAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,403);
                }else
                {
                     sprintf(Buferror,"\nFAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,403);
                }         
                } 
                     
                
                else{
                sprintf(message->buffer,"HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
                if (indicateoffset==0)
                {
                      sprintf(Buferror,"FAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,500);
                }else
                {
                     sprintf(Buferror,"\nFAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,500);
                }                             
                }

                   if(logfile!=NULL){

                    if(indicateoffset==0){
                   pthread_mutex_lock(&mutex);
                   offset=indicateoffset;
                   indicateoffset+=length;
                   pthread_mutex_unlock(&mutex);

                   }else
                   {
                  pthread_mutex_lock(&mutex);
                   offset=indicateoffset;
                   length=length+1;
                   indicateoffset+=length;
                   pthread_mutex_unlock(&mutex);
                    
                   }


                     pwrite(logdescriptor,Buferror,length,offset);
                     send(client_sockd,message->buffer,strlen(message->buffer),0);
                     close(client_sockd);
                     close(logdescriptor);
                    

                 }else{

                    send(client_sockd,message->buffer,strlen(message->buffer),0);
                    close(client_sockd);
                  


                 }                  
                 }


                   



          
          unsigned char logbuf[1024];
          unsigned char writebuf[20];
 int counter=message->content_length;
       

          if(logfile!=NULL){

               sprintf(logbuf,"%s /%s length %d\n",message->method,message->filename,counter);

                 size_t data =strlen(logbuf);
                 pthread_mutex_lock(&mutex);
                 offset=indicateoffset;
                 indicateoffset=indicateoffset+data+(69*(counter/20))+(9+((counter%20)*3))+9;
                 pthread_mutex_unlock(&mutex);
                 pwrite(logdescriptor,logbuf,data,offset);
                 offset +=data;




          }

          int bufferby=0;
        if(logfile!=NULL){
           while(counter>0){
                  

                      file_input=read(client_sockd, writebuf,20);

            if(logfile!=NULL){

                sprintf(logbuf,"%08zd ", bufferby);
                pwrite(logdescriptor,logbuf,9,offset);
                offset +=9;


                for(int k=0; k<file_input;k++){
                        if(k<file_input-1){
                             
                            sprintf(logbuf,"%02x ",  writebuf[k]);
                            pwrite(logdescriptor,logbuf,3,offset);
                        }else
                        {
                            
                            sprintf(logbuf,"%02x\n", writebuf[k]);
                            pwrite(logdescriptor,logbuf,3,offset);
                        }
                        offset +=3;

                }
                bufferby+=20;
            }           
                     write(file_descriptor,writebuf,file_input);
                      counter=counter-20;
           }

             
                pwrite(logdescriptor, "========\n",9,offset);
                offset+=9;

                    
                   if(file_input !=-1){

                       dprintf(client_sockd,"HTTP/1.1 201 Created\r\nContent-Length: 0\r\n\r\n");                      
                        close(client_sockd);
                       
                    
                }
                    close(logdescriptor);
                    close(file_descriptor);

        }else
        {
           
                     while(counter>0){
                  

                      file_input=read(client_sockd, writebuf,sizeof(buff1)) ;

                   
                     write(file_descriptor,writebuf,file_input);
                      counter=counter-sizeof(buff1);
           }
              
                             close(file_descriptor);
                             close(client_sockd);


        }
        
        
           }
















         if(strcmp(message->method,"HEAD")==0){

            
                 struct  stat st;
                 stat(message->filename,&st);
                 int contentlength=st.st_size;
                 
               

               ssize_t file_descriptor =open(message->filename, O_RDONLY);
             
         if(file_descriptor==-1){
                    
                     error=error+1;
                 if(errno==EACCES){

              
             
                sprintf(message->buffer,"HTTP/1.1 403 Forbideen\r\nContent-Length: %d\r\n\r\n",0);
                if (indicateoffset==0)
                {
                      sprintf(Buferror,"FAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,403);
                }else
                {
                     sprintf(Buferror,"\nFAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,403);
                }         
                }
                
                else if(errno==ENOENT){

               
             
                sprintf(message->buffer,"HTTP/1.1 404 Not Found\r\nContent-Length: %d\r\n\r\n",0);
                if (indicateoffset==0)
                {
                      sprintf(Buferror,"FAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,404);
                }else
                {
                     sprintf(Buferror,"\nFAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,404);
                }         
                
                } 
                     
                else{

                sprintf(message->buffer,"HTTP/1.1 500 Internal Server Error\r\nContent-Length: %d\r\n\r\n",0);
                if (indicateoffset==0)
                {
                      sprintf(Buferror,"FAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,500);
                }else
                {
                     sprintf(Buferror,"\nFAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,500);
                }         
                

        

                 }


                         if(logfile!=NULL){

                    if(indicateoffset==0){
                   pthread_mutex_lock(&mutex);
                   offset=indicateoffset;
                   indicateoffset+=length;
                   pthread_mutex_unlock(&mutex);

                   }else
                   {
                  pthread_mutex_lock(&mutex);
                   offset=indicateoffset;
                   length=length+1;
                   indicateoffset+=length;
                   pthread_mutex_unlock(&mutex);
                    
                   }


                     pwrite(logdescriptor,Buferror,length,offset);
                     send(client_sockd,message->buffer,strlen(message->buffer),0);
                     close(client_sockd);
                     close(logdescriptor);
                    

                 }else{

                    send(client_sockd,message->buffer,strlen(message->buffer),0);
                    close(client_sockd);
                  


                 }





         }










            if(file_descriptor !=-1){
          // printf("%d",indicateoffset);

       if(logfile==NULL){
             
                sprintf(message->buffer,"HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n",contentlength);
                   send(client_sockd,message->buffer,strlen(message->buffer),0);
                        close(client_sockd);
                       
                       
        


           }else if   (logfile!=NULL){

                         if(indicateoffset==0){
                   pthread_mutex_lock(&mutex);
                   offset=indicateoffset;
                   indicateoffset+=length;
                   pthread_mutex_unlock(&mutex);

                   }else
                   {
                  pthread_mutex_lock(&mutex);
                   offset=indicateoffset;
                   length=length+1;
                   indicateoffset+=length;
                   pthread_mutex_unlock(&mutex);

                      
                   }
             
                sprintf(message->buffer,"HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n",contentlength);
                  printf("%d",indicateoffset);
                if (offset==0)
                {
                      printf("test1\n");
                      sprintf(Buferror,"%s /%s HTTP/1.1 length %d\n========\n",message->method,message->filename,contentlength);
                }else
                {
                     printf("test2\n");
                     sprintf(Buferror,"\n%s /%s HTTP/1.1 length %d\n========\n",message->method,message->filename,contentlength);
                }         
                

                pwrite(logdescriptor,Buferror,length,offset);
                   send(client_sockd,message->buffer,strlen(message->buffer),0);
                        close(client_sockd);
                        close(logdescriptor);
                        
                      

           }


             }

         }





     
    if(strcmp(message->method,"GET")==0){
              

                 struct  stat st;
                 stat(message->filename,&st);
                 int contentlength=st.st_size;

              
            if( strcmp(message->filename,"healthcheck")==0){


                      if (logfile==NULL){

                   
                sprintf(message->buffer,"HTTP/1.1 404 Not Found\r\nContent-Length: %d\r\n\r\n",0);
                   send(client_sockd,message->buffer,strlen(message->buffer),0);
                        close(client_sockd);
                    

           }else
           
           {
               
                         
                    int prolenth=countdigit(process);
                    int errlenth=countdigit(error);
                    int totallength=prolenth+errlenth+1;
                           

            char health[totallength];

                         
             sprintf(message->buffer,"HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n",totallength);
                   send(client_sockd,message->buffer,strlen(message->buffer),0);
                         


               sprintf(health,"%d\n%d",error,process);
                 printf("%s\n",health);
                   write(client_sockd,health,totallength);
                   close(client_sockd);
                  
                  
           }
           

                           if (strcmp(message->filename,"healthcheck")==0){

                  process=process+1;
                 

                     }

                     return;
                    

            }





                      
             
         if(logfile!=NULL&&logdescriptor==-1){
                     error=error+1;
                 if(errno==EACCES){

                     if(logfile==NULL){
             
                sprintf(message->buffer,"HTTP/1.1 403 Forbideen\r\nContent-Length: %d\r\n\r\n",0);
                   send(client_sockd,message->buffer,strlen(message->buffer),0);
                        close(client_sockd);
                       
                       
        


           }else if   (logfile!=NULL){

                       if(indicateoffset==0){
                   pthread_mutex_lock(&mutex);
                   offset=indicateoffset;
                   indicateoffset+=length;
                   pthread_mutex_unlock(&mutex);

                   }else
                   {
                  pthread_mutex_lock(&mutex);
                   offset=indicateoffset;
                   length=length+1;
                   indicateoffset+=length;
                   pthread_mutex_unlock(&mutex);

                      
                   }
             
                sprintf(message->buffer,"HTTP/1.1 403 Forbideen\r\nContent-Length: %d\r\n\r\n",0);
                if (offset==0)
                {
                      sprintf(Buferror,"FAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,403);
                }else
                {
                     sprintf(Buferror,"\nFAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,403);
                }         
                

                pwrite(logdescriptor,Buferror,length,offset);
                   send(client_sockd,message->buffer,strlen(message->buffer),0);
                        close(client_sockd);
                        close(logdescriptor);
                        
                      

           }
                 }
         }
              
              
    
        



               ssize_t file_descriptor =open(message->filename, O_RDONLY,S_IRWXG);

         if(file_descriptor==-1){
             
             error=error+1;
            
             
                 if(errno==EACCES){

                sprintf(message->buffer,"HTTP/1.1 403 Forbideen\r\nContent-Length: %d\r\n\r\n",0);
                if (indicateoffset==0)
                {
                      sprintf(Buferror,"FAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,400);
                }
                else
                {
                     sprintf(Buferror,"\nFAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,400);
                }         
                
         
 
                }
                 
                else if(errno==ENOENT){

             
                sprintf(message->buffer,"HTTP/1.1 404 Not Found\r\nContent-Length: %d\r\n\r\n",0);
                if (indicateoffset==0)
                {
                      sprintf(Buferror,"FAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,404);
                }else
                {
                     sprintf(Buferror,"\nFAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,404);
                }         
                }


                else
                {

             
                sprintf(message->buffer,"HTTP/1.1 500 Internal Server Error\r\nContent-Length: %d\r\n\r\n",0);
                if (indicateoffset==0)
                {
                      sprintf(Buferror,"FAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,500);
                }else
                {
                    sprintf(Buferror,"\nFAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,500);
                }         
    

                }
                 
                 if(logfile!=NULL){

                    if(indicateoffset==0){
                   pthread_mutex_lock(&mutex);
                   offset=indicateoffset;
                   indicateoffset+=length;
                   pthread_mutex_unlock(&mutex);

                   }else
                   {
                  pthread_mutex_lock(&mutex);
                   offset=indicateoffset;
                   length=length+1;
                   indicateoffset+=length;
                   pthread_mutex_unlock(&mutex);
                    
                   }


                     pwrite(logdescriptor,Buferror,length,offset);
                     send(client_sockd,message->buffer,strlen(message->buffer),0);
                     close(client_sockd);
                     close(logdescriptor);
                    

                 }else{

                     send(client_sockd,message->buffer,strlen(message->buffer),0);
                    close(client_sockd);
                  


                 }




                 
         }
          




                if(file_descriptor!=-1){
                sprintf(message->buffer,"HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n",contentlength);
                   send(client_sockd,message->buffer,strlen(message->buffer),0);
                      
                       
                }




         unsigned char logbuf[1024];
          unsigned char writebuf[20];
          int counter=contentlength;
         


          if(logfile!=NULL){

               sprintf(logbuf,"%s /%s length %d\n",message->method,message->filename,contentlength);

                 size_t data =strlen(logbuf);
                 pthread_mutex_lock(&mutex);
                 offset=indicateoffset;
                 indicateoffset=indicateoffset+data+(69*(contentlength/20))+(9+((contentlength%20)*3))+9;
                 pthread_mutex_unlock(&mutex);
                 pwrite(logdescriptor,logbuf,data,offset);
                 offset +=data;

          }





if(logfile!=NULL){
          
            int bufferby=0;              
            file_input=read(file_descriptor, writebuf, 20) ;
           
            while(file_input>0){
   
              if(logfile!=NULL){

                sprintf(logbuf,"%08zd ", bufferby);
                pwrite(logdescriptor,logbuf,9,offset);
                offset +=9;


                for(int k=0; k<file_input;k++){
                        if(k<file_input-1){
                            sprintf(logbuf,"%02x ",writebuf[k]);
                            pwrite(logdescriptor,logbuf,3,offset);
                        }else
                        {      
                            
                            sprintf(logbuf,"%02x\n",writebuf[k]);
                            pwrite(logdescriptor,logbuf,3,offset);
                        }
                        offset +=3;

                }
                bufferby+=20;

            }
            
                     write(client_sockd, writebuf,file_input);
                      file_input=read(file_descriptor, writebuf,20) ;
  
           }


             pwrite(logdescriptor, "========\n",9,offset);
             offset+=9;
               
             
             close(file_descriptor);
             close(client_sockd);
             close(logdescriptor);


}else
{
   file_input=read(file_descriptor,buff1,sizeof(buff1));
   while (file_input>0)
   {
       write(client_sockd,buff1,file_input);
       file_input=read(file_descriptor,buff1,sizeof(buff1));
   }
   
                 close(file_descriptor);
             close(client_sockd);
}








}






          if(strcmp(message->method,"GET")!=0&& (strcmp(message->method,"PUT")!=0)&&(strcmp(message->method,"HEAD")!=0)&&logfile !=NULL)
          {
                     error=error+1;
                  if(logfile!=NULL){

                          if(indicateoffset==0){
                   pthread_mutex_lock(&mutex);
                   offset=indicateoffset;
                   indicateoffset+=length;
                   pthread_mutex_unlock(&mutex);

                   }else
                   {
                  pthread_mutex_lock(&mutex);
                   offset=indicateoffset;
                   length=length+1;
                   indicateoffset+=length;
                   pthread_mutex_unlock(&mutex);

                      
                   }
             
                sprintf(message->buffer,"HTTP/1.1 400 Bad Request\r\nContent-Length: %d\r\n\r\n",0);
                if (offset==0)
                {
                      sprintf(Buferror,"FAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,400);
                }else
                {
                     sprintf(Buferror,"\nFAIL: %s /%s HTTP/1.1 --- response %d\n========\n",message->method,message->filename,400);
                }         
                

                pwrite(logdescriptor,Buferror,length,offset);
                   send(client_sockd,message->buffer,strlen(message->buffer),0);
                        close(client_sockd);
                        close(logdescriptor);
                       

           }



          }

          process=process+1;

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

      int opt, N;
      int Nthread=0;
      char *logfile2;
      while((opt=getopt(argc,argv, "N:l: "))!=-1){
                  switch (opt)
                  {
                  case 'N':
                  Nthread=1;
                  N=atoi(optarg);
                      break;
                  
                  case 'l':
                  logfile2=optarg;
                  break;
                  }

      }

  
      if(Nthread==0){
          N=4;
      }
 



          
    char* port = argv[optind];
   // printf("%d",argv[optind]);
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
    ret = listen(server_sockd,128); // 5 should be enough, if not use SOMAXCONN

    if (ret < 0) {
        return EXIT_FAILURE;
    }

    /*
        Connecting with a client
    */
    struct sockaddr client_addr;
    socklen_t client_addrlen;
      
    struct httpObject message;
    struct logfile *logg =malloc(sizeof(struct logfile));
    
    
    logg->logdata=logfile2;
  
        

    int logdescrptro;

                if (logfile2!=NULL&&indicateoffset==0){
        logdescrptro= open(logfile2,O_CREAT|O_TRUNC|O_WRONLY,00700);

                }
        

            if (access(logfile2,R_OK|W_OK)!=-1)
            {
                logdescrptro= open(logfile2,O_CREAT|O_WRONLY,00700);

            }
            
            close(logdescrptro);








      
        
     myque=newQueue();

      pthread_t dispathThread[N];
 for(int i=0;i<N;i++){

    int error =pthread_create(&dispathThread[i],NULL,&workerThread,(void *)logg);
 }      
   

         pthread_mutex_init(&mutex,NULL);




    while (true) {

       
        
        printf("+server is waiting...\n");
          
        /*
         * 1. Accept Connection
         * 
         * 
         * 
         */
        int client_sockd = accept(server_sockd, &client_addr, &client_addrlen);
        // Remember errors happen

      


            pthread_mutex_lock(&mutex);
           
           Enqueue(myque,client_sockd);

           pthread_mutex_unlock(&mutex);

           pthread_cond_signal(&cond);
        


         printf("Response Sent\n");
      // close(client_sockd);
    }

    return EXIT_SUCCESS;
}