//­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­+
// Introducción al paralelismo Abr ­ Julio 2016						|
//																	|
// hola.c: Ejemplo de prueba del entorno MPI.						|
//­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­+
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <mpi.h>
#define LONG_BUFFER 100
//­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­
void esclavo(int me) {
	char buffer[LONG_BUFFER];
	struct timeval t0, tf, t;
	long int tiempo[2]; // Segundos y microsegundos
	gettimeofday (&t0, NULL);

	strcpy(buffer, "Hola, desde ");
	int name_len;
	MPI_Get_processor_name(buffer+strlen(buffer), &name_len);
	/*gethostname(buffer + strlen(buffer), LONG_BUFFER);*/

	MPI_Send (buffer, strlen(buffer), MPI_CHAR, 0, 1, MPI_COMM_WORLD);

	sleep (me);
	gettimeofday (&tf, NULL);
	timersub(&tf, &t0, &t);
	tiempo[0] = t.tv_sec;
	tiempo[1] = t.tv_usec;
	MPI_Send (tiempo, 2, MPI_LONG, 0, 2, MPI_COMM_WORLD);
}
//­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­
void maestro(int numEsclavos) {
	int i;
	char buffer[LONG_BUFFER];
	long int tiempo[2]; // Segundos y microsegundos
	MPI_Status estado;

	int name_len;
	MPI_Get_processor_name(buffer, &name_len);
	/*gethostname(buffer, LONG_BUFFER);*/

	printf ("Maestro ejecutandose en %s\n", buffer);
	for (i=0; i<numEsclavos; i++) {
		MPI_Recv(buffer, LONG_BUFFER, MPI_CHAR, MPI_ANY_SOURCE, 1,
				MPI_COMM_WORLD, &estado);
		printf("Del proceso %d: %s\n", estado.MPI_SOURCE, buffer);
	}
	for (i=0; i<numEsclavos; i++) {
		MPI_Recv(tiempo, 2, MPI_LONG, i+1, 2, MPI_COMM_WORLD, &estado);
		printf("Tiempo del Proceso[%d] = %ld:%ld (seg:mseg)\n",
				i+1, tiempo[0], tiempo[1]/1000);
	}
}


//­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­
int main(int argc, char *argv[]) {
	int rank, numProcess;
	setbuf (stdout, NULL); // Sin buffers de escritura
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcess);

	if (rank == 0) maestro(numProcess-1);
	else esclavo(rank);
	MPI_Finalize();
	return 0;
}
