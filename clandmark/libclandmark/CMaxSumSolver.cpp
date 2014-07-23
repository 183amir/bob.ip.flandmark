#include "CMaxSumSolver.h"

using namespace clandmark;

CMaxSumSolver::CMaxSumSolver(std::vector< Vertex > * const vertices_, std::vector< CDeformationCost* > * const edges_)
{
	vertices = vertices_;
	edges = edges_;
}

CMaxSumSolver::~CMaxSumSolver()
{
	;
}
