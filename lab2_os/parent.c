#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#define STREQUAL(x, y) (strncmp((x), (y), strlen(y)) == 0)

int times;
int paidi_fired=0;
int terma_fired=0;
int d_fired=0;
int e_fired=0;
int stop_fired=0;

void termagoneas()
{	
	terma_fired = 1;
}



void usr2goneas(){

	d_fired=1;
}

void usr1goneas(){

	e_fired=1;
}

void paidi()
{	
	paidi_fired=1;
}


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
		stop_fired=1;
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



int main(int argc, char **argv) {


	if (argc != 2){
		fprintf(stderr, "Usage: ./gates gates_state\n");
		return 0;
	}

	if (STREQUAL(argv[1], "--help")){
		printf("Usage: ./gates gates_state\n");
		return 0;
	}
	times = strlen(argv[1]);

	for (int i=0; i < times; i++){
		if (argv[1][i] != 't' && argv[1][i] != 'f'){

			fprintf(stderr, "Declare the state of the gates using only 't' and 'f'\n");
			return 0;}
	}


	struct sigaction sigtg, sig1g, sig2g, sigc;
	sigtg.sa_handler = termagoneas;
	sigtg.sa_flags = SA_RESTART;
	sigaction(SIGTERM, &sigtg, NULL);
	sig2g.sa_handler = usr2goneas;
	sig2g.sa_flags = SA_RESTART;
	sigaction(SIGUSR2, &sig2g, NULL);
	sig1g.sa_handler = usr1goneas;
	sig1g.sa_flags = SA_RESTART;
	sigaction(SIGUSR1, &sig1g, NULL);
	sigc.sa_handler = paidi;
	sigc.sa_flags = SA_RESTART;
	sigaction(SIGCHLD, &sigc, NULL);



	pid_t pinakas[times];
	for (int i = 0; i < times; i++){

		pid_t p = fork();
		check_neg(p, "fork");

		if (p == 0) {
			/* child, load "./child" executable */
			char c[] = {argv[1][i], '\0'};
			char d[] = {i, '\0'};

			char * argv1[] = {d, c,  NULL};
			int status = execv("./child", argv1);

			/* on success, execution will never reach this line */
			check_neg(status, "Failed to create child");
		}

		pinakas[i]=p;
		printf("[PARENT/PID=%d] Created child %d (PID=%d) and initial state '%c'\n", getpid(), i, p, argv[1][i]);
	}






	int counter = times;
	while(1){
		pause();
		if(terma_fired){
			for(int i = 0; i < times; i++){
				printf("[PARENT/PID=%d] Waiting for %d children to exit\n", getpid(), times-i);
				kill(pinakas[i], SIGTERM);
				int status;
				pid_t pid = wait(&status);
				describe_wait_status(pid, status);
			}
			printf("[PARENT/PID=%d] All children exited, terminating as well\n", getpid());
			terma_fired=0;
			exit(0);
		}
		if(paidi_fired){
			int i;
			int status;
			pid_t pid = waitpid(-1, &status, WUNTRACED);
			for(i = 0; i<times; i++){
				if(pinakas[i]==pid) break;
			}
			describe_wait_status(pid, status);
			if(stop_fired){
				kill(pid, SIGCONT);
				waitpid(-1, &status, WCONTINUED);
				describe_wait_status(pid, status);
				stop_fired=0;
			}
			else {
				pid_t p = fork();
				check_neg(p, "fork");

				if (p == 0) {
					/* child, load "./child" executable */
					char c[] = {argv[1][i], '\0'};
					char d[] = {i, '\0'};

					char * argv1[] = {d, c,  NULL};
					int status = execv("./child", argv1);

					/* on success, execution will never reach this line */
					check_neg(status, "Failed to create child");
				}

				pinakas[i]=p;
				printf("[PARENT/PID=%d] Created child %d (PID=%d) and initial state '%c'\n", getpid(), i, p, argv[1][i]);
			}
			paidi_fired=0;
		}

		if(e_fired){
			for(int i = 0; i < times; i++){
				kill(pinakas[i], SIGUSR1);
			}
			e_fired=0;
		}

		if(d_fired){
			for(int i = 0; i < times; i++){
				kill(pinakas[i], SIGUSR2);
			}
			d_fired=0;
		}

	}

	return 0;

}

