#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    // --- DON'T TOUCH ---
    MPI_Init(&argc, &argv);
    double start_time = MPI_Wtime();
    double pi_result;
    long long int tosses = atoi(argv[1]);
    int world_rank, world_size;
    // ---

    // TODO: MPI init
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    long long int local_tosses = (world_rank == 0 ? tosses % (world_size-1) : tosses / (world_size-1));
    long long int local_count = 0;
    unsigned int seed = (unsigned int)(time(NULL) * world_rank);
    
    for (long long int i = 0; i < local_tosses; i++) {
        double x = (double)rand_r(&seed) / RAND_MAX;
        double y = (double)rand_r(&seed) / RAND_MAX;
        if (x * x + y * y <= 1.0)
            local_count++;
    }

    long long int received[world_size];
    MPI_Request requests[world_size-1];
    // int has_pending_recv = 0;

    if (world_rank > 0)
    {
        // TODO: MPI workers
        MPI_Send(&local_count, 1, MPI_LONG_LONG_INT, 0, 0, MPI_COMM_WORLD);
    }
    else if (world_rank == 0)
    {
        for (int i = 1; i < world_size; i++) {
            MPI_Irecv(&received[i], 1, MPI_LONG_LONG_INT, i, 0, MPI_COMM_WORLD, &requests[i-1]);
            // total_count += received_count;
            // has_pending_recv++;
        }

        // TODO: non-blocking MPI communication.
        // Use MPI_Irecv, MPI_Wait or MPI_Waitall.

        // MPI_Request requests[];

        // MPI_Waitall();
        MPI_Waitall(world_size-1, requests, MPI_STATUSES_IGNORE);
        for(int i = 1; i < world_size; i++){
            local_count += received[i];
        }
    }

    if (world_rank == 0)
    {
        // TODO: PI result
        pi_result = 4.0 * (double)local_count / (double)tosses;
        // --- DON'T TOUCH ---
        double end_time = MPI_Wtime();
        printf("%lf\n", pi_result);
        printf("MPI running time: %lf Seconds\n", end_time - start_time);
        // ---
    }

    MPI_Finalize();
    return 0;
}
