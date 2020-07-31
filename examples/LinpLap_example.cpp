/** @file poisson_example.cpp

@brief Tutorial on how to use G+Smo to solve the Poisson equation,
see the \ref PoissonTutorial

This file is part of the G+Smo library.

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Author(s): 
*/

//! [Include namespace]
# include <gismo.h>

using namespace gismo;
//! [Include namespace]

int main(int argc, char *argv[])
{
	real_t eps = 1;
	real_t p = 1.1;

	//! [Function data]
	// Define source function
	gsFunctionExpr<> f("4*(" + std::to_string(eps*eps) + "+4*x^2+4*y^2)^(" + std::to_string(p) + "/2-1)+8*(" + std::to_string(p) + "-2)*(" + std::to_string(eps*eps) + "+4*x^2+4*y^2)^(" + std::to_string(p) + "/2-2)*(x^2+y^2)", 2);

	// Define exact solution (optional)
	gsFunctionExpr<> g("x^2+y^2", 2);

	// Print out source function and solution
	gsInfo << "Source function " << f << "\n";
	gsInfo << "Exact solution " << g << "\n\n";
	//! [Function data]

	//! [Geometry data]
	// Define Geometry, must be a gsMultiPatch object
	gsMultiPatch<> patches;

	patches = gsMultiPatch<>(*gsNurbsCreator<>::BSplineSquare(2));
	gsInfo << "The domain is a " << patches << "\n";
	//! [Geometry data]

	//! [Boundary conditions]
	gsBoundaryConditions<> bcInfo;
	bcInfo.addCondition(0, boundary::west, condition_type::dirichlet, &g);
	bcInfo.addCondition(0, boundary::east, condition_type::dirichlet, &g);
	bcInfo.addCondition(0, boundary::north, condition_type::dirichlet, &g);
	bcInfo.addCondition(0, boundary::south, condition_type::dirichlet, &g);
	
	//! [Boundary conditions]

	//! [Refinement]
	// Copy basis from the geometry
	gsMultiBasis<> refine_bases(patches);

	// Number for h-refinement of the computational (trial/test) basis.
	const int numRefine = 3;

	// Number for p-refinement of the computational (trial/test) basis.
	const int degree = 2;

	// h-refine each basis (4, one for each patch)
	for (int i = 0; i < numRefine; ++i)
		refine_bases.uniformRefine();

	// k-refinement (set degree)
	for (size_t i = 0; i < refine_bases.nBases(); ++i)
		refine_bases[i].setDegreePreservingMultiplicity(degree);

	//! [Refinement]

	int num = refine_bases.size();
	gsMatrix<real_t> w = gsMatrix<real_t>::Zero(num, 1);

	//! [Refinement]

	////////////// Setup solver and solve //////////////
	// Initialize Solver
	// Setup method for handling Dirichlet boundaries, options:
	//
	// * elimination: Eliminate the Dirichlet DoFs from the linear system.
	//
	// * nitsche: Keep the Dirichlet DoFs and enforce the boundary
	//
	// condition weakly by a penalty term.
	// Setup method for handling patch interfaces, options:
	//
	// * glue:Glue patches together by merging DoFs across an interface into one.
	//   This only works for conforming interfaces
	//
	// * dg: Use discontinuous Galerkin-like coupling between adjacent patches.
	//       (This option might not be available yet)
	
	gsLinpLapPde<real_t> pde(patches, bcInfo, f, eps, p, w);
	//gsLinpLapAssembler<real_t> assembler;
	gsMatrix<real_t> solVector;
	gsMultiPatch<real_t> mpsol;
	gsSparseSolver<real_t>::LU solver;

	for (int i = 0; i < 20; ++i)
	{
		gsInfo << "pde.w: " << pde.w.rows() << "x" << pde.w.cols() << "\n";
		//gsLinpLapPde<real_t> pde(patches, bcInfo, f, eps, p, w);
		gsLinpLapAssembler<real_t> assembler(pde, refine_bases,
			dirichlet::nitsche, iFace::glue);
		//dirichlet::nitsche, iFace::glue);

		// Generate system matrix and load vector
		assembler.assemble();
		
		//! [Solve]
		// Initialize the LU solver
		gsInfo << "Matrix: " << assembler.matrix().rows() << "x" << assembler.matrix().cols() << "\n"; 
		solver.compute(assembler.matrix());
		gsInfo << "Rhs: " << assembler.rhs().rows() << "x" << assembler.rhs().cols() << "\n";
		solVector = solver.solve(assembler.rhs());
		w = solVector;
		pde.w = w;
		gsInfo << "Solved the system with LU solver.\n";
		//! [Solve]

		//! [Construct solution]
		// Construct the solution as a scalar field
		assembler.constructSolution(solVector, mpsol);
		gsField<> sol(assembler.patches(), mpsol);
		//! [Construct solution]
		
		gsInfo << i << "\n";
		gsInfo << sol.distanceL2(g) << "\n";
	}

	std::cin.get();
	return EXIT_SUCCESS;

}// end main