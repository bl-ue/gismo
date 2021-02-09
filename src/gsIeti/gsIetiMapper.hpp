/** @file gsIetiMapper.hpp

    @brief Algorithms for the assembling of matrices required for IETI-Solvers

    This file is part of the G+Smo library.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

    Author(s): S. Takacs
*/

#pragma once

#include <gsCore/gsDofMapper.h>
#include <gsNurbs/gsKnotVector.h>
#include <gsUtils/gsCombinatorics.h>


/*    Concerning the status flag m_status:
 *       (m_status&1)!=0    means that the object has been initialized by calling init or the value constructor
 *       (m_status&2)!=0    means that the jump matrices have been computed
 *       (m_status&4)!=0    means that corners have been set up as primal constraints
 *
 *    Please add more uses to the list.
 */

namespace gismo
{

template <class T>
void gsIetiMapper<T>::init(
        const gsMultiBasis<T>& multiBasis,
        gsDofMapper dofMapperGlobal,
        const Matrix& fixedPart
    )
{
    const index_t nPatches = dofMapperGlobal.numPatches();
    m_multiBasis = &multiBasis;
    m_dofMapperGlobal = give(dofMapperGlobal);
    m_dofMapperLocal.clear();
    m_dofMapperLocal.resize(nPatches);
    m_fixedPart.clear();
    m_fixedPart.resize(nPatches);
    m_jumpMatrices.clear();
    m_nPrimalDofs = 0;
    m_primalConstraints.clear();
    m_primalConstraints.resize(nPatches);
    m_primalDofIndices.clear();
    m_primalDofIndices.resize(nPatches);
    m_status = 1;

    for (index_t k=0; k<nPatches; ++k)
    {
        const index_t nDofs = m_dofMapperGlobal.patchSize(k);
        m_dofMapperLocal[k].setIdentity(1,nDofs);
        for (index_t i=0; i<nDofs; ++i)
        {
            const index_t idx = m_dofMapperGlobal.index(i,k);
            if (m_dofMapperGlobal.is_boundary_index(idx))
                m_dofMapperLocal[k].eliminateDof(i,0);
        }
        m_dofMapperLocal[k].finalize();

        const index_t szFixedPart = m_dofMapperLocal[k].boundarySize();
        m_fixedPart[k].setZero(szFixedPart,1);
        for (index_t i=0; i<nDofs; ++i)
        {
            const index_t idx = m_dofMapperGlobal.index(i,k);
            if (m_dofMapperGlobal.is_boundary_index(idx))
            {
                const index_t globalBoundaryIdx = m_dofMapperGlobal.bindex(i,k);
                const index_t localBoundaryIdx = m_dofMapperLocal[k].bindex(i,0);
                m_fixedPart[k](localBoundaryIdx,0) = fixedPart(globalBoundaryIdx,0);
            }
        }
    }
}


template <class T>
typename gsIetiMapper<T>::Matrix
gsIetiMapper<T>::constructGlobalSolutionFromLocalSolutions( const std::vector<Matrix>& localContribs )
{
    GISMO_ASSERT( m_status&1, "gsIetiMapper: The class has not been initialized." );

    const index_t nPatches = m_dofMapperGlobal.numPatches();
    GISMO_ASSERT( nPatches == static_cast<index_t>(localContribs.size()),
        "gsIetiMapper::constructGlobalSolutionFromLocalSolutions; The number of local contributions does "
        "not argee with the number of patches." );

    Matrix result;
    result.setZero( m_dofMapperGlobal.freeSize(), localContribs[0].cols() );

    for (index_t k=0; k<nPatches; ++k)
    {
        const index_t sz=m_dofMapperLocal[k].size();
        for (index_t i=0; i<sz; ++i)
        {
            if (m_dofMapperLocal[k].is_free(i,0) && m_dofMapperGlobal.is_free(i,k))
                result.row(m_dofMapperGlobal.index(i,k)) = localContribs[k].row(m_dofMapperLocal[k].index(i,0));
        }
    }
    return result;
}

namespace{
struct dof_helper {
    index_t globalIndex;
    index_t patch;
    index_t localIndex;
    bool operator<(const dof_helper& other) const
    { return globalIndex < other.globalIndex; }
};
}

template <class T>
void gsIetiMapper<T>::cornersAsPrimals()
{
    GISMO_ASSERT( m_status&1, "gsIetiMapper: The class has not been initialized." );
    GISMO_ASSERT( !(m_status&4), "gsIetiMapper::cornersAsPrimals: This function has already been called." );
    m_status |= 4;

    const index_t nPatches = m_dofMapperLocal.size();

    // Construct all corners
    std::vector<dof_helper> corners;
    corners.reserve(4*nPatches);
    const index_t dim = m_multiBasis->dim();
    for (index_t k=0; k<nPatches; ++k)
        for (boxCorner it = boxCorner::getFirst(dim); it!=boxCorner::getEnd(dim); ++it)
        {
            const index_t idx = (*m_multiBasis)[k].functionAtCorner(it);
            dof_helper dh;
            dh.globalIndex = m_dofMapperGlobal.index( idx, k );
            dh.patch = k;
            dh.localIndex = m_dofMapperLocal[k].index( idx, 0 );
            if (m_dofMapperGlobal.is_free_index(dh.globalIndex))
                corners.push_back(dh);
        }
    std::sort(corners.begin(), corners.end());

    // Create data
    index_t lastIndex=-1;
    const index_t sz = corners.size();
    for (index_t i=0; i<sz; ++i)
    {
        if (lastIndex!=corners[i].globalIndex)
        {
            lastIndex = corners[i].globalIndex;
            ++m_nPrimalDofs;
        }
        const index_t cornerIndex = m_nPrimalDofs - 1;
        const index_t patch       = corners[i].patch;
        const index_t localIndex  = corners[i].localIndex;

        SparseVector constr(m_dofMapperLocal[patch].freeSize());
        constr[localIndex] = 1;

        m_primalConstraints[patch].push_back(give(constr));
        m_primalDofIndices[patch].push_back(cornerIndex);
    }

}

template <class T>
void gsIetiMapper<T>::customPrimalConstraints(std::vector< std::pair<index_t,SparseVector> > data)
{
    GISMO_ASSERT( m_status&1, "gsIetiMapper: The class has not been initialized." );

    const index_t sz = data.size();
    for (index_t i=0; i<sz; ++i)
    {
        const index_t patch = data[i].first;
        m_primalConstraints[patch].push_back( give(data[i].second) );
        m_primalDofIndices[patch].push_back(m_nPrimalDofs);
    }
    ++m_nPrimalDofs;
}

template <class T>
std::vector<index_t> gsIetiMapper<T>::skeletonDofs( index_t patch ) const
{
    GISMO_ASSERT( m_status&1, "gsIetiMapper: The class has not been initialized." );

    std::vector<index_t> result;
    const index_t patchSize = m_dofMapperGlobal.patchSize(patch);
    result.reserve(4*math::sqrt(patchSize));
    for (index_t i=0; i<patchSize; ++i)
    {
        if ( m_dofMapperGlobal.is_coupled(i,patch) )
            result.push_back(m_dofMapperLocal[patch].index(i,0));
    }
    return result;
}

template <class T>
void gsIetiMapper<T>::computeJumpMatrices(bool fullyRedundant, bool excludeCorners)
{
    GISMO_ASSERT( m_status&1, "gsIetiMapper: The class has not been initialized." );

    GISMO_ASSERT( !(m_status&2), "gsIetiMapper::computeJumpMatrices: This function has already been called." );
    m_status |= 2;

    const index_t nPatches = m_dofMapperGlobal.numPatches();
    const index_t coupledSize = m_dofMapperGlobal.coupledSize();

    // Find the groups of to be coupled indices
    std::vector< std::vector< std::pair<index_t,index_t> > > coupling;
    coupling.resize(coupledSize);

    for (index_t k=0; k<nPatches; ++k)
    {
        const index_t patchSize = m_dofMapperGlobal.patchSize(k);
        for (index_t i=0; i<patchSize; ++i)
        {
            const index_t globalIndex = m_dofMapperGlobal.index(i,k);
            if ( m_dofMapperGlobal.is_coupled_index(globalIndex) )
            {
                const index_t coupledIndex = m_dofMapperGlobal.cindex(i,k);
                coupling[coupledIndex].push_back(
                    std::pair<index_t,index_t>(k,i)
                );
            }
        }
    }

    // Erease data for corners if so desired
    if (excludeCorners)
    {
        const index_t dim = m_multiBasis->dim();
        for (index_t k=0; k<nPatches; ++k)
            for (boxCorner it = boxCorner::getFirst(dim); it!=boxCorner::getEnd(dim); ++it)
            {
                const index_t idx = (*m_multiBasis)[k].functionAtCorner(it);
                const index_t globalIndex = m_dofMapperGlobal.index(idx,k);
                if ( m_dofMapperGlobal.is_coupled_index(globalIndex) )
                {
                    const index_t coupledIndex = m_dofMapperGlobal.cindex(idx,k);
                    coupling[coupledIndex].clear();
                }
            }
    }

    // Compute the number of Lagrange multipliers
    index_t numLagrangeMult = 0;
    for (index_t i=0; i<coupledSize; ++i)
    {
        const index_t n = coupling[i].size();
        GISMO_ASSERT( n>1 || excludeCorners, "gsIetiMapper::computeJumpMatrices:"
            "Found a coupled dof that is not coupled to any other dof." );
        if (fullyRedundant)
            numLagrangeMult += (n * (n-1))/2;
        else
            numLagrangeMult += n-1;
    }

    // Compute the jump matrices
    m_jumpMatrices.clear();
    for (index_t i=0; i<nPatches; ++i)
        m_jumpMatrices.push_back(JumpMatrix(numLagrangeMult, m_dofMapperLocal[i].freeSize()));

    index_t multiplier = 0;
    for (index_t i=0; i<coupledSize; ++i)
    {
        const index_t n = coupling[i].size();
        index_t maxIndex = fullyRedundant ? (n-1) : 1;
        for (index_t j1=0; j1<maxIndex; ++j1)
        {
            const index_t patch1 = coupling[i][j1].first;
            const index_t localIndex1 = coupling[i][j1].second;
            const index_t localMappedIndex1 = m_dofMapperLocal[patch1].index(localIndex1,0);
            for (index_t j2=j1+1; j2<n; ++j2)
            {
                const index_t patch2 = coupling[i][j2].first;
                const index_t localIndex2 = coupling[i][j2].second;
                const index_t localMappedIndex2 = m_dofMapperLocal[patch2].index(localIndex2,0);
                m_jumpMatrices[patch1](multiplier,localMappedIndex1) = (T)1;
                m_jumpMatrices[patch2](multiplier,localMappedIndex2) = (T)-1;
                ++multiplier;
            }
        }
    }
    GISMO_ASSERT( multiplier == numLagrangeMult, "gsIetiMapper::computeJumpMatrices: Internal error: "
        << multiplier << "!=" << numLagrangeMult );

}


} // namespace gismo