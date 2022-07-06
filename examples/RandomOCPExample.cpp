#include "debug/RandomOCP.hpp"
#include "ocp/OCPLSRiccati.hpp"
#include "ocp/FatropOCP.hpp"
#include "aux/FatropVector.hpp"
#include <vector>
#include <iostream>
#include "blasfeo_wrapper/LinearAlgebraBlasfeo.hpp"
#include "sparse/SparseOCP.hpp"
using namespace std;
using namespace fatrop;
int main()
{
    int K = 150;
    int nu = 12;
    int nx = 12;
    int ng = 4;
    FatropVector<int> nu_ = vector<int>(K, nu);
    FatropVector<int> nx_ = vector<int>(K, nx);
    FatropVector<int> ng_ = vector<int>(K, ng);
    // nx_.at(40) = 25;
    // nu_.at(20) = 20;
    // ng_.at(K - 1) = nx;
    // ng_.at(0) = nx;

    RefCountPtr<BFOCP> ocptemplatebasic =
        new RandomOCP(nu_, nx_, ng_, K);
    RefCountPtr<OCP> ocptempladapter = new BFOCPAdapter(ocptemplatebasic);
    RefCountPtr<OCPLinearSolver> ocplsriccati = new OCPLSRiccati(ocptempladapter->GetOCPDims());
    RefCountPtr<FatropParams> params = new FatropParams();
    RefCountPtr<OCPScalingMethod> ocpscaler = new OCPNoScaling(params);
    FatropOCP ocpalg(ocptempladapter, ocplsriccati, ocpscaler);
    int N_opti_vars = sum(nu_ + nx_);
    int N_lags = sum(nx_) - nx_.at(0) + sum(ng_);
    blasfeo_timer timer;
    FatropMemoryVecBF ux(N_opti_vars, 2);
    FatropMemoryVecBF lags(N_lags, 2);
    blasfeo_tic(&timer);
    ocpalg.EvalHess(1.0, ux[0], lags[0]);
    ocpalg.EvalJac(ux[0], ux[0]);
    double el = blasfeo_toc(&timer);
    cout << "el time FE " << el << endl;
    int N = 1000;
    blasfeo_tic(&timer);
    for (int i = 0; i < N; i++)
    {
        ocplsriccati->computeSD(&ocpalg.ocpkktmemory_, 0.0, 0.0, 0.0, 0.0, ux[0], lags[0], lags[0], lags[0], lags[0], lags[0], lags[0], lags[0], lags[0], lags[0], lags[0]);
    }
    el = blasfeo_toc(&timer);
    cout << "el time riccati " << el / N << endl;
    RefCountPtr<OCPLinearSolver> ocplssparse = new Sparse_OCP(ocptempladapter->GetOCPDims(), ocpalg.ocpkktmemory_);
    ocplssparse->computeSD(&ocpalg.ocpkktmemory_, 0.0, 0.0,0.0,0.0, ux[1], lags[1], lags[0], lags[0], lags[0], lags[0], lags[0], lags[0], lags[0], lags[0], lags[0]);
    // ocpalg.ocpkktmemory_.BAbt[0].print();
    // cout << Eig(ux[0]) -Eig(ux[1]) << endl;
    cout << "inf-norm difference MUMPS - Fatrop  primal " << (Eig(ux[0]) - Eig(ux[1])).lpNorm<Eigen::Infinity>() << endl;
    cout << "inf-norm difference MUMPS - Fatrop  dual " << (Eig(lags[0]) - Eig(lags[1])).lpNorm<Eigen::Infinity>() << endl;
    cout << "inf-norm   primal " << Eig(ux[0]).lpNorm<Eigen::Infinity>() << endl;
    cout << "inf-norm   dual " << Eig(lags[0]).lpNorm<Eigen::Infinity>() << endl;
}