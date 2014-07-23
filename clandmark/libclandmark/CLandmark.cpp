#include "CLandmark.h"

#include "msvc-compat.h"

#include "CTimer.h"

#include <cfloat>
#include <cstring>
#include <cmath>
#include <math.h>

using namespace clandmark;

CLandmark::CLandmark(
		int landmarksCount,
		int edgesCount,
		int base_window_width,
		int base_window_height,
		DOUBLE base_window_margin_x,
		DOUBLE base_window_margin_y)
{
	init(landmarksCount, edgesCount, base_window_width, base_window_height, base_window_margin_x, base_window_margin_y);
}

CLandmark::CLandmark()
{
	kLandmarksCount = 0;
	kEdgesCount = 0;

	// convention for size [width x height]
	baseWindow[0] = 0;
	baseWindow[1] = 0;
	baseWindowMargin[0] = 0;
	baseWindowMargin[1] = 0;

	// normalized image frame related variables
	memset(scaleFactor, 0, sizeof(scaleFactor));
	memset(scaleFactorInverse, 0, sizeof(scaleFactorInverse));
	memset(offset, 0, sizeof(offset));

	g = 0x0;

	wDimension = 0;

	landmarksPositions = 0x0;
	landmarksPositionsNF = 0x0;

	solver = 0x0;

	// GroundTruth (for training only)
	groundTruthPositions = 0x0;
	groundTruthPositionsNF = 0x0;

	normalizationFactor = 1.0;

	psi = 0x0;

	normalizedFrame = 0x0;
	croppedImage = 0x0;

	Q = 0x0;
	G = 0x0;

	psiNodesDimension = 0;
}

void CLandmark::init(
		int landmarksCount,
		int edgesCount,
		int base_window_width,
		int base_window_height,
		DOUBLE base_window_margin_x,
		DOUBLE base_window_margin_y)
{
	kLandmarksCount = landmarksCount;
	kEdgesCount = edgesCount;

	baseWindow[0] = base_window_width;
	baseWindow[1] = base_window_height;
	baseWindowMargin[0] = base_window_margin_x;
	baseWindowMargin[1] = base_window_margin_y;

	groundTruthPositions = new DOUBLE[2*kLandmarksCount];
	groundTruthPositionsNF = new int[2*kLandmarksCount];

	landmarksPositions = new DOUBLE[2*kLandmarksCount];
	landmarksPositionsNF = new int[2*kLandmarksCount];

	memset(groundTruthPositions, 0, sizeof(DOUBLE)*2*kLandmarksCount);
	memset(groundTruthPositionsNF, 0, sizeof(int)*2*kLandmarksCount);
	memset(landmarksPositions, 0, sizeof(DOUBLE)*2*kLandmarksCount);
	memset(landmarksPositionsNF, 0, sizeof(int)*2*kLandmarksCount);

	for (int i=0; i < kLandmarksCount; ++i)
	{
		q.push_back(std::vector< DOUBLE* >());
	}

	g = new DOUBLE*[kEdgesCount];

	for (int i=0; i < kLandmarksCount+kEdgesCount; ++i)
	{
		w.push_back(std::vector< DOUBLE* >());
	}

	normalizedFrame =  new cimg_library::CImg<unsigned char>(base_window_width, base_window_height);
	croppedImage = new cimg_library::CImg<unsigned char>();

	normalizationFactor = 1.0;

	Q = new DOUBLE[kLandmarksCount];
	G = new DOUBLE[kEdgesCount];
}

void CLandmark::setGroundTruth(DOUBLE *ground_truth)
{
	memcpy(groundTruthPositions, ground_truth, 2*kLandmarksCount*sizeof(DOUBLE));

	// transform GT to normalized frame
	transformCoordinatesImage2NF(groundTruthPositions, groundTruthPositionsNF);
}

void CLandmark::setGroundTruth(int *ground_truth)
{
	memcpy(groundTruthPositionsNF, ground_truth, 2*kLandmarksCount*sizeof(int));
}

CLandmark::~CLandmark()
{
	if (!q.empty())
	{
		for (unsigned int i=0; i < q.size(); ++i)
		{
			for (unsigned int j=0; j < q[i].size(); ++j)
				delete [] q[i][j];
			q[i].clear();
		}
		q.clear();
	}

	if (g)
	{
		for (int i=0; i < kEdgesCount; ++i)
		{
			if (g[i])
				delete [] g[i];
		}
		delete [] g;
	}

	if (!w.empty())
	{
		for (unsigned int i=0; i < w.size(); ++i)
		{
			for (unsigned int j=0; j < w[i].size(); ++j)
				delete [] w[i][j];
			w[i].clear();
		}
		w.clear();
	}

	if (groundTruthPositions)
		delete [] groundTruthPositions;

	if (groundTruthPositionsNF)
		delete [] groundTruthPositionsNF;

	if (landmarksPositions)
		delete [] landmarksPositions;

	if (landmarksPositionsNF)
		delete [] landmarksPositionsNF;

	if (psi)
		delete [] psi;

	delete normalizedFrame;
	delete croppedImage;

	if (Q)
		delete [] Q;

	if (G)
		delete [] G;
}

void CLandmark::getQG(void)
{
	for (unsigned int i=0; i < vertices.size(); ++i)
	{
		int best = 0;

		for (unsigned int j=0; j < q[i].size(); ++j)
		{
			vertices[i].appearances[j]->update(normalizedFrame, w[i][j], q[i][j], &groundTruthPositionsNF[INDEX(0, i, 2)]);

			DOUBLE sum = 0.0;
			DOUBLE maximum = -DBL_MAX;
			for (int k=0; k < vertices[i].appearances[j]->getFeatureDimension(); ++k)
				sum += q[i][j][k];

			if (sum > maximum)
			{
				maximum = sum;
				best = j;
			}
		}

		vertices[i].best = best;
	}

	for (uint32_t i = 0; i < edges.size(); ++i)
		edges[i]->update(w[kLandmarksCount+i][0], g[i], normalizedFrame, groundTruthPositionsNF);
}

void CLandmark::getQGTableLoss()
{
	for (unsigned int i=0; i < vertices.size(); ++i)
	{
		int best = 0;

		for (unsigned int j=0; j < q[i].size(); ++j)
		{
			vertices[i].appearances[j]->update(normalizedFrame, w[i][j], q[i][j], 0x0);

			DOUBLE sum = 0.0;
			DOUBLE maximum = -DBL_MAX;
			for (int k=0; k < vertices[i].appearances[j]->getFeatureDimension(); ++k)
				sum += q[i][j][k];

			if (sum > maximum)
			{
				maximum = sum;
				best = j;
			}
		}

		vertices[i].best = best;
	}

	for (uint32_t i = 0; i < edges.size(); ++i)
		edges[i]->update(w[kLandmarksCount+i][0], g[i], normalizedFrame, groundTruthPositionsNF);
}

void CLandmark::detect(cimg_library::CImg<unsigned char> *inputImage, int *boundingBox, DOUBLE * const ground_truth)
{
	CTimer timer, timerPart;

	timer.tic();
	timerPart.tic();

	// obtain normalized frame
	getNormalizedFrame(inputImage, boundingBox);

	timings.normalizedFrame = timerPart.toc();

	// if ground_truth provided, store it
	if (ground_truth)
	{
		setGroundTruth(ground_truth);
	}

	//
	applyNormalizationFactor();

	timerPart.tic();

	// update values of unary and pair-wise potentials
	getQG();

	timings.features = timerPart.toc();

	timerPart.tic();

	// solve (Max,+) problem
	solver->solve(w, q, g, landmarksPositionsNF);

	// Get Q response
	for (int i=0; i < kLandmarksCount; ++i)
	{
		Q[i] = vertices[i].appearances[vertices[i].best]->getQvalue(&landmarksPositionsNF[INDEX(0, i, 2)], w[i][vertices[i].best]);
	}

	// Get G response
	for (int i=0; i < kEdgesCount; ++i)
	{
		G[i] = edges[i]->getGvalue(&landmarksPositionsNF[INDEX(0, edges[i]->getParent()->getNodeID(), 2)], &landmarksPositionsNF[INDEX(0, edges[i]->getChild()->getNodeID(), 2)], w[kLandmarksCount+i][0]);
	}

	timings.maxsum = timerPart.toc();

	// tranform coordinates from normalized image frame to original image
	transformCoordinatesNF2Image(landmarksPositionsNF, landmarksPositions);

	timings.overall = timer.toc();
}

void CLandmark::detect_mirrored(cimg_library::CImg<unsigned char> *inputImage, int *boundingBox, DOUBLE * const ground_truth)
{
	CTimer timer, timerPart;

	timer.tic();
	timerPart.tic();

	// obtain normalized frame
	getNormalizedFrame(inputImage, boundingBox);

	(*this->normalizedFrame) = this->normalizedFrame->get_mirror('x');

	timings.normalizedFrame = timerPart.toc();

	// if ground_truth provided, store it
	if (ground_truth)
	{
		setGroundTruth(ground_truth);
	}

	//
	applyNormalizationFactor();

	timerPart.tic();

	// update values of unary and pair-wise potentials
	getQG();

	timings.features = timerPart.toc();

	timerPart.tic();

	// solve (Max,+) problem
	solver->solve(w, q, g, landmarksPositionsNF);

	// Get Q response
	for (int i=0; i < kLandmarksCount; ++i)
	{
		Q[i] = vertices[i].appearances[vertices[i].best]->getQvalue(&landmarksPositionsNF[INDEX(0, i, 2)], w[i][vertices[i].best]);
	}

	// Get G response
	for (int i=0; i < kEdgesCount; ++i)
	{
		G[i] = edges[i]->getGvalue(&landmarksPositionsNF[INDEX(0, edges[i]->getParent()->getNodeID(), 2)], &landmarksPositionsNF[INDEX(0, edges[i]->getChild()->getNodeID(), 2)], w[kLandmarksCount+i][0]);
	}

	timings.maxsum = timerPart.toc();

	// tranform coordinates from normalized image frame to original image
	for (int i=0; i < 2*kLandmarksCount; i+=2)
	{
		landmarksPositionsNF[i] = baseWindow[0] - landmarksPositionsNF[i];
	}
	transformCoordinatesNF2Image(landmarksPositionsNF, landmarksPositions);

	timings.overall = timer.toc();
}

void CLandmark::detect_general_bbox(cimg_library::CImg<unsigned char> *inputImage, int *boundingBox, DOUBLE *const ground_truth)
{
	CTimer timer, timerPart;

	timer.tic();
	timerPart.tic();

	// obtain normalized frame
	DOUBLE angle = getNormalizedFrameGeneralBB(inputImage, boundingBox);

	// get center of the general bounding box
	DOUBLE C[2] = {0.0, 0.0};
	C[0] = (boundingBox[INDEX(0, 0, 2)]+boundingBox[INDEX(0, 2, 2)])/2.0;
	C[1] = (boundingBox[INDEX(1, 0, 2)]+boundingBox[INDEX(1, 2, 2)])/2.0;

	timings.normalizedFrame = timerPart.toc();

	// if ground_truth provided, store it
	if (ground_truth)
	{
		// TODO: tranform ground truth by rotating by obtained angle
		DOUBLE *GT = new DOUBLE[2*kLandmarksCount];
		//DOUBLE GT[2*kLandmarksCount];

		for (int i=0; i < 2*kLandmarksCount; i+=2)
		{
			GT[i]   = (ground_truth[i]-C[0])*cos(angle) - (ground_truth[i+1]-C[1])*sin(angle) + C[0];
			GT[i+1] = (ground_truth[i]-C[0])*cos(angle) - (ground_truth[i+1]-C[1])*sin(angle) + C[1];
		}

		//setGroundTruth(ground_truth);
		setGroundTruth(GT);

		delete [] GT;
	}

	//
	applyNormalizationFactor();

	timerPart.tic();

	// update values of unary and pair-wise potentials
	getQG();

	timings.features = timerPart.toc();

	timerPart.tic();

	// solve (Max,+) problem
	solver->solve(w, q, g, landmarksPositionsNF);

	// Get Q response
	for (int i=0; i < kLandmarksCount; ++i)
	{
		Q[i] = vertices[i].appearances[vertices[i].best]->getQvalue(&landmarksPositionsNF[INDEX(0, i, 2)], w[i][vertices[i].best]);
	}

	// Get G response
	for (int i=0; i < kEdgesCount; ++i)
	{
		G[i] = edges[i]->getGvalue(&landmarksPositionsNF[INDEX(0, edges[i]->getParent()->getNodeID(), 2)], &landmarksPositionsNF[INDEX(0, edges[i]->getChild()->getNodeID(), 2)], w[kLandmarksCount+i][0]);
	}

	timings.maxsum = timerPart.toc();

	// rotate the landmark coordinates by angle
	// x' = x * cos(theta) - y * sin(theta); y' = x * sin(theta) + y * cos(theta)
	DOUBLE center[2] = {baseWindow[0]/2.0, baseWindow[1]/2.0};
	DOUBLE * landmarksPositionsNFrot = new DOUBLE[2*kLandmarksCount];

	for (int i=0; i < 2*kLandmarksCount; i+=2)
	{
		landmarksPositionsNFrot[i]   = (landmarksPositionsNF[i]-center[0])*cos(-angle) - (landmarksPositionsNF[i+1]-center[1])*sin(-angle) + center[0];
		landmarksPositionsNFrot[i+1] = (landmarksPositionsNF[i]-center[0])*sin(-angle) + (landmarksPositionsNF[i+1]-center[1])*cos(-angle) + center[1];
	}

	for (int i = 0; i < 2*kLandmarksCount; i+=2)
	{
		landmarksPositions[i] = landmarksPositionsNFrot[i]*scaleFactor[0] + offset[0];
		landmarksPositions[i+1] = landmarksPositionsNFrot[i+1]*scaleFactor[1] + offset[1];
	}

	delete [] landmarksPositionsNFrot;

	timings.overall = timer.toc();
}

void CLandmark::detect_general_bbox_mirrored(cimg_library::CImg<unsigned char> *inputImage, int *boundingBox, DOUBLE *const ground_truth)
{
	CTimer timer, timerPart;

	timer.tic();
	timerPart.tic();

	// obtain normalized frame
	DOUBLE angle = getNormalizedFrameGeneralBB(inputImage, boundingBox);

	(*this->normalizedFrame) = this->normalizedFrame->get_mirror('x');

	// get center of the general bounding box
	DOUBLE C[2] = {0.0, 0.0};
	C[0] = (boundingBox[INDEX(0, 0, 2)]+boundingBox[INDEX(0, 2, 2)])/2.0;
	C[1] = (boundingBox[INDEX(1, 0, 2)]+boundingBox[INDEX(1, 2, 2)])/2.0;

	timings.normalizedFrame = timerPart.toc();

	// if ground_truth provided, store it
	if (ground_truth)
	{
		DOUBLE *GT = new DOUBLE[2*kLandmarksCount];

		for (int i=0; i < 2*kLandmarksCount; i+=2)
		{
			GT[i]   = (ground_truth[i]-C[0])*cos(angle) - (ground_truth[i+1]-C[1])*sin(angle) + C[0];
			GT[i+1] = (ground_truth[i]-C[0])*cos(angle) - (ground_truth[i+1]-C[1])*sin(angle) + C[1];
		}

		setGroundTruth(GT);

		delete [] GT;
	}

	//
	applyNormalizationFactor();

	timerPart.tic();

	// update values of unary and pair-wise potentials
	getQG();

	timings.features = timerPart.toc();

	timerPart.tic();

	// solve (Max,+) problem
	solver->solve(w, q, g, landmarksPositionsNF);

	// Get Q response
	for (int i=0; i < kLandmarksCount; ++i)
	{
		Q[i] = vertices[i].appearances[vertices[i].best]->getQvalue(&landmarksPositionsNF[INDEX(0, i, 2)], w[i][vertices[i].best]);
	}

	// Get G response
	for (int i=0; i < kEdgesCount; ++i)
	{
		G[i] = edges[i]->getGvalue(&landmarksPositionsNF[INDEX(0, edges[i]->getParent()->getNodeID(), 2)], &landmarksPositionsNF[INDEX(0, edges[i]->getChild()->getNodeID(), 2)], w[kLandmarksCount+i][0]);
	}

	timings.maxsum = timerPart.toc();

	// rotate the landmark coordinates by angle
	// x' = x * cos(theta) - y * sin(theta); y' = x * sin(theta) + y * cos(theta)
	DOUBLE center[2] = {baseWindow[0]/2.0, baseWindow[1]/2.0};
	DOUBLE * landmarksPositionsNFrot = new DOUBLE[2*kLandmarksCount];

	// mirror coordinates in NF
	for (int i=0; i < 2*kLandmarksCount; i+=2)
	{
		landmarksPositionsNF[i] = baseWindow[0] - landmarksPositionsNF[i];
	}

	for (int i=0; i < 2*kLandmarksCount; i+=2)
	{
		landmarksPositionsNFrot[i]   = (landmarksPositionsNF[i]-center[0])*cos(-angle) - (landmarksPositionsNF[i+1]-center[1])*sin(-angle) + center[0];
		landmarksPositionsNFrot[i+1] = (landmarksPositionsNF[i]-center[0])*sin(-angle) + (landmarksPositionsNF[i+1]-center[1])*cos(-angle) + center[1];
	}

	// tranform coordinates from normalized image frame to original image
	for (int i = 0; i < 2*kLandmarksCount; i+=2)
	{
		landmarksPositions[i] = landmarksPositionsNFrot[i]*scaleFactor[0] + offset[0];
		landmarksPositions[i+1] = landmarksPositionsNFrot[i+1]*scaleFactor[1] + offset[1];
	}

	delete [] landmarksPositionsNFrot;

	timings.overall = timer.toc();
}

void CLandmark::applyNormalizationFactor()
{
	// vertices
	for (int i=0; i < kLandmarksCount; ++i)
	{
		for (unsigned int k=0; k < vertices[i].appearances.size(); ++k)
		{
			vertices[i].appearances[k]->setLossNormalizationFactor(normalizationFactor);
		}
	}

	// edges
	for (int i=0; i < kEdgesCount; ++i)
	{
		edges[i]->setLossNormalizationFactor(normalizationFactor);
	}
}

void CLandmark::detect_base(cimg_library::CImg<unsigned char> *inputImage, int * const ground_truth)
{
	CTimer timer, timerPart;

	timer.tic();
	timerPart.tic();

	// if ground_truth provided, store it
	if (ground_truth)
	{
		setGroundTruth(ground_truth);
	}

	// update normalization factor in appearance model and deformation cost
	applyNormalizationFactor();

	// copy normalized frame
	for (int x = 0; x < normalizedFrame->width(); ++x)
		for (int y = 0; y < normalizedFrame->height(); ++y)
			(*normalizedFrame)(x, y) = (*inputImage)(x, y);

	timings.normalizedFrame = 0;

	timerPart.tic();

	// update values of unary and pair-wise potentials
	getQG();

	timings.features = timerPart.toc();

	timerPart.tic();

	// solve (Max,+) problem
	solver->solve(w, q, g, landmarksPositionsNF);

	// Get Q response
	for (int i=0; i < kLandmarksCount; ++i)
	{
		Q[i] = vertices[i].appearances[vertices[i].best]->getQvalue(&landmarksPositionsNF[INDEX(0, i, 2)], w[i][vertices[i].best]);
	}

	// Get G response
	for (int i=0; i < kEdgesCount; ++i)
	{
		G[i] = edges[i]->getGvalue(&landmarksPositionsNF[INDEX(0, edges[i]->getParent()->getNodeID(), 2)], &landmarksPositionsNF[INDEX(0, edges[i]->getChild()->getNodeID(), 2)], w[kLandmarksCount+i][0]);
	}


	timings.maxsum = timerPart.toc();

	timings.overall = timer.toc();
}

void CLandmark::nodemax_base(cimg_library::CImg<unsigned char> *inputImage, int *const ground_truth)
{
	setGroundTruth(ground_truth);

	// update normalization factor in appearance model and deformation cost
	applyNormalizationFactor();

	// copy normalized frame
	for (int x = 0; x < normalizedFrame->width(); ++x)
		for (int y = 0; y < normalizedFrame->height(); ++y)
			(*normalizedFrame)(x, y) = (*inputImage)(x, y);

	// get Qs
	getQG();

	// Solve the maximization	\hat{s} = \max_{s \in S} [ \ell(s^i, s) + < w, \Psi(I^i, s) > ]
	for (int i=0; i < kLandmarksCount; ++i)
	{
		DOUBLE maximum = -DBL_MAX;
		int argmax = 0;
		for (int j=0; j < vertices.at(i).appearances[vertices.at(i).best]->getLength(); ++j)
		{
			if (q[i][vertices.at(i).best][j] > maximum)
			{
				maximum = q[i][vertices.at(i).best][j];
				argmax = j;
			}
		}

		//
		const int *size = vertices.at(i).appearances[vertices.at(i).best]->getSize();
		int offset[2] = { vertices.at(i).appearances[vertices.at(i).best]->getSearchSpace()[0],
						  vertices.at(i).appearances[vertices.at(i).best]->getSearchSpace()[1] };

		landmarksPositionsNF[INDEX(0, i, 2)] = argmax / size[1] + offset[0];	// x-coordinate
		landmarksPositionsNF[INDEX(1, i, 2)] = argmax % size[1] + offset[1];	// y-coordinate
	}
}

DOUBLE* CLandmark::getFeatures(cimg_library::CImg<unsigned char> *inputImage, int * const boundingBox, int * const configuration)
{
	// obtain normalized frame
	getNormalizedFrame(inputImage, boundingBox);

	// update values of unary and pair-wise potentials
	getQG();

	// build vector psi
	getFeatures(configuration);

	return psi;
}

DOUBLE *CLandmark::getFeatures_base(cimg_library::CImg<unsigned char> *nf, int * const configuration)
{
//    *normalizedFrame = *nf;                     // TODO: check this !!!
	for (int x = 0; x < normalizedFrame->width(); ++x)
		for (int y = 0; y < normalizedFrame->height(); ++y)
			(*normalizedFrame)(x, y) = (*nf)(x, y);

	// update values of unary and pair-wise potentials
	getQG();

	// build vector psi
	getFeatures(configuration);

	return psi;
}

DOUBLE* CLandmark::getFeatures(int * const configuration)
{
	int psi_index = 0;
	int index = 0;
	int Si[2];
	int Sj[2];
	int P[2] = {0, 0};
	int parentID = 0;
	int childID = 0;

	// build vector psi

	// vertices
	for (int i=0; i < kLandmarksCount; ++i)
	{
		int best = vertices[i].best;

		P[0] = configuration[INDEX(0, i, 2)] - vertices[i].appearances[best]->getSearchSpace()[0];
		P[1] = configuration[INDEX(1, i, 2)] - vertices[i].appearances[best]->getSearchSpace()[1];
		index = INDEX(P[1], P[0], vertices.at(i).appearances[best]->getSize()[1]);

		vertices[i].appearances[best]->getFeatureAt(index, &psi[psi_index]);

		psi_index += vertices[i].appearances[best]->getFeatureDimension();
	}

	// edges
	for (int i=0; i < kEdgesCount; ++i)
	{
		int *tmp = new int[edges[i]->getDimension()];

		parentID = edges.at(i)->getParent()->getNodeID();
		childID = edges.at(i)->getChild()->getNodeID();

		Si[0] = configuration[INDEX(0, parentID, 2)];
		Si[1] = configuration[INDEX(1, parentID, 2)];
		Sj[0] = configuration[INDEX(0, childID, 2)];
		Sj[1] = configuration[INDEX(1, childID, 2)];

		//edges.at(i)->getDeformationCostAt(&Si[0], &Sj[0], (int*)&psi[psi_index]);
		edges.at(i)->getDeformationCostAt(&Si[0], &Sj[0], tmp);
		for (int j=0; j < edges[i]->getDimension(); ++j)
			psi[psi_index+j] = (DOUBLE)tmp[j];

		psi_index += edges.at(i)->getDimension();

		delete [] tmp;
	}

	return psi;
}

void CLandmark::setW(DOUBLE * const input_w)
{
	int start = 0;
	int end = 0;

	// vertices
	for (int i=0; i < kLandmarksCount; ++i)
	{
		int best = vertices[i].best;

		start = end;
		end += vertices[i].appearances[best]->getFeatureDimension();

		memcpy(w[i][best], &input_w[start], (end-start)*sizeof(DOUBLE));
	}

	// edges
	for (int i=0; i < kEdgesCount; ++i)
	{
		start = end;
		end += edges[i]->getDimension();

		memcpy(w[kLandmarksCount+i][0], &input_w[start], (end-start)*sizeof(DOUBLE));
	}
}

void CLandmark::setNodesW(DOUBLE *const input_w)
{
	int start = 0;
	int end = 0;

	// vertices
	for (int i=0; i < kLandmarksCount; ++i)
	{
		int best = vertices[i].best;

		start = end;
		end += vertices[i].appearances[best]->getFeatureDimension();

		memcpy(w[i][best], &input_w[start], (end-start)*sizeof(DOUBLE));
	}
}

void CLandmark::transformCoordinatesImage2NF(DOUBLE * const input, int *output)
{
	for (int i = 0; i < 2*kLandmarksCount; i+=2)
	{
		output[i] = int(floor(((input[i]-offset[0]) * scaleFactorInverse[0])+0.5));
		output[i+1] = int(floor(((input[i+1]-offset[1]) * scaleFactorInverse[1])+0.5));
	}
}

void CLandmark::transformCoordinatesNF2Image(int * const input, DOUBLE *output)
{
	for (int i = 0; i < 2*kLandmarksCount; i+=2)
	{
		output[i] = input[i]*scaleFactor[0] + offset[0];
		output[i+1] = input[i+1]*scaleFactor[1] + offset[1];
	}
}

void CLandmark::getNormalizedFrame(cimg_library::CImg<unsigned char> *inputImage, int *boundingBox)
{
	updateScaleFactor(boundingBox);
	(*croppedImage) = inputImage->get_crop(floor(region[0]+0.5), floor(region[1]+0.5), floor(region[0]+0.5)+floor(region[2]+0.5), floor(region[1]+0.5)+floor(region[3]+0.5));
	(*normalizedFrame) = croppedImage->get_resize(baseWindow[0], baseWindow[1], -100, -100, 3, 0, 0, 0, 0, 0);	// bilinear
}

//TODO: rewrite using affine transformation rather than rotating the whole image (=> speed-up)
DOUBLE CLandmark::getNormalizedFrameGeneralBB(cimg_library::CImg<unsigned char> *inputImage, int *boundingBox)
{
	DOUBLE angle = 0.0f;
	float angledeg = 0.0f;
	float center[2] = {0.0f, 0.0f};
	float X[2] = {0.0f, 0.0f};
	int rotbb[4] = {0, 0, 0, 0};

	// get center of the general bounding box
	center[0] = (boundingBox[INDEX(0, 0, 2)]+boundingBox[INDEX(0, 2, 2)])/2.0f;
	center[1] = (boundingBox[INDEX(1, 0, 2)]+boundingBox[INDEX(1, 2, 2)])/2.0f;

	X[0] = boundingBox[INDEX(0, 1, 2)];
	X[1] = boundingBox[INDEX(1, 0, 2)];

	// get angle of the in-plane rotation
	angle = (DOUBLE)std::asin( float(fabs(X[1]-boundingBox[INDEX(1, 1, 2)])/sqrt( square( boundingBox[INDEX(0, 0, 2)]-boundingBox[INDEX(0, 1, 2)] ) + square( boundingBox[INDEX(1, 0, 2)]-boundingBox[INDEX(1, 1, 2)] ) ) ));

	if (boundingBox[INDEX(1, 1, 2)] > X[1])
	{
		angle = -angle;
	}

	angledeg = angle * RADTODEG;

	// rotate input image
	inputImage->rotate(angledeg , center[0], center[1], 1.0f, 1, 0);

	// transform general bbox to axis aligned one (i.e. rotate the bounding box points)
	// x' = x * cos(theta) - y * sin(theta)
	// y' = x * sin(theta) + y * cos(theta)
	rotbb[0] = (boundingBox[INDEX(0, 0, 2)]-center[0])*cos(angle) - (boundingBox[INDEX(1, 0, 2)]-center[1])*sin(angle) + center[0];
	rotbb[1] = (boundingBox[INDEX(0, 0, 2)]-center[0])*sin(angle) + (boundingBox[INDEX(1, 0, 2)]-center[1])*cos(angle) + center[1];
	rotbb[2] = (boundingBox[INDEX(0, 2, 2)]-center[0])*cos(angle) - (boundingBox[INDEX(1, 2, 2)]-center[1])*sin(angle) + center[0];
	rotbb[3] = (boundingBox[INDEX(0, 2, 2)]-center[0])*sin(angle) + (boundingBox[INDEX(1, 2, 2)]-center[1])*cos(angle) + center[1];

	// update the scale factor
	updateScaleFactor(&rotbb[0]);

	(*croppedImage) = inputImage->get_crop(floor(region[0]+0.5), floor(region[1]+0.5), floor(region[0]+0.5)+floor(region[2]+0.5), floor(region[1]+0.5)+floor(region[3]+0.5));
	(*normalizedFrame) = croppedImage->get_resize(baseWindow[0], baseWindow[1], -100, -100, 3, 0, 0, 0, 0, 0);	// bilinear

	return angle;
}

void CLandmark::computeWdimension(void)
{
	wDimension = 0;

	// Nodes
	for (unsigned int i=0; i < vertices.size(); ++i)
	{
		int best = vertices[i].best;
		wDimension += vertices[i].appearances[best]->getFeatureDimension();
	}

	psiNodesDimension = wDimension;

	// Edges
	for (unsigned int i=0; i < edges.size(); ++i)
	{
		wDimension += edges[i]->getDimension();
	}
}

void CLandmark::write(const char *filename, bool writeW)
{
	XmlStorage fs(filename, FILE_WRITE);

	time_t rawtime;
	time(&rawtime);

	fs
	<< "name" << this->name	
	<< "version" << asctime(localtime(&rawtime))
	<< "num_nodes" << kLandmarksCount
	<< "num_edges" << kEdgesCount
	<< "graph_type" << TREE
	<< "bw_width" << baseWindow[0]
	<< "bw_height" << baseWindow[1]
	<< "bw_margin_x" << baseWindowMargin[0]
	<< "bw_margin_y" << baseWindowMargin[1];

	fs << "Nodes" << "[";
	for (int i=0; i < kLandmarksCount; ++i)
	{
		fs << "Node";
		fs << "{";

		vertices[i].write(fs);

		fs << "Appearances" << "[";
		for (unsigned int j=0; j < vertices[i].appearances.size(); ++j)
		{
			vertices[i].appearances[j]->write(fs, w[i][j], writeW);
		}
		fs << "]";

		fs << "}";
	}
	fs << "]";

	fs << "Edges" << "[";
	for (int i=0; i < kEdgesCount; ++i)
	{
		edges.at(i)->write(fs, w[kLandmarksCount+i][0], writeW);
	}
	fs << "]";

	fs.release();
}

int *CLandmark::getEdges(void)
{
	int *edgeList = new int[2*kEdgesCount];

	int edgeId = 0;
	for (unsigned int i=0; i < edges.size(); ++i)
	{
		edgeList[edgeId++] = edges[i]->getParent()->getNodeID();
		edgeList[edgeId++] = edges[i]->getChild()->getNodeID();
	}

	return edgeList;
}

int *CLandmark::getNodesDimensions(void)
{
	int *dimensions = new int[kLandmarksCount];

	for (int i=0; i < kLandmarksCount; ++i)
	{
		dimensions[i] = vertices[i].appearances[vertices[i].best]->getFeatureDimension();
	}

	return dimensions;
}
