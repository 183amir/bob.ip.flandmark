#include "CTableLoss.h"

using namespace clandmark;

CTableLoss::CTableLoss(int size_w, int size_h) : CLoss(size_w, size_h)
{
	table = new DOUBLE[kLength];
	memset(table, 0, sizeof(DOUBLE)*kLength);
	this->size_h = size_h;
	this->size_w = size_w;
}

CTableLoss::~CTableLoss()
{
	if (table)
		delete [] table;
}

DOUBLE CTableLoss::computeLoss(DOUBLE * const ground_truth, DOUBLE * const estimate)
{
	throw int();
}

DOUBLE CTableLoss::computeLoss(int * const ground_truth, int * const estimate)
{
	return table[ INDEX(estimate[1], estimate[0], size_h) ];
}

DOUBLE CTableLoss::getLossAt(int position)
{
	return table[position];
}
