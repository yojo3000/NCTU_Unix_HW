#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

void random_length_task() {
	int i;

	/*
	 * [low, up]
	 * => rand() % (up - low + 1) + low
	 */
	int s = 20 + rand() % 31;
	/*random integer in 20 ~50*/
	int n = 1 + rand() % 3;
	/*random integer in 1 ~ 3*/
	
	struct timeval tv; /*for gettimeofday*/
	/*
	 *	struct timeval {
	 *		time_t	tv_sec; 		// seconds 
	 *		suseconds_t	tv_usec;	// microseconds
	 *	};
	 */

	gettimeofday(&tv, NULL);
	/*gettimeofday: get current time, use timeval struct and time zone(Null)*/
	s = s - (s % n);
	printf("%ld.%06ld working for %d ms\n", tv.tv_sec, tv.tv_usec, s);
	/* %ld: 	long int
	 * %06ld: 	e.g.: 123 => 000123 
	 */
	for(i = 0; i < n; i++) {
		usleep(s / n * 1000);
		/*usleep: delay sereval time*/
		/*use 'micro-second' as it's parameter*/
		/*1 second = 10^6 micro-seconds*/
	}	
	return;
}
