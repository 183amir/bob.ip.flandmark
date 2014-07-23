#include "CDeformationCost.h"

using namespace clandmark;

CDeformationCost::CDeformationCost(CAppearanceModel * const parent_, CAppearanceModel * const child_, CLoss * const loss_)
{
	parent = parent_;
	child = child_;

	loss = loss_;
}

CDeformationCost::~CDeformationCost()
{
	if (loss)
		delete loss;
}

void CDeformationCost::update(DOUBLE * const w, DOUBLE *g, cimg_library::CImg<unsigned char> *image_data, int * const ground_truth)
{
	;
}
