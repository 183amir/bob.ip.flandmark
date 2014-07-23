/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 2013 Michal Uricar
 * Copyright (C) 2013 Michal Uricar
 */

#ifndef _CHOGAPPEARANCEMODEL_H__
#define _CHOGAPPEARANCEMODEL_H__

#include "CAppearanceModel.h"

namespace clandmark {

/**
 * @brief The CHOGAppearanceModel class
 */
class CHOGAppearanceModel : public CAppearanceModel {

public:

	/**
	 * @brief CHOGAppearanceModel
	 * @param nodeID
	 * @param search_space_min_x
	 * @param search_space_min_y
	 * @param search_space_max_x
	 * @param search_space_max_y
	 * @param window_size_w
	 * @param window_size_h
	 * @param n_bins
	 * @param cell_size
	 * @param block_size
	 * @param loss_
	 */
	CHOGAppearanceModel(
		int nodeID,
		int search_space_min_x,
		int search_space_min_y,
		int search_space_max_x,
		int search_space_max_y,
		int window_size_w,
		int window_size_h,
		int n_bins,
		int cell_size,
		int block_size,
		CLoss *loss_=0);

	/**
	 * Destructor
	 */
	~CHOGAppearanceModel();

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
	 * @param nBins
	 * @param cellSize
	 * @param blockSize
	 * @return
	 */
	int getDim(int img_nRows, int img_nCols, int nBins, int cellSize, int blockSize);


	/**
	 * @brief hogFeatures
	 * @param vec
	 */
	void hogFeatures(DOUBLE *vec);

private:

	const int kNbins;		/**< */
	const int kCellSize;	/**< */
	const int kBlockSize;	/**< */

	DOUBLE *HOGFeatures;	/**< */
	int *window;			/**< */

	int cW;
	int cH;
	int cDim;

	DOUBLE *derivx;
	DOUBLE *derivy;
	DOUBLE *angles;
	DOUBLE *magnitudes;
	DOUBLE *hist;
};

}

#endif // _CHOGAPPEARANCEMODEL_H__
