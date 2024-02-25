#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#define STREQUAL(x, y) (strncmp((x), (y), strlen(y)) == 0)
#define MAX(a, b) ((a) > (b) ? (a) : (b))



void check_neg(int ret, const char *msg) {
	if (ret < 0) {
		perror(msg);
		exit(EXIT_FAILURE);
	}
}

/**
 *  * \brief Parse result of wait() system call and print a descriptive message to
 *   * stdout
 *    *
 *     * Refer to `man 2 wait` for more details regarding the meaning of pid and
 *      * status.
 *       *
 *        * \param pid PID returned by wait()
 *         * \param status Status returned by wait()
 *          */
void describe_wait_status(pid_t pid, int status) {
	if (pid < 1) {
		perror("wait() call failed");
	}

	if (pid == 0) {
		printf("Nothing happened");
	}

	if (WIFSTOPPED(status)) {
		printf("Child with PID %d stopped\n", pid);
	} else if (WIFCONTINUED(status)) {
		printf("Child with PID %d continued\n", pid);
	} else if (WIFEXITED(status)) {
		printf("Child with PID %d exited with status code %d\n", pid,
				WEXITSTATUS(status));
	} else if (WIFSIGNALED(status)) {
		printf("Child with PID %d terminated by signal %d with status code %d\n",
				pid, WSTOPSIG(status), WEXITSTATUS(status));
	}
}

int isnumber (char number[]){
	for (int i = 0; i<strlen(number); i++){
		if (!isdigit(number[i])) return 0;

	}
	return 1;
}

int main(int argc, char **argv) {

	int round=1;

	if (argc > 3){
		fprintf(stderr, "Usage: ask3 <nChildren> [--random] [--round-robin]\n");
		return 0;
	}

	else if (argc == 3) {
		if (argv[1][0]=='0' || !isnumber(argv[1]) || !(STREQUAL(argv[2], "--random") || STREQUAL(argv[2], "--round-robin"))) {
			fprintf(stderr, "Usage: ask3 <nChildren> [--random] [--round-robin]\n");
			return 0;
		}
		if (STREQUAL(argv[2], "--random")){
			round=0;
		}
	}

	else if (argc == 2 ) {
		if (argv[1][0]=='0' || !isnumber(argv[1])) {
			fprintf(stderr, "Usage: ask3 <nChildren> [--random] [--round-robin]\n");
			return 0;
		}
	}

	else {
		fprintf(stderr, "Usage: ask3 <nChildren> [--random] [--round-robin]\n");
		return 0;
	}

	int times = atoi(argv[1]);
	int wp[times][2];
	int wc[times][2];


	pid_t pid_pinakas[times];
	for (int i = 0; i < times; i++){
		if(pipe(wc[i])!=0){
			perror("pipe_write_child");
			exit(0);
		}
		if(pipe(wp[i])!=0){
			perror("pipe_write_parent");
			exit(0);
		}

		pid_t p = fork();
		check_neg(p, "fork");

		if (p == 0) {

			if(close(wc[i][0])!=0){
				perror("close_child_in_child");
				exit(0);				
			}

			if(close(wp[i][1]) != 0){
				perror("close_parent_in_child");
				exit(0);
			}

			int val;

			while(1) {
				if(read(wp[i][0], &val, sizeof(int)) == -1){
					perror("read_child");
					exit(0);
				}

				printf("[Child %d] [%d] Child received %d!\n", i, getpid(), val);
				val++;
				sleep(5);
				if(write(wc[i][1], &val, sizeof(int)) == -1){
					perror("write_child");
					exit(0);
				}
				printf("[Child %d] [%d] Child Finished hard work, writing back %d\n", i, getpid(), val);
			}
		}

		else{
			pid_pinakas[i]=p;
			if(close(wp[i][0])!=0){
				perror("close_parent_in_parent");
				exit(0);				
			}

			if(close(wc[i][1]) != 0){
				perror("close_child_in_parent");
				exit(0);
			}
		}
	}

	int counter = 0;
	while(1){
		fd_set inset;
		int maxfd = STDIN_FILENO;
		FD_ZERO(&inset); 			  /* we must initialize before each call to select */
		FD_SET(STDIN_FILENO, &inset); /* select will check for input from stdin */
		for(int i = 0; i<times; i++){
			FD_SET(wc[i][0], &inset); /* select will check for input from pipe */
			maxfd = MAX(maxfd, wc[i][0]); /* select only considers file descriptors that are smaller than maxfd */
		}

		maxfd++;
		/* wait until any of the input file descriptors are ready to receive */
		int ready_fds = select(maxfd, &inset, NULL, NULL, NULL);
		if (ready_fds <= 0) {

			continue;                                       /* just try again */
		}
		/* user has typed something, we can read from stdin without blocking */
		if (FD_ISSET(STDIN_FILENO, &inset)) {
			char buffer[101];
			int n_read = read(STDIN_FILENO, buffer, 100); 
			if (n_read == -1) {
				perror("read_parent_from_console");
				exit(0);
			}
			buffer[n_read] = '\0';                          /* why? A:null terminated string */

			/* New-line is also read from the stream, discard it. */
			if (n_read > 0 && buffer[n_read-1] == '\n') {
				buffer[n_read-1] = '\0';
			}



			if (n_read >= 4 && strncmp(buffer, "exit", 4) == 0) {
				/* user typed 'exit', kill child and exit properly */
				for(int i=0; i < times; i++){	
					printf("[PARENT/PID=%d] Waiting for %d children to exit\n", getpid(), times-i);
					kill(pid_pinakas[i], SIGTERM);
					int status;
					pid_t pid = wait(&status);
					describe_wait_status(pid, status);
					if(close(wp[i][1])!=0){
						perror("close_parent_in_parent_exit");
						exit(0);				
					}

					if(close(wc[i][0]) != 0){
						perror("close_child_in_parent_exit");
						exit(0);
					}                            	
				}
				printf("[PARENT/PID=%d] All children exited, terminating as well\n", getpid());
				exit(0);
			}

			else if(n_read >=1 && isnumber(buffer)){
				int in = atoi(buffer);
				int worker;
				if(round){
					worker=counter%times;
					counter++;
				}
				else {
					worker=rand()%times;
				}
				if(write(wp[worker][1], &in, sizeof(int)) == -1){
					perror("write_parent");
					exit(1);
				}
				printf("Parent assigned %d to child %d\n", in, worker);
			}

			else if (n_read >= 1) {
				printf("Type a number to send job to a child!\n");
			}
		}
		for (int i = 0; i < times; i++){
			if (FD_ISSET(wc[i][0], &inset)) {
				int val;
				if(read(wc[i][0], &val, sizeof(int)) == -1){
					perror("read_parent");
					exit(0);
				}
				printf("[Parent] Received result from child %d --> %d\n", i, val);
			}

		}

	}


	return 0;

}

