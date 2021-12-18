# How to run code 

  - enter the make Makefile on the command line, it will generate an executable httpserver.o file 
  - enter the ./httpserver8080 -N 4 -l log_file , we run the server,and server start to listening to the client with four worker thread and logfile 
  - enter the .curl -T client_input.txt http://localhost:8080/server_file to run PUT instruction
  - enter the .curl http://localhost:8080/server_file to run GET instruction
  - enter the .curl -I http://localhost:8080/server_file to run HEAD instruction


## Potential issue

  - the program takes a long time to process a large source text file and write it to the logfile  


## Submission
  - README.md
  - DESIGN.pdf
  - Makefile
  - httpserver.c
  - WRITEUP.pdf
 

## Author 
  - bliu62@ucsc.edu
  - cruzid bliu62
