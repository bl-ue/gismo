/** @file gsVisitorDivDiv.h

    @brief Visitor for the equation \f$ div y = f, f \in L^2(\Omega) (*) \f$

    where \f$ y \in H(Omega, div) := \{ y \in L^2(\Omega, R^d) :  div y \in L^2(\Omega) \}\f$
    is an auxiliari vector-function (approximating the exact flux \f$\nabla u\f$).

    We assume that \f$y \in Y^M_div \subset H(Omega, div)\f$ and \f$Y^M_div := span \{ phi_1, ..., phi_M\\f$,
    where \f$phi_i\f$ are vector functions in \f$H(\Omega, div)\f$.

    Function \f$y\f$ is approximated as \f$y = \Sum_{i = 1}^{M} Yh_i phi_i\f$,
    where \f$Yh \in R^M\f$ is the vector of unknown DOFs.

    By testing (*) with div(phi_j), we obtain the local contributions

    \f$ DivDiv_{ij} := \int_{\Omega} div(phi_i) * div(phi_j) \dx \f$ (for the local matrices)

    and

    \f$ DivfRHs_{j} := \int_{\Omega} f * div(phi_j) \dx \f$ (for the local RHS).

    This file is part of the G+Smo library.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

    Author(s): S. Matculevich
*/

#pragma once

#include <gsAssembler/gsGaussRule.h>

namespace gismo
{

template <class T, bool paramCoef = false>
class gsVisitorDivDiv
{
public:

    /** \brief Constructor for gsVisitorDivDiv.*/
    explicit gsVisitorDivDiv(const gsPde<T> & pde)
    {
        pde_ptr = static_cast<const gsPoissonPde<T> *>(&pde);
        numActive = 0;
    }

    void initialize(const gsBasis<T>   & basis,
                    const index_t        patchIndex,
                    const gsOptionList & options,
                    gsQuadRule<T>      & rule)
    {
        // Grab right-hand side for current patch
        // The right-hand side is a gsFunctionExpr in most cases, hence, rhs.nPieces = 1
        rhs_ptr = &pde_ptr->rhs()->piece(0);

        // Setup Quadrature
        rule = gsGaussRule<T>(basis, options); // harmless slicing occurs here

        // Set Geometry evaluation flags
        md.flags = NEED_VALUE | NEED_MEASURE | NEED_GRAD_TRANSFORM;
    }

    // Evaluate on element.
    inline void evaluate(const gsBasis<T>    & basis,
                         const gsGeometry<T> & geo,
                         const gsMatrix<T>   & quNodes)
    {
        md.points = quNodes;

        // Compute the active basis functions
        // (assumes actives are the same for all quadrature points on the elements)
        basis.active_into(md.points.col(0), actives);
        numActive = actives.rows();

        // Evaluate basis functions on element
        basis.evalAllDers_into(md.points, 1, basisData);

        // Compute image of Gauss nodes under geometry mapping as well as Jacobians
        geo.computeMap(md);

        // Evaluate right-hand side at the geometry points paramCoef
        // specifies whether the RHS function should be
        // evaluated in parametric (true) or physical (false)
        // if true : use the specified md.points
        // if false : use the mapped md.points under geometry mapping that are stored in md.values[0]
        rhs_ptr->eval_into((paramCoef ? md.points : md.values[0]), rhsVals);

        int d = basis.dim();

        // Initialize local matrix/rhs for vector valued basis function
        localMat.setZero(d * numActive, d * numActive);
        localRhs.setZero(d * numActive, rhsVals.rows()); // m_rhsGradV.rows() for multiple right-hand sides
    }

    inline void assemble(gsDomainIterator<T> & element,
                         const gsVector<T>   & quWeights)
    {
        // Evaluation at the point
        gsMatrix<T> & bVals = basisData[0]; // [numActive]
        gsMatrix<T> & bGrads = basisData[1];

        numActive = bVals.rows();   // number of the active basis function on the element

        //int d = geoEval.geometry().targetDim(); //same as geoDim
        int d = md.dim.second;
        int dN = d * numActive;     // auxiliary variable for the local matrix

        gsMatrix<T> physGrad;
        gsMatrix<T> localMat_;
        gsMatrix<T> localRhs_;

        localMat_.setZero(d * numActive, d * numActive);
        localRhs_.setZero(d * numActive, rhsVals.rows());

        //#pragma omp for nowait //fill result_private in parallel
        for (index_t k = 0; k < quWeights.rows(); ++k) // loop over quadrature nodes
        {
            // Multiply weight by the geometry measure
            const T weight = quWeights[k] * md.measure(k);

            // Compute physical gradients at k-th evaluation point as a d x numActive matrix, where
            // d is the dimension of the problem and
            // numActive is the number of active basis functions on the element
            transformGradients(md, k, bGrads, physGrad);

            // Resize the physBasisGrad into array [(d * numActive) x 1] to store all the div(B_i) in a single array,
            // where B_i is the active basis function:
            // div (phi_i) = { (dx B_1) ... (dx B_N) (dy B_1) ... (dy B_N) } with vector basis functions
            // phi_1 ... phi_N phi_{1+N} ... phi_{2*N}
            // (B_1) ... (B_N) (  0)     ... (  0)
            // (  0) ... (  0) (B_1)     ... (B_N)
            // for y^(1)  and  for y^(2)

            physGrad.transposeInPlace();
            physGrad.resize(dN, 1);

            // Add local contribution div(phi_i) * div(phi_j) into the global matrix and
            // local contribution f * div(phi_j) into the global rhs
            // localMat.noalias() += weight * physBasisGrad * physBasisGrad.transpose();
            // localRhs.noalias() += weight * physBasisGrad * rhsVals.col(k);
            localMat_ += weight * physGrad * physGrad.transpose();
            localRhs_ += weight * physGrad * (- rhsVals.col(k));
        }
        localMat.noalias() += localMat_;
        localRhs.noalias() += localRhs_;
    }

    inline void localToGlobal(const int patchIndex,
                              const std::vector<gsMatrix<T> > & eliminatedDofs,
                              gsSparseSystem<T> & system)
    {
        int d = localMat.rows() / numActive;   // basisData[1].rows() / basisData[1].cols() gives wrong d ?

        // Map patch-local DoFs to global DoFs
        system.mapColIndices(actives, patchIndex, actives);

        // localMat must be decomposed into d x d blocks
        // localRhs ... into d blocks
        gsMatrix<T> localMatToPush(numActive, numActive);
        gsMatrix<T> localRhsToPush(numActive, 1);

        for (index_t i = 0; i < d; ++i)
        {
            for (index_t j = i; j < d; ++j)
            {
                localMatToPush = localMat.block(i * numActive, j * numActive, numActive, numActive);

                if (i == j)
                {
                    localRhsToPush = localRhs.middleRows(i * numActive, numActive);
                    system.push(localMatToPush, localRhsToPush, actives, eliminatedDofs.front(), i, j);
                }
                if (j > i)
                {
                    system.pushToMatrix(localMatToPush, actives, eliminatedDofs.front(), i, j);
                    system.pushToMatrix(localMatToPush.transpose(), actives, eliminatedDofs.front(), j, i);

                }
            }
        }
    }

protected:
    // Pointer to the pde data
    const gsPoissonPde<T> *pde_ptr;

protected:
    // Basis values
    std::vector<gsMatrix<T> > basisData;
    gsMatrix<index_t> actives;
    index_t numActive;

protected:
    // Right hand side ptr for current patch
    const gsFunction<T> *rhs_ptr;
    // Local values of the right hand side
    gsMatrix<T> rhsVals;

protected:
    // Local matrices
    gsMatrix<T> localMat;
    gsMatrix<T> localRhs;

    gsMapData<T> md;
};

} // namespace gismo
