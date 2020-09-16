#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <chrono>
#include <string.h>
#include <omp.h>
using namespace std;

int **A;
int **B;
int **C;
int N;
int *sendcount, *displacement;
bool m_print = false;

int** init_array(int r, int c, bool fill) {
    // Create 1D array to the size of rows * cols
    int *array_data = (int *)malloc(sizeof(int)* r * c);
    // Initialise array of pointers
    int **array = (int **)malloc(sizeof(int*) * r);
    
    // Assign array[i] to location of begining of rows
    for (int i=0; i< r; i++)
    {
        array[i] = &(array_data[c * i]);
    }

    // Fill array with random integers
    if (fill)
    {
        for (int i = 0; i < r; i++)
        {
            // printf("fill i %d\n", i);
            for (int j = 0; j < c; j++)
            {
                // printf("fill j %d\n", i);
                array[i][j] = rand() % 100;
                // printf("fill j after\n");
            }
        }
    }

    return array;
}

void multiply_matrix(int process_count)
{
    #pragma omp for
    for (int i = 0; i < process_count / N; i++) {
        for (int j=0; j < N; j++) {
            C[i][j] = 0;
            for (int k = 0; k < N; k++) {
               C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

void print_matrix(int** arr, int rows, int cols) {
    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            printf("%d\t", arr[i][j]);
        }
        printf("\n");
    }
}

void head_func(int num_processes)
{
    // Initialise arrays
    A = init_array(N, N, true);
    B = init_array(N, N, true);
    C = init_array(N, N, false);

    if (m_print) {
        printf("HEAD matrix A\n");
        print_matrix(A, N, N);
        printf("HEAD matrix B\n");
        print_matrix(B, N, N);
    }
    
    // Initialise element values
    int n_rows_per_process = N / num_processes;
    int n_elements_per_process = n_rows_per_process * N;
    int n_elements_total = (N * N);
    // Remainder for sendcount calculation
    int n_remainder = N % num_processes;
    // Sum for displacement
    int sum = 0;
    
    // calculate send counts and displacements
    for (int i = 0; i < num_processes; i++) {
        // Add number of elements based on rows
        sendcount[i] = n_elements_per_process;
        // If there are still/any remainders
        if (n_remainder > 0) {
            // Add a row
            sendcount[i] += N;
            // Minus a row from remainder
            n_remainder--;
        }

        // Add sum of total elements sent to displacement
        displacement[i] = sum;
        // Update total sent
        sum += sendcount[i];
    }

    for (int i = 0; i < num_processes; i++) {
        printf("sendcount[rank %d] = %d\n", i, sendcount[i]);
    }
    
    // Broadcast sendcounts and displacements for nodes to read
    MPI_Bcast(&sendcount[0], num_processes, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&displacement[0], num_processes, MPI_INT, 0, MPI_COMM_WORLD);

    // Get number of elements to process for head
    int process_count = sendcount[0];

    // Timer start
    auto start = chrono::high_resolution_clock::now();

    MPI_Bcast(&B[0][0], n_elements_total, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatterv(&A[0][0], sendcount, displacement, MPI_INT, &A, 0, MPI_INT, 0, MPI_COMM_WORLD);
    
    #pragma omp parallel
    {
        multiply_matrix(process_count);
    }

    MPI_Gatherv(MPI_IN_PLACE, n_elements_per_process, MPI_INT, &C[0][0], sendcount, displacement, MPI_INT, 0, MPI_COMM_WORLD);

    // Timer end
    auto end = chrono::high_resolution_clock::now();
    double time_taken = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    time_taken /= 1000;
    printf("MPI execution time: %0.3f seconds\n", time_taken);

    if (m_print) {
        printf("HEAD matrix C\n");
        print_matrix(C, N, N);
    }
}

void node_func(int process_rank, int num_processes)
{
    int n_rows_per_process = N / num_processes;
    int n_elements_per_process = n_rows_per_process * N;
    int n_elements_total = (N * N);

    MPI_Bcast(&sendcount[0], num_processes, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&displacement[0], num_processes, MPI_INT, 0, MPI_COMM_WORLD);

    // Get number of elements to process for process_rank
    int process_count = sendcount[process_rank];
    // printf("NODE rank %d send count: %d\n", process_rank, process_count);
    // printf("NODE rank %d row count: %d\n", process_rank, process_count / nCols);
    
    A = init_array(process_count / N, N, false);
    B = init_array(N, N, false);
    C = init_array(process_count / N, N, false);
    
    MPI_Bcast(&B[0][0], n_elements_total, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatterv(NULL, sendcount, displacement, MPI_INT, &A[0][0], process_count, MPI_INT, 0, MPI_COMM_WORLD);

    #pragma omp parallel
    {
        multiply_matrix(process_count);
    }

    MPI_Gatherv(&C[0][0], sendcount[process_rank], MPI_INT, NULL, sendcount, displacement, MPI_INT, 0, MPI_COMM_WORLD);
}



int main(int argc, char **argv)
{
    // Initialise random seed
    srand((unsigned int)time(NULL));

    if (argc <= 1) {
        printf("Arguments:\n");
        printf("\t1. Matrix size (int)\n");
        printf("--Optional arguments:\n");
        printf("\t2. Print matrices (true/false)\n");
        exit(0);
    }

    if (argc > 1) {
        //printf("Number of rows = %s \n", argv[1]);
        N = atoi(argv[1]);
    }
    if (argc > 2) {
        omp_set_num_threads(atoi(argv[2]));
    }
    if (argc > 3) {
        if (strcmp(argv[3], "true") == 0) m_print = true;
    }

    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

    // Get the number of processes
    int num_processes;
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    // Get the rank of the process
    int process_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);

    // Initialise sendcount and displacement arrays
    sendcount = (int*)malloc(num_processes * sizeof(int));
    displacement = (int*)malloc(num_processes * sizeof(int));

    // Run head/node functions
    if (process_rank == 0)
    {
        head_func(num_processes);
    } else
    {
        node_func(process_rank, num_processes);
    }
    
    // Free arrays
    free(A);
    free(B);
    free(C);
    free(sendcount);
    free(displacement);

    // Finalize the MPI environment.
    MPI_Finalize();
}