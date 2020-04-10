#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>

// number of gates
#define  N  6
// number of inputs
#define  K  12

#define MAX_SIZE               31
#define MAX_STATE           (1<<N)
#define MAX_TRANS       (1<<(N+1))
#define MASK        ((1<<(N+1))-1)
#define MASK_S        ((1<<(N))-1)


#define CLEAR      -1
#define FAIL       -2
#define SEARCHING  -3


uint32_t M[32];

int stack[N];
int taps[N][N+1];

int gtotal, found, gfound;
int kmax;
int state[2][MAX_STATE];

void print_bin(uint32_t x, int n) 
{
	for(int i=0; i<n; i++)
		if (x & 1<<i)
			printf("1");	
		else
			printf("0");
}


long long combi(int n,int k)
{
    long long ans=1;
    k=k>n-k?n-k:k;
    int j=1;
    for(;j<=k;j++,n--)
    {
        if(n%j==0)
        {
            ans*=n/j;
        }else
        if(ans%j==0)
        {
            ans=ans/j*n;
        }else
        {
            ans=(ans*n)/j;
        }
    }
    return ans;
}



void print_matrix()
{
	printf("echo \"digraph circuit {\n");
	for(int i=0; i<N; i++){
		//print_bin(M[i], N+1);
		//printf("\n");

		for(int j=0; j<N; j++)
		{
			if (M[i] & 1<<j)
				printf("\tx%d -> x%d\n", j, i);	
		}
		if (M[i] & 1<<N)
			printf("\tin -> x%d\n", i);

	}
	printf("} \" | dot -Tpdf > g%d.pdf\n\n", found);

	uint32_t val;
	int  i, j, k, ab, i1;

	// print truth table
	for(i=0; i<MAX_STATE; i++)
	{
		val = 0;
		for(j=N-1; j>=0; j--) {
			val = val<<1;
			if (!((i & M[j]) == M[j]))
				val |= 1;
		}

		if (val==(MASK_S & i)) {
			print_bin(val, N);
			printf(" 0 \n");
		}

		val = 0;
		i1 = i | (1<<N);
		for(j=N-1; j>=0; j--) {
			val = val<<1;
			if (!((i1 & M[j]) == M[j]))
				val |= 1;
		}
		if (val==(MASK_S & i)) {
			print_bin(val, N);
			printf(" 1 \n");
		
		}

	}

	
	for(int i=0; i<N; i++){
		printf("// ");
		print_bin(M[i], N+1);
		printf("\n");
	}
	printf("\n");
	exit(0);
}


int next_state(int state, int input)
{
uint32_t val = 0, xin;
int j;
	val = 0;
	for(j=N-1; j>=0; j--) {
		val = val << 1;
		xin = state | (input<<N);
		if (!((xin & M[j]) == M[j]))
			val |= 1;
	}

	return val;
}


int check_state(int s, int in)
{
uint32_t smask, val, x;
uint32_t next_s, next_t;


	if (state[in][s] >= 0)
		return state[in][s];

	if (state[in][s] == SEARCHING)
		return FAIL;

	state[in][s] = SEARCHING;

	x = next_state(s, in);
	val = x ^ s;

	smask = 1 << N;
	while(smask) {
		if (smask & val) {
			next_s = smask ^ s;			
			next_t = check_state(next_s, in);

			if (next_t == FAIL)
				return FAIL;

			if (state[in][s] == SEARCHING)
				state[in][s] = next_t;
			else
				if (state[in][s] != next_t)
					return FAIL;

		}
		smask = smask>>1; 
	}

	return state[in][s];
}


void analyse_matrix()
{
int s, k;
int s0 = 0, s1 = 0;   // number of stable states

int st_stable[2][2];
int sn00, sn01, sn10, sn11;

uint32_t smask, val, x0, x1;
uint32_t next_s;

	found++;      // a new potential network

	memset(state, CLEAR, 2*sizeof(int)*MAX_STATE);	

	// Calculate all the stable states
	for(s=0; s<MAX_STATE; s++)
	{
		x0 = next_state(s,0);
		x1 = next_state(s,1);

		if (s==x0 && s==x1)
			return;

		if (s==x0) {
			if (s0==2) return;

			// register the stable state
			st_stable[0][s0] = s;
			state[0][s] = s;
			s0++;
			
		}

		if (s==x1) { 
			if (s1==2) return;

			// registe the stable state
			st_stable[1][s1] = s;
			state[1][s] = s;
			s1++;	
		}
	}

	if (!(s0==2 && s1==2)) return;


	sn00 = check_state(st_stable[0][0], 1);
	if (sn00 == FAIL) return;

	sn01 = check_state(st_stable[0][1], 1);
	if (sn01 == FAIL) return;
	
	sn10 = check_state(st_stable[1][0], 0);
	if (sn10 == FAIL) return;

	sn11 = check_state(st_stable[1][1], 0);
	if (sn11 == FAIL) return;

	if (sn00 == sn01) return;
	if (sn10 == sn11) return;

	if (sn00 == st_stable[1][0] && sn10 == st_stable[0][0])
		return;

	if (sn00 == st_stable[1][1] && sn11 == st_stable[0][0])
		return;

	printf("---- %d -------\n", found);
	printf("(%d) --1--> (%d)\n", st_stable[0][0], sn00);
	printf("(%d) --1--> (%d)\n", st_stable[0][1], sn01);
	printf("(%d) --0--> (%d)\n", st_stable[1][0], sn10);
	printf("(%d) --0--> (%d)\n", st_stable[1][1], sn11);


	/*
	// Explore matrix
	for(s=0; s<MAX_STATE; s++)
	{

		state[0][s] = check_state(s, 0);
		if (state[0][s] == FAIL)
			return;

		state[1][s] = check_state(s, 1);
		if (state[1][s] == FAIL)
			return;

	}
	*/
}

void compose_matrix()
{
int i, j;

	// matrix completed
	// compose matrix here
	for(i=0; i<N; i++)
	{
		M[i] = 0;
		for(j=0; j<stack[i]; j++) 
			M[i] |= 1<<taps[i][j];
	}
		
	analyse_matrix();	
}

/* Create M matrix:

     min = minimum index usable

     l   = current gate
     p   = current input

     taps[][] --> indexes of gate inputs

    (Only consider one of the permutation among 
     the possible in each row).

*/
void m_matrix(int l, int min, int p)
{

	if (l==N) {
		compose_matrix();
		return;
	}

	if (p==stack[l]) {
		// row completed
		// start next row starting at zero
		m_matrix(l+1, 0, 0);
		return;
	}

	for(int i=min; i<N+2-stack[l]+p; i++) {
		if (i==l)
			continue;
		taps[l][p] = i;
		m_matrix(l, i+1, p+1);
	}

}

/* distribute k elements at level 'l'
   with a minimum of min elements per row  

   The row are sorted by increasing number of elements per row

*/
void distribute(int min, int k, int l)
{
	int total = 1;

	if (l==N-1)
	{
		/* level N-1 reached */
		if (k>N-1)
			return;

		stack[l] = k;        // allocate all to last row
		found = 0;
		m_matrix(0, 0, 0);   // compose the matrix with all the stack
		gfound += found;
		return;
	}

	for(int i=min; i*(N-1-l)<=k-i; i++)
	{
		/* add to the stack and distribute further */
		stack[l] = i;              // allocate i to this level
		distribute(i, k-i, l+1);   // and distribute futher to the next levels
	}
}


int main()
{
	distribute(1, K, 0);
	printf("%d\n", found);
}


