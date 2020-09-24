#include <stdio.h>

int main(int argc, char *argv[])
{
	printf("real uid: %d\n", getuid());
	printf("effective uid: %d\n", geteuid());
	return 0;
}
