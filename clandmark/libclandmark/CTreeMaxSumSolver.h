/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 2013 Michal Uricar
 * Copyright (C) 2013 Michal Uricar
 */

#ifndef _CTREEMAXSUMSOLVER_H__
#define _CTREEMAXSUMSOLVER_H__

#include "CMaxSumSolver.h"

#include <queue>

namespace clandmark {

/**
 *
 */
typedef struct node {
	int ID;
	std::vector< int > children;
	std::vector< int > ancestors;
	std::vector< int > incidentEdgeIDs;
} Node;

// NOTE: Specification of the root_id is not necessary since it will be found automatically based on provided oriented edges.

/**
 * @brief The CTreeMaxSumSolver class
 */
class CTreeMaxSumSolver : public CMaxSumSolver {

public:

	/** Default constructor */
	/**
	 * @brief CTreeMaxSumSolver
	 * @param vertices
	 * @param edges
	 */
	CTreeMaxSumSolver(std::vector< Vertex > * const vertices,  std::vector< CDeformationCost* > * const edges);

	/** Destructor */
	~CTreeMaxSumSolver();

	/**
	 * @brief solve
	 * @param w
	 * @param q
	 * @param g
	 * @param output
	 */
	void solve(const std::vector< std::vector< DOUBLE* > > &w, const std::vector< std::vector< DOUBLE* > > &q, DOUBLE **g, int *output);

private:

	/**
	 * @brief maximizeSum
	 * @param q
	 * @param g
	 * @param length
	 * @param maximum
	 * @param index
	 */
	void maximizeSum(DOUBLE *const q, DOUBLE *const g, const int length, DOUBLE *maximum, int *index);

private:

	std::queue< int > taskQueue;			/**< */
	std::vector< int > inDegrees;			/**< */
	std::vector< Node > graph;			/**< */
	std::queue< int > topsortQueue;			/**< */
	std::vector< int > order;			/**< */

	std::vector< DOUBLE* > intermediateResults;	/**< */
	std::vector< int* > intermediateIndices;	/**< */

	std::vector< int > argmaxIndices;		/**< */

	int kLandmarks;					/**< */
	int kEdges;					/**< */

};

}

#endif // _CTREEMAXSUMSOLVER_H__
