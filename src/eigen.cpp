#include "eigen.hpp"
#include <vector>
#include <Eigen/Core>

Eigen::VectorXd solve(const Eigen::SparseMatrix<double>& A, const Eigen::VectorXd& b) {
    Eigen::SimplicialLLT<Eigen::SparseMatrix<double>> cholesky;
    cholesky.compute(A);
    return cholesky.solve(b);
}

std::vector<Vec3> linear_solve(const SpMat<Mat3x3>& A, const std::vector<Vec3>& b) {
    int n = b.size();
    int dim = 3 * n;
    Eigen::SparseMatrix<double> At(dim, dim);
    std::vector<Eigen::Triplet<double>> tripletList;
    tripletList.reserve(A.rows.size() * 9);

    for (int i = 0; i < A.rows.size(); ++i) {
        for (int j_idx = 0; j_idx < A.rows[i].indices.size(); ++j_idx) {
            int j = A.rows[i].indices[j_idx];
            const Mat3x3& block = A.rows[i].entries[j_idx];
            
            for (int k = 0; k < 3; ++k) {
                for (int h = 0; h < 3; ++h) {
                    tripletList.emplace_back(3 * i + k, 3 * j + h, block(k, h));
                }
            }
        }
    }

    At.setFromTriplets(tripletList.begin(), tripletList.end());

    Eigen::VectorXd bt(dim);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < 3; ++j)
            bt(3 * i + j) = b[i][j];

    Eigen::VectorXd x = solve(At, bt);

    std::vector<Vec3> ans(n);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < 3; ++j)
            ans[i][j] = x(3 * i + j);
    
    return ans;
}

std::vector<double> linear_solve(const SpMat<double>& A, const std::vector<double>& b) {
    int n = b.size();
    Eigen::SparseMatrix<double> At(n, n);
    for (int i = 0; i < A.rows.size(); i++)
        for (int j = 0; j < A.rows[i].indices.size(); j++)
            At.coeffRef(i, A.rows[i].indices[j]) = A.rows[i].entries[j];
    
    Eigen::VectorXd bt(n);
    for (int i = 0; i < n; i++)
        bt(i) = b[i];
    
    Eigen::VectorXd x = solve(At, bt);

    std::vector<double> ans(n);
    for (int i = 0; i < n; i++)
        ans[i] = x(i);
    
    return ans;
}

int eigenvalue_decomposition(int n, double* A, double* w, bool computeEigenvalues) {
    Eigen::MatrixXd At(n, n);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            At(i, j) = A[i + j * n];

    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> ed(At, computeEigenvalues ? Eigen::DecompositionOptions::ComputeEigenvectors : Eigen::DecompositionOptions::EigenvaluesOnly);
    if (ed.info() != Eigen::ComputationInfo::Success)
        return -1;

    for (int i = 0; i < n; i++)
        w[i] = ed.eigenvalues()(i);   
    if (computeEigenvalues)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                A[i + j * n] = ed.eigenvectors()(i, j);

    return 0;
}

int singular_value_decomposition(int m, int n, double* A, double* s, double* U, double* Vt) {
    Eigen::MatrixXd At(m, n);
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            At(i, j) = A[i + j * m];
    
    Eigen::JacobiSVD<Eigen::MatrixXd> svd(At, Eigen::ComputeFullU | Eigen::ComputeFullV);
    if (svd.info() != Eigen::ComputationInfo::Success)
        return -1;

    for (int i = 0; i < n; i++)
        s[i] = svd.singularValues()(i);
    for (int i = 0; i < m; i++)
        for (int j = 0; j < m; j++)
            U[i + j * m] = svd.matrixU()(i, j);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            Vt[i + j * m] = svd.matrixV()(j, i);

    return 0;
}