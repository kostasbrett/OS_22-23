#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

time_t arxh;
int d;
char c;
int s;
int alarm_fired = 0;
int out_fired = 0;

void terma()
{
	_exit(0);
}

void al(){
	alarm_fired = 1;
}

void usr2(){
	out_fired = 1;
}

void usr1(){
	s = !s;
	out_fired = 1;
}


int main(int argc, char **argv) {

	struct sigaction sigt, sig1, sig2, sigal;
	sigt.sa_handler = terma;
	sigt.sa_flags = SA_RESTART;
	sigaction(SIGTERM, &sigt, NULL);
	sigal.sa_handler = al;
	sigal.sa_flags = SA_RESTART;
	sigaction(SIGALRM, &sigal, NULL);
	sig2.sa_handler = usr2;
	sig2.sa_flags = SA_RESTART;
	sigaction(SIGUSR2, &sig2, NULL);
	sig1.sa_handler = usr1;
	sig1.sa_flags = SA_RESTART;
	sigaction(SIGUSR1, &sig1, NULL);



	arxh = time(NULL);
	d=argv[0][0];
	c=argv[1][0]; //isws na to katarghsoume
	s = (c=='f') ? 0 : 1;
	time_t twra = time(NULL);
	printf("[GATE=%d/PID=%d/TIME=%lds]", d, getpid(), (twra-arxh));
	if(s) {
		printf("The gates are open!\n");
	}
	else printf("The gates are closed!\n");

	alarm(15);
	while(1){


		pause();

		if(alarm_fired || out_fired){
			time_t twra = time(NULL);
			printf("[GATE=%d/PID=%d/TIME=%lds]", d, getpid(), (twra-arxh));
			if(s) {
				printf("The gates are open!\n");
			}
			else printf("The gates are closed!\n");
			if(alarm_fired){
				alarm(15);
				alarm_fired = 0;
			}
			out_fired = 0;
		}

	}
	return 0;
}

