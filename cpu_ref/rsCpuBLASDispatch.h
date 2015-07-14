#ifndef RS_COMPATIBILITY_LIB
#include "cblas.h"
#else
#include <dlfcn.h>
/*
 * ===========================================================================
 * Prototypes for level 2 BLAS
 * ===========================================================================
 */

/* 
 * Routines with standard 4 prefixes (S, D, C, Z)
 */
enum CBLAS_ORDER {CblasRowMajor=101, CblasColMajor=102};
enum CBLAS_TRANSPOSE {CblasNoTrans=111, CblasTrans=112, CblasConjTrans=113};
enum CBLAS_UPLO {CblasUpper=121, CblasLower=122};
enum CBLAS_DIAG {CblasNonUnit=131, CblasUnit=132};
enum CBLAS_SIDE {CblasLeft=141, CblasRight=142};

typedef void (*FnPtr_cblas_sgemv)(const enum CBLAS_ORDER order,
                                  const enum CBLAS_TRANSPOSE TransA, const int M, const int N,
                                  const float alpha, const float *A, const int lda,
                                  const float *X, const int incX, const float beta,
                                  float *Y, const int incY);
typedef void (*FnPtr_cblas_sgbmv)(const enum CBLAS_ORDER order,
                                  const enum CBLAS_TRANSPOSE TransA, const int M, const int N,
                                  const int KL, const int KU, const float alpha,
                                  const float *A, const int lda, const float *X,
                                  const int incX, const float beta, float *Y, const int incY);
typedef void (*FnPtr_cblas_strmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const float *A, const int lda, 
                                  float *X, const int incX);
typedef void (*FnPtr_cblas_stbmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const int K, const float *A, const int lda, 
                                  float *X, const int incX);
typedef void (*FnPtr_cblas_stpmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const float *Ap, float *X, const int incX);
typedef void (*FnPtr_cblas_strsv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const float *A, const int lda, float *X,
                                  const int incX);
typedef void (*FnPtr_cblas_stbsv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const int K, const float *A, const int lda,
                                  float *X, const int incX);
typedef void (*FnPtr_cblas_stpsv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const float *Ap, float *X, const int incX);

typedef void (*FnPtr_cblas_dgemv)(const enum CBLAS_ORDER order,
                                  const enum CBLAS_TRANSPOSE TransA, const int M, const int N,
                                  const double alpha, const double *A, const int lda,
                                  const double *X, const int incX, const double beta,
                                  double *Y, const int incY);
typedef void (*FnPtr_cblas_dgbmv)(const enum CBLAS_ORDER order,
                                  const enum CBLAS_TRANSPOSE TransA, const int M, const int N,
                                  const int KL, const int KU, const double alpha,
                                  const double *A, const int lda, const double *X,
                                  const int incX, const double beta, double *Y, const int incY);
typedef void (*FnPtr_cblas_dtrmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const double *A, const int lda, 
                                  double *X, const int incX);
typedef void (*FnPtr_cblas_dtbmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const int K, const double *A, const int lda, 
                                  double *X, const int incX);
typedef void (*FnPtr_cblas_dtpmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const double *Ap, double *X, const int incX);
typedef void (*FnPtr_cblas_dtrsv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const double *A, const int lda, double *X,
                                  const int incX);
typedef void (*FnPtr_cblas_dtbsv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const int K, const double *A, const int lda,
                                  double *X, const int incX);
typedef void (*FnPtr_cblas_dtpsv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const double *Ap, double *X, const int incX);

typedef void (*FnPtr_cblas_cgemv)(const enum CBLAS_ORDER order,
                                  const enum CBLAS_TRANSPOSE TransA, const int M, const int N,
                                  const void *alpha, const void *A, const int lda,
                                  const void *X, const int incX, const void *beta,
                                  void *Y, const int incY);
typedef void (*FnPtr_cblas_cgbmv)(const enum CBLAS_ORDER order,
                                  const enum CBLAS_TRANSPOSE TransA, const int M, const int N,
                                  const int KL, const int KU, const void *alpha,
                                  const void *A, const int lda, const void *X,
                                  const int incX, const void *beta, void *Y, const int incY);
typedef void (*FnPtr_cblas_ctrmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const void *A, const int lda, 
                                  void *X, const int incX);
typedef void (*FnPtr_cblas_ctbmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const int K, const void *A, const int lda, 
                                  void *X, const int incX);
typedef void (*FnPtr_cblas_ctpmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const void *Ap, void *X, const int incX);
typedef void (*FnPtr_cblas_ctrsv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const void *A, const int lda, void *X,
                                  const int incX);
typedef void (*FnPtr_cblas_ctbsv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const int K, const void *A, const int lda,
                                  void *X, const int incX);
typedef void (*FnPtr_cblas_ctpsv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const void *Ap, void *X, const int incX);

typedef void (*FnPtr_cblas_zgemv)(const enum CBLAS_ORDER order,
                                  const enum CBLAS_TRANSPOSE TransA, const int M, const int N,
                                  const void *alpha, const void *A, const int lda,
                                  const void *X, const int incX, const void *beta,
                                  void *Y, const int incY);
typedef void (*FnPtr_cblas_zgbmv)(const enum CBLAS_ORDER order,
                                  const enum CBLAS_TRANSPOSE TransA, const int M, const int N,
                                  const int KL, const int KU, const void *alpha,
                                  const void *A, const int lda, const void *X,
                                  const int incX, const void *beta, void *Y, const int incY);
typedef void (*FnPtr_cblas_ztrmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const void *A, const int lda, 
                                  void *X, const int incX);
typedef void (*FnPtr_cblas_ztbmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const int K, const void *A, const int lda, 
                                  void *X, const int incX);
typedef void (*FnPtr_cblas_ztpmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const void *Ap, void *X, const int incX);
typedef void (*FnPtr_cblas_ztrsv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const void *A, const int lda, void *X,
                                  const int incX);
typedef void (*FnPtr_cblas_ztbsv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const int K, const void *A, const int lda,
                                  void *X, const int incX);
typedef void (*FnPtr_cblas_ztpsv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const void *Ap, void *X, const int incX);


/* 
 * Routines with S and D prefixes only
 */
typedef void (*FnPtr_cblas_ssymv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const float alpha, const float *A,
                                  const int lda, const float *X, const int incX,
                                  const float beta, float *Y, const int incY);
typedef void (*FnPtr_cblas_ssbmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const int K, const float alpha, const float *A,
                                  const int lda, const float *X, const int incX,
                                  const float beta, float *Y, const int incY);
typedef void (*FnPtr_cblas_sspmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const float alpha, const float *Ap,
                                  const float *X, const int incX,
                                  const float beta, float *Y, const int incY);
typedef void (*FnPtr_cblas_sger)(const enum CBLAS_ORDER order, const int M, const int N,
                                 const float alpha, const float *X, const int incX,
                                 const float *Y, const int incY, float *A, const int lda);
typedef void (*FnPtr_cblas_ssyr)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                 const int N, const float alpha, const float *X,
                                 const int incX, float *A, const int lda);
typedef void (*FnPtr_cblas_sspr)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                 const int N, const float alpha, const float *X,
                                 const int incX, float *Ap);
typedef void (*FnPtr_cblas_ssyr2)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const float alpha, const float *X,
                                  const int incX, const float *Y, const int incY, float *A,
                                  const int lda);
typedef void (*FnPtr_cblas_sspr2)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const float alpha, const float *X,
                                  const int incX, const float *Y, const int incY, float *A);

typedef void (*FnPtr_cblas_dsymv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const double alpha, const double *A,
                                  const int lda, const double *X, const int incX,
                                  const double beta, double *Y, const int incY);
typedef void (*FnPtr_cblas_dsbmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const int K, const double alpha, const double *A,
                                  const int lda, const double *X, const int incX,
                                  const double beta, double *Y, const int incY);
typedef void (*FnPtr_cblas_dspmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const double alpha, const double *Ap,
                                  const double *X, const int incX,
                                  const double beta, double *Y, const int incY);
typedef void (*FnPtr_cblas_dger)(const enum CBLAS_ORDER order, const int M, const int N,
                                 const double alpha, const double *X, const int incX,
                                 const double *Y, const int incY, double *A, const int lda);
typedef void (*FnPtr_cblas_dsyr)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                 const int N, const double alpha, const double *X,
                                 const int incX, double *A, const int lda);
typedef void (*FnPtr_cblas_dspr)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                 const int N, const double alpha, const double *X,
                                 const int incX, double *Ap);
typedef void (*FnPtr_cblas_dsyr2)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const double alpha, const double *X,
                                  const int incX, const double *Y, const int incY, double *A,
                                  const int lda);
typedef void (*FnPtr_cblas_dspr2)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const double alpha, const double *X,
                                  const int incX, const double *Y, const int incY, double *A);


/* 
 * Routines with C and Z prefixes only
 */
typedef void (*FnPtr_cblas_chemv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const void *alpha, const void *A,
                                  const int lda, const void *X, const int incX,
                                  const void *beta, void *Y, const int incY);
typedef void (*FnPtr_cblas_chbmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const int K, const void *alpha, const void *A,
                                  const int lda, const void *X, const int incX,
                                  const void *beta, void *Y, const int incY);
typedef void (*FnPtr_cblas_chpmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const void *alpha, const void *Ap,
                                  const void *X, const int incX,
                                  const void *beta, void *Y, const int incY);
typedef void (*FnPtr_cblas_cgeru)(const enum CBLAS_ORDER order, const int M, const int N,
                                  const void *alpha, const void *X, const int incX,
                                  const void *Y, const int incY, void *A, const int lda);
typedef void (*FnPtr_cblas_cgerc)(const enum CBLAS_ORDER order, const int M, const int N,
                                  const void *alpha, const void *X, const int incX,
                                  const void *Y, const int incY, void *A, const int lda);
typedef void (*FnPtr_cblas_cher)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                 const int N, const float alpha, const void *X, const int incX,
                                 void *A, const int lda);
typedef void (*FnPtr_cblas_chpr)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                 const int N, const float alpha, const void *X,
                                 const int incX, void *A);
typedef void (*FnPtr_cblas_cher2)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo, const int N,
                                  const void *alpha, const void *X, const int incX,
                                  const void *Y, const int incY, void *A, const int lda);
typedef void (*FnPtr_cblas_chpr2)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo, const int N,
                                  const void *alpha, const void *X, const int incX,
                                  const void *Y, const int incY, void *Ap);

typedef void (*FnPtr_cblas_zhemv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const void *alpha, const void *A,
                                  const int lda, const void *X, const int incX,
                                  const void *beta, void *Y, const int incY);
typedef void (*FnPtr_cblas_zhbmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const int K, const void *alpha, const void *A,
                                  const int lda, const void *X, const int incX,
                                  const void *beta, void *Y, const int incY);
typedef void (*FnPtr_cblas_zhpmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const void *alpha, const void *Ap,
                                  const void *X, const int incX,
                                  const void *beta, void *Y, const int incY);
typedef void (*FnPtr_cblas_zgeru)(const enum CBLAS_ORDER order, const int M, const int N,
                                  const void *alpha, const void *X, const int incX,
                                  const void *Y, const int incY, void *A, const int lda);
typedef void (*FnPtr_cblas_zgerc)(const enum CBLAS_ORDER order, const int M, const int N,
                                  const void *alpha, const void *X, const int incX,
                                  const void *Y, const int incY, void *A, const int lda);
typedef void (*FnPtr_cblas_zher)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                 const int N, const double alpha, const void *X, const int incX,
                                 void *A, const int lda);
typedef void (*FnPtr_cblas_zhpr)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                 const int N, const double alpha, const void *X,
                                 const int incX, void *A);
typedef void (*FnPtr_cblas_zher2)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo, const int N,
                                  const void *alpha, const void *X, const int incX,
                                  const void *Y, const int incY, void *A, const int lda);
typedef void (*FnPtr_cblas_zhpr2)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo, const int N,
                                  const void *alpha, const void *X, const int incX,
                                  const void *Y, const int incY, void *Ap);

/*
 * ===========================================================================
 * Prototypes for level 3 BLAS
 * ===========================================================================
 */

/* 
 * Routines with standard 4 prefixes (S, D, C, Z)
 */
typedef void (*FnPtr_cblas_sgemm)(const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA,
                                  const enum CBLAS_TRANSPOSE TransB, const int M, const int N,
                                  const int K, const float alpha, const float *A,
                                  const int lda, const float *B, const int ldb,
                                  const float beta, float *C, const int ldc);
typedef void (*FnPtr_cblas_ssymm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const int M, const int N,
                                  const float alpha, const float *A, const int lda,
                                  const float *B, const int ldb, const float beta,
                                  float *C, const int ldc);
typedef void (*FnPtr_cblas_ssyrk)(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
                                  const float alpha, const float *A, const int lda,
                                  const float beta, float *C, const int ldc);
typedef void (*FnPtr_cblas_ssyr2k)(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
                                   const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
                                   const float alpha, const float *A, const int lda,
                                   const float *B, const int ldb, const float beta,
                                   float *C, const int ldc);
typedef void (*FnPtr_cblas_strmm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const enum CBLAS_TRANSPOSE TransA,
                                  const enum CBLAS_DIAG Diag, const int M, const int N,
                                  const float alpha, const float *A, const int lda,
                                  float *B, const int ldb);
typedef void (*FnPtr_cblas_strsm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const enum CBLAS_TRANSPOSE TransA,
                                  const enum CBLAS_DIAG Diag, const int M, const int N,
                                  const float alpha, const float *A, const int lda,
                                  float *B, const int ldb);

typedef void (*FnPtr_cblas_dgemm)(const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA,
                                  const enum CBLAS_TRANSPOSE TransB, const int M, const int N,
                                  const int K, const double alpha, const double *A,
                                  const int lda, const double *B, const int ldb,
                                  const double beta, double *C, const int ldc);
typedef void (*FnPtr_cblas_dsymm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const int M, const int N,
                                  const double alpha, const double *A, const int lda,
                                  const double *B, const int ldb, const double beta,
                                  double *C, const int ldc);
typedef void (*FnPtr_cblas_dsyrk)(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
                                  const double alpha, const double *A, const int lda,
                                  const double beta, double *C, const int ldc);
typedef void (*FnPtr_cblas_dsyr2k)(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
                                   const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
                                   const double alpha, const double *A, const int lda,
                                   const double *B, const int ldb, const double beta,
                                   double *C, const int ldc);
typedef void (*FnPtr_cblas_dtrmm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const enum CBLAS_TRANSPOSE TransA,
                                  const enum CBLAS_DIAG Diag, const int M, const int N,
                                  const double alpha, const double *A, const int lda,
                                  double *B, const int ldb);
typedef void (*FnPtr_cblas_dtrsm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const enum CBLAS_TRANSPOSE TransA,
                                  const enum CBLAS_DIAG Diag, const int M, const int N,
                                  const double alpha, const double *A, const int lda,
                                  double *B, const int ldb);

typedef void (*FnPtr_cblas_cgemm)(const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA,
                                  const enum CBLAS_TRANSPOSE TransB, const int M, const int N,
                                  const int K, const void *alpha, const void *A,
                                  const int lda, const void *B, const int ldb,
                                  const void *beta, void *C, const int ldc);
typedef void (*FnPtr_cblas_csymm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const int M, const int N,
                                  const void *alpha, const void *A, const int lda,
                                  const void *B, const int ldb, const void *beta,
                                  void *C, const int ldc);
typedef void (*FnPtr_cblas_csyrk)(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
                                  const void *alpha, const void *A, const int lda,
                                  const void *beta, void *C, const int ldc);
typedef void (*FnPtr_cblas_csyr2k)(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
                                   const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
                                   const void *alpha, const void *A, const int lda,
                                   const void *B, const int ldb, const void *beta,
                                   void *C, const int ldc);
typedef void (*FnPtr_cblas_ctrmm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const enum CBLAS_TRANSPOSE TransA,
                                  const enum CBLAS_DIAG Diag, const int M, const int N,
                                  const void *alpha, const void *A, const int lda,
                                  void *B, const int ldb);
typedef void (*FnPtr_cblas_ctrsm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const enum CBLAS_TRANSPOSE TransA,
                                  const enum CBLAS_DIAG Diag, const int M, const int N,
                                  const void *alpha, const void *A, const int lda,
                                  void *B, const int ldb);

typedef void (*FnPtr_cblas_zgemm)(const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA,
                                  const enum CBLAS_TRANSPOSE TransB, const int M, const int N,
                                  const int K, const void *alpha, const void *A,
                                  const int lda, const void *B, const int ldb,
                                  const void *beta, void *C, const int ldc);
typedef void (*FnPtr_cblas_zsymm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const int M, const int N,
                                  const void *alpha, const void *A, const int lda,
                                  const void *B, const int ldb, const void *beta,
                                  void *C, const int ldc);
typedef void (*FnPtr_cblas_zsyrk)(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
                                  const void *alpha, const void *A, const int lda,
                                  const void *beta, void *C, const int ldc);
typedef void (*FnPtr_cblas_zsyr2k)(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
                                   const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
                                   const void *alpha, const void *A, const int lda,
                                   const void *B, const int ldb, const void *beta,
                                   void *C, const int ldc);
typedef void (*FnPtr_cblas_ztrmm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const enum CBLAS_TRANSPOSE TransA,
                                  const enum CBLAS_DIAG Diag, const int M, const int N,
                                  const void *alpha, const void *A, const int lda,
                                  void *B, const int ldb);
typedef void (*FnPtr_cblas_ztrsm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const enum CBLAS_TRANSPOSE TransA,
                                  const enum CBLAS_DIAG Diag, const int M, const int N,
                                  const void *alpha, const void *A, const int lda,
                                  void *B, const int ldb);


/* 
 * Routines with prefixes C and Z only
 */
typedef void (*FnPtr_cblas_chemm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const int M, const int N,
                                  const void *alpha, const void *A, const int lda,
                                  const void *B, const int ldb, const void *beta,
                                  void *C, const int ldc);
typedef void (*FnPtr_cblas_cherk)(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
                                  const float alpha, const void *A, const int lda,
                                  const float beta, void *C, const int ldc);
typedef void (*FnPtr_cblas_cher2k)(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
                                   const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
                                   const void *alpha, const void *A, const int lda,
                                   const void *B, const int ldb, const float beta,
                                   void *C, const int ldc);

typedef void (*FnPtr_cblas_zhemm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const int M, const int N,
                                  const void *alpha, const void *A, const int lda,
                                  const void *B, const int ldb, const void *beta,
                                  void *C, const int ldc);
typedef void (*FnPtr_cblas_zherk)(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
                                  const double alpha, const void *A, const int lda,
                                  const double beta, void *C, const int ldc);
typedef void (*FnPtr_cblas_zher2k)(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
                                   const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
                                   const void *alpha, const void *A, const int lda,
                                   const void *B, const int ldb, const double beta,
                                   void *C, const int ldc);



/*
 * ===========================================================================
 * Prototypes for level 2 BLAS
 * ===========================================================================
 */

/* 
 * Routines with standard 4 prefixes (S, D, C, Z)
 */
FnPtr_cblas_sgemv cblas_sgemv;
FnPtr_cblas_sgbmv cblas_sgbmv;
FnPtr_cblas_strmv cblas_strmv;
FnPtr_cblas_stbmv cblas_stbmv;
FnPtr_cblas_stpmv cblas_stpmv;
FnPtr_cblas_strsv cblas_strsv;
FnPtr_cblas_stbsv cblas_stbsv;
FnPtr_cblas_stpsv cblas_stpsv;

FnPtr_cblas_dgemv cblas_dgemv;
FnPtr_cblas_dgbmv cblas_dgbmv;
FnPtr_cblas_dtrmv cblas_dtrmv;
FnPtr_cblas_dtbmv cblas_dtbmv;
FnPtr_cblas_dtpmv cblas_dtpmv;
FnPtr_cblas_dtrsv cblas_dtrsv;
FnPtr_cblas_dtbsv cblas_dtbsv;
FnPtr_cblas_dtpsv cblas_dtpsv;

FnPtr_cblas_cgemv cblas_cgemv;
FnPtr_cblas_cgbmv cblas_cgbmv;
FnPtr_cblas_ctrmv cblas_ctrmv;
FnPtr_cblas_ctbmv cblas_ctbmv;
FnPtr_cblas_ctpmv cblas_ctpmv;
FnPtr_cblas_ctrsv cblas_ctrsv;
FnPtr_cblas_ctbsv cblas_ctbsv;
FnPtr_cblas_ctpsv cblas_ctpsv;

FnPtr_cblas_zgemv cblas_zgemv;
FnPtr_cblas_zgbmv cblas_zgbmv;
FnPtr_cblas_ztrmv cblas_ztrmv;
FnPtr_cblas_ztbmv cblas_ztbmv;
FnPtr_cblas_ztpmv cblas_ztpmv;
FnPtr_cblas_ztrsv cblas_ztrsv;
FnPtr_cblas_ztbsv cblas_ztbsv;
FnPtr_cblas_ztpsv cblas_ztpsv;

/* 
 * Routines with S and D prefixes only
 */
FnPtr_cblas_ssymv cblas_ssymv;
FnPtr_cblas_ssbmv cblas_ssbmv;
FnPtr_cblas_sspmv cblas_sspmv;
FnPtr_cblas_sger cblas_sger;
FnPtr_cblas_ssyr cblas_ssyr;
FnPtr_cblas_sspr cblas_sspr;
FnPtr_cblas_ssyr2 cblas_ssyr2;
FnPtr_cblas_sspr2 cblas_sspr2;

FnPtr_cblas_dsymv cblas_dsymv;
FnPtr_cblas_dsbmv cblas_dsbmv;
FnPtr_cblas_dspmv cblas_dspmv;
FnPtr_cblas_dger cblas_dger;
FnPtr_cblas_dsyr cblas_dsyr;
FnPtr_cblas_dspr cblas_dspr;
FnPtr_cblas_dsyr2 cblas_dsyr2;
FnPtr_cblas_dspr2 cblas_dspr2;


/* 
 * Routines with C and Z prefixes only
 */
FnPtr_cblas_chemv cblas_chemv;
FnPtr_cblas_chbmv cblas_chbmv;
FnPtr_cblas_chpmv cblas_chpmv;
FnPtr_cblas_cgeru cblas_cgeru;
FnPtr_cblas_cgerc cblas_cgerc;
FnPtr_cblas_cher cblas_cher;
FnPtr_cblas_chpr cblas_chpr;
FnPtr_cblas_cher2 cblas_cher2;
FnPtr_cblas_chpr2 cblas_chpr2;

FnPtr_cblas_zhemv cblas_zhemv;
FnPtr_cblas_zhbmv cblas_zhbmv;
FnPtr_cblas_zhpmv cblas_zhpmv;
FnPtr_cblas_zgeru cblas_zgeru;
FnPtr_cblas_zgerc cblas_zgerc;
FnPtr_cblas_zher cblas_zher;
FnPtr_cblas_zhpr cblas_zhpr;
FnPtr_cblas_zher2 cblas_zher2;
FnPtr_cblas_zhpr2 cblas_zhpr2;

/*
 * ===========================================================================
 * Prototypes for level 3 BLAS
 * ===========================================================================
 */

/* 
 * Routines with standard 4 prefixes (S, D, C, Z)
 */
FnPtr_cblas_sgemm cblas_sgemm;
FnPtr_cblas_ssymm cblas_ssymm;
FnPtr_cblas_ssyrk cblas_ssyrk;
FnPtr_cblas_ssyr2k cblas_ssyr2k;
FnPtr_cblas_strmm cblas_strmm;
FnPtr_cblas_strsm cblas_strsm;

FnPtr_cblas_dgemm cblas_dgemm;
FnPtr_cblas_dsymm cblas_dsymm;
FnPtr_cblas_dsyrk cblas_dsyrk;
FnPtr_cblas_dsyr2k cblas_dsyr2k;
FnPtr_cblas_dtrmm cblas_dtrmm;
FnPtr_cblas_dtrsm cblas_dtrsm;

FnPtr_cblas_cgemm cblas_cgemm;
FnPtr_cblas_csymm cblas_csymm;
FnPtr_cblas_csyrk cblas_csyrk;
FnPtr_cblas_csyr2k cblas_csyr2k;
FnPtr_cblas_ctrmm cblas_ctrmm;
FnPtr_cblas_ctrsm cblas_ctrsm;

FnPtr_cblas_zgemm cblas_zgemm;
FnPtr_cblas_zsymm cblas_zsymm;
FnPtr_cblas_zsyrk cblas_zsyrk;
FnPtr_cblas_zsyr2k cblas_zsyr2k;
FnPtr_cblas_ztrmm cblas_ztrmm;
FnPtr_cblas_ztrsm cblas_ztrsm;


/* 
 * Routines with prefixes C and Z only
 */
FnPtr_cblas_chemm cblas_chemm;
FnPtr_cblas_cherk cblas_cherk;
FnPtr_cblas_cher2k cblas_cher2k;

FnPtr_cblas_zhemm cblas_zhemm;
FnPtr_cblas_zherk cblas_zherk;
FnPtr_cblas_zher2k cblas_zher2k;


bool loadBLASLib() {
    void* handle = NULL;
    handle = dlopen("libblasV8.so", RTLD_LAZY | RTLD_LOCAL);

    if (handle == NULL) {
        return false;
    }
    //load the function pointers
    cblas_sgemv = (FnPtr_cblas_sgemv)dlsym(handle, "cblas_sgemv");
    if (cblas_sgemv == nullptr) {
        return false;
    }
    cblas_sgbmv = (FnPtr_cblas_sgbmv)dlsym(handle, "cblas_sgbmv");
    if (cblas_sgbmv == nullptr) {
        return false;
    }
    cblas_strmv = (FnPtr_cblas_strmv)dlsym(handle, "cblas_strmv");
    if (cblas_strmv == nullptr) {
        return false;
    }
    cblas_stbmv = (FnPtr_cblas_stbmv)dlsym(handle, "cblas_stbmv");
    if (cblas_stbmv == nullptr) {
        return false;
    }
    cblas_stpmv = (FnPtr_cblas_stpmv)dlsym(handle, "cblas_stpmv");
    if (cblas_stpmv == nullptr) {
        return false;
    }
    cblas_strsv = (FnPtr_cblas_strsv)dlsym(handle, "cblas_strsv");
    if (cblas_strsv == nullptr) {
        return false;
    }
    cblas_stbsv = (FnPtr_cblas_stbsv)dlsym(handle, "cblas_stbsv");
    if (cblas_stbsv == nullptr) {
        return false;
    }
    cblas_stpsv = (FnPtr_cblas_stpsv)dlsym(handle, "cblas_stpsv");
    if (cblas_stpsv == nullptr) {
        return false;
    }

    cblas_dgemv = (FnPtr_cblas_dgemv)dlsym(handle, "cblas_dgemv");
    if (cblas_dgemv == nullptr) {
        return false;
    }
    cblas_dgbmv = (FnPtr_cblas_dgbmv)dlsym(handle, "cblas_dgbmv");
    if (cblas_dgbmv == nullptr) {
        return false;
    }
    cblas_dtrmv = (FnPtr_cblas_dtrmv)dlsym(handle, "cblas_dtrmv");
    if (cblas_dtrmv == nullptr) {
        return false;
    }
    cblas_dtbmv = (FnPtr_cblas_dtbmv)dlsym(handle, "cblas_dtbmv");
    if (cblas_dtbmv == nullptr) {
        return false;
    }
    cblas_dtpmv = (FnPtr_cblas_dtpmv)dlsym(handle, "cblas_dtpmv");
    if (cblas_dtpmv == nullptr) {
        return false;
    }
    cblas_dtrsv = (FnPtr_cblas_dtrsv)dlsym(handle, "cblas_dtrsv");
    if (cblas_dtrsv == nullptr) {
        return false;
    }
    cblas_dtbsv = (FnPtr_cblas_dtbsv)dlsym(handle, "cblas_dtbsv");
    if (cblas_dtbsv == nullptr) {
        return false;
    }
    cblas_dtpsv = (FnPtr_cblas_dtpsv)dlsym(handle, "cblas_dtpsv");
    if (cblas_dtpsv == nullptr) {
        return false;
    }

    cblas_cgemv = (FnPtr_cblas_cgemv)dlsym(handle, "cblas_cgemv");
    if (cblas_cgemv == nullptr) {
        return false;
    }
    cblas_cgbmv = (FnPtr_cblas_cgbmv)dlsym(handle, "cblas_cgbmv");
    if (cblas_cgbmv == nullptr) {
        return false;
    }
    cblas_ctrmv = (FnPtr_cblas_ctrmv)dlsym(handle, "cblas_ctrmv");
    if (cblas_ctrmv == nullptr) {
        return false;
    }
    cblas_ctbmv = (FnPtr_cblas_ctbmv)dlsym(handle, "cblas_ctbmv");
    if (cblas_ctbmv == nullptr) {
        return false;
    }
    cblas_ctpmv = (FnPtr_cblas_ctpmv)dlsym(handle, "cblas_ctpmv");
    if (cblas_ctpmv == nullptr) {
        return false;
    }
    cblas_ctrsv = (FnPtr_cblas_ctrsv)dlsym(handle, "cblas_ctrsv");
    if (cblas_ctrsv == nullptr) {
        return false;
    }
    cblas_ctbsv = (FnPtr_cblas_ctbsv)dlsym(handle, "cblas_ctbsv");
    if (cblas_ctbsv == nullptr) {
        return false;
    }
    cblas_ctpsv = (FnPtr_cblas_ctpsv)dlsym(handle, "cblas_ctpsv");
    if (cblas_ctpsv == nullptr) {
        return false;
    }

    cblas_zgemv = (FnPtr_cblas_zgemv)dlsym(handle, "cblas_zgemv");
    if (cblas_zgemv == nullptr) {
        return false;
    }
    cblas_zgbmv = (FnPtr_cblas_zgbmv)dlsym(handle, "cblas_zgbmv");
    if (cblas_zgbmv == nullptr) {
        return false;
    }
    cblas_ztrmv = (FnPtr_cblas_ztrmv)dlsym(handle, "cblas_ztrmv");
    if (cblas_ztrmv == nullptr) {
        return false;
    }
    cblas_ztbmv = (FnPtr_cblas_ztbmv)dlsym(handle, "cblas_ztbmv");
    if (cblas_ztbmv == nullptr) {
        return false;
    }
    cblas_ztpmv = (FnPtr_cblas_ztpmv)dlsym(handle, "cblas_ztpmv");
    if (cblas_ztpmv == nullptr) {
        return false;
    }
    cblas_ztrsv = (FnPtr_cblas_ztrsv)dlsym(handle, "cblas_ztrsv");
    if (cblas_ztrsv == nullptr) {
        return false;
    }
    cblas_ztbsv = (FnPtr_cblas_ztbsv)dlsym(handle, "cblas_ztbsv");
    if (cblas_ztbsv == nullptr) {
        return false;
    }
    cblas_ztpsv = (FnPtr_cblas_ztpsv)dlsym(handle, "cblas_ztpsv");
    if (cblas_ztpsv == nullptr) {
        return false;
    }

    cblas_ssymv = (FnPtr_cblas_ssymv)dlsym(handle, "cblas_ssymv");
    if (cblas_ssymv == nullptr) {
        return false;
    }
    cblas_ssbmv = (FnPtr_cblas_ssbmv)dlsym(handle, "cblas_ssbmv");
    if (cblas_ssbmv == nullptr) {
        return false;
    }
    cblas_sspmv = (FnPtr_cblas_sspmv)dlsym(handle, "cblas_sspmv");
    if (cblas_sspmv == nullptr) {
        return false;
    }
    cblas_sger = (FnPtr_cblas_sger)dlsym(handle, "cblas_sger");
    if (cblas_sger == nullptr) {
        return false;
    }
    cblas_ssyr = (FnPtr_cblas_ssyr)dlsym(handle, "cblas_ssyr");
    if (cblas_ssyr == nullptr) {
        return false;
    }
    cblas_sspr = (FnPtr_cblas_sspr)dlsym(handle, "cblas_sspr");
    if (cblas_sspr == nullptr) {
        return false;
    }
    cblas_ssyr2 = (FnPtr_cblas_ssyr2)dlsym(handle, "cblas_ssyr2");
    if (cblas_ssyr2 == nullptr) {
        return false;
    }
    cblas_sspr2 = (FnPtr_cblas_sspr2)dlsym(handle, "cblas_sspr2");
    if (cblas_sspr2 == nullptr) {
        return false;
    }

    cblas_dsymv = (FnPtr_cblas_dsymv)dlsym(handle, "cblas_dsymv");
    if (cblas_dsymv == nullptr) {
        return false;
    }
    cblas_dsbmv = (FnPtr_cblas_dsbmv)dlsym(handle, "cblas_dsbmv");
    if (cblas_dsbmv == nullptr) {
        return false;
    }
    cblas_dspmv = (FnPtr_cblas_dspmv)dlsym(handle, "cblas_dspmv");
    if (cblas_dspmv == nullptr) {
        return false;
    }
    cblas_dger = (FnPtr_cblas_dger)dlsym(handle, "cblas_dger");
    if (cblas_dger == nullptr) {
        return false;
    }
    cblas_dsyr = (FnPtr_cblas_dsyr)dlsym(handle, "cblas_dsyr");
    if (cblas_dsyr == nullptr) {
        return false;
    }
    cblas_dspr = (FnPtr_cblas_dspr)dlsym(handle, "cblas_dspr");
    if (cblas_dspr == nullptr) {
        return false;
    }
    cblas_dsyr2 = (FnPtr_cblas_dsyr2)dlsym(handle, "cblas_dsyr2");
    if (cblas_dsyr2 == nullptr) {
        return false;
    }
    cblas_dspr2 = (FnPtr_cblas_dspr2)dlsym(handle, "cblas_dspr2");
    if (cblas_dspr2 == nullptr) {
        return false;
    }

    cblas_chemv = (FnPtr_cblas_chemv)dlsym(handle, "cblas_chemv");
    if (cblas_chemv == nullptr) {
        return false;
    }
    cblas_chbmv = (FnPtr_cblas_chbmv)dlsym(handle, "cblas_chbmv");
    if (cblas_chbmv == nullptr) {
        return false;
    }
    cblas_chpmv = (FnPtr_cblas_chpmv)dlsym(handle, "cblas_chpmv");
    if (cblas_chpmv == nullptr) {
        return false;
    }
    cblas_cgeru = (FnPtr_cblas_cgeru)dlsym(handle, "cblas_cgeru");
    if (cblas_cgeru == nullptr) {
        return false;
    }
    cblas_cgerc = (FnPtr_cblas_cgerc)dlsym(handle, "cblas_cgerc");
    if (cblas_cgerc == nullptr) {
        return false;
    }
    cblas_cher = (FnPtr_cblas_cher)dlsym(handle, "cblas_cher");
    if (cblas_cher == nullptr) {
        return false;
    }
    cblas_chpr = (FnPtr_cblas_chpr)dlsym(handle, "cblas_chpr");
    if (cblas_chpr == nullptr) {
        return false;
    }
    cblas_cher2 = (FnPtr_cblas_cher2)dlsym(handle, "cblas_cher2");
    if (cblas_cher2 == nullptr) {
        return false;
    }
    cblas_chpr2 = (FnPtr_cblas_chpr2)dlsym(handle, "cblas_chpr2");
    if (cblas_chpr2 == nullptr) {
        return false;
    }

    cblas_zhemv = (FnPtr_cblas_zhemv)dlsym(handle, "cblas_zhemv");
    if (cblas_zhemv == nullptr) {
        return false;
    }
    cblas_zhbmv = (FnPtr_cblas_zhbmv)dlsym(handle, "cblas_zhbmv");
    if (cblas_zhbmv == nullptr) {
        return false;
    }
    cblas_zhpmv = (FnPtr_cblas_zhpmv)dlsym(handle, "cblas_zhpmv");
    if (cblas_zhpmv == nullptr) {
        return false;
    }
    cblas_zgeru = (FnPtr_cblas_zgeru)dlsym(handle, "cblas_zgeru");
    if (cblas_zgeru == nullptr) {
        return false;
    }
    cblas_zgerc = (FnPtr_cblas_zgerc)dlsym(handle, "cblas_zgerc");
    if (cblas_zgerc == nullptr) {
        return false;
    }
    cblas_zher = (FnPtr_cblas_zher)dlsym(handle, "cblas_zher");
    if (cblas_zher == nullptr) {
        return false;
    }
    cblas_zhpr = (FnPtr_cblas_zhpr)dlsym(handle, "cblas_zhpr");
    if (cblas_zhpr == nullptr) {
        return false;
    }
    cblas_zher2 = (FnPtr_cblas_zher2)dlsym(handle, "cblas_zher2");
    if (cblas_zher2 == nullptr) {
        return false;
    }
    cblas_zhpr2 = (FnPtr_cblas_zhpr2)dlsym(handle, "cblas_zhpr2");
    if (cblas_zhpr2 == nullptr) {
        return false;
    }

    cblas_sgemm = (FnPtr_cblas_sgemm)dlsym(handle, "cblas_sgemm");
    if (cblas_sgemm == nullptr) {
        return false;
    }
    cblas_ssymm = (FnPtr_cblas_ssymm)dlsym(handle, "cblas_ssymm");
    if (cblas_ssymm == nullptr) {
        return false;
    }
    cblas_ssyrk = (FnPtr_cblas_ssyrk)dlsym(handle, "cblas_ssyrk");
    if (cblas_ssyrk == nullptr) {
        return false;
    }
    cblas_ssyr2k = (FnPtr_cblas_ssyr2k)dlsym(handle, "cblas_ssyr2k");
    if (cblas_ssyr2k == nullptr) {
        return false;
    }
    cblas_strmm = (FnPtr_cblas_strmm)dlsym(handle, "cblas_strmm");
    if (cblas_strmm == nullptr) {
        return false;
    }
    cblas_strsm = (FnPtr_cblas_strsm)dlsym(handle, "cblas_strsm");
    if (cblas_strsm == nullptr) {
        return false;
    }

    cblas_dgemm = (FnPtr_cblas_dgemm)dlsym(handle, "cblas_dgemm");
    if (cblas_dgemm == nullptr) {
        return false;
    }
    cblas_dsymm = (FnPtr_cblas_dsymm)dlsym(handle, "cblas_dsymm");
    if (cblas_dsymm == nullptr) {
        return false;
    }
    cblas_dsyrk = (FnPtr_cblas_dsyrk)dlsym(handle, "cblas_dsyrk");
    if (cblas_dsyrk == nullptr) {
        return false;
    }
    cblas_dsyr2k = (FnPtr_cblas_dsyr2k)dlsym(handle, "cblas_dsyr2k");
    if (cblas_dsyr2k == nullptr) {
        return false;
    }
    cblas_dtrmm = (FnPtr_cblas_dtrmm)dlsym(handle, "cblas_dtrmm");
    if (cblas_dtrmm == nullptr) {
        return false;
    }
    cblas_dtrsm = (FnPtr_cblas_dtrsm)dlsym(handle, "cblas_dtrsm");
    if (cblas_dtrsm == nullptr) {
        return false;
    }

    cblas_cgemm = (FnPtr_cblas_cgemm)dlsym(handle, "cblas_cgemm");
    if (cblas_cgemm == nullptr) {
        return false;
    }
    cblas_csymm = (FnPtr_cblas_csymm)dlsym(handle, "cblas_csymm");
    if (cblas_csymm == nullptr) {
        return false;
    }
    cblas_csyrk = (FnPtr_cblas_csyrk)dlsym(handle, "cblas_csyrk");
    if (cblas_csyrk == nullptr) {
        return false;
    }
    cblas_csyr2k = (FnPtr_cblas_csyr2k)dlsym(handle, "cblas_csyr2k");
    if (cblas_csyr2k == nullptr) {
        return false;
    }
    cblas_ctrmm = (FnPtr_cblas_ctrmm)dlsym(handle, "cblas_ctrmm");
    if (cblas_ctrmm == nullptr) {
        return false;
    }
    cblas_ctrsm = (FnPtr_cblas_ctrsm)dlsym(handle, "cblas_ctrsm");
    if (cblas_ctrsm == nullptr) {
        return false;
    }

    cblas_zgemm = (FnPtr_cblas_zgemm)dlsym(handle, "cblas_zgemm");
    if (cblas_zgemm == nullptr) {
        return false;
    }
    cblas_zsymm = (FnPtr_cblas_zsymm)dlsym(handle, "cblas_zsymm");
    if (cblas_zsymm == nullptr) {
        return false;
    }
    cblas_zsyrk = (FnPtr_cblas_zsyrk)dlsym(handle, "cblas_zsyrk");
    if (cblas_zsyrk == nullptr) {
        return false;
    }
    cblas_zsyr2k = (FnPtr_cblas_zsyr2k)dlsym(handle, "cblas_zsyr2k");
    if (cblas_zsyr2k == nullptr) {
        return false;
    }
    cblas_ztrmm = (FnPtr_cblas_ztrmm)dlsym(handle, "cblas_ztrmm");
    if (cblas_ztrmm == nullptr) {
        return false;
    }
    cblas_ztrsm = (FnPtr_cblas_ztrsm)dlsym(handle, "cblas_ztrsm");
    if (cblas_ztrsm == nullptr) {
        return false;
    }


    cblas_chemm = (FnPtr_cblas_chemm)dlsym(handle, "cblas_chemm");
    if (cblas_chemm == nullptr) {
        return false;
    }
    cblas_cherk = (FnPtr_cblas_cherk)dlsym(handle, "cblas_cherk");
    if (cblas_cherk == nullptr) {
        return false;
    }
    cblas_cher2k = (FnPtr_cblas_cher2k)dlsym(handle, "cblas_cher2k");
    if (cblas_cher2k == nullptr) {
        return false;
    }

    cblas_zhemm = (FnPtr_cblas_zhemm)dlsym(handle, "cblas_zhemm");
    if (cblas_zhemm == nullptr) {
        return false;
    }
    cblas_zherk = (FnPtr_cblas_zherk)dlsym(handle, "cblas_zherk");
    if (cblas_zherk == nullptr) {
        return false;
    }
    cblas_zher2k = (FnPtr_cblas_zher2k)dlsym(handle, "cblas_zher2k");
    if (cblas_zher2k == nullptr) {
        return false;
    }

    return true;
}

#endif
