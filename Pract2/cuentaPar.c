//­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­+
//																	|
// CuentaPar.c: Cuenta aparaciones de un numero en un arreglo muy	|
// grande. Version paralela simple									|
// ESQUELETO														|
//­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­+
#include <assert.h>
#include <mpi.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#define MAX_ENTERO 1000
#define NUM_VECTORES 10000
#define NUM_BUSCADO 8



int count(int *vector, int n){
	printf("contando %d\n",n);
	int numVeces = 0, i, j;
	for (i=0; i<NUM_VECTORES; i++)
		for (j=0; j<n; j++)
			if (vector[j] == NUM_BUSCADO) numVeces++;
	return numVeces;
}

//­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­
void esclavo(void) {
	// Recibir el numero de elementos
	int n;
	MPI_Status estado;

	MPI_Recv(&n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &estado);

	printf("size es %d\n",n);

	/*MPI_Get_count(&estado, MPI_INT, &count);*/

	// Crear el arreglo para los elementos
	int *vector;
	assert((vector =(int *)malloc(sizeof(int)*(n)))!=NULL);
	MPI_Recv(vector, n, MPI_INT, 0, 1, MPI_COMM_WORLD, &estado);

	/*MPI_Get_count(&estado, MPI_INT, &count);*/
	/*printf("count %d\n",count);*/

	// Contar las ocurrencias
	int result = count(vector, n);

	// Enviar resultado
	MPI_Send(&result, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);

	/*free(vector);*/
}
//­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­
void maestro (int NumProcesos, int Cardinalidad) {
	int i,j, totalNumVeces;
	int *vector;
	struct timeval t0, tf, t;
	MPI_Status estado;

	// Inicializar el vector
	assert((vector =(int *)malloc(sizeof(int)*Cardinalidad))!=NULL);
	for (i=0; i<Cardinalidad; i++)
		vector[i] = random() % MAX_ENTERO;
	assert (gettimeofday (&t0, NULL) == 0);

	// Repartir trabajo
	// Mandar a cada uno cuantos elementos son, despues enviarle el arreglo
	// El proceso i trabajara [i*Card/n, (i+1)*Card/n)
	for(i=1; i<NumProcesos; i++){
		int ini = i * Cardinalidad / NumProcesos,
			fin = (i+1) * Cardinalidad / NumProcesos;
		int size = fin-ini;

		printf("enviando [%d, %d)\n",ini,fin);
		MPI_Send(&size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
		MPI_Send(vector + ini, size, MPI_INT, i, 1, MPI_COMM_WORLD);
	}

	// Computar mi trozo
	// Llamar a la funcion generica para esto
	totalNumVeces = count(vector, Cardinalidad / NumProcesos);

	// Recoger resultados
	// Iterar sobre los procesos recogiendo el resultado
	for(i=1; i<NumProcesos; i++){
		int result;
		MPI_Recv(&result, 1, MPI_INT, i, 2, MPI_COMM_WORLD, &estado);
		totalNumVeces += result;
	}

	assert (gettimeofday (&tf, NULL) == 0);
	timersub(&tf, &t0, &t);
	printf ("Numero de veces que aparece el %d = %d\n",
			NUM_BUSCADO, totalNumVeces);
	printf ("tiempo total = %ld:%3ld\n", t.tv_sec, t.tv_usec/1000);

	/*free(vector);*/
}
//­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­
int main( int argc, char *argv[] ) {
	int yo, numProcesos;
	if (argc != 2) {
		printf ("Uso: cuentaPar cardinalidad \n");
		return 0;
	}
	int laCardinalidad = atoi(argv[1]);
	assert (laCardinalidad > 0);
	setbuf (stdout, NULL);
	MPI_Init (&argc, &argv);
	MPI_Comm_rank (MPI_COMM_WORLD, &yo);
	MPI_Comm_size (MPI_COMM_WORLD, &numProcesos);
	if (yo == 0) maestro(numProcesos,laCardinalidad);
	else esclavo();

	MPI_Finalize();
	return 0;
}
