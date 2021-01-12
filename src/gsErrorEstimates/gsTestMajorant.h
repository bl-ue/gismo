/** @file gsTestMajorant.h

@brief A class that contains all the utils for testing the majorant
        for the elliptic equation.

This file is part of the G+Smo library.

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Author(s): S. Matculevich
*/
#include <sys/stat.h>
#include <fstream>
#include <functional>

#include <gsErrorEstimates/gsVisitorDivDiv.h>
#include <gsErrorEstimates/gsVisitorDualPoisson.h>
#include <gsErrorEstimates/gsErrEstEquilMajorant.h>
#include <gsErrorEstimates/gsErrEstDualMajorant.h>
#include <gsErrorEstimates/gsErrEstMinorant.h>

#include <gsAssembler/gsAdaptiveRefUtils.h>
#include <gsAssembler/gsPoissonHeterogeneousAssembler.h>
#include <gsAssembler/gsNorm.h>
#include <gsSolver/gsIterativeSolver.h>
#include <gsSolver/gsSolverUtils.h>

namespace gismo {

    template<unsigned d>
    class gsTestMajorant {

    private:
        void initGivenProblemData();

    public:
#if defined(GISMO_WITH_PARDISO)
    typedef typename gsSparseSolver<real_t>::PardisoLU sparseLUfact; //Threadsave
#elif defined(GISMO_WITH_SUPERLU)
    typedef typename gsSparseSolver<T>::SuperLU sparseLUfact; //Not Threadsave
#else
    typedef typename gsSparseSolver<real_t>::LU sparseLUfact; //Threadsave
#endif

        // Constructor
        gsTestMajorant(const unsigned _exampleNumber, bool _isAdaptive) :
                exampleNumber(_exampleNumber), isAdaptive(_isAdaptive) {
            dim = (int) d;
            initGivenProblemData();
        }

        void gsGetInitialBasis(int, int, int,
                               gsMultiBasis<> &, gsMultiBasis<> &, gsMultiBasis<> &,
                               int, int, int);

        void gsLogProblemData();

        void gsCreateResultsFolder(bool, int, int, int, int, int, int, MarkingStrategy, real_t);

        void gsLogRefinementBasisInfo(int, int, gsMultiBasis<> &, gsMultiBasis<> & , gsMultiBasis<> & );

        void gsLogAssemblingSolvingTime(int refCount,
                                          gsVector<double> & timeAsmbDivDivY,
                                          gsVector<double> & timeAsmbY,
                                          gsVector<double> & timeAsmbW,
                                          gsVector<double> & timeAsmbMMY,
                                          gsMatrix<double> & timeSolvV,
                                          gsMatrix<double> & timeSolvY,
                                          gsMatrix<double> & timeSolvW,
                                          gsVector<double> & timeAsmbMaj,
                                          gsVector<double> & timeAsmbMin);

        void gsInitializeProblemData(gsFunctionExpr<> &, gsFunctionExpr<> &, gsPiecewiseFunction<> &, gsFunctionExpr<> &, gsBoundaryConditions<> &);

        void gsRecontructV(int refCount, gsPoissonHeterogeneousAssembler<real_t>& poissonAssembler,
                              gsBoundaryConditions<> &bcInfo,
                              gsMatrix<real_t> &vVector,
                              gsMultiPatch<real_t> &mpV,
                              gsField<>& v,
                              gsVector<index_t> &vDOFs,
                              gsVector<real_t> &stopcritVector,
                              gsVector<double> &timeAsmbV,
                              gsMatrix<double> &timeSolvV);

        void gsSolveKhfhSystem(gsPoissonHeterogeneousAssembler<real_t> &,
                               gsMatrix<>&,
                               gsMatrix<real_t>&,
                               int,
                               gsVector<real_t>&);

        void gsSolveKhwhfhSystem(gsPoissonHeterogeneousAssembler<real_t> &,
                                 gsMatrix<> &,
                                 gsMatrix<> &,
                                 int,
                                 gsVector<real_t>& );

        void gsSolveMajorantOptimalSystem(gsSparseMatrix<real_t> &, gsMatrix<> &, gsMatrix<> &,
                                          gsMatrix<real_t> &, int, int, gsMatrix<index_t> &,
                                          gsVector<real_t> & );

        void gsRecontructMajorantBasedOnOptimalFlux(int, gsMultiBasis<real_t> &, int,
                                                    gsMatrix<real_t> &, gsMultiPatch<real_t> &, gsMatrix<index_t> &,
                                                    const gsMultiPatch<real_t> &, const gsField<> &,
                                                    gsVector<real_t> &stopcritVector,
                                                    const gsFunctionExpr<real_t> &,
                                                    gsVector<real_t> &, gsVector<real_t> &, gsVector<real_t> &,
                                                    gsMatrix<real_t> &,
                                                    gsVector<real_t> &,
                                                    gsVector<real_t> &, gsVector<real_t> &, gsVector<real_t> &,
                                                    std::vector<real_t> &, std::vector<real_t> &,
                                                    const std::vector<size_t>&);

        void gsRecontructMajorantBasedOnEquilibratedFlux(int, gsMultiBasis<real_t> &, int,
                                                    gsMatrix<real_t> &, gsMultiPatch<real_t> &, gsVector<index_t> &,
                                                    gsMultiPatch<real_t> &, gsField<> &, gsFunctionExpr<real_t> &,
                                                    gsVector<real_t> &, gsMatrix<real_t> &, gsVector<real_t> &,
                                                    gsVector<real_t> &, gsVector<real_t> &, gsVector<real_t> &,
                                                    std::vector<real_t> &, std::vector<real_t> &,
                                                    int);

        void gsRecontructMinorantBasedOnAuxiliaryW(int, gsPoissonHeterogeneousAssembler<real_t> & , gsBoundaryConditions<> &,
                                                   gsMatrix<real_t> &, gsMultiPatch<real_t> &, gsField<> &, gsVector<index_t> &,
                                                   gsVector<real_t> &,
                                                   const gsField<> &, const gsFunctionExpr<real_t> &,
                                                   gsVector<real_t> &,
                                                   gsMatrix<real_t> &,
                                                   gsVector<real_t> &,
                                                   gsVector<real_t> &,
                                                   std::vector<real_t> &,
                                                   int );

        void gsGenerateDivDivMMMatrices(int, gsPoissonPde<> &, gsPoissonHeterogeneousPde<real_t> &,
                                        gsMultiBasis<real_t> &,
                                        int, gsMatrix<index_t> &,
                                        const gsMultiPatch<real_t> &, const gsField<> &v, const gsFunctionExpr<real_t> &,
                                        gsAssembler<> &, gsAssembler<> &,
                                        gsVector<real_t> &, gsVector<real_t> &, gsVector<real_t> &, size_t, bool);

        void gsExecuteRefinement(gsPoissonAssembler<real_t> &,
                                 gsMultiBasis<real_t> &, gsPoissonAssembler<real_t> &,
                                 std::vector<gsMultiBasis<> > &, std::vector<gsMultiBasis<> > &,
                                 std::vector<real_t> &,
                                 MarkingStrategy,
                                 real_t,
                                 int, int, int);

        void gsCalculateDistribution(gsNorm<real_t> &, std::vector<real_t> &, int,
                                     gsVector<real_t> &, gsVector<real_t> &,
                                     int);

        void gsLogTestResults(int, int, int,
                              int, int,
                              int, int,
                              real_t, int, int,
                              gsVector<index_t> &, gsMatrix<index_t> &, gsVector<index_t> &,
                              gsVector<real_t> &, gsVector<double> &, gsVector<real_t> &, gsVector<double> &, gsVector<real_t> &,
                              gsMatrix<real_t> &, gsMatrix<double> &, gsMatrix<real_t> &,
                              gsVector<real_t> &, gsVector<double> &, gsVector<real_t> &, gsVector<double> &,
                              gsVector<real_t> &, gsVector<real_t> &, gsVector<real_t> &,
                              gsVector<real_t> &, gsVector<real_t> &, gsVector<real_t> &, gsVector<real_t> &, gsVector<real_t> &);

        void gsLogRefinementIterationInfo(int refCounter, gsVector<index_t> &, gsMatrix<index_t> &, gsVector<index_t> &, gsVector<real_t> &, gsVector<real_t> &, gsVector<real_t> &,
                                          gsVector<real_t> &, gsVector<real_t> &, gsVector<real_t> &, gsVector<real_t> &, gsVector<real_t> &);

        void gsLogGeometryBasisData(gsGeometry<> &,
                                    int, int, int, int);

        void gsSaveToFileRefinementIterationInfo(bool save,
                                                 const gsField<> &v,
                                                 gsMultiBasis<> &basis,
                                                 std::vector<real_t> edDistr,
                                                 std::vector<real_t> mdDistr,
                                                 int refCounter);

        void gsSaveToFileTestResults(bool save,
                                     gsVector<index_t> &vDOFs, gsMatrix<index_t> &yDOFs, gsVector<index_t> &wDOFs,
                                     gsVector<real_t> &eVector, gsVector<real_t> &majVector, gsVector<real_t> &minVector, gsVector<real_t> &etaVector,
                                     int refTotal);

        void gsSaveToFileDivDivMMMatrices(bool, gsSparseMatrix<real_t> &, gsSparseMatrix<real_t> &,
                                          int refCounter);

        void gsSaveToFileKMatrix(bool, gsPoissonAssembler<real_t> &,
                                 int refCounter);

        //void gsSetVVector(gsMatrix<> &vector){ this->vVector = vector; }
        void gsSetVRefVector(gsMatrix<> &vector){ this->vRefVector = vector; }
        gsMatrix<> getVRefVector(){ return this->vRefVector; }

        //void gsSetWVector(gsMatrix<> &vector){ this->wVector = vector; }
        void gsSetWRefVector(gsMatrix<> &vector){ this->wRefVector = vector; }
        gsMatrix<> getWRefVector(){ return this->wRefVector; }

        //void gsSetYVector(gsMatrix<> & vector){ this->yVector = vector; }
        void gsSetYRefVector(gsMatrix<> & vector){ this->yRefVector = vector; }
        gsMatrix<> gsGetYRefVector(){ return this->yRefVector; }

        void gsInitializeBasis(int, gsMultiBasis<> &, int);

        void interpolateToRefVectorWithDirichletBC(const gsPoissonHeterogeneousAssembler<real_t> &,
                                                   const gsMultiPatch<> &,
                                                   gsMatrix<> &,
                                                   gsMatrix<> &,
                                                   size_t);
        void interpolateToRefVectorDDim(gsMultiPatch<> & mpY,
                                        gsMatrix<> &,
                                        gsMatrix<> &);

    public:
        gsMultiPatch<> patches;
        int dim;
        real_t cFriedrichs;

        std::string domainName; // domain \Omega
        std::string uExpr;      // solution
        std::string fExpr;      // right-had side
        std::string uDExpr;     // Dirichlet BC
        gsPiecewiseFunction<> aMat; // Diffusion coefficient

        std::string resultFolder;     // Dirichlet BC

        // Define test-case parameters
        const unsigned exampleNumber;
        const bool isAdaptive;

    private:
        gsMatrix<> vRefVector;
        gsMatrix<> wRefVector;
        gsMatrix<> yRefVector;

        std::vector<gsSparseMatrix<real_t> > m_localDivDivMats; /// TODO:Not optimal -> use pointers!!
        std::vector<gsSparseMatrix<real_t> > m_localDualMats;
        std::vector<gsMatrix<real_t> > m_localDivRhs;
        std::vector<gsMatrix<real_t> > m_localDualRhs;

    };


    template <typename T>
    std::vector<T> operator+(const std::vector<T>& a, const std::vector<T>& b)
    {
        assert(a.size() == b.size());

        std::vector<T> result;
        result.reserve(a.size());

        std::transform(a.begin(), a.end(), b.begin(),
                       std::back_inserter(result), std::plus<T>());
        return result;
    }
    template<unsigned d>
    void gsTestMajorant<d>::initGivenProblemData() {

        const real_t PI = 3.14159265359;
        const int NUM_PATCHES = 1;

        switch (d) {
            case 2:
                // --------------------------------------------------------------------------------
                //  Unit-square domains examples
                // --------------------------------------------------------------------------------

                if (exampleNumber == 2 || exampleNumber == 3 || exampleNumber == 4 || exampleNumber == 5 ||
                    exampleNumber == 6) {
                    real_t unit_side(1.0);
                    patches = gsNurbsCreator<>::BSplineSquareGrid(NUM_PATCHES, NUM_PATCHES+1, unit_side);
                    cFriedrichs = 1.0 / (math::sqrt((real_t) dim) * PI);
                    domainName = "unit square";
                } else if (exampleNumber == 7 || exampleNumber == 8) {
                    // --------------------------------------------------------------------------------
                    //  Rectangular domains examples
                    // --------------------------------------------------------------------------------
                    real_t lx(2.0), ly(1.0), x0(0.0), y0(0.0);
                    patches = *gsNurbsCreator<>::BSplineRectangle(x0, y0, lx, ly);
                    cFriedrichs = 1.0 / (math::sqrt((real_t) 5 / (real_t) 4) *
                                         PI);   // cFriedrichs <= 1 / (pi * sqrt(l_1^{-2} + ... + l_n^{-2}));
                    domainName = "rectangle $(0, 2) \\times (0, 1)$";

                }
                // --------------------------------------------------------------------------------
                //  L-shape domains examples
                // --------------------------------------------------------------------------------
                else if (exampleNumber == 9 || exampleNumber == 11 || exampleNumber == 12 || exampleNumber == 16) {
                    if (exampleNumber == 9) {
                        /// L-Shaped domain represented as a tensor B-spline of degree 1
                        patches = *gsNurbsCreator<>::BSplineLShape_p1();
                    } else if (exampleNumber == 11) {
                        /// L-Shaped domain represented as a tensor B-spline of degree 2
                        /// with C0-continuity across the diagonal.
                        patches = *gsNurbsCreator<>::BSplineLShape_p2C0();
                    } else if (exampleNumber == 12) {
                        /// L-Shaped domain represented as a tensor B-spline of degree 2
                        /// with C1-continuity and real_t control points at the corners.
                        patches = *gsNurbsCreator<>::BSplineLShape_p2C1();
                    }
                    cFriedrichs = 1.0 / (math::sqrt((real_t) 1 / (real_t) 2) * PI);
                    domainName = "l-shape $ ( (-1, 1) \\times (-1, 1) ) \\backslash ( [0 , 1) \\times [0, 1) )$";

                } else if (exampleNumber == 10) {
                    // --------------------------------------------------------------------------------
                    //  Annulus domains examples
                    // --------------------------------------------------------------------------------
                    int deg(2);
                    patches = *gsNurbsCreator<>::BSplineQuarterAnnulus(deg);
                    cFriedrichs = 1.0 / (math::sqrt((real_t) 1 / (real_t) 2) * PI);
                    domainName = "quater of annulus";

                } else if (exampleNumber == 11) {
                    // --------------------------------------------------------------------------------
                    //  Circle domains examples
                    // --------------------------------------------------------------------------------
                    real_t radius(1.0);
                    patches = *gsNurbsCreator<>::BSplineFatCircle(radius, 0.0, 0.0);
                    cFriedrichs = 1.0 / (math::sqrt((real_t) 1 / (real_t) 2) * PI);
                    domainName = "cirle";

                }

                break;

            case 3:
                if (exampleNumber == 13 || exampleNumber == 17 || exampleNumber == 19 || exampleNumber == 20) {
                    int deg(2);
                    patches = *gsNurbsCreator<>::BSplineCube(deg);
                    cFriedrichs = 1.0 / (math::sqrt((real_t) dim) * PI);
                    domainName = "unit cube";
                }
                else if (exampleNumber == 14 || exampleNumber == 18){
                    std::string fileSrc = gsFileManager::find( "volumes/GshapedVolume.xml" );
                    gsReadFile<real_t>(fileSrc, patches);

                    cFriedrichs = 1.0 / (math::sqrt((real_t) dim) * PI);
                    domainName = "g-shaped domain";
                }

                else if (exampleNumber == 15)
                {
                    // --------------------------------------------------------------------------------
                    //  3d L-shaped domain
                    // --------------------------------------------------------------------------------
                    real_t z(1.0);
                    gsTensorBSpline<2,real_t>::uPtr geo2D = gsNurbsCreator<>::BSplineLShape_p2C1();
                    patches = * gsNurbsCreator<>::lift3D(*geo2D, z);
                    cFriedrichs = 1.0 / (math::sqrt(3.0 / 2.0) * PI);
                    domainName = "L-shape, 3d";
                }
                break;

            default:
                gsInfo << "WARNING: The geometry has not been prescribed.\n";
        }

        switch (exampleNumber) {
            case 2:
                uExpr = "(1 - x)*x*x*(1 - y)*y";
                fExpr = "-((2 - 6*x)*(1 - y)*y + (1 - x)*x*x*(-2))";
                uDExpr = uExpr;

                break;

            case 3:
                uExpr = "sin(pi*x)*sin(pi*y)";
                fExpr = "-(-2.0*pi*pi*sin(pi*x)*sin(pi*y))";
                uDExpr = uExpr;

                break;

            case 4:
                uExpr = "sin(6.0*pi*x)*sin(3.0*pi*y)";
                fExpr = "-(-45.0*pi*pi*sin(6.0*pi*x)*sin(3.0*pi*y))";
                uDExpr = uExpr;

                break;

            case 5:
                uExpr = "cos(x)*exp(y)";
                fExpr = "0.0";
                uDExpr = uExpr;

                break;

            case 6:
                uExpr = "(x^2 - x) * (y^2 - y) * exp(-100 * ((x - 0.8)^2 + (y - 0.05)^2))";
                fExpr = "-((y^2 - y) * exp(-100 * ((x - 0.8)^2 + (y - 0.05)^2)) * ("
                        "(-100) * 2 * (x - 0.8) * ((2*x - 1) + (x^2 - x) * (-100) * 2 * (x - 0.8))"
                        "+ (2 + (-100) * 2 * ((2 * x - 1) * (x - 0.8) + (x^2 - x)))"
                        ")"
                        "+ "
                        "(x^2 - x) * exp(-100 * ((x - 0.8)^2 + (y - 0.05)^2)) * ("
                        "(-100) * 2 * (y - 0.05) * ((2*y - 1) + (y^2 - y) * (-100) * 2 * (y - 0.05))"
                        "+ (2 + (-100) * 2 * ((2 * y - 1) * (y - 0.05) + (y^2 - y)))"
                        "))";
                uDExpr = uExpr;

                break;

            case 7:
                uExpr = "(2 - x)*x*x*(1 - y)*y";
                fExpr = "-(((4 - 6*x)*(1 - y)*y + (2 - x)*x*x*(-2)))";
                uDExpr = uExpr;

                break;

            case 8:
                uExpr = "(x^2 - 2*x) * (y^2 - y) * exp(-100 * ((x - 1.4)^2 + (y - 0.95)^2))";
                fExpr = "-(exp(-100 * ((x - 1.4)^2 + (y - 0.95)^2)) * "
                        "((-100) * 2 * (x - 1.4) * ((2*x - 2) * (y^2 - y) + (x^2 - 2*x) * (y^2 - y) * (-100) * 2*(x - 1.4))"
                        "+ ((2) * (y^2 - y) + (2*x - 2) * (y^2 - y) * (-100) * 2 *(x - 1.4) + (x^2 - 2*x) * (y^2 - y) * (-100) * 2))"
                        "+ "
                        "exp(-100 * ((x - 1.4)^2 + (y - 0.95)^2)) * "
                        "((-100) * (2*(y - 0.95)) * ((x^2 - 2*x) * (2*y - 1)    + (x^2 - 2*x) * (y^2 - y) * (-100) * 2*(y - 0.95))"
                        "+ ((x^2 - 2*x) * (2)   + (x^2 - 2*x) * (2*y - 1) * (-100) * 2*(y - 0.95) + (x^2 - 2*x) * (y^2 - y) * (-100) * 2)))";
                uDExpr = uExpr;

                break;

            case 9:
            case 11:
            case 12:
            case 16:
                uExpr = "if( y > 0, (x^2 + y^2)^(1.0/3.0) * sin( (2.0*atan2(y,x) - pi)/3.0 ), "
                        "           (x^2 + y^2)^(1.0/3.0) * sin( (2.0*atan2(y,x) + 3.0*pi)/3.0 ) )";
                fExpr = "0.0";
                uDExpr = uExpr;

                break;

            case 10:
                uExpr = "cos(x)*exp(y)";
                fExpr = "0.0";
                uDExpr = uExpr;

                break;

            case 13:
                uExpr = "(1 - x)*x^2*(1 - y)*y^2*(1 - z)*z^2";
                fExpr = "-("
                        "(2 - 6*x)*(1 - y)*y^2*(1 - z)*z^2 "
                        " + (1 - x)*x^2*(2 - 6*y)*(1 - z)*z^2"
                        " + (1 - x)*x^2*(1 - y)*y^2*(2 - 6*z)"
                        ")";
                uDExpr = uExpr;

                break;

            case 14:
                // G-shape domain
                uExpr = "cos(x)*exp(y)*10*(z - 0.5)";
                fExpr = "0";
                uDExpr = uExpr;

                break;

            case 17:
                // unit cube domain
                uExpr = "cos(x)*exp(y)*(z + 1)";
                fExpr = "0";
                uDExpr = uExpr;

                break;
            case 18:
                // G-shape domain
                uExpr = "tanh(1 - 100*(x + 2*y + 4*z - 4))";
                fExpr = "420000 * tanh(1 - 100*(x + 2*y + 4*z - 4)) * (1 - (tanh(1 - 100*(x + 2*y + 4*z - 4)))^2)";
                uDExpr = uExpr;

                break;
            case 19:
                // unit cube domain
                uExpr = "tanh(1 - (x + 2*y + 4*z - 2))";
                fExpr = "42 * tanh(1 - (x + 2*y + 4*z - 2)) * (1 - (tanh(1 - (x + 2*y + 4*z - 2)))^2)";
                uDExpr = uExpr;

                break;

            case 20:
                // unit cube domain
                uExpr = "sin(pi*x)*sin(pi*y)*sin(2*pi*z)";
                fExpr = "6*pi*pi*sin(pi*x)*sin(pi*y)*sin(2*pi*z)";
                uDExpr = uExpr;

                break;
            default :
                gsInfo << "WARNING: The data functions were prescribed.\n";
        }

        gsFunctionExpr<> a1("1.e-0",dim);
        gsFunctionExpr<> a2("1.e+0",dim);
        for(size_t np = 0; np < this->patches.nPatches(); ++np)
            if(np % 2 == 0)
                aMat.addPiece(a1);
            else
                aMat.addPiece(a2);

        m_localDivDivMats.resize(this->patches.nPatches());
        m_localDualMats.resize(this->patches.nPatches());
        m_localDivRhs.resize(this->patches.nPatches());
        m_localDualRhs.resize(this->patches.nPatches());
    }

    template<unsigned d>
    void gsTestMajorant<d>::gsCreateResultsFolder(bool save, int vDegree, int yDegree, int wDegree,
                                                  int yBasisRefDelay, int wBasisRefDelay,
                                                  int numTotalAdaptRef, MarkingStrategy adaptRefCrit,
                                                  real_t markingParamTheta) {
        if (save) {
            std::string folder = "majorant-tests-poisson/example-" + util::to_string(exampleNumber)
                                 + (this->isAdaptive ? ("-adapt-marker-" + util::to_string(adaptRefCrit)
                                                        + "-theta-" + util::to_string(markingParamTheta * 100)
                                                        + "-M-" + util::to_string(yBasisRefDelay)
                                                        + "-L-" + util::to_string(wBasisRefDelay))
                                                     : ("-uniform-M-" + util::to_string(yBasisRefDelay)))
                                 + "-v-" + util::to_string(vDegree)
                                 + "-y-" + util::to_string(yDegree)
                                 + "-w-" + util::to_string(wDegree)
                                 + "-total-ref-" + util::to_string(numTotalAdaptRef);
            //+ "-based-on-eta";
            struct stat st = {0};
            if (stat(folder.c_str(), &st) == -1) gsFileManager::mkdir(folder.c_str());

            resultFolder = folder;
        }
    }

    template<unsigned d>
    void gsTestMajorant<d>::gsLogProblemData() {
        gsInfo
                << "%-------------------------------------------------------------------------------------------------%\n";
        gsInfo << "Problem statement: \n";
        gsInfo
                << "%-------------------------------------------------------------------------------------------------%\n";
        gsInfo << "u      : " << this->uExpr << "\n";
        gsInfo << "f      : " << this->fExpr << "\n";
        gsInfo << "domain : " << this->domainName << "\n";
        gsInfo << "dim    : " << this->dim << "\n";
        gsInfo
                << "%-------------------------------------------------------------------------------------------------%\n";

    }

    template<unsigned d>
    void gsTestMajorant<d>::gsLogRefinementBasisInfo(int refCounter, int numTotalAdaptRef,
                                                     gsMultiBasis<> & thbMultBasisV,
                                                     gsMultiBasis<> & thbMultBasisY,
                                                     gsMultiBasis<> & thbMultBasisW) {
        gsInfo << "\n%-----------------------------------------------------------------\n";
        gsInfo << "Refinement step " << refCounter << " of " << numTotalAdaptRef << "\n";
        gsInfo << "%-----------------------------------------------------------------\n\n";

        unsigned int numPatches = patches.nPatches();
        for (unsigned int i = 0; i < numPatches; ++i) {
            std::cout << "Patch # " << i << "\n";
            gsInfo << "thbMultBasisV info: " << thbMultBasisV.basis(i) << "\n";
            gsInfo << "thbMultBasisY info: " << thbMultBasisY.basis(i) << "\n";
            gsInfo << "thbMultBasisW info: " << thbMultBasisW.basis(i) << "\n";
        }
    }

    template<unsigned d>
    void gsTestMajorant<d>::gsLogAssemblingSolvingTime(int refCount,
                                                      gsVector<double> & timeAsmbDivDivY,
                                                      gsVector<double> & timeAsmbY,
                                                      gsVector<double> & timeAsmbW,
                                                      gsVector<double> & timeAsmbMMY,
                                                      gsMatrix<double> & timeSolvV,
                                                      gsMatrix<double> & timeSolvY,
                                                      gsMatrix<double> & timeSolvW,
                                                      gsVector<double> & timeAsmbMaj,
                                                      gsVector<double> & timeAsmbMin){
        gsInfo << "\ntime for solving V-system \n";
        gsInfo << "\t with direct solver: " << timeSolvV(refCount, 0) << " sec.\n";
        gsInfo << "\t with linear solver: " << timeSolvV(refCount, 1) << " sec.\n\n";
        gsInfo << "\n%-------------------------------------------------------------------------------------------------%\n";
        gsInfo << " Testing the majorant \n";
        gsInfo << "%-------------------------------------------------------------------------------------------------%\n";
        gsInfo << "time for assembling DivDiv-system          : " << timeAsmbDivDivY[refCount] << " sec.\n";
        gsInfo << "time for assembling MM-system              : " << timeAsmbMMY[refCount] << " sec.\n";
        gsInfo << "time for assembling DivDiv- and MM-systems : " << timeAsmbY[refCount] << " sec.\n";
        gsInfo << "\ntime for solving Y-system: \n" <<
               "\t with direct solver : " << timeSolvY(refCount, 0) << " sec.\n" <<
               "\t with linear solver : " << timeSolvY(refCount, 1) << " sec.\n";


        gsInfo << "%-------------------------------------------------------------------------------------------------%\n";
        gsInfo << " Testing the adanced majorant \n";
        gsInfo << "%-------------------------------------------------------------------------------------------------%\n";
        gsInfo << "time for assembling W-system : " << timeAsmbW[refCount] << " sec.\n";
        gsInfo << "time for solving W-system    : \n" <<
               "\twith direct solver : " << timeSolvW(refCount, 0) << " sec.\n" <<
               "\twith linear solver : " << timeSolvW(refCount, 1) << " sec.\n";

        gsInfo << "time for element-wise evaluation of the majorant:           " << timeAsmbMaj[refCount] << " sec.\n";
        gsInfo << "time for element-wise evaluation of the minorant:           " << timeAsmbMin[refCount] << " sec.\n";
    }

    template<unsigned d>
    void gsTestMajorant<d>::gsInitializeProblemData(gsFunctionExpr<> &uDFunc, gsFunctionExpr<> &fFunc,
                                                    gsPiecewiseFunction<> &alpha,
                                                    gsFunctionExpr<> &uFunc,
                                                    gsBoundaryConditions<> &bcInfo) {
        uDFunc = gsFunctionExpr<>(this->uDExpr, (int) d);
        fFunc = gsFunctionExpr<>(this->fExpr, (int) d);
        alpha = aMat;
        uFunc = gsFunctionExpr<>(this->uExpr, (int) d);

        //! [Set the Dirichlet Boundary Conditions]
        // constructor of gsFunctionExpr by an expression string and the domain dimension (real function)
        for (gsMultiPatch<>::const_biterator bit = this->patches.bBegin(); bit != this->patches.bEnd(); ++bit) {
            bcInfo.addCondition(*bit, condition_type::dirichlet, &uDFunc);
        }
        //! [Set the Dirichlet Boundary Conditions]
    }
    template<unsigned d>
    void gsTestMajorant<d>::gsRecontructV(int refCount,
                                        gsPoissonHeterogeneousAssembler<real_t>& poissonAssembler,
                                        gsBoundaryConditions<> &bcInfo,
                                        gsMatrix<real_t> &vVector,
                                        gsMultiPatch<real_t> &mpV,
                                        gsField<real_t>& v,
                                        gsVector<index_t> &vDOFs,
                                        gsVector<real_t> &stopcritVector,
                                        gsVector<double> &timeAsmbV,
                                        gsMatrix<double> &timeSolvV) {
        gsCPUStopwatch clock;
        poissonAssembler.refresh();
        poissonAssembler.system().rhs().conservativeResize(Eigen::NoChange, 1); // need to be set every time

        //! [Assemble System of Discretized Poisson Equation]
        // ---------------------------------------------------------------------------------------------------------- //
        clock.restart();
        poissonAssembler.assemble();
        timeAsmbV[refCount] = clock.stop();
        gsInfo << "time for assembling V-system: " << timeAsmbV[refCount] << " sec.\n";
        clock.restart();
        //! [Assemble System of Discretized Poisson Equation]

        //! [Dof mapper]
        // ---------------------------------------------------------------------------------------------------------- //
        vDOFs[refCount] = poissonAssembler.system().colMapper(0).size();
        //! [Dof mapper]

        //! [Solve System]
        // ---------------------------------------------------------------------------------------------------------- //
        gsSolveKhfhSystem(poissonAssembler, vVector, timeSolvV, refCount, stopcritVector);
        //! [SolvevSystem]

        //! [Recover the Approximation Field]
        // Construct the solution as gsMultiPatch and gsField
        // ---------------------------------------------------------------------------------------------------------- //
        poissonAssembler.constructSolution(vVector, mpV);
        //v = gsField<>(poissonAssembler.patches(), mpV);
        v = poissonAssembler.constructSolution(vVector);
        //! [Recover the Approximation Field]

    }
    template<unsigned d>
    void
    gsTestMajorant<d>::gsCalculateDistribution(gsNorm<real_t> &residual, std::vector<real_t> &resDistr, int elemNum,
                                               gsVector<real_t> &resValsVector,
                                               gsVector<real_t> &timeAsmb,
                                               int refCounter) {
        gsCPUStopwatch clock;
        // Compute the residual error indicator
        clock.restart();
        resValsVector[refCounter] = residual.compute(true, elemNum);
        resDistr = residual.elementNorms();

        timeAsmb[refCounter] = clock.stop();
    }

    template<unsigned d>
    void gsTestMajorant<d>::gsRecontructMajorantBasedOnOptimalFlux(int refCounter,
                                                                   gsMultiBasis<real_t> &basisY, int yDegree,
                                                                   gsMatrix<real_t> &yVector, gsMultiPatch<real_t> &mpY,
                                                                   gsMatrix<index_t> &yDOFs,
                                                                   const gsMultiPatch<real_t> &mpV, const gsField<real_t> &v,
                                                                   gsVector<real_t> &stopcritVector,
                                                                   const gsFunctionExpr<real_t> &fFunc,
                                                                   gsVector<real_t> &timeAsmblDivDivY,
                                                                   gsVector<real_t> &timeAsmblVectMassY,
                                                                   gsVector<real_t> &timeAsmblY,
                                                                   gsMatrix<real_t> &timeSolvY,
                                                                   gsVector<real_t> &timeAsmblMaj,
                                                                   gsVector<real_t> &majVector,
                                                                   gsVector<real_t> &mdVector,
                                                                   gsVector<real_t> &meqVector,
                                                                   std::vector<real_t> &mdDistr,
                                                                   std::vector<real_t> &majDistr,
                                                                   const std::vector<size_t>& elemNum) {

        // Initialize the assembler for divPDE: div(y) = f with free BC based on new basis for the flux
        // Initialize the assembler for dualPDE: y = grad(v) with free BC
        gsAssembler<> divdivAssembler, dualAssembler;
        gsCPUStopwatch clock_, clock;

        // Initialize the BVP for the generating of the optimal flux for the majorant
        gsBoundaryConditions<> freeBC;
        gsBoundaryConditions<> * pfreeBC = new gsBoundaryConditions<>(freeBC);  // get a deep copy of the free BC

        // Construct a gsPiecewiseFunction from the solution of the PDE
        gsMultiPatch<real_t> mpVCopy = mpV; // this copy is unfortunately necessary because mp is consumed in the next step
        gsPiecewiseFunction<real_t> dualRhs((gsPiecewiseFunction<double>::FunctionContainer &) mpVCopy.patches());

        // Initialize the BVP for the generating of the optimal flux for the majorant
        gsPoissonPde<real_t> divPde(this->patches, freeBC, fFunc);
        gsPoissonHeterogeneousPde<real_t> dualPde(this->patches, * pfreeBC, dualRhs, aMat);
        divdivAssembler.initialize(divPde, basisY);
        dualAssembler.initialize(dualPde, basisY);

        // Define local assemblers and pde's
        gsAssembler<>* DivA = new gsPoissonHeterogeneousAssembler<real_t>;
        gsAssembler<>* DualA = new gsPoissonAssembler<real_t>;
        gsPde<>* DivPde = NULL, *DualPde = NULL;
        index_t sysmatDofs = 0;
        gsInfo << "---------------------------------------------------------------------------------\n";
        gsInfo << " Assembling optimal system for the majorant \n";
        gsInfo << "---------------------------------------------------------------------------------\n";
        for(size_t np = 0; np < this->patches.nPatches(); ++np)
        {
            DivPde = divdivAssembler.pde().restrictToPatch(np);
            DualPde = dualAssembler.pde().restrictToPatch(np);
            DivA->initialize(*DivPde, basisY.basis(np), divdivAssembler.options());
            DualA->initialize(*DualPde, basisY.basis(np), dualAssembler.options());
            DivA->system().rhs().conservativeResize(Eigen::NoChange, 1); //TODO: find a better option
            DualA->system().rhs().conservativeResize(Eigen::NoChange, 1); //TODO: find a better option

            // Assembling of the system
            // (MM + C_F^2 / beta * DivDiv) * Y = RHS_{MM} + C_F^2 / beta * RHS_{DivDiv}
            this->gsGenerateDivDivMMMatrices(refCounter, divPde, dualPde,
                                             basisY, yDegree, yDOFs,
                                             mpV, v, fFunc,
                                             *DivA, *DualA,
                                             timeAsmblDivDivY, timeAsmblVectMassY, timeAsmblY, np, false);

            m_localDivDivMats[np] = DivA->matrix();
            m_localDualMats[np] = DualA->matrix();
            m_localDivRhs[np] = DivA->rhs();
            m_localDualRhs[np] = DualA->rhs();

            sysmatDofs += m_localDivDivMats[np].rows();
/*
            gsInfo << "divdivM = \n" << m_localDivDivMats[np].toDense() << "\n";
            gsInfo << "divdivRhs = \n" << m_localDivRhs[np] << "\n";
            gsInfo << "vectmassM = \n" << m_localDualMats[np].toDense() << "\n";
            gsInfo << "vectmassRhs = \n" << m_localDualRhs[np] << "\n";
*/

        }
        gsSparseMatrix<real_t> yM(sysmatDofs, sysmatDofs); //divdivM(sysmatDofs, sysmatDofs), vectmassM(sysmatDofs, sysmatDofs);
        gsMatrix<real_t> yRhs(sysmatDofs, 1); //divdivRhs, vectmassRhs;

        real_t beta(1.0);
        real_t mEq(0.0), mD(0.0), maj(0.0);
        gsMatrix<real_t> tmpMeq;
        std::vector<real_t> meqDistr;
        real_t ratioDualEq(5.0);
        int iterMajOpt = 2;


        gsInfo << "---------------------------------------------------------------------------------\n";
        gsInfo << " Assemble and solve the big system \n";
        gsInfo << "---------------------------------------------------------------------------------\n";
        // TODO: Subtract the part related to the jump of y
        gsSparseEntries<> entries;
        for (index_t i = 0; i < iterMajOpt; i++)
        {
            entries.clear();
            sysmatDofs = 0; //Reuse the variable from above
            // Algorithm 3.2, Section 3.3, Mali, Repin, Neittaanmaki, 2014
            //yM = vectmassM + math::pow(this->cFriedrichs, 2) / beta * divdivM;
            //yRhs = vectmassRhs + math::pow(this->cFriedrichs, 2) / beta * divdivRhs;
            for (size_t np = 0; np < this->patches.nPatches(); ++np)
            {
                m_localDivDivMats[np] *= math::pow(this->cFriedrichs, 2) / beta;
                m_localDivRhs[np] *= math::pow(this->cFriedrichs, 2) / beta;
                for (index_t i = 0; i < m_localDivDivMats[np].rows(); ++i)
                    for (index_t j = 0; j < m_localDivDivMats[np].cols(); ++j)
                        entries.add(sysmatDofs + i, sysmatDofs + j, m_localDivDivMats[np](i, j)
                            + m_localDualMats[np](i, j));

                yRhs.block(sysmatDofs, 0, m_localDivDivMats[np].rows(), 1) =
                    m_localDivRhs[np] + m_localDualRhs[np];
                sysmatDofs += m_localDivDivMats[np].rows();
            }
            yM.setFrom(entries);
            //gsInfo << "matrix: \n" << yM.toDense() << "\n";


            this->gsSolveMajorantOptimalSystem(yM, yRhs, yVector, timeSolvY, refCounter, i, yDOFs, stopcritVector);
            //yVector.resize(yDOFs[refCounter], this->dim);
            this->gsSetYRefVector(yVector);


            gsInfo << "---------------------------------------------------------------------------------\n";
            gsInfo << " Compute the errors element wise on each patch \n";
            gsInfo << "---------------------------------------------------------------------------------\n";
            gsMultiPatch<real_t> mpYpatch;
            gsMatrix<real_t> ypatchVector;
            sysmatDofs = 0;
            for (size_t np = 0; np < this->patches.nPatches(); ++np)
            {
                gsInfo << "Patch #" << np << "\n";
                gsMultiPatch<real_t> cur; cur.addPatch(this->patches.patch(np));
                ypatchVector = gsMatrix<real_t>(yVector.block(sysmatDofs, 0, m_localDivRhs[np].rows(),1)).reshape(yDOFs(refCounter, np), this->dim);
                DivA->constructSolution(ypatchVector, mpYpatch);
                sysmatDofs += m_localDivRhs[np].rows();
                const gsField<real_t> y(cur, mpYpatch);

                clock.restart();

                // Calculate the terms of the majorant separately
                gsMultiPatch<real_t> patchV; patchV.addPatch(mpV.patch(np));
                gsField<real_t> vField(cur, patchV);
                gsErrEstDualMajorant<real_t> mdIndicator(vField, mpYpatch);
                gsErrEstEquilMajorant<real_t> meqIndicator(y, fFunc);


                gsCPUStopwatch clock_1, clock_2;

#pragma omp parallel sections
                {
#pragma omp section
                    {
                        clock_1.restart();
                        mEq = meqIndicator.compute(true, elemNum[np]);
                        gsInfo << "time for element-wise evaluation of the mEq: " << clock_1.stop() << " sec.\n";
                    }
#pragma omp section
                    {
                        clock_2.restart();
                        mD = mdIndicator.compute(true, elemNum[np]);
                        gsInfo << "time for element-wise evaluation of the mD: " << clock_2.stop() << " sec.\n";
                    }
                }

                timeAsmblMaj[refCounter] += clock.stop();

                //gsInfo << "mD  = " << mD << "\n";
                //gsInfo << "mEq = " << mEq << "\n";
                gsInfo << "increment time for element-wise evaluation of the majorant: " << clock.stop() << " sec.\n";

                // Update beta
                beta = this->cFriedrichs * mEq / mD;
                // Update error estimate
                if (mEq < 1e-16)
                {
                    maj = mD;
                    //majDistr = mdDistr;
                }
                else
                {
                    if (mD < 1e-16)
                    {
                        maj = cFriedrichs * mEq;
                        //majDistr = math::pow(cFriedrichs, 2) * meqDistr;
                    }
                    else
                    {
                        maj = math::sqrt(
                            (1 + beta) * math::pow(mD, 2) + (1 + 1 / beta) * math::pow(cFriedrichs * mEq, 2));
                        //maj_ = math::sqrt((1 + beta) * mD_ + (1 + 1 / beta_) * math::pow(cFriedrichs, 2) * mEq_);
                        //majDistr = (1 + beta) * mdDistr + (1 + 1 / beta) * math::pow(cFriedrichs, 2) * meqDistr;
                    }
                }
                std::cout << std::scientific;
                gsInfo << "iter " << i << ": \t" << "maj  = " << maj << "\t" << "majD  = " << mD << "\t" << "majEq  = "
                       << mEq
                       << "\n";
                std::cout.unsetf(std::ios::scientific);

                // If the ratio between m_d and mEq is more then certain threshhold, do no continue to optimize
                if (mD / mEq >= ratioDualEq) iterMajOpt = i;

                majVector[refCounter] = maj;
                mdVector[refCounter] = mD;
                meqVector[refCounter] = mEq;

                // Get distribution of the local majorant only at the last optimisation iteration
                ///*
                if ((i == iterMajOpt) || (i == iterMajOpt - 1))
                {
                    mdDistr = mdIndicator.elementNorms();
                    meqDistr = meqIndicator.elementNorms();

                    if (mEq < 1e-16) majDistr = mdDistr;
                    else
                    {
                        if (mD < 1e-16)
                        {
                            std::transform(meqDistr.begin(), meqDistr.end(), majDistr.begin(),
                                           std::bind1st(std::multiplies<real_t>(), math::pow(cFriedrichs, 2)));
                        }
                        else
                        {
                            std::transform(mdDistr.begin(), mdDistr.end(), mdDistr.begin(),
                                           std::bind1st(std::multiplies<real_t>(), (1 + beta)));
                            std::transform(meqDistr.begin(), meqDistr.end(), meqDistr.begin(),
                                           std::bind1st(std::multiplies<real_t>(),
                                                        (1 + 1 / beta) * math::pow(cFriedrichs, 2)));
                            majDistr.reserve(mdDistr.size());
                            std::transform(mdDistr.begin(), mdDistr.end(), meqDistr.begin(),
                                           std::back_inserter(majDistr), std::plus<real_t>());
                        }
                    }
                }
                //*/
            } // End error computation
        } // End optimization loop

        delete DivPde; delete DualPde;
        delete DivA; delete DualA;
    }

    template<unsigned d>
    void gsTestMajorant<d>::gsRecontructMajorantBasedOnEquilibratedFlux(int refCounter,
                                                                        gsMultiBasis<real_t> &basisY, int yDegree,
                                                                        gsMatrix<real_t> &yVector, gsMultiPatch<real_t> &mpY,
                                                                        gsVector<index_t> &yDOFs,
                                                                        gsMultiPatch<real_t> &mpV, gsField<> &v,
                                                                        gsFunctionExpr<real_t> &fFunc,
                                                                        gsVector<real_t> &timeAsmblEquilY,
                                                                        gsMatrix<real_t> &timeSolvEquilY,
                                                                        gsVector<real_t> &timeAsmblMaj,
                                                                        gsVector<real_t> &majVector,
                                                                        gsVector<real_t> &mdVector,
                                                                        gsVector<real_t> &meqVector,
                                                                        std::vector<real_t> &mdDistr,
                                                                        std::vector<real_t> &majDistr,
                                                                        int elemNum) {

        // Initialtize the assembler for div(y) = f with free BC based on new basis for the flux y
        gsAssembler<> divdivAssembler;
        gsBoundaryConditions<> freeBC;
        int numMappers = 1;
        gsCPUStopwatch clock;

        //gsInfo << "\n%-------------------------------------------------------------------------------------------------%\n";
        //gsInfo << " DivDiv assembling ... \n";
        //gsInfo << "%-------------------------------------------------------------------------------------------------%\n";
        // Definition of the auxialary equation div(y) = f with free BC,
        // which will be tested by div(w) to generate [div(y) * div(w)]_{ij} matrix and [f * div(w)]_{j} rhs
        gsPoissonPde<> divPde(this->patches, freeBC, fFunc);
        // Initialtize the assembler for div(y) = f with free BC based on new basis for the flux y
        // gsAssembler<> divdivAssembler;
        // Initialtize Dof poissonMapper for div(y) = f equation
        gsDofMapper divdivMapper; // Gets the indices mapped from Basis --> Matrix

        divdivAssembler.initialize(divPde, basisY);
        divdivAssembler.options().setInt("InterfaceStrategy", iFace::conforming);

        basisY.getMapper((iFace::strategy) divdivAssembler.options().getInt("InterfaceStrategy"),
                         divdivMapper, 0);
        divdivMapper.finalize();
        yDOFs[refCounter] = divdivMapper.size();

        std::vector<gsDofMapper> divdivMappers(numMappers, divdivMapper);
        // Generate the sparse matrix for div(y) * div(w) = f * div(w) system
        clock.restart();
        gsSparseSystem<> divdivSys(divdivMappers, this->dim, this->dim);
        //divdivSys.reserve(this->dim * math::ipow(this->dim * yDegree + 1, this->dim), divPde.numRhs());
        divdivSys.reserve(math::ipow(this->dim * (this->dim * yDegree + 1), this->dim), divPde.numRhs());
        divdivAssembler.setSparseSystem(divdivSys);             // set the spare matrix for the system
        divdivAssembler.push<gsVisitorDivDiv<real_t> >();       // push the assembling procedure
        divdivAssembler.finalize();
        timeAsmblEquilY[refCounter] = clock.stop();
        gsInfo << "time for equilibration : " << timeAsmblEquilY[refCounter] << "\n";


        real_t beta(1.0);

        gsSparseMatrix<real_t> divdivM = divdivAssembler.matrix();
        gsMatrix<real_t> divdivRhs = divdivAssembler.rhs();

        gsSparseMatrix<real_t> yM;
        gsMatrix<real_t> yRhs;

        real_t mEq(0.0), mD(0.0), maj(0.0);
        std::vector<real_t> meqDistr;

        yM = divdivM;
        yRhs = divdivRhs;

        //this->gsSolveMajorantOptimalSystem(yM, yRhs, yVector, timeSolvEquilY, refCounter, 0, yDOFs, stopcritVector);
        yVector.resize(yDOFs[refCounter], this->dim);
        this->gsSetYRefVector(yVector);

        divdivAssembler.constructSolution(yVector, mpY);
        const gsField<real_t> y(divdivAssembler.patches(), mpY);

        clock.restart();
        // Calculate the terms of the majorant separately
        gsErrEstDualMajorant<real_t> mdIndicator(v, mpY);
        gsErrEstEquilMajorant<real_t> meqIndicator(y, fFunc);

        clock.restart();
        mEq = meqIndicator.compute(true, elemNum);
        mD = mdIndicator.compute(true, elemNum);
        timeAsmblMaj[refCounter] = clock.stop();
        gsInfo << "increment time for element-wise evaluation of the majorant: " << timeAsmblMaj[refCounter] << " sec.\n";

        // Update beta
        beta = this->cFriedrichs * mEq / mD;
        // Update error estimate
        if (mEq < 1e-16) {
            maj = mD;
        } else {
            if (mD < 1e-16) {
                maj = cFriedrichs * mEq;
            } else {
                maj = math::sqrt((1 + beta) * math::pow(mD, 2) + (1 + 1 / beta) * math::pow(cFriedrichs * mEq, 2));
            }
        }
        std::cout << std::scientific;
        gsInfo << "maj = " << maj << "\t" << "majD = " << mD << "\t" << "majEq = " << mEq << "\n";
        std::cout.unsetf(std::ios::scientific);


        majVector[refCounter] = maj;
        mdVector[refCounter] = mD;
        meqVector[refCounter] = mEq;

        mdDistr = mdIndicator.elementNorms();

    }


    template<unsigned d>
    void gsTestMajorant<d>::gsRecontructMinorantBasedOnAuxiliaryW(int refCounter, gsPoissonHeterogeneousAssembler<real_t>& poissonAssemblerW, gsBoundaryConditions<> &bcInfo,
                                                                   gsMatrix<real_t> &wVector, gsMultiPatch<real_t> &mpW, gsField<>& w, gsVector<index_t> &wDOFs,
                                                                   gsVector<real_t> &stopcritVector,
                                                                   const gsField<> &v, const gsFunctionExpr<real_t> &fFunc,
                                                                   gsVector<real_t> &timeAsmblW,
                                                                   gsMatrix<real_t> &timeSolvW,
                                                                   gsVector<real_t> &timeAsmblMin,
                                                                   gsVector<real_t> &minVector,
                                                                   std::vector<real_t> &minDistr,
                                                                   int elemNum) {
        gsCPUStopwatch clock;

        poissonAssemblerW.refresh();
        poissonAssemblerW.system().rhs().conservativeResize(Eigen::NoChange, 1); // needs to be set every time

        //! [Assemble System of Discretized Poisson Equation]
        // ---------------------------------------------------------------------------------------------------------- //
        clock.restart();
        poissonAssemblerW.assemble();
        timeAsmblW[refCounter] = clock.stop();
        clock.restart();
        //! [Assemble System of Discretized Poisson Equation]

        //! [Dof mapper]
        // ---------------------------------------------------------------------------------------------------------- //
        gsDofMapper poissonMapper; // Gets the indices mapped from Basis --> Matrix
        poissonAssemblerW.multiBasis(0).getMapper((dirichlet::strategy) poissonAssemblerW.options().getInt("DirichletStrategy"),
                                                             (iFace::strategy) poissonAssemblerW.options().getInt("InterfaceStrategy"),
                                                             bcInfo, poissonMapper, 0);
        wDOFs[refCounter] = poissonMapper.size();
        //! [Dof mapper]

        //! [Solve System]
        // ---------------------------------------------------------------------------------------------------------- //
        this->gsSolveKhwhfhSystem(poissonAssemblerW, wVector, timeSolvW, refCounter, stopcritVector);
        //! [SolvevSystem]

        //! [Recover the Approximation Field]
        // Construct the solution as gsMultiPatch and gsField
        poissonAssemblerW.constructSolution(wVector, mpW);
        w = gsField<>(poissonAssemblerW.patches(), mpW);

        real_t min(0.0);
        gsErrEstMinorant<real_t> minIndicator(v, w, fFunc);

        clock.restart();
        min = minIndicator.compute(false, elemNum);
        timeAsmblMin[refCounter] = clock.stop();

        minVector[refCounter] = min;
    }

    template<unsigned d>
    void gsTestMajorant<d>::gsGenerateDivDivMMMatrices(int refCounter,
                                                       gsPoissonPde<> &divPde, gsPoissonHeterogeneousPde<real_t> &dualPde,
                                                       gsMultiBasis<real_t> &basisY, int yDegree,
                                                       gsMatrix<index_t> &yDOFs,
                                                       const gsMultiPatch<real_t> &mpV, const gsField<> &v,
                                                       const gsFunctionExpr<real_t> &fFunc,
                                                       gsAssembler<> &divdivAssembler, gsAssembler<> &dualAssembler,
                                                       gsVector<real_t> &timeAsmblDivDivY,
                                                       gsVector<real_t> &timeAsmblVectMassY,
                                                       gsVector<real_t> &timeAsmblY,
                                                       size_t np,
                                                       bool saveToFile) {
        //gsBoundaryConditions<> freeBC;
        int numMappers = 1;
        gsCPUStopwatch clock_1, clock_2, clock;

        // Initialtize Dof poissonMapper for div(y) = f equation
        gsDofMapper divdivMapper; // Gets the indices mapped from Basis --> Matrix
        // Initialtize Dof poissonMapper for y = grad(v) equation with free BC
        gsDofMapper dualMapper;

        clock.restart();

        #pragma omp parallel sections
        {
            #pragma omp section
            {
                //divdivAssembler.initialize(divPde, basisY);
                //divdivAssembler.options().setInt("InterfaceStrategy", iFace::conforming);

                basisY.getMapper((iFace::strategy) iFace::conforming,
                                 divdivMapper, 0);
                divdivMapper.finalize();
                yDOFs(refCounter, np) = divdivMapper.size();

                std::vector<gsDofMapper> divdivMappers(numMappers, divdivMapper);
                // Generate the sparse matrix for div(y) * div(w) = f * div(w) system
                clock_1.restart();
                gsSparseSystem<> divdivSys(divdivMappers, this->dim, this->dim);
                //divdivSys.reserve(this->dim * math::ipow(this->dim * yDegree + 1, this->dim), divPde.numRhs());
                divdivSys.reserve(math::ipow(this->dim * (this->dim * yDegree + 1), this->dim), divPde.numRhs());
                divdivAssembler.setSparseSystem(divdivSys);             // set the sparse matrix for the system
                divdivAssembler.push<gsVisitorDivDiv<real_t> >();       // push the assembling procedure
                divdivAssembler.finalize();
                timeAsmblDivDivY[refCounter] = clock_1.stop();
            }
            #pragma omp section
            {
                //gsPoissonPde<> dualPde(this->patches, freeBC, mpV.piece(0));
                //dualAssembler.initialize(dualPde, basisY);
                basisY.getMapper((iFace::strategy) dualAssembler.options().getInt("InterfaceStrategy"), dualMapper, 0);
                dualMapper.finalize();
                std::vector<gsDofMapper> dualMappers(numMappers, dualMapper);
                clock_2.restart();
                gsSparseSystem<> vectMassSys(dualMappers, this->dim, this->dim);
                // this->dim * (this->dim * yDegree + 1)
                //vectMassSys.reserve(math::ipow(this->dim * yDegree + 1, this->dim), dualPde.numRhs());
                // this->dim * math::ipow(this->dim * yDegree + 1
                vectMassSys.reserve(math::ipow(this->dim * (this->dim * yDegree + 1), this->dim), dualPde.numRhs());
                dualAssembler.setSparseSystem(vectMassSys);
                dualAssembler.push<gsVisitorDualPoisson<real_t> >();
                dualAssembler.finalize();
                timeAsmblVectMassY[refCounter] = clock_2.stop();
            }
        }
        timeAsmblY[refCounter] = clock.stop();
    }

    template<unsigned d>
    void gsTestMajorant<d>::gsLogTestResults(int vDegree, int yDegree, int wDegree,
                                             int m, int l,
                                             int yBasisRefDelay, int wBasisRefDelay,
                                             real_t markingParamTheta, int numInitUniformRef, int numTotalAdaptRef,
                                             gsVector<index_t> &vDOFs, gsMatrix<index_t> &yDOFs,
                                             gsVector<index_t> &wDOFs,
                                             gsVector<real_t> &timeAsmbV, gsVector<double> &timeAsmbDivDivY,
                                             gsVector<real_t> &timeAsmbMMY, gsVector<double> &timeAsmbY,
                                             gsVector<real_t> &timeAsmbW,
                                             gsMatrix<real_t> &timeSolvV, gsMatrix<double> &timeSolvY,
                                             gsMatrix<real_t> &timeSolvW,
                                             gsVector<real_t> &timeAsmbError, gsVector<double> &timeAsmbMajorant,
                                             gsVector<real_t> &timeAsmbMinorant, gsVector<double> &timeAsmbEtaIndicator,
                                             gsVector<real_t> &eVector, gsVector<real_t> &relErrorVector,
                                             gsVector<real_t> &relError0Vector,
                                             gsVector<real_t> &majVector, gsVector<real_t> &mdVector,
                                             gsVector<real_t> &meqVector, gsVector<real_t> &minVector,
                                             gsVector<real_t> &etaVector) {
        gsInfo
                << "\n%-------------------------------------------------------------------------------------------------%\n";

        if (this->isAdaptive) gsInfo << "Adaptive refinement results: \n";
        else
            gsInfo << "Uniform refinement results: \n";
        gsInfo
                << "%-------------------------------------------------------------------------------------------------%\n";

        gsInfo << "v in S^{" << vDegree << ", " << vDegree << "}\n";
        gsInfo << "y in S^{" << yDegree << ", " << yDegree << "} + S^{" << yDegree << ", " << yDegree << "}\n";
        gsInfo << "w in S^{" << wDegree << "}\n\n";


        gsInfo << "Inital refiniments:     REF_0 = " << numInitUniformRef << "\n";
        gsInfo << "Total refiniments:      REF   = " << numTotalAdaptRef << "\n";
        gsInfo << "Difference in the degrees of v and y: m = " << m << "\n";
        gsInfo << "Difference in the degrees of v and w: l = " << l << "\n";

        if (this->isAdaptive) {
            gsInfo << "Bulk parameter:         theta = " << markingParamTheta * 100 << " % \n";
            gsInfo << "Delay in refining of y: M     = " << yBasisRefDelay << "\n";
            gsInfo << "Delay in refining of w: L     = " << wBasisRefDelay << "\n";
        } else {
            gsInfo << "Coarsening parameter: M = " << yBasisRefDelay << "\n";
            gsInfo << "Coarsening parameter: L = " << wBasisRefDelay << "\n";
        }

        gsInfo << "\n\n REF &    DOFs(v) &    DOFs(y) &    DOFs(w) & "
               << " t_{as}(v) &  t_{as}(y) &  t_{as}(w) &"
               << " t_{sol,  dir}(v) & t_{sol,  dir}(y) & t_{sol,  dir}(w) &"
               << " t_{err, ass} & t_{maj, ass} & t_{min, ass} & t_{res, ass} \\\\\n";
        for (int refCounter = 0; refCounter < numTotalAdaptRef; ++refCounter) {
            std::printf("%4u & %10u & %10u & %10u & %10.2e & %10.2e & %10.2e & %16.2e & %16.2e & %16.2e & %12.2e & %12.2e & %12.2e & %12.2e \\\\\n",
                        refCounter + 1,
                        vDOFs[refCounter],
                        yDOFs.row(refCounter).sum(),
                        wDOFs[refCounter],
                        timeAsmbV[refCounter],
                        timeAsmbY[refCounter], //timeAsmbDivDivY[refCounter] + timeAsmbDivDivY[refCounter],
                        timeAsmbW[refCounter],
                        timeSolvV(refCounter, 0),
                        timeSolvY(refCounter, 0),
                        timeSolvW(refCounter, 0),
                        timeAsmbError[refCounter],
                        timeAsmbMajorant[refCounter],
                        timeAsmbMinorant[refCounter],
                        timeAsmbEtaIndicator[refCounter]);
        }

        gsInfo << "\n REF &    DOFs(v) &    DOFs(y) &    DOFs(w) & "
               << " t_{as}(v) &  t_{as}(y) &  t_{as}(w) &"
               << " t_{sol, iter}(v) & t_{sol, iter}(y) & t_{sol, iter}(w) &"
               << " t_{err, ass} & t_{maj, ass} & t_{min, ass} & t_{res, ass} \\\\\n";
        for (int refCounter = 0; refCounter < numTotalAdaptRef; ++refCounter) {
            std::printf("%4u & %10u & %10u & %10u & %10.2e & %10.2e & %10.2e & %16.2e & %16.2e & %16.2e & %12.2e & %12.2e & %12.2e & %12.2e \\\\\n",
                        refCounter + 1,
                        vDOFs[refCounter],
                        yDOFs.row(refCounter).sum(),
                        wDOFs[refCounter],
                        timeAsmbV[refCounter],
                        timeAsmbY[refCounter], //timeAsmbDivDivY[refCounter] + timeAsmbDivDivY[refCounter],
                        timeAsmbW[refCounter],
                        timeSolvV(refCounter, 1),
                        timeSolvY(refCounter, 1),
                        timeSolvW(refCounter, 1),
                        timeAsmbError[refCounter],
                        timeAsmbMajorant[refCounter],
                        timeAsmbMinorant[refCounter],
                        timeAsmbEtaIndicator[refCounter]);
        }


        gsInfo << "\n REF &    DOFs(v) &    DOFs(y) &    DOFs(w) & t_{sol,  dir}(v) & t_{sol, iter}(v) &  t_{sol,  dir}(y) & t_{sol, iter}(y) & t_{sol,  dir}(w) & t_{sol, iter}(w) \\\\\n";
        for (int refCounter = 0; refCounter < numTotalAdaptRef; ++refCounter) {
            std::printf("%4u & %10u & %10u & %10u & %16.2e & %16.2e & %16.2e & %16.2e & %16.2e & %16.2e \\\\\n",
                        refCounter + 1,
                        vDOFs[refCounter],
                        yDOFs.row(refCounter).sum(),
                        wDOFs[refCounter],
                        timeSolvV(refCounter, 0), timeSolvV(refCounter, 1),
                        timeSolvY(refCounter, 0), timeSolvY(refCounter, 1),
                        timeSolvW(refCounter, 0), timeSolvW(refCounter, 1));
        }

        gsInfo << "\n REF &  || u - u_i || &  || u_i - u_{i-1} || & \t        maj & \t      majD & \t    majEq & \t     min & I_{eff}(maj) & I_{eff}(min) &    maj / min & I_{eff}(eta) & order(e) \\\\\n";
        for (int refCounter = 0; refCounter < numTotalAdaptRef - 1; ++refCounter) {
            std::printf("%4u & %14.4e & %20.4e & %12.4e & %12.4e & %12.4e & %12.4e & %12.4f & %12.4f & %12.4f & %12.4f & %8.4f \\\\\n",
                        refCounter + 2,
                        cast<real_t, double>(eVector[refCounter + 1]),
                        cast<real_t, double>(relErrorVector[refCounter]),
                        cast<real_t, double>(majVector[refCounter + 1]),
                        cast<real_t, double>(mdVector[refCounter + 1]),
                        cast<real_t, double>(meqVector[refCounter + 1]),
                        cast<real_t, double>(minVector[refCounter + 1]),
                        cast<real_t, double>(majVector[refCounter + 1] / eVector[refCounter + 1]),
                        cast<real_t, double>(minVector[refCounter + 1] / eVector[refCounter + 1]),
                        cast<real_t, double>(majVector[refCounter + 1] / minVector[refCounter + 1]),
                        cast<real_t, double>(etaVector[refCounter + 1] / eVector[refCounter + 1]),
                        cast<real_t, double>(math::log(eVector[refCounter + 1] / eVector[refCounter]) /
                                             math::log(math::pow((real_t) vDOFs[refCounter + 1],
                                                               ((real_t) (-1) / (real_t) d)) /
                                                      math::pow((real_t) vDOFs[refCounter],
                                                               ((real_t) (-1) / (real_t) d)))));
        }

        gsInfo << "\n REF &  || u - u_i || & \t maj & I_{eff}(maj) &  \t       min & I_{eff}(min) &     maj / min & order(e) \\\\\n";
        for (int refCounter = 0; refCounter < numTotalAdaptRef - 1; ++refCounter) {
            std::printf("%4u & %14.4e & %12.4e & %12.4f & %12.4e & %12.4f & %12.4f & %8.4f \\\\\n",
                        refCounter + 2,
                        cast<real_t, double>(eVector[refCounter + 1]),
                        cast<real_t, double>(majVector[refCounter + 1]),
                        cast<real_t, double>(majVector[refCounter + 1] / eVector[refCounter + 1]),
                        cast<real_t, double>(minVector[refCounter + 1]),
                        cast<real_t, double>(minVector[refCounter + 1] / eVector[refCounter + 1]),
                        cast<real_t, double>(majVector[refCounter + 1] / minVector[refCounter + 1]),
                        cast<real_t, double>(math::log(eVector[refCounter + 1] / eVector[refCounter]) /
                                             math::log(math::pow((real_t) vDOFs[refCounter + 1],
                                                               ((real_t) (-1) / (real_t) d)) /
                                                      math::pow((real_t) vDOFs[refCounter],
                                                               ((real_t) (-1) / (real_t) d)))));
        }

        gsInfo << "\n-------------------------------------------------------------------------------------------------\n";
        gsInfo << " Table with the best times: \n";
        gsInfo << "-------------------------------------------------------------------------------------------------\n";
        gsInfo << "\n";
        gsInfo << " REF &    DOFs(v) &    DOFs(y) &    DOFs(w) & "
               << " t_{as}(v) &  t_{as}(y) &  t_{as}(w) & "
               << " t_{sol, opt}(v) &  t_{sol, opt}(y) &  t_{sol, opt}(w) & "
               << " t_{err, ass} & t_{maj, ass} & t_{min, ass} & t_{res, ass} \\\\\n";
        for (int refCounter = 0; refCounter < numTotalAdaptRef; ++refCounter) {
            std::printf("%4u & %10u & %10u & %10u & %10.2e & %10.2e & %10.2e & %16.2e & %16.2e & %16.2e & %12.2e & %12.2e & %12.2e & %12.2e \\\\\n",
                        refCounter + 1,
                        vDOFs[refCounter],
                        yDOFs.row(refCounter).sum(),
                        wDOFs[refCounter],
                        timeAsmbV[refCounter],
                        timeAsmbY[refCounter], //timeAsmbDivDivY[refCounter] + timeAsmbDivDivY[refCounter],
                        timeAsmbW[refCounter],
                        (timeSolvV(refCounter, 0) < timeSolvV(refCounter, 1) && timeSolvV(refCounter, 0) != 0.0) ? timeSolvV(refCounter, 0) : timeSolvV(refCounter, 1),
                        (timeSolvY(refCounter, 1) < timeSolvY(refCounter, 0) && timeSolvY(refCounter, 1) != 0.0) ? timeSolvY(refCounter, 1) : timeSolvY(refCounter, 0),
                        (timeSolvW(refCounter, 1) < timeSolvW(refCounter, 0) && timeSolvW(refCounter, 1) != 0.0) ? timeSolvW(refCounter, 1) : timeSolvW(refCounter, 0),
                        timeAsmbError[refCounter],
                        timeAsmbMajorant[refCounter],
                        timeAsmbMinorant[refCounter],
                        timeAsmbEtaIndicator[refCounter]);
        }
    }

    template<unsigned d>
    void gsTestMajorant<d>::gsLogRefinementIterationInfo(int refCounter,
                                                         gsVector<index_t> &vDOFs, gsMatrix<index_t> &yDOFs, gsVector<index_t> &wDOFs,
                                                         gsVector<real_t> &eVector, gsVector<real_t> &relErrorVector, gsVector<real_t> &relError0Vector, gsVector<real_t> &thetaVector, gsVector<real_t> &stopcritVector,
                                                         gsVector<real_t> &majVector, gsVector<real_t> &minVector,
                                                         gsVector<real_t> &etaVector) {

        gsInfo
                << "\n%-------------------------------------------------------------------------------------------------%\n";
        gsInfo << " Resulting values of the majorant and the residual error indicator: \n";
        gsInfo
                << "%-------------------------------------------------------------------------------------------------%\n";
        gsInfo << "DOFs(v)   = " << vDOFs[refCounter] << "\n"
               << "DOFs(y)   = " << yDOFs.row(refCounter).sum() << "\n"
               << "DOFs(w)   = " << wDOFs[refCounter] << "\n";
        gsInfo << "min   = " << minVector[refCounter] << "\t"
               << "iEff(min)   = " << minVector[refCounter] / eVector[refCounter] << "\n"
               << "[e]   = " << eVector[refCounter] << "\n"
               << "maj   = " << majVector[refCounter] << "\t"
               << "iEff(maj)   = " << majVector[refCounter] / eVector[refCounter] << "\n"
               << "maj / min   = " << majVector[refCounter] / minVector[refCounter] << "\n";
        gsInfo << "eta_K = " << etaVector[refCounter] << "\t"
               << "iEff(eta_K) = " << etaVector[refCounter] / eVector[refCounter] << "\n\n";

        if (refCounter > 0) {

            gsInfo << "Theta_i     = [e]_i / [e]_{i-1}   = " << thetaVector[refCounter-1] << "\n";
            gsInfo << "eps0_i      = || u_i - u_0 ||     = " << relErrorVector[refCounter-1] << "\n";
            gsInfo << "delta_i     = || u_i - u_{i-1} || = " << relError0Vector[refCounter-1] << "\n";
            gsInfo << "stop_crit_i = (1 - Theta_i) * TOL * rho * eps0_i = " << stopcritVector[refCounter-1] << "\n";

            gsInfo << "|| u - u_{i-1} || <= || u_i - u_{i-1} || / (1 - Theta): "
                   << relErrorVector[refCounter-1] << " <= "
                   << relErrorVector[refCounter-1] / (1 - eVector[refCounter] / eVector[refCounter - 1])<< "\n\n";
        };
    }

    template<unsigned d>
    void gsTestMajorant<d>::gsInitializeBasis(int degree,
                                              gsMultiBasis<> &multBasis,
                                              int numInitUniformRef) {
        if (this->isAdaptive) {
            // Copy basis from the multi-patch geometry (one per patch) for v and flux
            std::vector<gsBasis<real_t>* > basisContainer(this->patches.nPatches());
            for(size_t np = 0; np < this->patches.nPatches(); ++np)
            {
                gsTensorBSpline<d, real_t>
                    * geo = dynamic_cast< gsTensorBSpline<d, real_t>* >( &(this->patches.patch(np)));
                gsTensorBSplineBasis<d, real_t> tensorBSplineBasis = geo->basis();

                // Set the degree of the basises for the approximation
                tensorBSplineBasis.setDegree(degree);
                gsTHBSplineBasis<d, real_t> thbBasis(tensorBSplineBasis);
                basisContainer[np] = &thbBasis;
            }

            gsBoxTopology topology(this->patches.topology());
            multBasis = gsMultiBasis<>(basisContainer, topology);

        } else {
            // Copy basis from the multi-patch geometry (one per patch) for v and flux
            multBasis = gsMultiBasis<>(patches);
            multBasis.setDegree(degree); //multiBasis.setDegreePreservingMultiplicity(degree)
        }

        //! [Initial Refinement]
        for (int i = 0; i < numInitUniformRef; ++i)    multBasis.uniformRefine();
        //! [Initial Refinement]
    }

    template<unsigned d>
    void gsTestMajorant<d>::gsGetInitialBasis(int vDegree, int yDegree, int wDegree,
                                              gsMultiBasis<> &multBasisV,
                                              gsMultiBasis<> &multBasisY,
                                              gsMultiBasis<> &multBasisW,
                                              int numInitUniformRefV,
                                              int numInitUniformRefY,
                                              int numInitUniformRefW) {

        gsInitializeBasis(vDegree, multBasisV, numInitUniformRefV);
        gsInitializeBasis(yDegree, multBasisY, numInitUniformRefY);
        gsInitializeBasis(wDegree, multBasisW, numInitUniformRefW);
        this->gsLogGeometryBasisData(this->patches.patch(0),
                                     vDegree, yDegree, wDegree, 1);

        for(size_t np = 0; np < this->patches.nPatches(); ++np)
        {
            gsInfo << "Patch : " << np << "\n";
            gsInfo << "VBasis:      \n" << multBasisV.basis(np) << "\n";
            gsInfo << "YBasis:      \n" << multBasisY.basis(np) << "\n";
            gsInfo << "WBasis:      \n" << multBasisW.basis(np) << "\n";
        }
    }

    template<unsigned d>
    void gsTestMajorant<d>::gsLogGeometryBasisData(gsGeometry<> &pGeom,
                                                   int vDegree, int yDegree, int wDegree, int numPatches) {
        gsInfo
                << "%-------------------------------------------------------------------------------------------------%\n";
        gsInfo << "Geometry and basis info: \n";
        gsInfo
                << "%-------------------------------------------------------------------------------------------------%\n";
        gsInfo << "Geom. coeff: \n" << pGeom.coefs() << "\n"; // [N x dim] matrix of the [N control points] x [dim]
        gsInfo << "Geom. basis: \n" << pGeom.basis() << "\n";
        gsInfo
                << "%-------------------------------------------------------------------------------------------------%\n";
        gsInfo << "$v \\in S^{" << vDegree << ", " << vDegree << "}$\n";
        gsInfo << "$y \\in S^{" << yDegree << ", " << yDegree << "} + S^{" << yDegree << ", " << yDegree << "}$\n";
        gsInfo << "$w \\in S^{" << wDegree << ", " << wDegree << "}$\n";
        gsInfo
                << "%-------------------------------------------------------------------------------------------------%\n";
    }

    template<unsigned d>
    void gsTestMajorant<d>::gsExecuteRefinement(gsPoissonAssembler<real_t> &poissonAssembler,
                                                gsMultiBasis<real_t> &thbMultBasisY,
                                                gsPoissonAssembler<real_t> &poissonAssemblerW,
                                                std::vector<gsMultiBasis<> > &thbMultBasisYVector,
                                                std::vector<gsMultiBasis<> > &thbMultBasisWVector,
                                                std::vector<real_t> &mdDistr, MarkingStrategy adaptRefCrit,
                                                real_t markingParamTheta,
                                                int refCounter,
                                                int yBasisRefDelay, int wBasisRefDelay) {

        gsCPUStopwatch clock;

        if (this->isAdaptive) {
            // ------------------------------------------------------------------------------------------------------ //
            // Refining poissonAssembler mesh (basis for V)
            // ------------------------------------------------------------------------------------------------------ //
            //! [Marking procedure]
            // Get element-wise energy error indicator generated by majorant contributions
            std::vector<bool> elMarked(mdDistr.size());
            clock.restart();
            // Mark elements for refinement, based on the computed local indicators, the refinement-criterion and -parameter.
            gsMarkElementsForRef(mdDistr, adaptRefCrit, markingParamTheta, elMarked);
            gsInfo << "number of marked elements: " << std::count_if(elMarked.begin(), elMarked.end(),
                                                                     [](bool elem) {
                                                                         return elem == true;
                                                                     }) << "\n";
            gsInfo << "time for marking : " << clock.stop() << " sec.\n";
            //! [Marking procedure]

            //! [Refining procedure]
            clock.restart();
            // Refine the marked elements with a 1-ring of cells around marked elements
            gsRefineMarkedElements(poissonAssembler.multiBasis(), elMarked, 1);
            poissonAssembler.multiBasis().repairInterfaces(this->patches.interfaces());
            gsInfo << "time for refining the basis for v : " << clock.stop() << " sec.\n";

            // ------------------------------------------------------------------------------------------------------ //
            // Saving basises for Y and W
            // ------------------------------------------------------------------------------------------------------ //
            if (refCounter == 0) {
                // Refinement with optimization of the effort
                for (int j = 0; j < yBasisRefDelay; j++) {
                    // First save the currect basis
                    gsMultiBasis<> &basis = thbMultBasisY;
                    thbMultBasisYVector.push_back(basis);
                }
                // Refinement with optimization of the effort
                for (int j = 0; j < wBasisRefDelay; j++) {
                    gsMultiBasis<> &basis = poissonAssemblerW.multiBasis();
                    thbMultBasisWVector.push_back(basis);
                }
            }
            // ------------------------------------------------------------------------------------------------------ //
            // Refining basis for Y
            // ------------------------------------------------------------------------------------------------------ //
            // Make the refinement and safe the new basis for y
            clock.restart();
            // Refine the marked elements with a 1-ring of cells around marked elements
            gsRefineMarkedElements(thbMultBasisY, elMarked, 1);
            thbMultBasisY.repairInterfaces(this->patches.interfaces());
            gsInfo << "time for refining the basis for y : " << clock.stop() << " sec.\n";
            if (refCounter >= yBasisRefDelay) {
                gsMultiBasis<> &basis = thbMultBasisY;
                thbMultBasisYVector.push_back(basis);
            }
            thbMultBasisY = thbMultBasisYVector[refCounter];

            // ------------------------------------------------------------------------------------------------------ //
            // Refining basis for W
            // ------------------------------------------------------------------------------------------------------ //
            // Make the refinement and safe the new basis for w
            clock.restart();
            // Refine the marked elements with a 1-ring of cells around marked elements
            gsRefineMarkedElements(poissonAssemblerW.multiBasis(), elMarked, 1);
            poissonAssemblerW.multiBasis().repairInterfaces(this->patches.interfaces());
            gsInfo << "time for refining the basis for w : " << clock.stop() << " sec.\n";
            if (refCounter >= wBasisRefDelay) {
                gsMultiBasis<>& basis = poissonAssemblerW.multiBasis();
                thbMultBasisWVector.push_back(basis);
            }
            poissonAssemblerW.initialize(poissonAssemblerW.pde(), thbMultBasisWVector[refCounter],
                                         poissonAssemblerW.options());
            poissonAssemblerW.options().setInt("DirichletValues", dirichlet::l2Projection);


        } else {
            poissonAssembler.multiBasis().uniformRefine();  // Uniform refinement of the basis for V
            if (refCounter >= yBasisRefDelay - 1)
                thbMultBasisY.uniformRefine();              // uniform refinement of basis of Y
            if (refCounter >= wBasisRefDelay - 1)
                poissonAssemblerW.multiBasis().uniformRefine();       // uniform refinement of basis of W

        }
    }


    template<unsigned d>
    void gsTestMajorant<d>::gsSaveToFileRefinementIterationInfo(bool save,
                                                                const gsField<> &v,
                                                                gsMultiBasis<> &basis,
                                                                std::vector<real_t> edDistr,
                                                                std::vector<real_t> mdDistr,
                                                                int refCounter) {
        if (save) {
            // Saving Paraview files
            std::string refTag = util::to_string(exampleNumber) + "-refnum-"
                                 + util::to_string(refCounter);
            if (this->isAdaptive){
                gsMesh<> mesh;
                gsTensorBSpline<d, real_t> *geo = dynamic_cast< gsTensorBSpline<d, real_t> * >( &(this->patches.patch(0)));
                makeMesh(basis.basis(0), mesh);
                std::string meshParam("param-domain-mesh-" + refTag),
                        meshPhys("phys-domain-mesh-" + refTag),
                        vPhys("v-" + refTag);

                gsWriteParaview(mesh, this->resultFolder + "/" + meshParam, true);
                geo->evaluateMesh(mesh);
                gsWriteParaview(mesh, this->resultFolder + "/" + meshPhys, true);
                gsWriteParaview(v, this->resultFolder + "/" + vPhys, 5001, true);
            }
            // Saving txt files
            std::string resultsFile = "results-" + refTag + ".txt";

            std::ofstream file((this->resultFolder + "/" + resultsFile).c_str());
            if (!file.is_open())
                std::cout << "Problem opening the result file!" << resultsFile << std::endl;
            else {
                int size = edDistr.size();
                file << "e_distr = [";
                for (int elCount = 0; elCount < size; ++elCount) file << edDistr[elCount] << ", ";
                file << "]; \n";

                file << "m_distr = [";
                for (int elCount = 0; elCount < size; ++elCount) file << mdDistr[elCount] << ", ";
                file << "]; \n";

                file.close();
            }

        }
    }

    template<unsigned d>
    void gsTestMajorant<d>::gsSaveToFileTestResults(bool save,
                                                    gsVector<index_t> &vDOFs, gsMatrix<index_t> &yDOFs, gsVector<index_t> &wDOFs,
                                                    gsVector<real_t> &eVector,
                                                    gsVector<real_t> &majVector,
                                                    gsVector<real_t> &minVector,
                                                    gsVector<real_t> &etaVector,
                                                    int refTotal) {

        if (save) {
            std::string refTag = util::to_string(exampleNumber);
            std::string resultsFile = "results-" + refTag + ".txt";

            std::ofstream file((this->resultFolder + "/" + resultsFile).c_str());
            if (!file.is_open())
                std::cout << "Problem opening the result file!" << resultsFile << std::endl;
            else {
                file << "vDOFs = [";
                for (int refCounter = 0; refCounter < refTotal; ++refCounter)
                    file << vDOFs[refCounter] << ", ";
                file << "]; \n";

                file << "yDOFs = [";
                for (int refCounter = 0; refCounter < refTotal; ++refCounter)
                    file << yDOFs.row(refCounter).sum() << ", ";
                file << "]; \n";

                file << "wDOFs = [";
                for (int refCounter = 0; refCounter < refTotal; ++refCounter)
                    file << wDOFs[refCounter] << ", ";
                file << "]; \n";

                file << "e = [";
                for (int refCounter = 0; refCounter < refTotal; ++refCounter)
                    file << eVector[refCounter] << ", ";
                file << "]; \n";

                file << "majorant = [";
                for (int refCounter = 0; refCounter < refTotal; ++refCounter)
                    file << majVector[refCounter] << ", ";
                file << "]; \n";

                file << "minorant = [";
                for (int refCounter = 0; refCounter < refTotal; ++refCounter)
                    file << minVector[refCounter] << ", ";
                file << "]; \n";

                file << "eta = [";
                for (int refCounter = 0; refCounter < refTotal; ++refCounter)
                    file << etaVector[refCounter] << ", ";
                file << "]; \n";
                file.close();

            }

        }
    }

    template<unsigned d>
    void gsTestMajorant<d>::gsSaveToFileDivDivMMMatrices(bool save,
                                                         gsSparseMatrix<real_t> &DivDiv,
                                                         gsSparseMatrix<real_t> &MM,
                                                         int refCounter) {
        {
            if (save && refCounter <= 3 && ! this->isAdaptive) {
                // Saving Paraview files
                std::string refTag = "-refnum-" \
                                     + util::to_string(refCounter) \
                                     + "-example-"
                                     + util::to_string(exampleNumber);
                // Saving txt files
                std::string divdivFile = "DivDiv" + refTag + ".txt";
                std::string mmFile = "MM" + refTag + ".txt";

#pragma omp parallel sections
                {
#pragma omp section
                    {
                        std::ofstream streamDivDiv((this->resultFolder + "/" + divdivFile).c_str());
                        if (!streamDivDiv.is_open())
                            std::cout << "Problem opening the result file!" << divdivFile << std::endl;
                        else {
                            std::cout << std::scientific;
                            streamDivDiv << DivDiv.toDense() << " ";
                            std::cout.unsetf(std::ios::scientific);
                            streamDivDiv.close();

                        }
                    }
#pragma omp section
                    {

                        std::ofstream streamMM((this->resultFolder + "/" + mmFile).c_str());
                        if (!streamMM.is_open())
                            std::cout << "Problem opening the result file!" << mmFile << std::endl;
                        else {
                            std::cout << std::scientific;
                            streamMM << MM.toDense();
                            std::cout.unsetf(std::ios::scientific);
                            streamMM.close();
                        }
                    }
                }
            }
        }

    }

    template<unsigned d>
    void gsTestMajorant<d>::gsSaveToFileKMatrix(bool save,
                                                gsPoissonAssembler<real_t> &assembler, int refCounter) {
        {
            if (save) {
                std::string refTag = "-refnum-" + util::to_string(refCounter) + "-example-"
                                     + util::to_string(exampleNumber);
                // Saving txt files
                std::string kFile = "K" + refTag + ".txt";
                std::ofstream streamDivDiv((this->resultFolder + "/" + kFile).c_str());
                if (!streamDivDiv.is_open())
                    std::cout << "Problem opening the result file!" << kFile << std::endl;
                else {
                    streamDivDiv << assembler.matrix().toDense() << " ";
                    streamDivDiv.close();
                }
            }
        }

    }

    template<unsigned d>
    void gsTestMajorant<d>::gsSolveKhfhSystem(gsPoissonHeterogeneousAssembler<real_t>& assembler,
                                              gsMatrix<>& vVector,
                                              gsMatrix<real_t>& timeSolvV,
                                              int refCounter,
                                              gsVector<real_t>& stopcritVector) {

        // Initialise the time spent on the iterative and direct solvers
        real_t timeConjugateGradient(0.0), timeSimplicialLDLT(0.0);
        // Initialise vectors to stor iterative and direct solutions
        gsMatrix<> vVectorIter, vVectorDir;

        // Define parameters, options, and type of the iterative solver
        int N = assembler.rhs().size();
        real_t hPowD = math::pow((real_t) N, (real_t)(- 1));
        index_t maxItersCGV = 3 * N;
        real_t tolCGV = (refCounter > 1) ? stopcritVector[refCounter-2] : hPowD * math::pow(10.0, -2);
        gsLinearOperator<>::Ptr preConMat = gsIdentityOp<>::make(N);
        gsOptionList opt = gsIterativeSolver<real_t>::defaultOptions();
        opt.setInt ("MaxIterations", maxItersCGV);
        opt.setReal("Tolerance"    , tolCGV);
        gsConjugateGradient<> solverCG(assembler.matrix(), preConMat);

        // If we are at the first refinement:  solve the system with LU
        if (refCounter == 0) {
            gsSparseSolver<>::LU solverLU;
            solverLU.compute(assembler.matrix());
            vVectorIter = solverLU.solve(assembler.rhs());
            //this->gsSetVVector(vVectorIter);

        }// If we are further then the first refinement, load the vRefVector projected from the previous iterative step
         // to the currect basis
        else{
            vVectorIter = getVRefVector();
            //gsSetVVector(vVectorIter0);
            gsSetVRefVector(vVectorIter);
            //if (!isAdaptive) { // defined v0 from the previous iteration only for uniform refinement
            //    vVectorIter = this->getVRefVector();
            //    //this->gsSetVVector(vVectorIter);
            //}
        }

        #pragma omp parallel sections
        {
            #pragma omp section
            {
                // Solve with iterative solver
                gsCPUStopwatch clock_1;
                clock_1.restart();
                solverCG.setOptions(opt);
                solverCG.solve(assembler.rhs(), vVectorIter);
                timeConjugateGradient = clock_1.stop();
                gsInfo << "gsConjugateGradient for V: " << solverCG.detail() << " \n\n";
            }

            #pragma omp section
            {
                // Solve with direct solver (only for the d = 2, too expensive for d = 3)
                if (this->dim == 2) {
                    gsCPUStopwatch clock_2;
                    clock_2.restart();
                    gsSparseSolver<>::SimplicialLDLT solverSLDLT;
                    solverSLDLT.compute(assembler.matrix());
                    vVectorDir = solverSLDLT.solve(assembler.rhs());
                    timeSimplicialLDLT = clock_2.stop();
                }
            }

        }
        // Save the time spent on the direct and iterative solvers
        timeSolvV(refCounter, 0) = timeSimplicialLDLT;
        timeSolvV(refCounter, 1) = timeConjugateGradient;

        // If the iterative solver converged, take the result as a solution
        if (solverCG.iterations() < maxItersCGV)    vVector = vVectorIter;
        else                                        vVector = vVectorDir;
        // Hard-coded selection of the solution:
        //vVector = vVectorIter; // return as the iterative solver solution as a results
        //vVector = vVectorDir;  // return as the direct solver solution as a results
    }


    template<unsigned d>
    void gsTestMajorant<d>::gsSolveKhwhfhSystem(gsPoissonHeterogeneousAssembler<real_t>& assembler,
                                                gsMatrix<>& wVector, gsMatrix<> & timeSolvW,
                                                int refCounter, gsVector<real_t>& stopcritVector) {

        real_t timeConjugateGradient(0.0), timeSimplicialLDLT(0.0);
        gsMatrix<> wVectorIter, wVectorDir;


        int N = assembler.rhs().size();
        real_t hPowD = math::pow((real_t) N, (real_t)(- 1)); // O(h^d) ~ O(1/N)
        real_t tolCGW = (refCounter > 1) ? stopcritVector[refCounter-2] * math::pow(10.0, -3) : hPowD * math::pow(10.0, -4);
        index_t maxItersCGW = 3 * N;

        gsLinearOperator<>::Ptr preConMat = gsIdentityOp<>::make(N);
        gsOptionList opt = gsIterativeSolver<real_t>::defaultOptions();
        opt.setInt ("MaxIterations", maxItersCGW);
        opt.setReal("Tolerance"    , tolCGW);

        gsSparseSolver<>::SimplicialLDLT solverSLDLT;
        gsConjugateGradient<> solverCG(assembler.matrix(), preConMat);

        if (refCounter == 0) {
            gsSparseSolver<>::LU solverLU;
            solverLU.compute(assembler.matrix());
            wVectorIter = solverLU.solve(assembler.rhs());
            //this->gsSetWVector(wVectorIter);

        } else{
            wVectorIter = getWRefVector();
            //gsSetVVector(vVectorIter0);
            gsSetWRefVector(wVectorIter);

            //if (!isAdaptive) { // defined v0 from the previous iteration only for uniform refinement
            //    wVectorIter = this->getWRefVector();
                //this->gsSetVVector(wVectorIter);
            //}
        }

        #pragma omp parallel sections
        {
            #pragma omp section
            {
                gsCPUStopwatch clock_1;
                clock_1.restart();
                solverCG.setOptions(opt);
                solverCG.solve(assembler.rhs(), wVectorIter);
                timeConjugateGradient = clock_1.stop();
                gsInfo << "\ngsConjugateGradient for W: " << solverCG.detail() << " \n\n";
            }

            #pragma omp section
            {
                if (this->dim == 2) {
                    gsCPUStopwatch clock_2;
                    clock_2.restart();
                    solverSLDLT.compute(assembler.matrix());
                    wVectorDir = solverSLDLT.solve(assembler.rhs());
                    timeSimplicialLDLT = clock_2.stop();
                }
            }
        }

        timeSolvW(refCounter, 0) = timeSimplicialLDLT;
        timeSolvW(refCounter, 1) = timeConjugateGradient;

        if (solverCG.iterations() < maxItersCGW)    wVector = wVectorIter;
        else                                        wVector = wVectorDir;
        wVector = wVectorDir;

    }

    template <unsigned d>
    void gsTestMajorant<d>:: gsSolveMajorantOptimalSystem(gsSparseMatrix<real_t> & yM, gsMatrix<real_t> & yRhs, gsMatrix<real_t> & yVector,
                                                          gsMatrix<real_t> & timeSolvY, int refCounter, int iterMaj, gsMatrix<index_t> &yDOFs,
                                                          gsVector<real_t> & stopcritVector)
    {

        real_t timeCGSolver(0.0), timeSLDLTSolver(0.0);
        gsMatrix<> yVectorIter, yVectorDir;

        int N = yRhs.size();
        real_t hPowD = math::pow((real_t) N, -1.0);
        //real_t tolCGY = hPowD * math::pow(10.0, -6);
        real_t tolCGY = (refCounter > 1) ? stopcritVector[refCounter-2] * math::pow(10.0, -2) : hPowD * math::pow(10.0, -6);
        index_t maxIters = 3 * N;

        gsOptionList opt = gsIterativeSolver<real_t>::defaultOptions();
        opt.setInt ("MaxIterations", maxIters);
        opt.setReal("Tolerance"    , tolCGY);
        gsLinearOperator<>::Ptr preConMat = gsIdentityOp<>::make(N);

        if (refCounter == 0 && iterMaj == 0) {
            sparseLUfact solverLU;
            solverLU.compute(yM);
            if(solverLU.succeed())
                yVectorIter = solverLU.solve(yRhs);
            else
                GISMO_ERROR("LU decomposition not successful");
            //this->gsSetYVector(yVectorIter);

        } else{
            yVectorIter = this->gsGetYRefVector();
            //this->gsSetYVector(yVectorIter);
            //yVectorIter.resize(yDOFs[refCounter]*this->dim, 1);
            yVectorIter.resize(N, 1);
        }
        gsInfo << ">> refCounter = " << refCounter << "; majIter = " << iterMaj << "\n";

        if (this->dim == 2) {
            #pragma omp parallel sections
            {
#pragma omp section
                {
                    gsCPUStopwatch clock_1;
                    clock_1.restart();
                    gsSparseSolver<>::SimplicialLDLT solverSLDLT;
                    solverSLDLT.compute(yM);
                    yVectorDir = solverSLDLT.solve(yRhs);
                    timeSLDLTSolver = clock_1.stop();
                }
#pragma omp section
                {
                    gsCPUStopwatch clock_2;
                    clock_2.restart();
                    gsConjugateGradient<> solverCG(yM, preConMat);
                    solverCG.setOptions(opt);
                    solverCG.solve(yRhs, yVectorIter);
                    timeCGSolver = clock_2.stop();
                    gsInfo << "gsConjugateGradient for Y: " << solverCG.detail() << "";
                }
            }
        }
        else if (this->dim == 3)    {
            gsCPUStopwatch clock_2;
            clock_2.restart();
            gsConjugateGradient<> solverCG(yM, preConMat);
            solverCG.setOptions(opt);
            solverCG.solve(yRhs, yVectorIter);
            yVector = yVectorIter;
            timeCGSolver = clock_2.stop();
            gsInfo << "gsConjugateGradient for Y: " << solverCG.detail() << "";
        }
        timeSolvY(refCounter, 0) += timeSLDLTSolver;
        timeSolvY(refCounter, 1) += timeCGSolver;

        yVector = yVectorIter;
        //yVector = yVectorDir;

    }

    template <unsigned d>
    void gsTestMajorant<d>:: interpolateToRefVectorWithDirichletBC(const gsPoissonHeterogeneousAssembler<real_t> & assembler,
                                                                   const gsMultiPatch<> &mpV,
                                                                   gsMatrix<> &interpPoints,
                                                                   gsMatrix<> &freeInterpVector,
                                                                   size_t np) {

        gsMatrix<real_t> interpVector = mpV.patch(np).eval(interpPoints);
        interpVector.transposeInPlace();
        const index_t curRows = freeInterpVector.rows();
        //gsInfo << "full interpolated vector: \n" << interpVector << "\n";

        // Reconstruct solution coefficients on patch p
        int unk = 0;
        index_t freeDofs = 0, coupledDofs = 0;
        const gsDofMapper &mapper = assembler.system().colMapper(unk);
        for (index_t i = 0; i < interpVector.size(); ++i)
        {
            if(mapper.is_free(i, np))
                freeDofs++;
            if(mapper.is_coupled(i, np))
                coupledDofs++;

        }
        freeDofs -= coupledDofs;
        freeInterpVector.conservativeResize(curRows + freeDofs, interpVector.cols());

        index_t j = 0;
        for (index_t i = 0; i < interpVector.size(); ++i) {
            if (mapper.is_free(i, np)) freeInterpVector.row(curRows + j) = interpVector.row(i), j++;
        }
        //gsInfo << "free coeffs of interpolated vector: \n" << freeInterpVector << "\n";

    }
}

