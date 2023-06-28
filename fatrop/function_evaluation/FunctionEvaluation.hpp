#ifndef FUNCTIONEVALUATIONINCLUDED
#define FUNCTIONEVALUATIONINCLUDED
#include <vector>
#include <cstring>
#include "blasfeo_wrapper/LinearAlgebraBlasfeo.hpp"

#ifdef ENABLE_MULTITHREADING
#include <omp.h>
#endif

namespace fatrop
{
    /// Class used to evaluate a numerical functions. Functions can be implemented by hand or casadi codegen API or by plain casadi.
    class EvalBase
    {
    public:
        /// number of input vectors of the function
        int n_in;
        /// number of columns in output matrix
        int out_m;
        /// number of rows in output matrix
        int out_n;
        /// number of nonzeros in output matrix
        int out_nnz;
        /// sparsity pattern of output matrix sparsity pattern [m,n|0,ncol0, ncol0:1 , ..., | nnz | row_el0, row_el1, ...]
        std::vector<int> sparsity_out;
        /// buffer to safe evaluation result, in a buffer we always save a matrix in CCS format with lda==out_m
        #ifndef ENABLE_MULTITHREADING
        std::vector<double> buffer;
        #else
        vector<vector<double>> buffer = vector<vector<double>>(omp_get_max_threads());
        #endif
        /// evaluate function and save res in "ccs format with lda==out_m"
        virtual int eval_buffer(const double **arg) = 0;
        /// evaluate function and save res in "blasfeo format"
        int eval_bf(const double **arg, MAT *bf_mat);
        int eval_array(const double **arg, double *array);
        ~EvalBase(){};
    };
};     // namespace fatrop
#endif // FUNCTIONEVALUATIONINCLUDED
