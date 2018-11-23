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
#include <vector>
#define MAX_ENTERO 100
#define NUM_VECTORES 10000
#define NUM_BUSCADO 8

using namespace std;


int count(vector<int> &v, int n){
	//printf("parte:");
	//for(int i=0; i<n; i++) printf(" %d",v[i]);
	//printf("\n");

	int numVeces = 0;
	for (int i=0; i<NUM_VECTORES; i++)
		for (int j=0; j<n; j++)
			if (v[j] == NUM_BUSCADO) numVeces++;
	return numVeces;
}

//­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­
void esclavo(void) {
	// Recibir el numero de elementos
	int n;
	MPI_Status estado;

	MPI_Recv(&n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &estado);

	// Crear el arreglo para los elementos
	vector<int> v(n);
	MPI_Recv(&v[0], n, MPI_INT, 0, 1, MPI_COMM_WORLD, &estado);

	// Contar las ocurrencias
	int result = count(v, n);

	// Enviar resultado
	MPI_Send(&result, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);

}
//­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­
void maestro (int NumProcesos, int Cardinalidad) {
	struct timeval t0, tf, t;
	MPI_Status estado;

	// Inicializar el vector
	vector<int> v(Cardinalidad);
	for (int i=0; i<Cardinalidad; i++)
		v[i] = random() % MAX_ENTERO;

	//printf("todo:");
	//for(int x : v) printf(" %d",x);
	//printf("\n");

	assert (gettimeofday (&t0, NULL) == 0);

	// Repartir trabajo
	// El proceso i trabajara [i/n * Card, (i+1)/n * Card)
	for(int i=1; i<NumProcesos; i++){
		int ini = i * Cardinalidad / NumProcesos,
			fin = (i+1) * Cardinalidad / NumProcesos;
		int size = fin-ini;

		MPI_Send(&size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
		MPI_Send(&v[ini], size, MPI_INT, i, 1, MPI_COMM_WORLD);
	}

	// Computar mi trozo
	int totalNumVeces = count(v, Cardinalidad / NumProcesos);

	// Recoger resultados
	for(int i=1; i<NumProcesos; i++){
		int result;
		MPI_Recv(&result, 1, MPI_INT, i, 2, MPI_COMM_WORLD, &estado);
		totalNumVeces += result;
	}

	assert (gettimeofday (&tf, NULL) == 0);
	timersub(&tf, &t0, &t);
	printf ("Numero de veces que aparece el %d = %d\n",
			NUM_BUSCADO, totalNumVeces);
	printf ("tiempo total = %ld:%3ld\n", t.tv_sec, t.tv_usec/1000);

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
