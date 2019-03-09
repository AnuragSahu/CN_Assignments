#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/select.h>
#include <dirent.h>

#define PORT 8080
#define BUFFER_SIZE 10000


char* give_me_files_list(){
    DIR *dir;
    struct dirent *dp;
    char * file_name = " ";
    dir = opendir(".");
    while ((dp=readdir(dir)) != NULL) {
        if ( !strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..") )
        {
            // filter out the . and ..
        } 
        else {
            char * new_str ;
            if((new_str = malloc(strlen(file_name)+strlen(dp->d_name)+4)) != NULL){
                new_str[0] = '\0';   // ensures the memory is an empty string
                strcat(new_str,file_name);
                strcat(new_str,"\n");
                strcat(new_str,dp->d_name);
            }
            file_name = new_str;
        }
    }
    closedir(dir);
    return file_name;
}

char* splitcommands(char str[1024])
{
    char *dimlem = " \n";
    char *dimlem1 = ";";
    char* anotherPointer;
    char* anotherPointer1;
    int looper1=0;
    char* token;
    char* token1;
    token1 = strtok_r(str,dimlem1,&anotherPointer1);
    while(token1!=NULL)
    {
        char* argumentList[50];
        int looper=0;
        token = strtok_r(token1,dimlem,&anotherPointer);
        while(token!=NULL)
        {
            argumentList[looper++]=token;
            token = strtok_r(NULL,dimlem,&anotherPointer);
        }       
        token1 = strtok_r(NULL,dimlem1,&anotherPointer1);
        //executeInbuild(argumentList,looper);
        if(looper >= 3){
            return "Multiple Files Requests not entertained.\n";
        }
        else{
            return argumentList[1];
        }
        //printf("%s\n",argumentList[1]);
    }
}

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;  // sockaddr_in - references elements of the socket address. "in" for internet
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";
    char *fileslist;
    fd_set clientfds;
    int fd_max, sd;
    int clients[10] = {0}; // do all clients zero

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)  // creates socket, SOCK_STREAM is for TCP. SOCK_DGRAM for UDP
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // This is to lose the pesky "Address already in use" error message
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt))) // SOL_SOCKET is the socket layer itself
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;  // Address family. For IPv6, it's AF_INET6. 29 others exist like AF_UNIX etc.
    address.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any IP address - listens from all interfaces.
    address.sin_port = htons( PORT );    // Server port to open. Htons converts to Big Endian - Left to Right. RTL is Little Endian

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Port bind is done. You want to wait for incoming connections and handle them in some way.
    // The process is two step: first you listen(), then you accept()
    if (listen(server_fd, 3) < 0) // 3 is the maximum size of queue - connections you haven't accepted
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    while(1){

        FD_ZERO(&clientfds);                // initializing the fds
        FD_SET(server_fd, &clientfds);      // Setting the FDS
        fd_max = server_fd;                 // max setting
        for(int i=0;i<10;i++){
            sd = clients[i];                // giving the client numbers
            if(sd > 0)
                FD_SET(sd,&clientfds);      // setting the sd with client 
            if(sd>fd_max)
                fd_max = sd;
        }

        if(select(fd_max+1,&clientfds,NULL, NULL, NULL) == -1){
            perror("Error");
            exit(0);
        }

        if(FD_ISSET(server_fd,&clientfds)){
            // returns a brand new socket file descriptor to use for this single accepted connection. Once done, use send and recv
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address,   // giving new socket addresses
                               (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            //printf("Connection Established\n");
            for(int i = 0;i<10 ;i++){
                if(clients[i] == 0) {
                    clients[i] = new_socket;        // giving the client added new socket address
                    printf("New Client Added\n");
                    break;
                }
            }
        }

        for(int i=0;i<10;i++){
            sd = clients[i];
            if(FD_ISSET(sd,&clientfds))             // checking if the socket is there
            {
                valread = read( sd , buffer, 1024);  // read infromation received into the buffer 
                if(valread > 0)                      // cheching if the data is comming
                {
                    
                    if(strcmp(buffer,"listall\n")==0){
                        fileslist = give_me_files_list();    // function that return the files present int the current directory.
                        send(sd , fileslist , strlen(fileslist) , 0 );  
                        printf("Listing the files to Client\n");  // message on the server interface.
                    }
                    else if(strncmp(buffer,"send",4)==0){                   /// if clinent asks for downloading
                        printf("Client is requesting downloads\n");
                        char* send_filename;
                        send_filename = splitcommands(buffer);
                        if(strcmp(send_filename,"Multiple Files Requests not entertained.\n")==0){
                            printf("Multiple Files Requested, sending error Message\n");  // Show errir that the multiple files cannot be sent at ones.
                        }else{
                            FILE *fp;
                            
                            fp = fopen(send_filename,"r");
                            if(fp==NULL){
                                printf("File Not Found\n");               // message if file is not found
                                send(sd, "File Not Found",strlen("File Not Found"), 0);
                            }else
                            {
                                send(sd, "File Found",strlen("File Found"),0);
                                printf("sending file : %s\n",send_filename);   // sending files 
                                int lines = 0;
                                size_t c;
                                size_t len = 0;
                                char * line = NULL;
                                char bur[BUFFER_SIZE];
                                fread(bur, BUFFER_SIZE,sizeof(char), fp);
                                printf("%s\n",bur);
                                send(sd,bur,BUFFER_SIZE,0);
                                for(int i=0;i<BUFFER_SIZE;i++){ bur[i] = 0;}  // cleaning the buffer
                                //while(c = getline(&line, &len, fp)!= -1){
                                //    lines++;
                                //}
                                //char lin[10];
                                //sprintf(lin, "%d", lines);
                                //send(new_socket,lin,10,0);
                                //printf("lines : %d\n",lines);
                                //fseek(fp,0,SEEK_SET);
                                ////rewind(fp);
                                //while(c = getline(&line, &len, fp)!= -1)
                                //{
                                //  send(new_socket,line,512,0);
                                //  printf("%s",line);
                                //}
                                    printf("The file was sent successfully\n");
                            }
                        }
                    }
                    for(int i=0;i<1024;i++){ buffer[i] = 0;}
                }
                else
                {
                    close(sd);
                    clients[i] = 0;
                }
            }
        }
    }
    return 0;
}
