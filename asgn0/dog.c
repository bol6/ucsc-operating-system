
#include <unistd.h>
#include<stdio.h> 
#include <fcntl.h>
#include <err.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <err.h>
#define MAX_LEN 32768


      int main(int argc, char * argv[]){

         char buffer[MAX_LEN];
         int fd;
         int count;
         int n;

        if (argc==1){
         n=read(0,buffer,sizeof(buffer));
            while (n>0)
            {
               write(1,buffer,n);
               n=read(0,buffer,sizeof(buffer));
            }
         }

        for (int i = argc-1; i >=1; i--){
         if(strcmp(argv[i],"-")!=0){
            fd= open(argv[i],O_RDONLY);
            count=read(fd, buffer, sizeof(buffer));

             if(fd==-1){
                perror("Error");
                exit(EXIT_FAILURE);
             }





            
            while(count>0){
              write(1,buffer,count);
              count=read(fd, buffer, sizeof(buffer));

         }

        }else{

            n=read(0,buffer,sizeof(buffer));
            while (n>0){
             write(1,buffer,n);
             n=read(0,buffer,sizeof(buffer));
           }
           }

        close(fd);
      }
   return(0);
 }
