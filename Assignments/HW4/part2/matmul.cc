#include <mpi.h>
#include <cstdio>
#include <algorithm>
void construct_matrices(
    int n, int m, int l, const int *a_mat, const int *b_mat, int **a_mat_ptr, int **b_mat_ptr)
{
    /* TODO: The data is stored in a_mat and b_mat.
     * You need to allocate memory for a_mat_ptr and b_mat_ptr,
     * and copy the data from a_mat and b_mat to a_mat_ptr and b_mat_ptr, respectively.
     * You can use any size and layout you want if they provide better performance.
     * Unambitiously copying the data is also acceptable.
     *
     * The matrix multiplication will be performed on a_mat_ptr and b_mat_ptr.
     */
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
        // printf("construct rank 0: allocating and copying\n");

        *a_mat_ptr = new int[n * m];
        *b_mat_ptr = new int[m * l];

        std::copy(a_mat, a_mat + n * m, *a_mat_ptr);
        std::copy(b_mat, b_mat + m * l, *b_mat_ptr);
    }
}

void matrix_multiply(
    const int n, const int m, const int l, const int *a_mat, const int *b_mat, int *out_mat)
{
    /* TODO: Perform matrix multiplication on a_mat and b_mat. Which are the matrices you've
     * constructed. The result should be stored in out_mat, which is a continuous memory placing n *
     * l elements of int. You need to make sure rank 0 receives the result.
     */
    // printf("multiply initing\n");
    int world_rank, world_size;

    // MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // // printf("%d %d\n", world_rank, world_size);
    // while((n % world_size) != 0){
    //     world_size--;
    // }

    // if (world_rank >= world_size)
    //     return;
    // // printf("%d %d\n", world_rank, world_size);
    // int rows_per_proc = n / world_size;


    int *sendcounts = new int[world_size];
    int *displs_send = new int[world_size];
    int *recvcounts = new int[world_size];
    int *displs_recv = new int[world_size];

    int rows_per_proc = n / world_size;
    int remainder = n % world_size;  // 多出來的行數，分配給前幾個 rank

    int offset_send = 0;
    int offset_recv = 0;
    for (int i = 0; i < world_size; i++) {
        int rows = rows_per_proc + (i < remainder ? 1 : 0); // 多分一行給前面的rank
        sendcounts[i] = rows * m;  // 每個人拿幾個元素
        recvcounts[i] = rows * l;
        displs_send[i] = offset_send;
        displs_recv[i] = offset_recv;
        offset_send += rows * m;
        offset_recv += rows * l;
    }

    int local_rows = rows_per_proc + (world_rank < remainder ? 1 : 0);
    int *a_local = new int[local_rows * m];
    int *c_local = new int[local_rows * l];
    // int *a_local = new int[rows_per_proc * m];
    // int *c_local = new int[rows_per_proc * l];

    
    // Scatter a_mat
    // MPI_Scatter((world_rank == 0) ? a_mat : NULL, rows_per_proc * m, MPI_INT, a_local, rows_per_proc * m, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Scatterv(
        a_mat,        // 傳送端資料
        sendcounts,   // 每個人送幾個
        displs_send,       // 每個人的起始位移
        MPI_INT,      // 資料型態
        a_local,      // 接收端資料
        local_rows * m,  // 自己接收的大小
        MPI_INT,      // 接收端資料型態
        0,            // root
        MPI_COMM_WORLD
    );

    // printf("%d %d\n", a_mat[0], b_mat[0]);
    int *b_copy = new int[m * l];
    if (world_rank == 0) {
        std::copy(b_mat, b_mat + m * l, b_copy);
    }
    MPI_Bcast(b_copy, m*l, MPI_INT, 0, MPI_COMM_WORLD);

    // printf("%d %d\n", b_mat[0], b_copy[0]);
    // for(int n_ptr = 0; n_ptr < n; n_ptr++){
    //     for (int l_ptr = 0; l_ptr < l; l_ptr++){
    //         out_mat[n_ptr*l + l_ptr] = 0;
    //         for (int m_ptr = 0; m_ptr < m; m_ptr++){
    //             out_mat[n_ptr * l + l_ptr] += a_mat[n_ptr * m + m_ptr] * b_mat[l_ptr * m + m_ptr];
    //         }
    //     }
    // }

    // for(int n_ptr = 0; n_ptr < rows_per_proc; n_ptr++){
    for(int n_ptr = 0; n_ptr < local_rows; n_ptr++){
        for (int l_ptr = 0; l_ptr < l; l_ptr++){
            c_local[n_ptr*l + l_ptr] = 0;
            for (int m_ptr = 0; m_ptr < m; m_ptr++){
                c_local[n_ptr * l + l_ptr] += a_local[n_ptr * m + m_ptr] * b_copy[l_ptr * m + m_ptr];
            }
        }
    }

    // MPI_Gather(c_local, rows_per_proc * l, MPI_INT, out_mat, rows_per_proc * l, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Gatherv(
        c_local,          // 自己算好的部分結果
        local_rows * l,      // 要送的元素數量
        MPI_INT,          // 資料型態
        out_mat,          // 最後 rank 0 收所有人的結果
        recvcounts,       // 每個人送多少
        displs_recv,           // 每個人放哪裡
        MPI_INT,          // 資料型態
        0,                // root
        MPI_COMM_WORLD
    );

    delete[] a_local;
    delete[] c_local;
    delete[] b_copy;
    delete[] recvcounts;
    delete[] displs_recv;
    delete[] sendcounts;
    delete[] displs_send;
}

void destruct_matrices(int *a_mat, int *b_mat)
{
    /* TODO */
    // free(a_mat);
    // free(b_mat);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0){
        delete[] a_mat;
        delete[] b_mat;
    }
    
}
