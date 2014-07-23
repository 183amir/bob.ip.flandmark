/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 2013 Michal Uricar
 * Copyright (C) 2013 Michal Uricar
 */

#ifndef _CEXTENDEDSPARSELBPAPPEARANCEMODEL_H__
#define _CEXTENDEDSPARSELBPAPPEARANCEMODEL_H__

#include "CAppearanceModel.h"

namespace clandmark {

class CExtendedSparseLBPAppearanceModel : public CAppearanceModel {

public:

	/**
	 * @brief CSparseLBPAppearanceModel
	 * @param nodeID
	 * @param search_space_min_x
	 * @param search_space_min_y
	 * @param search_space_max_x
	 * @param search_space_max_y
	 * @param window_size_w
	 * @param window_size_h
	 * @param height_of_pyramid
	 * @param loss_
	 */
	CExtendedSparseLBPAppearanceModel(
		int nodeID,
		int search_space_min_x,
		int search_space_min_y,
		int search_space_max_x,
		int search_space_max_y,
		int window_size_w,
		int window_size_h,
		int height_of_pyramid,
		CLoss *loss_=0
	);

	/**
	 * Destructor
	 */
	~CExtendedSparseLBPAppearanceModel();

	/**
	 * @brief getFeatureAt
	 * @param index
	 * @param feature_vector
	 */
	void getFeatureAt(const int index, DOUBLE *feature_vector);

	/**
	 * @brief getQvalue
	 * @param s_i
	 * @param w
	 * @return
	 */
	DOUBLE getQvalue(int *const s_i, DOUBLE *const w);

	/**
	 * @brief write
	 * @param fs
	 */
	void write(XmlStorage &fs, DOUBLE * const w, bool writeW = true) const;

	//=====
//	virtual int * returnInternalFeatureRep(int *dim, int *length)
//	{ return 0x0; }
	//=====

protected:

	/**
	 * @brief computeFeatures
	 */
	void computeFeatures(void);

	/**
	 * @brief dotProductWithWq
	 * @param w
	 * @param q
	 */
	void dotProductWithWq(DOUBLE * const w, DOUBLE *q);

private:

	/**
	 * @brief getDim
	 * @param img_nRows
	 * @param img_nCols
	 * @param nPyramids
	 * @return
	 */
	int getDim(int img_nRows, int img_nCols, int nPyramids);

	/**
	 * @brief featuresSparse
	 * @param vec
	 */
	void featuresSparse(int* vec);

private:

	const int kHeightOfPyramid;		/**< */

	int *LBPFeatures;				/**< */
	int kSparseFeatureDimension;	/**< */
	int *window;					/**< */

};

}

#endif // _CEXTENDEDSPARSELBPAPPEARANCEMODEL_H__
