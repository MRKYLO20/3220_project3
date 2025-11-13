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
/*
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

	void * page = malloc(2000);

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
}*/

// Mega-barrage allocator test.
// Exercises malloc/calloc/realloc/free with millions of ops and data checking.
// Avoids printf to prevent recursion; uses write + assert.
// You can tune OPS and MAXLIVE with env vars OPS, MAXLIVE.

#define _GNU_SOURCE
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

// ---- minimal logging ----
static void say(const char *s){ (void)!write(STDERR_FILENO, s, strlen(s)); }
static void say_u64(const char *prefix, unsigned long long v, const char *suffix){
    char buf[64]; int n=0; unsigned long long x=v; char tmp[32]; int m=0;
    if(prefix) (void)!write(STDERR_FILENO, prefix, strlen(prefix));
    if(x==0){ tmp[m++]='0'; } else { while(x){ tmp[m++]= '0'+(x%10); x/=10; } }
    for(int i=m-1;i>=0;i--) buf[n++]=tmp[i];
    (void)!write(STDERR_FILENO, buf, n);
    if(suffix) (void)!write(STDERR_FILENO, suffix, strlen(suffix));
}

typedef struct {
    unsigned char *p;
    size_t n;
    unsigned char tag;
} slot_t;

// RNG (LCG)
static unsigned long long rng = 0xDEADBEEFCAFEBABEull;
static inline unsigned rnd(){ rng = rng*1103515245ull + 12345ull; return (unsigned)(rng>>1); }

static inline void poison(unsigned char *p, size_t n, unsigned char tag){
    memset(p, tag, n);
}
static inline void check(const slot_t *s){
    for(size_t i=0;i<s->n;i++) { if(s->p[i] != s->tag){ assert(0 && "DATA CORRUPTION"); } }
}

static size_t get_env_or_default(const char *name, size_t def){
    const char *v = getenv(name);
    if(!v || !*v) return def;
    size_t out=0;
    for(const char *c=v; *c; ++c){
        if(*c<'0'||*c>'9') break;
        out = out*10 + (*c - '0');
    }
    return out?out:def;
}

static size_t size_skewed(unsigned max){
    // small-heavy distribution with occasional big
    unsigned r = rnd()%1000;
    if(r<600) return (rnd()%256)+1;         // 60% tiny
    if(r<900) return (rnd()%4096)+1;        // 30% small
    if(r<990) return (rnd()%65536)+1;       // 9% medium
    return (rnd()% (max?max:262144)) + 1;   // 1% large
}

int main(void){
    say("[barrage] start\n");

    void *brk0 = sbrk(0);

    size_t OPS      = get_env_or_default("OPS", 2000000);   // total ops
    size_t MAXLIVE  = get_env_or_default("MAXLIVE", 65536); // max live slots
    size_t MAXSZ    = get_env_or_default("MAXSZ", 262144);  // occasional large cap

    // allocate slot table
    slot_t *live = (slot_t*)calloc(MAXLIVE, sizeof(slot_t));
    assert(live);

    // Free(NULL) must be a no-op
    free(NULL);

    // Phase A: warmup calloc bursts (verify zero)
    for(size_t i=0;i<MAXLIVE/4;i++){
        size_t n = (rnd()%1024)+1;
        unsigned char *p = (unsigned char*)calloc(n,1);
        assert(p);
        for(size_t k=0;k<n;k++){ assert(p[k]==0); }
        poison(p, n, 0x7A);
        live[i].p=p; live[i].n=n; live[i].tag=0x7A;
    }
    // free half
    for(size_t i=0;i<MAXLIVE/4; i+=2){ check(&live[i]); free(live[i].p); live[i].p=NULL; }

    // Phase B: realloc grow/shrink churn
    for(size_t i=0;i<MAXLIVE/8;i++){
        size_t n = (rnd()%512)+16;
        unsigned char *p = (unsigned char*)malloc(n);
        assert(p); poison(p,n,0x33);
        for(int step=0; step<6; step++){
            size_t newn = (step%2==0) ? n*2 : n/2 + 1;
            unsigned char *q = (unsigned char*)realloc(p,newn);
            assert(q);
            // prefix preserved
            size_t keep = (n<newn)?n:newn;
            for(size_t k=0;k<keep;k++){ if(q[k]!=0x33){ assert(0 && "realloc lost data"); } }
            if(newn>keep) poison(q+keep, newn-keep, 0x33);
            p=q; n=newn;
        }
        free(p);
    }

    // Edge cases
    void *z = malloc(0); if(z) free(z);
    size_t big = (size_t)1<<63; // trigger overflow on 64-bit if calloc multiplies
    void *ov = calloc(big, 3);
    assert(ov==NULL);

    // Phase C: Mega random barrage
    size_t corrupt = 0, alloc_fail = 0, malloc_ops=0, realloc_ops=0, free_ops=0;
    for(size_t op=0; op<OPS; op++){
        unsigned action = rnd()%3; // 0=malloc,1=realloc,2=free
        size_t i = rnd()%MAXLIVE;

        if(action==0){
            if(live[i].p) continue;
            size_t n = size_skewed(MAXSZ);
            unsigned char tag = (unsigned char)(rnd()&0xFF);
            unsigned char *p = (unsigned char*)malloc(n);
            if(!p){ alloc_fail++; continue; }
            poison(p,n,tag);
            live[i].p=p; live[i].n=n; live[i].tag=tag;
            malloc_ops++;
        } else if(action==1){
            if(!live[i].p) continue;
            // verify then grow/shrink randomly
            check(&live[i]);
            size_t newn = size_skewed(MAXSZ);
            unsigned char *q = (unsigned char*)realloc(live[i].p, newn);
            assert(q);
            size_t keep = (live[i].n<newn)?live[i].n:newn;
            for(size_t k=0;k<keep;k++){ if(q[k]!=live[i].tag) { corrupt++; break; } }
            // set new region to flipped tag
            for(size_t k=keep;k<newn;k++){ q[k]= (unsigned char)(live[i].tag ^ 0xFF); }
            live[i].p=q; live[i].n=newn;
            realloc_ops++;
        } else {
            if(!live[i].p) continue;
            check(&live[i]);
            free(live[i].p);
            live[i].p=NULL; live[i].n=0;
            free_ops++;
        }

        // occasional calloc wave to mix in zeroing
        if((op % 50000)==0){
            size_t idx = rnd()%MAXLIVE;
            if(!live[idx].p){
                size_t n = (rnd()%2048)+1;
                unsigned char *p = (unsigned char*)calloc(n,1);
                if(p){
                    for(size_t k=0;k<n;k++){ if(p[k]!=0){ assert(0 && "calloc not zeroed"); } }
                    unsigned char tag = (unsigned char)(rnd()&0xFF);
                    poison(p,n,tag);
                    live[idx].p=p; live[idx].n=n; live[idx].tag=tag;
                }
            }
        }

        // every so often, realloc-to-zero pattern
        if((op % 120000)==0){
            size_t idx = rnd()%MAXLIVE;
            if(live[idx].p){
                void *r = realloc(live[idx].p, 0);
                if(r) free(r);
                live[idx].p=NULL; live[idx].n=0;
            }
        }
    }

    // cleanup
    for(size_t i=0;i<MAXLIVE;i++){
        if(live[i].p){ check(&live[i]); free(live[i].p); }
    }
    free(live);

    void *brk1 = sbrk(0);
    // Your allocator should not move the program break if it uses mmap exclusively
    assert(brk0 == brk1);

    say_u64("[barrage] malloc ops: ", (unsigned long long)malloc_ops, "\n");
    say_u64("[barrage] realloc ops: ", (unsigned long long)realloc_ops, "\n");
    say_u64("[barrage] free ops: ", (unsigned long long)free_ops, "\n");
    say_u64("[barrage] alloc failures (okay under pressure): ", (unsigned long long)alloc_fail, "\n");
    say("[barrage] done\n");
    return 0;
}
