/*#include <stdio.h>
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
			const char msg2[] = "validating2\n";
        	(void)!write(STDERR_FILENO, msg2, sizeof msg2 - 1);

			assert (bufs[i][b] == i);
		}

		free(bufs[i]);
	}

	void * lastbreak = sbrk(0);

	//verify that the program break never moved up.
	assert (firstbreak == lastbreak);

	return 0;
}*/

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void say(const char *s){ (void)!write(STDERR_FILENO, s, strlen(s)); }

static void fill(uint8_t *p, size_t n, uint8_t v){ for(size_t i=0;i<n;i++) p[i]=v; }
static void check(uint8_t *p, size_t n, uint8_t v){ for(size_t i=0;i<n;i++) assert(p[i]==v); }

int main(void){
    say("[test_realloc] start\n");
    void *brk0 = sbrk(0);

    // realloc(NULL, n) == malloc(n)
    uint8_t *p = (uint8_t*)realloc(NULL, 40);
    assert(p); fill(p,40,0x11); check(p,40,0x11);

    // grow within small classes (40 -> 80)
    p = (uint8_t*)realloc(p, 80);
    // first 40 must be preserved
    check(p,40,0x11);
    fill(p+40,40,0x22);
    check(p+40,40,0x22);

    // grow across boundary (80 -> 200 -> 520 -> 1050 -> 2048(large twice pages maybe))
    size_t sizes[] = {200,520,1050,2048};
    for(int i=0;i<4;i++){
        size_t old = (i==0?80:sizes[i-1]);
        p = (uint8_t*)realloc(p, sizes[i]);
        assert(p);
        // verify prefix preserved
        for(size_t k=0;k< (old<40?old:40); k++){ assert(p[k]==0x11); }
        // set last byte to a sentinel
        p[sizes[i]-1] = (uint8_t)(0xA0 + i);
    }

    // shrink (2048 -> 16), preservation of first 16 of old content not required beyond min(old,new)
    p = (uint8_t*)realloc(p, 16);
    assert(p);
    // We can't guarantee sentinels survived, but it must be writable
    for(int i=0;i<16;i++) p[i]=0xCC;

    // realloc(p, 0) == free(p) and may return NULL or a unique freeable pointer per POSIX.
    void *q = realloc(p, 0);
    // Whatever returned, it must be safe not to free if NULL, or free if not NULL
    if(q) free(q);

    // Also do a stable grow/shrink loop
    uint8_t *x = (uint8_t*)malloc(32); assert(x);
    for(int n=32;n<=1024;n*=2){
        x = (uint8_t*)realloc(x, n); assert(x);
        memset(x, 0x7E, n);
    }
    for(int n=1024;n>=8;n/=2){
        x = (uint8_t*)realloc(x, n); assert(x);
        for(int i=0;i<n;i++) x[i]^=0x7E; // touch bytes
    }
    free(x);

    void *brk1 = sbrk(0);
    assert(brk0 == brk1);
    say("[test_realloc] ok\n");
    return 0;
}