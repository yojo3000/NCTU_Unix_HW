#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
extern void random_length_task();

int main(int argc, char *argv[]) {
	int n = 10;
	
	srand(time(0) ^ getpid());
	/*time(0): get current time*/
	/*getpid: get current process's pid*/
	/*srand is used to create a random number based on parameter*/
	
	sigset_t block;
	/* declare a signal set */
	
	sigemptyset(&block);
	/* init signal set and empty the set */

	sigaddset(&block, SIGALRM);
	/* add signal to signal set */
	
	sigprocmask(SIG_BLOCK, &block, NULL);
	/* blok the delivery of the signal until unblock*/

	ualarm(100000, 100000);
	/* send signal after parameter one microsecond, 
	 * signal 
	 */

	/*do 10 times random_length_task*/
	while(n-- > 0) {
		random_length_task();
		sigwaitinfo(&block, NULL);
		/* wait for signal */
	}
	return 0;
}
