__kernel void add_matrices(const int M, const int N, const int K,
                      const __global int* A,
                      const __global int* B,
                      __global int* C) {
    
    // Thread identifiers
    const int globalRow = get_global_id(0); // Row ID of C (0..M)
    const int globalCol = get_global_id(1); // Col ID of C (0..N)
 
    printf("Kernel print::(%d,%d)\n ", globalRow, globalCol);
    C[globalCol*M + globalRow] = A[globalCol*M + globalRow] + B[globalCol*M + globalRow];
    
}


__kernel void multiply_matrices(const int M, const int N, const int K,
                      const __global int* A,
                      const __global int* B,
                      __global int* C) {
    
    const int globalRow = get_global_id(0); // Row ID of C (0..M)
    const int globalCol = get_global_id(1); // Col ID of C (0..N)

    int value = 0;

    // globalRow and globalCol exceed the max global worksize at seemingly random array sizes (even with 1 process)
    // therefore a check is added
    if (globalRow < M && globalCol < M)
    {
        for(int i = 0; i < M; i++)
        {
            value += A[globalRow*M + i ] * B[i*M + globalCol];
        }
        C[globalRow*M + globalCol] = value;
    }
    
    //printf("Kernel print::(%d,%d), value=%d\n ", globalRow, globalCol, value);
}




/*
    //printf("(%d,%d)\n ", globalRow, globalCol);
    // Compute a single element (loop over K)
    int acc = 0.0f;
    for (int k=0; k<K; k++) {
        acc += B[k*M + globalRow] * A[globalCol*K + k];
     //   printf("(%d,%d), values = (%d, %d)\n ", k*M + globalRow, globalCol*K + k, A[k*M + globalRow] , B[globalCol*K + k]);
    }
*/