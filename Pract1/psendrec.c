#include <stdio.h>
#include <unistd.h>
#include "mpi.h"
#define N 3
#define VECES 5

//­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­
void esclavo(void) {
	int i, j, tabla[N], n;
	MPI_Status estado;
	sleep(2);
	for (i=0; i<VECES; i++) {
		MPI_Recv (tabla, N, MPI_INT, 0, 1, MPI_COMM_WORLD, &estado);
		MPI_Get_count (&estado, MPI_INT, &n);
		printf ("E: recibe => ");
		for (j=0; j<N; j++) printf("%d ", tabla[j]);
		printf (" de tid = %d eti = %d elementos = %d \n",
				estado.MPI_SOURCE, estado.MPI_TAG, n);
	}
}
//­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­
void maestro (void) {
	int i, j, vector[N];
	for (i=0; i<VECES; i++) {
		printf ("M: envia => ");
		for (j=0; j<N; j++) {
			vector[j] = i*N+j;
			printf("%d ", vector[j]);
		}
		printf ("\n");
		MPI_Send (vector, N, MPI_INT, 1, 1, MPI_COMM_WORLD);
	}
}
//­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­
int main( int argc, char *argv[] ) {
	int yo;
	setbuf(stdout, NULL); // Sin buffers en escritura
	MPI_Init (&argc, &argv);
	MPI_Comm_rank (MPI_COMM_WORLD, &yo);
	if (yo == 0) maestro();
	else
		esclavo();
	MPI_Finalize();
	return 0;
}
