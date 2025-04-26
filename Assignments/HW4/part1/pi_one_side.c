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

    MPI_Win win;

    // TODO: MPI init
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // 每個 process 計算 local tosses
    long long int local_tosses = tosses / world_size;
    long long int local_count = 0;

    unsigned int seed = (unsigned int)(time(NULL) + world_rank);
    for (long long int i = 0; i < local_tosses; i++) {
        double x = (double)rand_r(&seed) / RAND_MAX;
        double y = (double)rand_r(&seed) / RAND_MAX;
        if (x * x + y * y <= 1.0)
            local_count++;
    }

    // === 建立 MPI Window ===
    long long int *win_buffer = NULL;
    if (world_rank == 0) {
        // rank 0 要準備 buffer 收大家的資料
        win_buffer = (long long int *)malloc(sizeof(long long int) * world_size);
    }
    MPI_Win_create(win_buffer, 
                    world_rank == 0 ? sizeof(long long int) * world_size : 0, 
                    sizeof(long long int),
                    MPI_INFO_NULL, MPI_COMM_WORLD, &win);
    MPI_Win_fence(0, win);

    if (world_rank == 0){
        // Main
        win_buffer[0] = local_count;
    }
    else{
        // Workers
        MPI_Put(&local_count, 1, MPI_LONG_LONG_INT,
            0, world_rank, 1, MPI_LONG_LONG_INT, win);
    }

    MPI_Win_free(&win);

    if (world_rank == 0)
    {
        // TODO: handle PI result
        long long int total_count = 0;
        for (int i = 0; i < world_size; i++) {
            total_count += win_buffer[i];
        }

        pi_result = 4.0 * (double)total_count / (double)tosses;

        free(win_buffer); // 釋放 memory
        // --- DON'T TOUCH ---
        double end_time = MPI_Wtime();
        printf("%lf\n", pi_result);
        printf("MPI running time: %lf Seconds\n", end_time - start_time);
        // ---
    }

    MPI_Finalize();
    return 0;
}
