/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 2013 Michal Uricar
 * Copyright (C) 2013 Michal Uricar
 */

#ifndef _CMAXSUMSOLVER_H__
#define _CMAXSUMSOLVER_H__

#include "CAppearanceModel.h"
#include "CDeformationCost.h"

#include <vector>

namespace clandmark {

/**
 * @brief The CMaxSumSolver class
 */
class CMaxSumSolver {

public:

	/**
	 * @brief CMaxSumSolver
	 * @param vertices_
	 * @param edges_
	 */
	CMaxSumSolver(std::vector< Vertex > * const vertices_, std::vector< CDeformationCost* > * const edges_);

	/**
	 * @brief ~CMaxSumSolver
	 */
	virtual ~CMaxSumSolver();

	/**
	 * @brief solve
	 * @param w
	 * @param q
	 * @param g
	 * @param output
	 */
	virtual void solve(const std::vector< std::vector< DOUBLE* > > &w, const std::vector< std::vector< DOUBLE* > > &q, DOUBLE **g, int *output) = 0;

protected:

	std::vector< Vertex > *vertices;		/**< */
	std::vector< CDeformationCost* > *edges;	/**< */

};

}

#endif // _CMAXSUMSOLVER_H__
