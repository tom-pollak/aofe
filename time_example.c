#include <stdio.h>
#include <time.h>

int main(void)
{
	long int seconds_since_epoch;
	struct tm current_time, *time_ptr;
	int hour, minute, second, day, month, year;

	seconds_since_epoch = time(0);
	printf("time() - seconds since epoch: %ld\n", seconds_since_epoch);

	time_ptr = &current_time; // Set time_ptr to the address of
							  // the current time struct
	localtime_r(&seconds_since_epoch, time_ptr);

	// Three differenct ways to access struct elements:
	hour = current_time.tm_hour; // Direct access
	minute = time_ptr->tm_min;   // Access via pointer (common)
	second = *((int *) time_ptr); // Hacky pointer access

	printf("Current time is: %02d:%02d:%02d\n", hour, minute, second);
	return 0;
}
