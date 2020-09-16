#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <chrono>
#include <string.h>
#include <CL/cl.h>
using namespace std;

int **A;
int **B;
int **C;
int N;
int *sendcount, *displacement;
bool m_print = false;

// Open CL variables
cl_mem bufA, bufB, bufC;
cl_device_id device_id;
cl_context context;
cl_program program;
cl_kernel kernel;
cl_command_queue  queue;
cl_event event = NULL;

int err;
const int TS = 4;
const size_t local[2] = { (size_t)TS, (size_t)TS };
size_t global[2]; 

cl_device_id create_device();
void setup_openCL_device_context_queue_kernel(char* filename, char* kernelname);
cl_program build_program(cl_context ctx, cl_device_id dev, const char* filename);
void setup_kernel_memory();
void copy_kernel_args();
void free_memory();

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
            for (int j = 0; j < c; j++)
            {
                array[i][j] = rand() % 100;
            }
        }
    }

    return array;
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
    
    // Setup kernel on head
    setup_kernel_memory();
    copy_kernel_args();

    //submit the kernel for execution 
    clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global, local, 0, NULL, &event);
    clWaitForEvents(1, &event);

    //copying data from the device back to host c matrix
    clEnqueueReadBuffer(queue, bufC, CL_TRUE, 0, (process_count / N) * N *sizeof(int), &C[0][0], 0, NULL, NULL);

    // Gather other nodes calculations
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

    // Recieve sendcounts and displacement values for reference
    MPI_Bcast(&sendcount[0], num_processes, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&displacement[0], num_processes, MPI_INT, 0, MPI_COMM_WORLD);

    // Get number of elements to process for process_rank
    int process_count = sendcount[process_rank];
    // printf("NODE rank %d send count: %d\n", process_rank, process_count);
    // printf("NODE rank %d row count: %d\n", process_rank, process_count / nCols);
    
    A = init_array(process_count / N, N, false);
    B = init_array(N, N, false);
    C = init_array(process_count / N, N, false);
    
    // Recieve arrays
    MPI_Bcast(&B[0][0], n_elements_total, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatterv(NULL, sendcount, displacement, MPI_INT, &A[0][0], process_count, MPI_INT, 0, MPI_COMM_WORLD);

    setup_kernel_memory();
    copy_kernel_args();

    //submit the kernel for execution 
    clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global, local, 0, NULL, &event);
    clWaitForEvents(1, &event);

    //copying data from the device back to host c matrix
    clEnqueueReadBuffer(queue, bufC, CL_TRUE, 0, (process_count / N) * N *sizeof(int), &C[0][0], 0, NULL, NULL);

    // Send results to head
    MPI_Gatherv(&C[0][0], sendcount[process_rank], MPI_INT, NULL, sendcount, displacement, MPI_INT, 0, MPI_COMM_WORLD);
}

int main(int argc, char **argv)
{
    // Initialise random seed
    // srand((unsigned int)time(NULL));
    srand(1);

    if (argc <= 1) {
        printf("Arguments:\n");
        printf("\t1. Matrix size (int)\n");
        printf("--Optional arguments:\n");
        printf("\t2. Print matrices (true/false)\n");
        exit(0);
    }

    if (argc > 1) {
        N = atoi(argv[1]);
    }
    if (argc > 2) {
        if (strcmp(argv[2], "true") == 0) m_print = true;
    }

    // Setup OpenCL device, queue and kernel
    setup_openCL_device_context_queue_kernel( (char*) "./matrix_ops.cl" , (char*) "multiply_matrices");
    // Initialize global work size to size of array
    global[0] = (size_t)N;
    global[1] = (size_t)N;

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

    free_memory();
    // Finalize the MPI environment.
    MPI_Finalize();
}

void free_memory() {
   //free the buffers
   clReleaseMemObject(bufA);
   clReleaseMemObject(bufB);
   clReleaseMemObject(bufC);

    //free opencl objects
   clReleaseKernel(kernel);
   clReleaseCommandQueue(queue);
   clReleaseProgram(program);
   clReleaseContext(context);
   
}

void copy_kernel_args() {
    clSetKernelArg(kernel, 0, sizeof(int), (void*)&N);
    clSetKernelArg(kernel, 1, sizeof(int), (void*)&N);
    clSetKernelArg(kernel, 2, sizeof(int), (void*)&N);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), (void*)&bufA);
    clSetKernelArg(kernel, 4, sizeof(cl_mem), (void*)&bufB);
    clSetKernelArg(kernel, 5, sizeof(cl_mem), (void*)&bufC);
    if(err < 0) {
      perror("Couldn't create a kernel argument");
      printf("error = %d", err);
      exit(1);
   }
}

void setup_kernel_memory() {
    bufA = clCreateBuffer(context, CL_MEM_READ_ONLY,  N*N*sizeof(int), NULL, NULL);
    bufB = clCreateBuffer(context, CL_MEM_READ_ONLY,  N*N*sizeof(int), NULL, NULL);
    bufC = clCreateBuffer(context, CL_MEM_READ_WRITE, N*N*sizeof(int), NULL, NULL);

    // Copy matrices to the GPU
    clEnqueueWriteBuffer(queue, bufA, CL_TRUE, 0, N*N*sizeof(int), &A[0][0], 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, bufB, CL_TRUE, 0, N*N*sizeof(int), &B[0][0], 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, bufC, CL_TRUE, 0, N*N*sizeof(int), &C[0][0], 0, NULL, NULL);
}

void setup_openCL_device_context_queue_kernel(char* filename, char* kernelname) {
    device_id = create_device();
    cl_int err;
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
   if(err < 0) {
      perror("Couldn't create a context");
      exit(1);   
    }

    program = build_program(context, device_id, filename );
    queue = clCreateCommandQueueWithProperties(context, device_id, 0, &err);
    if(err < 0) {
      perror("Couldn't create a command queue");
      exit(1);   
    };

    kernel = clCreateKernel(program, kernelname, &err);
    if(err < 0) {
      perror("Couldn't create a kernel");
      printf("error =%d", err);
      exit(1);
    };

}
cl_program build_program(cl_context ctx, cl_device_id dev, const char* filename) {

   cl_program program;
   FILE *program_handle;
   char *program_buffer, *program_log;
   size_t program_size, log_size;
  

   /* Read program file and place content into buffer */
   program_handle = fopen(filename, "r");
   if(program_handle == NULL) {
      perror("Couldn't find the program file");
      exit(1);
   }
   fseek(program_handle, 0, SEEK_END);
   program_size = ftell(program_handle);
   rewind(program_handle);
   program_buffer = (char*)malloc(program_size + 1);
   program_buffer[program_size] = '\0';
   fread(program_buffer, sizeof(char), program_size, program_handle);
   fclose(program_handle);

   /* Create program from file 

   Creates a program from the source code in the add_numbers.cl file. 
   Specifically, the code reads the file's content into a char array 
   called program_buffer, and then calls clCreateProgramWithSource.
   */
   program = clCreateProgramWithSource(ctx, 1, 
      (const char**)&program_buffer, &program_size, &err);
   if(err < 0) {
      perror("Couldn't create the program");
      exit(1);
   }
   free(program_buffer);

   /* Build program 

   The fourth parameter accepts options that configure the compilation. 
   These are similar to the flags used by gcc. For example, you can 
   define a macro with the option -DMACRO=VALUE and turn off optimization 
   with -cl-opt-disable.
   */
   err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
   if(err < 0) {

      /* Find size of log and print to std output */
      clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 
            0, NULL, &log_size);
      program_log = (char*) malloc(log_size + 1);
      program_log[log_size] = '\0';
      clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 
            log_size + 1, program_log, NULL);
      printf("%s\n", program_log);
      free(program_log);
      exit(1);
   }

   return program;
}

cl_device_id create_device() {

   cl_platform_id platform;
   cl_device_id dev;
   int err;

   /* Identify a platform */
   err = clGetPlatformIDs(1, &platform, NULL);
   if(err < 0) {
      perror("Couldn't identify a platform");
      exit(1);
   } 

   // Access a device
   // GPU
   err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
   if(err == CL_DEVICE_NOT_FOUND) {
      // CPU
      err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
   }
   if(err < 0) {
      perror("Couldn't access any devices");
      exit(1);   
   }

   return dev;
}