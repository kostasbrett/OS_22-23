#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

#define STREQUAL(x, y) (strncmp((x), (y), strlen(y)) == 0)

int main(int argc, char **argv) {

	if (argc != 2){
	fprintf(stderr, "Usage: ./a.out filename\n");
	return 1;
	}

	if (STREQUAL(argv[1], "--help")){
	printf("Usage: ./a.out filename\n");
	return 0;
	}
								    
	if (!(access(argv[1], F_OK))){
	fprintf(stderr, "Error: output.txt already exists\n");
	return 1;
	}

	int status;
	pid_t child;
	child = fork();
	if(child<0){
	perror("fork");
	}
								
	if(child==0){
		int fd = open(argv[1], O_CREAT | O_APPEND | O_WRONLY, 0644);  
	if (fd == -1) {  
		perror("open");    
		return 1; 
	}
		
	char buf[100];
	snprintf(buf, 100, "[CHILD] getpid()=%d, getppid()=%d\n", getpid(), getppid());
	if (write(fd, buf, strlen(buf)) < strlen(buf)) {   
		perror("write");    
		return 1;
	}
	close(fd);
	exit(0);
	}
	else {
		wait(&status);
		int fd = open(argv[1], O_CREAT | O_APPEND | O_WRONLY, 0644);  
	if (fd == -1) {   
		perror("open");    
		return 1;
	}
	char buf[100];
	snprintf(buf,100,"[PARENT] getpid()=%d, getppid()=%d\n", getpid(), getppid());
	if (write(fd, buf, strlen(buf)) < strlen(buf)) {   
		perror("write");    
		return 1;
	}

	close(fd); 
	exit(0);
	return 0;
	}
}


