#include <assert.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#include <vector>
#include <algorithm>



int main( int argc, char *argv[] ) {
	sleep(1); // so after consecutive calls we have a different seed
	int N, T, B;
	N = atoi(argv[1]);
	T = atoi(argv[2]);
	B = atoi(argv[3]);
	srand(time(NULL));

	printf("%d\n%d\n",N,T);
	for(int i=0; i<T; i++){
		int X=rand()%N, Y=rand()%N, Tar=rand()%(100*N);
		Tar *= (rand()%2)*2-1;
		printf("%d %d %d\n",X,Y,Tar);
	}

	printf("%d\n",B);
	for(int i=0; i<T; i++){
		int X=rand()%N, Y=rand()%N, R=rand()%(100)+1, P=rand()%(200)+1;
		printf("%d %d %d\n",X,Y,R,P);
	}
}
