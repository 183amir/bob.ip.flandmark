#include "CNormalizedEuclideanLoss.h"

#include <cmath>

using namespace clandmark;

CNormalizedEuclideanLoss::CNormalizedEuclideanLoss(int size_w, int size_h)
	: CLoss(size_w, size_h)
{
	;
}

CNormalizedEuclideanLoss::~CNormalizedEuclideanLoss()
{
	;
}

DOUBLE CNormalizedEuclideanLoss::computeLoss(DOUBLE * const ground_truth, DOUBLE * const estimate)
{
	return sqrt((ground_truth[0]-estimate[0])*(ground_truth[0]-estimate[0])	+ (ground_truth[1]-estimate[1])*(ground_truth[1]-estimate[1]))*normalizationFactor;
}

DOUBLE CNormalizedEuclideanLoss::computeLoss(int * const ground_truth, int * const estimate)
{
	return sqrt(DOUBLE(ground_truth[0]-estimate[0])*(ground_truth[0]-estimate[0]) + (ground_truth[1]-estimate[1])*(ground_truth[1]-estimate[1]))*normalizationFactor;
}
