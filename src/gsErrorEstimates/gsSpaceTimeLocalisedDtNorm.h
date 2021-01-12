/** @file gsSpaceTimeLocalisedDtNorm.h

    @brief Computes the localised (\delta_K * || \partial_t e ||^2_K)^{1/2} norm.

    This file is part of the G+Smo library.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

    Author(s): S. Matculevich
*/
#pragma once

#include<gsAssembler/gsNorm.h>

namespace gismo
{

/** @brief The gsSpaceTimeDtDeltaKNorm class provides the functionality
 * to calculate special localised space-time norm (\delta_K * || \partial_t e ||^2_K)^{1/2}.
 *
 * \ingroup Assembler
*/
    template <class T>
    class gsSpaceTimeLocalisedDtNorm : public gsNorm<T>
    {
        friend  class gsNorm<T>;
        typedef typename gsMatrix<T>::RowsBlockXpr Rows;

    public:

        gsSpaceTimeLocalisedDtNorm(const gsField<T> & _field1,
                          const gsFunction<T> & _func2,
                          bool _f2param = false)
                : gsNorm<T>(_field1,_func2), dfunc2(NULL), f2param(_f2param)
        {

        }

        gsSpaceTimeLocalisedDtNorm(const gsField<T> & _field1,
                          const gsFunction<T> & _func2,
                          const gsFunction<T> & _dfunc2,
                          bool _f2param = false)
                : gsNorm<T>(_field1,_func2), dfunc2(&_dfunc2), f2param(_f2param)
        {

        }

        gsSpaceTimeLocalisedDtNorm(const gsField<T> & _field1)
                : gsNorm<T>(_field1), dfunc2(NULL), f2param(false)
        { }

    public:

        T compute(bool storeElWise = false)
        {
            this->apply(*this,storeElWise);
            //gsInfo << "|| e_t || = " << m_value << "\n";
            return m_value;
        }


    protected:

        void initialize(const gsBasis<T> & basis,
                        gsQuadRule<T> & rule,
                        unsigned      & evFlags) // replace with geoEval ?
        {
            // Setup Quadrature
            const unsigned d = basis.dim();
            gsVector<index_t> numQuadNodes( d );
            for (unsigned i = 0; i < d; ++i)
                numQuadNodes[i] = basis.degree(i) + 1;

            // Setup Quadrature
            rule = gsGaussRule<T>(numQuadNodes);// harmless slicing occurs here

            // Set Geometry evaluation flags
            evFlags = NEED_MEASURE| NEED_VALUE| NEED_JACOBIAN | NEED_2ND_DER | NEED_GRAD_TRANSFORM;
        }

        // Evaluate on element.
        void evaluate(gsGeometryEvaluator<T> & geoEval,
                      const gsFunction<T>    & func1,
                      const gsFunction<T>    & _func2,
                      gsMatrix<T>            & quNodes)
        {
            // Evaluate first function
            func1.deriv_into(quNodes, func1Derivs);
            // get the gradients to columns

            // Evaluate second function (defined of physical domain)
            geoEval.evaluateAt(quNodes);

            if(dfunc2==NULL)
            {
                // get the gradients to columns
                _func2.deriv_into(geoEval.values(), func2Derivs);
            }
            else {
                dfunc2->eval_into(geoEval.values(), func2Derivs);
            }

        }

        // assemble on element
        inline T compute(gsDomainIterator<T>    & element,
                         gsGeometryEvaluator<T> & geoEval,
                         gsVector<T> const      & quWeights,
                         T & accumulated)
        {
            T sum(0.0);
            const unsigned d = element.dim();
            gsMatrix<> ones = gsMatrix<>::Identity(d-1, 1);
            ones.setOnes();
            gsMatrix<T> func1SpaceLaplace, func2SpaceLaplace;

            for (index_t k = 0; k < quWeights.rows(); ++k) // loop over quadrature nodes
            {
                // Transform the gradients
                // f1pders : N X dim
                // f1pdersSpace, f1pdersTime : N X (dim-1)
                geoEval.transformGradients(k, func1Derivs, ph_func1Derivs);

                Rows ph_func1TimeGrad = ph_func1Derivs.bottomRows(1);
                Rows ph_func2TimeGrad  = func2Derivs.bottomRows(1);

                const T weight = quWeights[k] *  geoEval.measure(k);

                T h_K     = element.getCellSize();
                T C_invK  = 1;
                T theta_K = h_K / ( d * math::pow(C_invK, 2));
                T delta_K = theta_K * h_K;

                sum += weight * ( delta_K * (ph_func1TimeGrad - ph_func2TimeGrad.col(k)).squaredNorm() );

            }

            accumulated += sum;
            //gsInfo << "element-wise contrib = " << accumulated << "\n";
            return sum;
        }


        inline T takeRoot(const T v) { return math::sqrt(v);}

    private:
        // first derivative of func2:
        const gsFunction<T> * dfunc2; // If this is NULL a numerical approximation will be used

        using gsNorm<T>::m_value;
        using gsNorm<T>::m_elWise;

        gsMatrix<T> func1Derivs, func2Derivs, ph_func1Derivs, ph_func2Derivs;

        bool f2param;// not used yet
    };
} // namespace gismo
