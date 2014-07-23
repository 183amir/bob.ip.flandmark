/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 2013 Michal Uricar
 * Copyright (C) 2013 Michal Uricar
 */

#ifndef __CTABLELOSS_H__
#define __CTABLELOSS_H__

#include "CLoss.h"

#include <cstring>

namespace clandmark {

/**
 * @brief The CTableLoss class
 */
class CTableLoss : public CLoss {

public:

	/**
	 * Constructor
	 *
	 * @brief CTableLoss
	 * @param size_w
	 * @param size_h
	 */
	CTableLoss(int size_w, int size_h);

	/**
	 * Destructor
	 */
	~CTableLoss();

	/**
	 * @brief computeLoss
	 * @param ground_truth
	 * @param estimate
	 * @return
	 */
	DOUBLE computeLoss(DOUBLE * const ground_truth, DOUBLE * const estimate);

	/**
	 * @brief computeLoss
	 * @param ground_truth
	 * @param estimate
	 * @return
	 */
	DOUBLE computeLoss(int * const ground_truth, int * const estimate);

	/**
	 * @brief getLossAt
	 * @param position
	 * @return
	 */
	DOUBLE getLossAt(int position);

	/**
	 * @brief setLossTable
	 * @param table_data
	 */
	inline void setLossTable(DOUBLE *table_data)
	{
		memcpy(table, table_data, sizeof(DOUBLE)*kLength);
	}

	/**
	 * @brief getLossType
	 * @return
	 */
	ELossType getType(void) { return TABLE_LOSS; }

	/**
	 * @brief getName
	 * @return
	 */
	std::string getName(void) { return "TABLE_LOSS"; }

private:

	DOUBLE *table;
	int size_h, size_w;

};

}

#endif // __CTABLELOSS_H__
