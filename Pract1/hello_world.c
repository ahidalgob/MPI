#include <mpi.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
	int me, numProcess;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcess);
	MPI_Comm_rank(MPI_COMM_WORLD, &me);

	if (me == 0)
		printf ("Se han creado %d procesos\n", numProcess);

    char procesador[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(procesador, &name_len);

	printf ("Soy el proceso %d, ejecutando en %s\n", me, procesador);

	MPI_Finalize();
	return 0;
}
