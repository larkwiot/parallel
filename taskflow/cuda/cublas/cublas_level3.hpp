#pragma once

#include "cublas_handle.hpp"

namespace tf {

/** 
@file cublas_level3.hpp
*/

// ----------------------------------------------------------------------------
// geam
// ----------------------------------------------------------------------------

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

  1. the user can reset matrix C to zero by setting @c alpha and @c beta to 0
  2. the user can transpose matrix A by setting @c alpha to 1 and @c beta to 0

The input matrices are in column-major storage.

@tparam T data type

@param handle cublas library handle
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
*/
template <typename T>
void cublas_geam(
  cublasHandle_t handle,
  cublasOperation_t ta, cublasOperation_t tb,
  int m, int n,
  const T *alpha,
  const T *A, int lda,
  const T *beta,
  const T *B, int ldb,
  T *C, int ldc
) {

  cublasStatus_t stat;

  if constexpr(std::is_same_v<T, float>) {
    stat = cublasSgeam(handle,
      ta, tb, m, n, alpha, A, lda, beta, B, ldb, C, ldc
    );
  }
  else if constexpr(std::is_same_v<T, double>) {
    stat = cublasDgeam(handle,
      ta, tb, m, n, alpha, A, lda, beta, B, ldb, C, ldc
    );
  }
  else {
    static_assert(dependent_false_v<T>, "unknown cublas data type");
  }

  TF_CHECK_CUBLAS(stat, "failed to run geam");
}

/**
@brief similar to tf::cublas_geam but operates on C-styled row-major layout
*/
template <typename T>
void cublas_c_geam(
  cublasHandle_t handle,
  cublasOperation_t ta, cublasOperation_t tb,
  int m, int n,
  const T *alpha,
  const T *A, int lda,
  const T *beta,
  const T *B, int ldb,
  T *C, int ldc
) {

  cublasStatus_t stat;

  if constexpr(std::is_same_v<T, float>) {
    stat = cublasSgeam(handle,
      ta, tb, n, m, alpha, A, lda, beta, B, ldb, C, ldc
    );
  }
  else if constexpr(std::is_same_v<T, double>) {
    stat = cublasDgeam(handle,
      ta, tb, n, m, alpha, A, lda, beta, B, ldb, C, ldc
    );
  }
  else {
    static_assert(dependent_false_v<T>, "unknown cublas data type");
  }

  TF_CHECK_CUBLAS(stat, "failed to run c_geam");
}

// ----------------------------------------------------------------------------
// gemm
// ----------------------------------------------------------------------------

/** 
@brief performs matrix-matrix multiplication

This function performs matrix-matrix multiplication:

<tt>C = alpha * op (A) * op (B) + beta * C</tt>,

where @c alpha and @c beta are scalars, and @c A, @c B, and @c C
are 2D matrices stored in column-major format 
with dimension @c op(A) as @c m by @c k,
dimension @c op(B) as @c k by @c n, and @c C as @c m by @c n.

The input matrices are in column-major storage.

@tparam T data type

@param handle cublas library handle
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
*/
template <typename T>
void cublas_gemm(
  cublasHandle_t handle,
  cublasOperation_t ta, cublasOperation_t tb,
  int m, int n, int k,
  const T *alpha,
  const T *A, int lda,
  const T *B, int ldb,
  const T *beta,
  T *C, int ldc
) {

  cublasStatus_t stat;

  if constexpr(std::is_same_v<T, float>) {
    stat = cublasSgemm(handle,
      ta, tb, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc
    );
  }
  else if constexpr(std::is_same_v<T, double>) {
    stat = cublasDgemm(handle,
      ta, tb, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc
    );
  }
  else {
    static_assert(dependent_false_v<T>, "unknown cublas data type");
  }

  TF_CHECK_CUBLAS(stat, "failed to run gemm");
}

/** 
@brief similar to tf::cublas_gemm but operates on C-styled row-major layout
*/
template <typename T>
void cublas_c_gemm(
  cublasHandle_t handle,
  cublasOperation_t ta, cublasOperation_t tb,
  int m, int n, int k,
  const T *alpha,
  const T *A, int lda,
  const T *B, int ldb,
  const T *beta,
  T *C, int ldc
) {
  cublasStatus_t stat;
  if constexpr(std::is_same_v<T, float>) {
    stat = cublasSgemm(handle,
      tb, ta, n, m, k, alpha, B, ldb, A, lda, beta, C, ldc
    );
  }
  else if constexpr(std::is_same_v<T, double>) {
    stat = cublasDgemm(handle,
      tb, ta, n, m, k, alpha, B, ldb, A, lda, beta, C, ldc
    );
  }
  else {
    static_assert(dependent_false_v<T>, "unknown cublas data type");
  }
  TF_CHECK_CUBLAS(stat, "failed to run c_gemm");
}

// ----------------------------------------------------------------------------
// gemm_batched
// ----------------------------------------------------------------------------

/**
@brief performs matrix-matrix multiplication over a batch of matrices 

The batch must be @em uniform.
All instances in the batch must have the same dimensions <tt>(m, n, k)</tt>, 
leading dimensions <tt>(lda, ldb, ldc)</tt> and transpositions 
<tt>(ta, tb)</tt> for their respective @c A, @c B and @c C matrices. 
The address of the input matrices and the output matrix of each instance 
of the batch are read from arrays of pointers passed to the function by the caller.

<tt>C[i]= alpha * op (A[i]) * op (B[i]) + beta * C[i], i in [0, bc)</tt>,

where @c alpha and @c beta are scalars, and @c A[i], @c B[i], and @c C[i]
are 2D matrices stored in column-major format 
with dimension @c op(A) as @c m by @c k,
dimension @c op(B) as @c k by @c n, and @c C as @c m by @c n.

The input matrices are in column-major storage.

@tparam T data type

@param handle cublas library handle
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

*/
template <typename T>
void cublas_gemm_batched(
  cublasHandle_t handle,
  cublasOperation_t ta, cublasOperation_t tb,
  int m, int n, int k,
  const T *alpha,
  const T *A[], int lda,
  const T *B[], int ldb,
  const T *beta,
  T *C[], int ldc,
  int bc
) {

  cublasStatus_t stat;

  if constexpr(std::is_same_v<T, float>) {
    stat = cublasSgemmBatched(handle,
      ta, tb, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc, bc
    );
  }
  else if constexpr(std::is_same_v<T, double>) {
    stat = cublasDgemmBatched(handle,
      ta, tb, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc, bc
    );
  }
  else static_assert(dependent_false_v<T>, "unknown cublas data type");

  TF_CHECK_CUBLAS(stat, "failed to run gemm_batched");
}

/**
@brief similar to tf::cublas_gemm_batched but operates on C-styled row-major layout
*/ 
template <typename T>
void cublas_c_gemm_batched(
  cublasHandle_t handle,
  cublasOperation_t ta, cublasOperation_t tb,
  int m, int n, int k,
  const T *alpha,
  const T *A[], int lda,
  const T *B[], int ldb,
  const T *beta,
  T *C[], int ldc,
  int bc
) {
  cublasStatus_t stat;

  if constexpr(std::is_same_v<T, float>) {
    stat = cublasSgemmBatched(handle,
      tb, ta, n, m, k, alpha, B, ldb, A, lda, beta, C, ldc, bc
    );
  }
  else if constexpr(std::is_same_v<T, double>) {
    stat = cublasDgemmBatched(handle,
      tb, ta, n, m, k, alpha, B, ldb, A, lda, beta, C, ldc, bc
    );
  }
  else static_assert(dependent_false_v<T>, "unknown cublas data type");

  TF_CHECK_CUBLAS(stat, "failed to run c_gemm_batched");
}

// ----------------------------------------------------------------------------
// gemm_sbatched
// ----------------------------------------------------------------------------

/**
@brief performs matrix-matrix multiplication over a batch of matrices 
       with strided memory access

Here, we use @c A[i], @c B[i], @c C[i] as notation 
for A, B and C matrices in the @c i-th instance of the batch, 
implicitly assuming they are respectively address offsets 
@c sA, @c sB, @c sC away from @c A[i-1], @c B[i-1], @c C[i-1].

The input matrices are in column-major storage.

@tparam T data type

@param handle cublas library handle
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
void cublas_gemm_sbatched(
  cublasHandle_t handle,
  cublasOperation_t ta, cublasOperation_t tb,
  int m, int n, int k,
  const T *alpha,
  const T *A, int lda, long long int sA,
  const T *B, int ldb, long long int sB,
  const T *beta,
  T *C, int ldc, long long int sC,
  int bc
) {
  cublasStatus_t stat;
  if constexpr(std::is_same_v<T, float>) {
    stat = cublasSgemmStridedBatched(handle,
      ta, tb, m, n, k, alpha, A, lda, sA, B, ldb, sB, beta, C, ldc, sC, bc
    );
  }
  else if constexpr(std::is_same_v<T, double>) {
    stat = cublasDgemmStridedBatched(handle,
      ta, tb, m, n, k, alpha, A, lda, sA, B, ldb, sB, beta, C, ldc, sC, bc
    );
  }
  else static_assert(dependent_false_v<T>, "unknown cublas data type");
  TF_CHECK_CUBLAS(stat, "failed to run gemm_sbatched");
}

/** 
@brief similar to tf::cublas_gemm_sbatached but operates on 
       C-styled row-major layout
*/
template <typename T>
void cublas_c_gemm_sbatched(
  cublasHandle_t handle,
  cublasOperation_t ta, cublasOperation_t tb,
  int m, int n, int k,
  const T *alpha,
  const T *A, int lda, long long int sA,
  const T *B, int ldb, long long int sB,
  const T *beta,
  T *C, int ldc, long long int sC,
  int bc
){
  cublasStatus_t stat;
  if constexpr(std::is_same_v<T, float>) {
    stat = cublasSgemmStridedBatched(handle,
      tb, ta, n, m, k, alpha, B, ldb, sB, A, lda, sA, beta, C, ldc, sC, bc
    );
  }
  else if constexpr(std::is_same_v<T, double>) {
    stat = cublasDgemmStridedBatched(handle,
      tb, ta, n, m, k, alpha, B, ldb, sB, A, lda, sA, beta, C, ldc, sC, bc
    );
  }
  else static_assert(dependent_false_v<T>, "unknown cublas data type");

  TF_CHECK_CUBLAS(stat, "failed to run c_gemm_sbatched");
}

// ---------------------------------------------------------------------------- 
// cublasFlowCapturere level-3 functions
// ---------------------------------------------------------------------------- 

// Function: geam
template <typename T>
cudaTask cublasFlowCapturer::geam(
  cublasOperation_t ta, cublasOperation_t tb,
  int m, int n,
  const T *alpha,
  const T *A, int lda,
  const T *beta,
  const T *B, int ldb,
  T *C, int ldc
) {
  return on([this, ta, tb, m, n, alpha, A, lda, beta, B, ldb, C, ldc] 
  (cudaStream_t stream) mutable {
    _stream(stream);
    cublasStatus_t stat;
    if constexpr(std::is_same_v<T, float>) {
      stat = cublasSgeam(_handle,
        ta, tb, m, n, alpha, A, lda, beta, B, ldb, C, ldc
      );
    }
    else if constexpr(std::is_same_v<T, double>) {
      stat = cublasDgeam(_handle,
        ta, tb, m, n, alpha, A, lda, beta, B, ldb, C, ldc
      );
    }
    else {
      static_assert(dependent_false_v<T>, "unknown cublas data type");
    }
    TF_CHECK_CUBLAS(stat, "failed to run geam");
  });
}

// Function: c_geam
template <typename T>
cudaTask cublasFlowCapturer::c_geam(
  cublasOperation_t ta, cublasOperation_t tb,
  int m, int n,
  const T *alpha,
  const T *A, int lda,
  const T *beta,
  const T *B, int ldb,
  T *C, int ldc
) {
  return on([this, ta, tb, m, n, alpha, A, lda, beta, B, ldb, C, ldc] 
  (cudaStream_t stream) mutable {
    _stream(stream);
    cublasStatus_t stat;
    if constexpr(std::is_same_v<T, float>) {
      stat = cublasSgeam(_handle,
        ta, tb, n, m, alpha, A, lda, beta, B, ldb, C, ldc
      );
    }
    else if constexpr(std::is_same_v<T, double>) {
      stat = cublasDgeam(_handle,
        ta, tb, n, m, alpha, A, lda, beta, B, ldb, C, ldc
      );
    }
    else {
      static_assert(dependent_false_v<T>, "unknown cublas data type");
    }
    TF_CHECK_CUBLAS(stat, "failed to run c_geam");
  });
}

// Function: gemm
template <typename T>
cudaTask cublasFlowCapturer::gemm(
  cublasOperation_t ta, cublasOperation_t tb,
  int m, int n, int k,
  const T *alpha,
  const T *A, int lda,
  const T *B, int ldb,
  const T *beta,
  T *C, int ldc
) {
  return on([this, ta, tb, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc] 
  (cudaStream_t stream) mutable {
    _stream(stream);
    cublasStatus_t stat;
    if constexpr(std::is_same_v<T, float>) {
      stat = cublasSgemm(_handle,
        ta, tb, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc
      );
    }
    else if constexpr(std::is_same_v<T, double>) {
      stat = cublasDgemm(_handle,
        ta, tb, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc
      );
    }
    else {
      static_assert(dependent_false_v<T>, "unknown cublas data type");
    }
    TF_CHECK_CUBLAS(stat, "failed to run gemm");
  });
}
    
template <typename T>
cudaTask cublasFlowCapturer::c_gemm(
  cublasOperation_t ta, cublasOperation_t tb,
  int m, int n, int k,
  const T *alpha,
  const T *A, int lda,
  const T *B, int ldb,
  const T *beta,
  T *C, int ldc
) {
  return on([this, ta, tb, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc] 
  (cudaStream_t stream) mutable {
    _stream(stream);
    cublasStatus_t stat;
    if constexpr(std::is_same_v<T, float>) {
      stat = cublasSgemm(_handle,
        tb, ta, n, m, k, alpha, B, ldb, A, lda, beta, C, ldc
      );
    }
    else if constexpr(std::is_same_v<T, double>) {
      stat = cublasDgemm(_handle,
        tb, ta, n, m, k, alpha, B, ldb, A, lda, beta, C, ldc
      );
    }
    else {
      static_assert(dependent_false_v<T>, "unknown cublas data type");
    }
    TF_CHECK_CUBLAS(stat, "failed to run c_gemm");
  });
}
    
// Function: gemm_batched
template <typename T>
cudaTask cublasFlowCapturer::gemm_batched(
  cublasOperation_t ta, cublasOperation_t tb,
  int m, int n, int k,
  const T *alpha,
  const T *A[], int lda,
  const T *B[], int ldb,
  const T *beta,
  T *C[], int ldc,
  int bc
) {
  return on([this, ta, tb, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc, bc] 
  (cudaStream_t stream) mutable {
    _stream(stream);
    cublasStatus_t stat;
    if constexpr(std::is_same_v<T, float>) {
      stat = cublasSgemmBatched(_handle,
        ta, tb, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc, bc
      );
    }
    else if constexpr(std::is_same_v<T, double>) {
      stat = cublasDgemmBatched(_handle,
        ta, tb, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc, bc
      );
    }
    else static_assert(dependent_false_v<T>, "unknown cublas data type");
    TF_CHECK_CUBLAS(stat, "failed to run gemm_batched");
  });
}

// Function: c_gemm_batched
template <typename T>
cudaTask cublasFlowCapturer::c_gemm_batched(
  cublasOperation_t ta, cublasOperation_t tb,
  int m, int n, int k,
  const T *alpha,
  const T *A[], int lda,
  const T *B[], int ldb,
  const T *beta,
  T *C[], int ldc,
  int bc
) {
  return on([this, ta, tb, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc, bc] 
  (cudaStream_t stream) mutable {
    _stream(stream);
    cublasStatus_t stat;
    if constexpr(std::is_same_v<T, float>) {
      stat = cublasSgemmBatched(_handle,
        tb, ta, n, m, k, alpha, B, ldb, A, lda, beta, C, ldc, bc
      );
    }
    else if constexpr(std::is_same_v<T, double>) {
      stat = cublasDgemmBatched(_handle,
        tb, ta, n, m, k, alpha, B, ldb, A, lda, beta, C, ldc, bc
      );
    }
    else static_assert(dependent_false_v<T>, "unknown cublas data type");
    TF_CHECK_CUBLAS(stat, "failed to run c_gemm_batched");
  });
}

// Function: gemm_sbatched (strided)    
template <typename T>
cudaTask cublasFlowCapturer::gemm_sbatched(
  cublasOperation_t ta, cublasOperation_t tb,
  int m, int n, int k,
  const T *alpha,
  const T *A, int lda, long long int sA,
  const T *B, int ldb, long long int sB,
  const T *beta,
  T *C, int ldc, long long int sC,
  int bc
) {
  return on([this, ta, tb, m, n, k, alpha, A, lda, sA, B, ldb, sB, beta, C, ldc, sC, bc] 
  (cudaStream_t stream) mutable {
    _stream(stream);
    cublasStatus_t stat;
    if constexpr(std::is_same_v<T, float>) {
      stat = cublasSgemmStridedBatched(_handle,
        ta, tb, m, n, k, alpha, A, lda, sA, B, ldb, sB, beta, C, ldc, sC, bc
      );
    }
    else if constexpr(std::is_same_v<T, double>) {
      stat = cublasDgemmStridedBatched(_handle,
        ta, tb, m, n, k, alpha, A, lda, sA, B, ldb, sB, beta, C, ldc, sC, bc
      );
    }
    else static_assert(dependent_false_v<T>, "unknown cublas data type");
    TF_CHECK_CUBLAS(stat, "failed to run gemm_sbatched");
  });
}

// Function: c_gemm_sbatched (strided)    
template <typename T>
cudaTask cublasFlowCapturer::c_gemm_sbatched(
  cublasOperation_t ta, cublasOperation_t tb,
  int m, int n, int k,
  const T *alpha,
  const T *A, int lda, long long int sA,
  const T *B, int ldb, long long int sB,
  const T *beta,
  T *C, int ldc, long long int sC,
  int bc
){
  return on(
  [this, ta, tb, m, n, k, alpha, A, lda, sA, B, ldb, sB, beta, C, ldc, sC, bc] 
  (cudaStream_t stream) mutable {
    _stream(stream);
    cublasStatus_t stat;
    if constexpr(std::is_same_v<T, float>) {
      stat = cublasSgemmStridedBatched(_handle,
        tb, ta, n, m, k, alpha, B, ldb, sB, A, lda, sA, beta, C, ldc, sC, bc
      );
    }
    else if constexpr(std::is_same_v<T, double>) {
      stat = cublasDgemmStridedBatched(_handle,
        tb, ta, n, m, k, alpha, B, ldb, sB, A, lda, sA, beta, C, ldc, sC, bc
      );
    }
    else static_assert(dependent_false_v<T>, "unknown cublas data type");
    TF_CHECK_CUBLAS(stat, "failed to run c_gemm_sbatched");
  });
}

}  // end of namespace tf -----------------------------------------------------

