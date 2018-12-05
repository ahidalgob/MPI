#!/bin/bash
#
#SBATCH --partition=all
#SBATCH --job-name=hello_world_c
#SBATCH --time=10:00
#SBATCH --output=hello_world_c.%j.out
#SBATCH --ntasks=20
#

echo "-------Inicio de Proceso en mpi------"
prun a.out
echo "-------Fin de Proceso en mpi------"
