#define _GNU_SOURCE
#include <stdio.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <dirent.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <pthread.h>
#include <ctype.h>
#include <stdbool.h>
#include <regex.h>
#include <time.h>
#include <wait.h>
#include <fcntl.h>
#include <limits.h>
#include <syslog.h>
#include <errno.h>

#define PORT 8080
#define buffSize 256

typedef struct user{
	char username[buffSize],password[buffSize],input[buffSize];
	int sock,isRoot;
} user;
user* client[10];

void* inRoutine(void *arg){
	int i = *(int*) arg-1;
	char buffer[buffSize];
	while (1){
		recv(client[i]->sock,buffer,buffSize,0);
		strcpy(client[i]->input, buffer);
	}
}

void* outRoutine(void *arg){
	int i = *(int*) arg-1;
	char buffer[256];
	char buffer_name[256];
	char path[256];
	while(1){
		//DDL DML below
		if(strlen(client[i]->input)!= 0){
			send(client[i]->sock,client[i]->input,buffSize,0);
			send(client[i]->sock,"\n",buffSize,0);
			memset(client[i]->input,0,256);
		}
	}
}


void killer(int pid);

int main(int argc,char* argv[])
{

	char s[PATH_MAX];
	getcwd(s,sizeof(s));
	int indexClient = 0;
	
	pthread_t clientThread[10][2];
	
	pid_t pid,sid;
	pid = fork();
	
	if(pid <0){
		exit(EXIT_FAILURE);
	}
	if(pid > 0){
		exit(EXIT_SUCCESS);
	}
	umask(0);
	sid = setsid();
	if(sid < 0){
		exit(EXIT_FAILURE);
	}
	if((chdir(s))<0){
		exit(EXIT_FAILURE);
	}
	
	
//	close(STDIN_FILENO);
//	close(STDOUT_FILENO);
//	close(STDERR_FILENO);
	
	killer((int)getpid());
	while(1){
		int opt = 1;
		int server_fd, valread;
		struct sockaddr_in address;
		int addrlen = sizeof(address);

		  
		if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		    perror("socket failed");
		    exit(EXIT_FAILURE);
		}
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		    perror("setsockopt");
		    exit(EXIT_FAILURE);
		}
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons( PORT );
		  
		if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
		    perror("bind failed");
		    exit(EXIT_FAILURE);
		}

		if (listen(server_fd, 10) < 0) {
		    perror("listen");
		    exit(EXIT_FAILURE);
		}
		for(indexClient=0;indexClient<10;indexClient++){
			client[indexClient] = (user*)malloc(sizeof(user));
			if ((client[indexClient]->sock = accept(server_fd,(struct sockaddr*)&address,(socklen_t*)&addrlen))<0){
				perror("accept");
		    	exit(EXIT_FAILURE);
			}
			pthread_create(&clientThread[indexClient][0],NULL,inRoutine,(void*)&indexClient);
			pthread_create(&clientThread[indexClient][1],NULL,outRoutine,(void*)&indexClient);
		}
		for(indexClient=0;indexClient<10;indexClient++){

			pthread_join(clientThread[indexClient][0],NULL);
			pthread_join(clientThread[indexClient][1],NULL);
		}
	}
	//return 0;
}


//mkae killer for easy termination
void killer(int pid){
	FILE *tex;
	char* script;
	tex = fopen("Killer.sh", "w");
	asprintf(&script,"\#!/bin/bash\nkill -9 %d\nrm -- \"$0\"",pid);
	fputs(script,tex);
	fclose(tex);
}

