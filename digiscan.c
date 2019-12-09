#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define  N  6
#define  K  8


#define MAX_SIZE            31
#define MAX_N           (1<<N)
#define MAX         (1<<(N+1))
#define MASK    ((1<<(N+1))-1)
#define MASK_S    ((1<<(N))-1)

uint32_t M[32];

int stack[N];
int taps[N][N+1];

int gtotal, found, gfound;
int kmax;

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
	printf("digraph circuit {\n");
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
	printf("}\n\n");
}

void found_valid_matrix()
{
int i, j, k, i1;
uint32_t val;

	k=0;

	for(i=0; i<MAX_N; i++)
	{
		val = 0;
		for(j=N-1; j>=0; j--) {
			val = val<<1;
			if (!((i & M[j]) == M[j]))
				val |= 1;
		}
		if (val==(MASK_S & i)) {
			k++;
			continue;
		}

		val = 0;
		i1 = i | (1<<N);
		for(j=N-1; j>=0; j--) {
			val = val<<1;
			if (!((i1 & M[j]) == M[j]))
				val |= 1;
		}
		if (val==(MASK_S & i))
			k++;

	}
	if (k == 4) {
		//kmax=k;
		// printf("k=%d\n", k);
		print_matrix();
	}
}

void compose_matrix()
{
int i, j;
uint32_t cover;

	// matrix completed
	// compose matrix here
	cover = 0;
	for(i=0; i<N; i++)
	{
		M[i]=0;
		for(j=0; j<stack[i]; j++) 
			M[i]|=1<<taps[i][j];
		cover |= M[i]; 
	}
	if (cover==MASK){
		found++;
		found_valid_matrix();
	}	
}

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


void distribute(int min, int k, int l)
{
	int total = 1;

	if (l==N-1)
	{
		if (k>N-1)
			return;

		stack[l] = k;	

		// for(int i=0; i<N; i++) {
		//	printf("%d ", stack[i]);
		//	total *= combi(N,stack[i]);
		// }
		// printf(" %d\n", total);
		// gtotal += total;
		found = 0;
		m_matrix(0, 0, 0);
		gfound+=found;
		//printf("found = %d\n", found);
		return;
	}


	for(int i=min; i*(N-1-l)<=k-i; i++)
	{
		stack[l] = i;
		distribute(i, k-i, l+1);
	}
}


int main()
{
	distribute(1, K,0);
	//printf("gtotal=%d gfound=%d\n", gtotal, gfound);
	//printf("kmax=%d\n",kmax);

}


