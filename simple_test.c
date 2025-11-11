#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#define NUMBUFS 10
int bufsizes[NUMBUFS] = {2,3,7,14,65,36,700,12,15,64};

int main()
{

	uint8_t *bufs[NUMBUFS];

	void * firstbreak = sbrk(0);
	
	free(NULL); //just for kicks

	for (int i=0; i < NUMBUFS; i++)
	{
		const char msg2[] = "running loop\n";
        (void)!write(STDERR_FILENO, msg2, sizeof msg2 - 1);
		//allocate the next block
		bufs[i] = malloc(bufsizes[i]);
		assert(bufs[i] != NULL); //should never return NULL

		//write some data into the buffer
		memset(bufs[i], i, bufsizes[i]);
		for (int b=0; b < bufsizes[i]; b++)
		{
			const char msg2[] = "validating\n";
        	(void)!write(STDERR_FILENO, msg2, sizeof msg2 - 1);

			assert (bufs[i][b] == i);
		}
		assert(bufs[i][0] == (uint8_t) i);
	}

	const char msg2[] = "check here\n";
    (void)!write(STDERR_FILENO, msg2, sizeof msg2 - 1);
	void * midbreak = sbrk(0);
	assert(firstbreak == midbreak);

	for (int i=0; i < NUMBUFS; i++)
	{
		//check whether or not the memory is still intact
		for (int b=0; b < bufsizes[i]; b++)
		{
			const char msg2[] = "validating\n";
        	(void)!write(STDERR_FILENO, msg2, sizeof msg2 - 1);

			assert (bufs[i][b] == i);
		}

		free(bufs[i]);
	}

	void * lastbreak = sbrk(0);

	//verify that the program break never moved up.
	assert (firstbreak == lastbreak);

	return 0;
}