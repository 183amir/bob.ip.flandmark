//
//  CZeroLoss.cpp
//
//
//  Created by Kostia on 1/27/14.
//
//

#include "CZeroLoss.h"

using namespace clandmark;

CZeroLoss::CZeroLoss(int size_w, int size_h)
: CLoss(size_w, size_h)
{
	;
}

CZeroLoss::~CZeroLoss()
{
	;
}

DOUBLE CZeroLoss::computeLoss(DOUBLE * const ground_truth, DOUBLE * const estimate)
{
	return 0;
}

DOUBLE CZeroLoss::computeLoss(int * const ground_truth, int * const estimate)
{
	return 0;
}
