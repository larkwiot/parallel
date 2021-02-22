#pragma once

#include "cublas_handle.hpp"

/** 
@file taskflow/cuda/cublas/cublas_flow.hpp
*/

namespace tf {

// ----------------------------------------------------------------------------
// cublasFlowCapturer definition
// ----------------------------------------------------------------------------

/**
@class cublasFlowCapturer

@brief class object to construct a cuBLAS task graph

%cublasFlowCapturer provides a higher-level interface over the @cuBLAS library
and hide concurrency details from users.
It inherits methods from tf::cudaFlowCapturerBase and must be used from
a tf::cudaFlowCapturer object.
All pointers used to %cublasFlowCapturer methods must be in GPU memory space or managed 
(i.e., @c cudaMallocManaged),
including scalars, @c alpha and @c beta, input data and output data pointers.
The following example uses @c cublas<t>amax to find the minimum index of the element
of the maximum absolute magnitude in a vector.

@code{.cpp}
tf::Executor executor;
tf::Taskflow taskflow;

size_t N = 1024;
float *x = nullptr;
int *d_res;
int  h_res;

std::vector<float> host(N, 0.0f);
host[512] = 100.0f;  // artificially set the mid-position to the largest

cudaMalloc(&x, N*sizeof(float));
cudaMalloc(&d_res, sizeof(int));

taskflow.emplace([&](tf::cudaFlowCapturer& capturer){
  tf::cublasFlowCapturer* cublas = capturer.make_capturer<tf::cublasFlowCapturer>();

  tf::cudaTask h2d      = capturer.copy(x, host.data(), N);
  tf::cudaTask find_max = cublas->amax(N, x, 1, d_res);  
  tf::cudaTask d2h      = capturer.copy(&h_res, d_res, 1);
  
  h2d.precede(find_max);  // amax runs before host-to-device copy
  find_max.precede(d2h);  // amax runs after  device-to-host copy
});

executor.run(taskflow).wait();

assert(h_res == 512);
@endcode

Currently, %cublasFlowCapturer supports only @c float and @c double data types.

Please refer to @cuBLAS for more details.
*/
class cublasFlowCapturer : public cudaFlowCapturerBase {

  public:
    
    /**
    @brief constructs a cublas flow capturer
     */
    cublasFlowCapturer() = default;
    
    /**
    @brief gets the native cublas handle associated with this %cublasFlowCapturer

    @return a native cublas handle of type cublasHandle_t
    */
    cublasHandle_t native_handle();
    
    /**
    @brief copies vector data from host to device
    
    This method copies @c n elements from a vector @c h in host memory space 
    to a vector @c d in GPU memory space. 
    The storage spacing between consecutive elements is given by @c inch for 
    the source vector @c h and by @c incd for the destination vector @c d.
    
    This method calls native @c cublasSetVectorAsync with packed parameters,
    <tt>(handle, args...)</tt>, where @c handle is manaed by 
    the %cublasFlowCapturer and @c args... are the given arguments.
    
    @tparam T data type
    @param n number of elements
    @param d target device pointer
    @param incd spacing between consecutive elements in @c d
    @param h source host pointer
    @param inch spacing between consecutive elements in @c h

    @return a tf::cudaTask handle
    */
    template <typename T,
      std::enable_if_t<!std::is_same_v<T, void>, void>* = nullptr
    >
    cudaTask vset(size_t n, const T* h, int inch, T* d, int incd);

    /**
    @brief copies vector data from device to host
    
    This method copies @c n elements from a vector @c d in GPU memory space 
    to a vector @c h in host memory space. 
    The storage spacing between consecutive elements is given by @c inch for 
    the target vector @c h and by @c incd for the source vector @c d.
    
    This method calls native @c cublasGetVectorAsync with packed parameters,
    <tt>(handle, args...)</tt>, where @c handle is manaed by 
    the %cublasFlowCapturer and @c args... are the given arguments.
    
    @tparam T data type
    @param n number of elements
    @param h target host pointer
    @param inch spacing between consecutive elements in @c h
    @param d source device pointer
    @param incd spacing between consecutive elements in @c d
    
    @return a tf::cudaTask handle
    */
    template <typename T,
      std::enable_if_t<!std::is_same_v<T, void>, void>* = nullptr
    >
    cudaTask vget(size_t n, const T* d, int incd, T* h, int inch);
    
    // ------------------------------------------------------------------------
    // Level-1 vector-vector operations
    // ------------------------------------------------------------------------

    /**
    @brief finds the smallest index of the element of the maximum 
           absolute magnitude
    
    This method calls native @c cublas<t>amax with packed parameters,
    <tt>(handle, args...)</tt>, where @c handle is manaed by 
    the %cublasFlowCapturer and @c args... are the given arguments.
    
    @tparam T data type
    
    @param n number of elements in vector @c x
    @param x pointer to the memory address of the vector
    @param incx stride between consecutive elements of @c x
    @param result the resulting index (1-based indexing)
    
    @return a tf::cudaTask handle
    */
    template <typename T>
    cudaTask amax(int n, const T* x, int incx, int* result);
    
    /**
    @brief finds the smallest index of the element of the minimum 
           absolute magnitude
    
    This method calls native @c cublas<t>amin with packed parameters,
    <tt>(handle, args...)</tt>, where @c handle is manaed by 
    the %cublasFlowCapturer and @c args... are the given arguments.
    
    @tparam T data type
    
    @param n number of elements in vector @c x
    @param x pointer to the memory address of the vector
    @param incx stride between consecutive elements of @c x
    @param result the resulting index (1-based indexing)
    
    @return a tf::cudaTask handle
    */
    template <typename T>
    cudaTask amin(int n, const T* x, int incx, int* result);
    
    /**
    @brief finds the sum of absolute values of the elements over a vector
    
    This method calls native @c cublas<t>asum with packed parameters,
    <tt>(handle, args...)</tt>, where @c handle is manaed by 
    the %cublasFlowCapturer and @c args... are the given arguments.
    
    @tparam T data type
    
    @param n number of elements in vector @c x
    @param x pointer to the memory address of the vector
    @param incx stride between consecutive elements of @c x
    @param result the result
    
    @return a tf::cudaTask handle
    */
    template <typename T>
    cudaTask asum(int n, const T* x, int incx, T* result);
    
    /**
    @brief multiples a vector by a scalar and adds it to a vector
    
    This function multiplies the vector @c x by the scalar @c alpha and 
    adds it to the vector @c y overwriting the latest vector with the result. 
    Hence, the performed operation is:
    
      <tt>y[j] = alpha * x[k] + y[j]</tt>, 
      
    where @c j and @c k are indices of @c n elements with step sizes 
    @c incy and @c incx.
    
    This method calls native @c cublas<t>asum with packed parameters,
    <tt>(handle, args...)</tt>, where @c handle is manaed by 
    the %cublasFlowCapturer and @c args... are the given arguments.
    
    @tparam T data type
    
    @param n number of elements in vectors @c x and @c y
    @param alpha scalar used to multiplication
    @param x pointer to the memory address of the vector @c x
    @param incx stride between consecutive elements of @c x
    @param y pointer to the memory address of the vector @c y
    @param incy stride between consecutive elements of @c y
    
    @return a tf::cudaTask handle
    */
    template <typename T>
    cudaTask axpy(
      int n, const T *alpha, const T *x, int incx, T *y, int incy
    );

    /**
    @brief copies a vector to another vector
    
    This function copies @c n elements from a vector @c x of a step size @c incx 
    to another vector @c y of step size @c incy.
     
    adds it to the vector @c y overwriting the latest vector with the result. 
    Hence, the performed operation is:
    
      <tt>y[j] = x[k]</tt>, 
      
    where @c j and @c k are indices of @c n elements with step sizes 
    @c incy and @c incx.
    
    This method calls native @c cublas<t>copy with packed parameters,
    <tt>(handle, args...)</tt>, where @c handle is manaed by 
    the %cublasFlowCapturer and @c args... are the given arguments.
    
    @tparam T data type
    
    @param n number of elements to copy
    @param x pointer to the memory address of the vector @c x
    @param incx stride between consecutive elements of @c x
    @param y pointer to the memory address of the vector @c y
    @param incy stride between consecutive elements of @c y
    
    @return a tf::cudaTask handle
    */
    template <typename T>
    cudaTask vcopy(int n, const T* x, int incx, T* y, int incy);
    
    /**
    @brief computes the dot product of two vectors

    <tt>sum += x[i] * y[i]</tt>
    
    This method calls native @c cublas<t>dot with packed parameters,
    <tt>(handle, args...)</tt>, where @c handle is manaed by 
    the %cublasFlowCapturer and @c args... are the given arguments.

    @tparam T data type
    
    @param n number of elements to perform the dot product
    @param x pointer to the memory address of the vector @c x
    @param incx stride between consecutive elements of @c x
    @param y pointer to the memory address of the vector @c y
    @param incy stride between consecutive elements of @c y
    @param result the resulting dot product
    
    @return a tf::cudaTask handle
    */
    template <typename T>
    cudaTask dot(int n, const T* x, int incx, const T* y, int incy, T* result);
    
    /**
    @brief computes the Euclidean norm of a vector
    
    This method calls native @c cublas<t>nrm2 with packed parameters,
    <tt>(handle, args...)</tt>, where @c handle is manaed by 
    the %cublasFlowCapturer and @c args... are the given arguments.
    
    @tparam T data type
    
    @param n number of elements in vector @c x
    @param x pointer to the memory address of the vector
    @param incx stride between consecutive elements of @c x
    @param result the result
    
    @return a tf::cudaTask handle
    */
    template <typename T>
    cudaTask nrm2(int n, const T* x, int incx, T* result);
    
    /**
    @brief scales a vector by a scalar
    
    This method calls native @c cublas<t>scal with packed parameters,
    <tt>(handle, args...)</tt>, where @c handle is manaed by 
    the %cublasFlowCapturer and @c args... are the given arguments.
    
    @tparam T data type
    
    @param n number of elements in vector @c x
    @param scalar scalar used for multiplication
    @param x pointer to the memory address of the vector
    @param incx stride between consecutive elements of @c x
    
    @return a tf::cudaTask handle
    */
    template <typename T>
    cudaTask scal(int n, const T* scalar, T* x, int incx);
    
    /**
    @brief swaps elements between two vectors
    
    This function interchanges the elements of vectors @c x and @c y. 
    Hence, the performed operation is:
    
    <tt>y[j] <-> x[k]</tt>,
    
    where @c j is the index of element in @c y with a step size @c incy and
    @c k is the index of element in @c x with a step size @c incx.
    
    This method calls native @c cublas<t>swap with packed parameters,
    <tt>(handle, args...)</tt>, where @c handle is manaed by 
    the %cublasFlowCapturer and @c args... are the given arguments.
    
    @tparam T data type
    
    @param n number of elements to perform the dot product
    @param x pointer to the memory address of the vector @c x
    @param incx stride between consecutive elements of @c x
    @param y pointer to the memory address of the vector @c y
    @param incy stride between consecutive elements of @c y
    
    @return a tf::cudaTask handle
    */
    template <typename T>
    cudaTask swap(int n, T* x, int incx, T* y, int incy);

    // ------------------------------------------------------------------------
    // TODO Level-2 matrix_vector operations
    // ------------------------------------------------------------------------
    
    // Documentation
    template <typename T>
    cudaTask gemv();

    template <typename T>
    cudaTask c_gemv();
    
    // ------------------------------------------------------------------------
    // Level-3 matrix-matrix operations
    // ------------------------------------------------------------------------
    
    /**
    @brief performs matrix-matrix addition and transposition
    
    This function performs the matrix-matrix addition/transposition:
    
      <tt>C = alpha * op(A) + beta * op(B)</tt>,
    
    where @c alpha and @c beta are scalars, and @c A, @c B and @c C are matrices 
    stored in column-major format with dimensions @c op(A) as @c m by @c n, 
    @c op(B) as @c m by @c n and @c C as @c m by @c n, respectively. 
    
    The operation is out-of-place if @c C does not overlap @c A or @c B.
    
    The in-place mode supports the following two operations:
    
      1. <tt>C = alpha * C + beta * op(B)</tt>
      2. <Tt>C = alpha op(A) + beta * C</tt>
    
    The operation includes the following special cases:
    
      1. the user can reset matrix @c C to zero by setting @c alpha and 
         @c beta to 0
      2. the user can transpose matrix @c A by setting @c alpha to 1 and 
         @c beta to 0
    
    The input matrices are in column-major storage.
    
    This method calls native @c cublas<t>geam with packed parameters,
    <tt>(handle, args...)</tt>, where @c handle is manaed by 
    the %cublasFlowCapturer and @c args... are the given arguments.
    
    @tparam T data type
    
    @param ta transport operation @c op(A)
    @param tb transport operation @c op(B)
    @param m number of rows of matrix @c C and @c op(A)
    @param n number of columns of matrix @c C and @c op(B)
    @param alpha pointer to the @c alpha scalar
    @param A pointer to the address of @c A
    @param lda leading dimension of 2D array used to store the matrix @c A
    @param beta pointer to the @c beta scalar
    @param B pointer to the address of @c B
    @param ldb leading dimension of 2D array used to store the matrix @c B
    @param C pointer to the address of @c C 
    @param ldc leading dimension of 2D array used to store the matrix @c C
    
    @return a tf::cudaTask handle
    */
    template <typename T>
    cudaTask geam(
      cublasOperation_t ta, cublasOperation_t tb,
      int m, int n,
      const T *alpha,
      const T *A, int lda,
      const T *beta,
      const T *B, int ldb,
      T *C, int ldc
    );
    
    /** 
    @brief similar to tf::cublasFlowCapturer::geam but on row-major layout
    */
    template <typename T>
    cudaTask c_geam(
      cublasOperation_t ta, cublasOperation_t tb,
      int m, int n,
      const T *alpha,
      const T *A, int lda,
      const T *beta,
      const T *B, int ldb,
      T *C, int ldc
    );

    /** 
    @brief performs matrix-matrix multiplication
    
    This function performs matrix-matrix multiplication:
    
    <tt>C = alpha * op (A) * op (B) + beta * C</tt>,
    
    where @c alpha and @c beta are scalars, and @c A, @c B, and @c C
    are 2D matrices stored in column-major format 
    with dimension @c op(A) as @c m by @c k,
    dimension @c op(B) as @c k by @c n, and @c C as @c m by @c n.
    
    The input matrices are in column-major storage.
    
    This method calls native @c cublas<t>gemm with packed parameters,
    <tt>(handle, args...)</tt>, where @c handle is manaed by 
    the %cublasFlowCapturer and @c args... are the given arguments.
    
    @tparam T data type
    
    @param ta transport operation @c op(A)
    @param tb transport operation @c op(B)
    @param m number of rows of matrix @c C and @c op(A)
    @param n number of columns of matrix @c C and @c op(B)
    @param k number of columns of @c op(A) and rows of @c op(B)
    @param alpha pointer to the @c alpha scalar
    @param A pointer to the address of @c A
    @param lda leading dimension of 2D array used to store the matrix @c A
    @param B pointer to the address of @c B
    @param ldb leading dimension of 2D array used to store the matrix @c B
    @param beta pointer to the @c beta scalar
    @param C pointer to the address of @c C 
    @param ldc leading dimension of 2D array used to store the matrix @c C
    
    @return a tf::cudaTask handle
    */
    template <typename T>
    cudaTask gemm(
      cublasOperation_t ta, cublasOperation_t tb,
      int m, int n, int k,
      const T *alpha,
      const T *A, int lda,
      const T *B, int ldb,
      const T *beta,
      T *C, int ldc
    );

    /**
    @brief similar to tf::cublasFlowCapturer::gemm but operates on C-styled 
           row-major layout
    */
    template <typename T>
    cudaTask c_gemm(
      cublasOperation_t ta, cublasOperation_t tb,
      int m, int n, int k,
      const T *alpha,
      const T *A, int lda,
      const T *B, int ldb,
      const T *beta,
      T *C, int ldc
    );

    /**
    @brief performs matrix-matrix multiplication over a batch of matrices 
    
    The batch must be @em uniform.
    All instances in the batch must have the same dimensions <tt>(m, n, k)</tt>, 
    leading dimensions <tt>(lda, ldb, ldc)</tt> and transpositions 
    <tt>(ta, tb)</tt> for their respective @c A, @c B and @c C matrices. 
    The address of the input matrices and the output matrix of each instance 
    of the batch are read from arrays of pointers passed to the function 
    by the caller.
    
    <tt>C[i]= alpha * op (A[i]) * op (B[i]) + beta * C[i], i in [0, bc)</tt>,
    
    where @c alpha and @c beta are scalars, and @c A[i], @c B[i], and @c C[i]
    are 2D matrices stored in column-major format 
    with dimension @c op(A) as @c m by @c k,
    dimension @c op(B) as @c k by @c n, and @c C as @c m by @c n.
    
    The input matrices are in column-major storage.
    
    This method calls native @c cublas<t>gemmBatched with packed parameters,
    <tt>(handle, args...)</tt>, where @c handle is manaed by 
    the %cublasFlowCapturer and @c args... are the given arguments.
    
    @tparam T data type
    
    @param ta transport operation @c op(A[i])
    @param tb transport operation @c op(B[i])
    @param m number of rows of matrix @c C[i] and @c op(A[i])
    @param n number of columns of matrix @c C[i] and @c op(B[i])
    @param k number of columns of @c op(A[i]) and rows of @c op(B[i])
    @param alpha pointer to the @c alpha scalar
    @param A array pointer to @c A batch
    @param lda leading dimension of 2D array used to store the matrix @c A[i]
    @param B array pointer to @c B batch
    @param ldb leading dimension of 2D array used to store the matrix @c B[i]
    @param beta pointer to the @c beta scalar
    @param C array pointer to @c C batch
    @param ldc leading dimension of 2D array used to store the matrix @c C[i]
    @param bc batch size (number of matrices)
    
    @return a tf::cudaTask handle
    */
    template <typename T>
    cudaTask gemm_batched(
      cublasOperation_t ta, cublasOperation_t tb,
      int m, int n, int k,
      const T *alpha,
      const T *A[], int lda,
      const T *B[], int ldb,
      const T *beta,
      T *C[], int ldc,
      int bc
    );
    
    /**
    @brief similar to tf::cublasFlowCapturer::gemm_batched but operates on 
           C-styled row-major layout
    */
    template <typename T>
    cudaTask c_gemm_batched(
      cublasOperation_t ta, cublasOperation_t tb,
      int m, int n, int k,
      const T *alpha,
      const T *A[], int lda,
      const T *B[], int ldb,
      const T *beta,
      T *C[], int ldc,
      int bc
    );
    
    /**
    @brief performs matrix-matrix multiplication over a batch of matrices 
           with strided memory access
    
    Here, we use @c A[i], @c B[i], @c C[i] as notation 
    for A, B and C matrices in the @c i-th instance of the batch, 
    implicitly assuming they are respectively address offsets 
    @c sA, @c sB, @c sC away from @c A[i-1], @c B[i-1], @c C[i-1].
    
    The input matrices are in column-major storage.
    
    This method calls native @c cublas<t>gemmStridedBatched with 
    packed parameters, <tt>(handle, args...)</tt>, where @c handle is manaed by 
    the %cublasFlowCapturer and @c args... are the given arguments.
    
    @tparam T data type
    
    @param ta transport operation @c op(A[i])
    @param tb transport operation @c op(B[i])
    @param m number of rows of matrix @c C[i] and @c op(A[i])
    @param n number of columns of matrix @c C[i] and @c op(B[i])
    @param k number of columns of @c op(A[i]) and rows of @c op(B[i])
    @param alpha pointer to the @c alpha scalar
    @param A pointer to @c A batch
    @param lda leading dimension of 2D array used to store the matrix @c A[i]
    @param sA address offset between @c A[i] and @c A[i+1]
    @param B pointer to @c B batch
    @param ldb leading dimension of 2D array used to store the matrix @c B[i]
    @param sB address offset between @c B[i] and @c B[i+1]
    @param beta pointer to the @c beta scalar
    @param C pointer to @c C batch
    @param ldc leading dimension of 2D array used to store the matrix @c C[i]
    @param sC address offset between @c C[i] and @c C[i+1]
    @param bc batch size (number of matrices)
    
    @return a tf::cudaTask handle
    
    The batch must be @em uniform. 
    All instances in the batch must have the same dimensions <tt>(m, n, k)</tt>, 
    leading dimensions <tt>(lda, ldb, ldc)</tt> and transpositions 
    <tt>(ta, tb)</tt> for their respective @c A, @c B and @c C matrices. 
    Input matrices @c A, @c B and output matrix @c C for each instance of the batch 
    are located at fixed address offsets from their locations in the previous instance. 
    Pointers to @c A, @c B and @c C matrices for the first instance are passed 
    to the function by the user along with the address @em offsets - 
    @c sA, @c sB and @c sC that determine the locations 
    of input and output matrices in future instances.
    
    <tt>C + i*sC = alpha * op (A + i*sA) * op (B + i*sB) + beta * (C + i*sC), i in [0, bc)</tt>,
    
    where @c alpha and @c beta are scalars, and @c A[i], @c B[i], and @c C[i]
    are 2D matrices stored in column-major format 
    with dimension @c op(A) as @c m by @c k,
    dimension @c op(B) as @c k by @c n, and @c C as @c m by @c n.
    
    On certain problem sizes, it might be advantageous to create multiple gemm tasks
    to take advantage of concurrent kernels, rather than this method.
    */
    template <typename T>
    cudaTask gemm_sbatched(
      cublasOperation_t ta, cublasOperation_t tb,
      int m, int n, int k,
      const T *alpha,
      const T *A, int lda, long long int sA,
      const T *B, int ldb, long long int sB,
      const T *beta,
      T *C, int ldc, long long int sC,
      int bc
    );
    
    /**
    @brief similar to tf::cublasFlowCapturer::c_gemm_sbatched but operates on
           C-styled row-major layout
    */
    template <typename T>
    cudaTask c_gemm_sbatched(
      cublasOperation_t ta, cublasOperation_t tb,
      int m, int n, int k,
      const T *alpha,
      const T *A, int lda, long long int sA,
      const T *B, int ldb, long long int sB,
      const T *beta,
      T *C, int ldc, long long int sC,
      int bc
    );
    
  private:
    
    cublasScopedPerThreadHandle _handle;

    void _stream(cudaStream_t);
};

// Procedure: _stream
inline void cublasFlowCapturer::_stream(cudaStream_t stream) {
  TF_CHECK_CUBLAS(
    cublasSetStream(_handle, stream), "failed to set cublas stream"
  );
}

// Function: native_handle
inline cublasHandle_t cublasFlowCapturer::native_handle() {
  return _handle;
}

}  // end of namespace tf -----------------------------------------------------


