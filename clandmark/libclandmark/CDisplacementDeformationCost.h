/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 2013 Michal Uricar
 * Copyright (C) 2013 Michal Uricar
 */

#ifndef _CDISPLACEMENTDEFORMATIONCOST_H__
#define _CDISPLACEMENTDEFORMATIONCOST_H__

#include "CDeformationCost.h"

namespace clandmark {

/**
 * @brief The CDisplacementDeformationCost class
 */
class CDisplacementDeformationCost : public CDeformationCost {

public:

	// Constructor
	/**
	 * @brief CDisplacementDeformationCost
	 * @param parent
	 * @param child
	 * @param dimension
	 */
	CDisplacementDeformationCost(CAppearanceModel * const parent, CAppearanceModel * const child, int dimension=4);

	/** Destructor */
	~CDisplacementDeformationCost();

	/**
	 * @brief dotProductWithWg
	 * @param w
	 * @param g
	 * @param index
	 */
	void dotProductWithWg(DOUBLE * const w, DOUBLE *g, const int index);

	/**
	 * @brief update
	 * @param w
	 * @param g
	 * @param image_data
	 * @param ground_truth
	 */
	void update(DOUBLE * const w, DOUBLE *g, cimg_library::CImg<unsigned char> *image_data, int * const ground_truth=0);

	/**
	 * @brief getDeformationCostAt
	 * @param s_i
	 * @param s_j
	 * @param deformation_cost
	 */
	void getDeformationCostAt(int * const s_i, int * const s_j, int *deformation_cost);

	/**
	 * @brief getGvalue
	 * @param s_i
	 * @param s_j
	 * @param w
	 * @return
	 */
	DOUBLE getGvalue(int *const s_i, int *const s_j, DOUBLE *const w);

	/**
	 * @brief write
	 * @param fs
	 */
	void write(XmlStorage &fs, DOUBLE * const w, bool writeW=true);

private:

	/**
	 * @brief computeDeformationCosts
	 */
	void computeDeformationCosts();

	/**
	 * @brief distanceTransform1D
	 * @param f
	 * @param n
	 * @return
	 */
	DOUBLE *distanceTransform1D(DOUBLE *f, int n);

	/**
	 * @brief distanceTransform2D
	 * @param rectangle
	 * @param width
	 * @param height
	 */
	void distanceTransform2D(DOUBLE *rectangle, int width, int height);

private:

	int **deformationCosts;
	int parentLength;
	int childLength;

};

}

#endif // _CDISPLACEMENTDEFORMATIONCOST_H__
