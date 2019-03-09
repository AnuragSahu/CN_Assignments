// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#define PORT 8080
#define BUFFER_SIZE 10000

char* read_command()
{
	ssize_t bufferS = 0;
	char* command = NULL;
	if(getline(&command,&bufferS,stdin)==-1){
		printf("Cannot Read pls enter again.\n");//Cannot read the line
		return read_command();
	}
	
	return command;
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
        printf("%s\n",argumentList[1]);
        return argumentList[1];
    }
}

void writing_file_1(int sock,char msg[1024])
{
	FILE *fp;
    char * filename = splitcommands(msg);
	//fp = fopen("aFile.txt","w+");   // opening the file
    fp = fopen(filename,"w+");
	char buf[BUFFER_SIZE];
	read(sock,buf,BUFFER_SIZE);        // Reading the socket file
	fwrite(buf,sizeof(char),strlen(buf),fp);      // writing into the file
	printf("%s\n",buf);
	printf("The  file was received successfully\n");
	for(int i=0;i<BUFFER_SIZE;i++){ buf[i] = 0;}  // cleaning the buffer.
    fclose(fp);
	
}

void writing_file(int sock)
{
	FILE *fp;
	fp = fopen("aFile.txt","w+");
    int line = 0;
    int lines;
    char buffer[1024];
	read(sock, buffer, 10);
	lines = (int)strtol(buffer, (char**)NULL,10);
	printf("Number of Lines Coping: %d\n",lines);
    while(line!=lines)
    {
    	line++;
    	read(sock , buffer , 512);
    	printf("%s",buffer);
	    fwrite(buffer,sizeof(char),strlen(buffer),fp);
	    for(int i=0;i<512;i++){ buffer[i] = 0;}
	}
	fclose(fp);
    printf("The file was received successfully\n");

}


int main(int argc, char const *argv[])
{
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char *msg;
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr)); // to make sure the struct is empty. Essentially sets sin_zero as 0
                                                // which is meant to be, and rest is defined below

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Converts an IP address in numbers-and-dots notation into either a
    // struct in_addr or a struct in6_addr depending on whether you specify AF_INET or AF_INET6.
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)  // connect to the server address
    {
        printf("\nConnection Failed\n");
        return -1;
    }
    printf("Connection Established\n");
    //send(sock , hello , strlen(hello) , 0 );  // send the message.
    //printf("Hello message sent\n");
    //valread = read( sock , buffer, 1024);  // receive message back from server, into the buffer
    //printf("%s\n",buffer );
    while(1){
    	printf(">> ");
    	msg = read_command();  // Scanning the command
    	send(sock,msg,strlen(msg),0);  //  Socket sending the command
    	if(strncmp(msg,"send",4)==0 ){  // if message is regarding downloading
    		read(sock,buffer,1024);
    		if(strcmp(buffer,"File Found")==0){
    			//writing_file(sock);
    			writing_file_1(sock,msg);   // function to save the file
    		}else{    printf("File Not Found\n"); }
    	}
    	if(strncmp(msg,"listall",7)==0){  // if the command is listall
   			valread = read(sock,buffer,1024);
    		printf("%s\n",buffer);
    	}
    	for(int i=0;i<1024;i++){ buffer[i] = 0;}  // cleaning the buffer
    }
    close(sock);
    return 0;
}
