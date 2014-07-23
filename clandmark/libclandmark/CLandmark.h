/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 2013, 2014 Michal Uricar
 * Copyright (C) 2013, 2014 Michal Uricar
 */

#ifndef _CLANDMARK__H__
#define _CLANDMARK__H__

#include "CAppearanceModel.h"
#include "CDeformationCost.h"
#include "CMaxSumSolver.h"

#define cimg_verbosity 1		// we don't need window output capabilities of CImg
#define cimg_display 0			// we don't need window output capabilities of CImg

#include "CImg.h"

#include <vector>

// IO functions
#include <iostream>
#include <iomanip>

namespace clandmark {

// DOUBLE_PRECISION
#if DOUBLE_PRECISION==1
	typedef double DOUBLE;
#else
	typedef float DOUBLE;
#endif

/**
 *
 */
typedef enum {
	TREE=1,
	SIMPLE_NET=2,
	GENERAL_GRAPH=3
} EGraphType;

/**
 *
 */
typedef struct timings_struct {
	DOUBLE overall;				/**< */
	DOUBLE normalizedFrame;			/**< */
	DOUBLE features;			/**< */
	DOUBLE maxsum;				/**< */
} Timings;


/**
 * @brief The CLandmark class
 */
class CLandmark {

public:

	/**
	 * @brief CLandmark
	 * @param landmarksCount
	 * @param edgesCount
	 * @param base_window_width
	 * @param base_window_height
	 * @param base_window_margin_x
	 * @param base_window_margin_y
	 */
	CLandmark(
		int landmarksCount,
		int edgesCount,
		int base_window_width,
		int base_window_height,
		DOUBLE base_window_margin_x,
		DOUBLE base_window_margin_y
	);

	/**
	 * @brief Default CLandmark
	 */
	CLandmark();

	/**
	 * @brief ~CLandmark destructor
	 */
	virtual ~CLandmark();

	/**
	 * @brief Function detects landmarks within given bounding box in a given image.
	 * @param inputImage	Input image
	 * @param boundingBox	Bounding box (format: [min_x, min_y, max_x, max_y]) of object of interest (i.e. axis aligned)
	 * @param ground_truth
	 */
	void detect(cimg_library::CImg<unsigned char> *inputImage, int *boundingBox, DOUBLE * const ground_truth=0);

	/**
	 * @brief detect_mirrored
	 * @param inputImage
	 * @param boundingBox
	 * @param ground_truth
	 */
	void detect_mirrored(cimg_library::CImg<unsigned char> *inputImage, int *boundingBox, DOUBLE * const ground_truth=0);

	/**
	 * @brief detect_general_bbox
	 * @param inputImage			Input image
	 * @param boundingBox			Bounding box (format: [LeftTop_x, LeftTop_y, RightTop_x, RightTop_y, RightBot_x, RightBot_y, LeftBot_x, LeftBot_y]) of object of interest (i.e. with the inplane rotation)
	 * @param ground_truth
	 */
	void detect_general_bbox(cimg_library::CImg<unsigned char> *inputImage, int *boundingBox, DOUBLE *const ground_truth=0);

	/**
	 * @brief detect_general_bbox
	 * @param inputImage
	 * @param boundingBox
	 * @param ground_truth
	 */
	void detect_general_bbox_mirrored(cimg_library::CImg<unsigned char> *inputImage, int *boundingBox, DOUBLE *const ground_truth=0);

	/**
	 * @brief detect
	 * @param inputImage	normalized image frame
	 * @param ground_truth
	 */
	void detect_base(cimg_library::CImg<unsigned char> *inputImage, int * const ground_truth=0);

	/**
	 * @brief nodemax_base
	 * @param inputImage
	 * @param ground_truth
	 */
	void nodemax_base(cimg_library::CImg<unsigned char> *inputImage, int * const ground_truth=0);

	/**
	 * @brief getRegion
	 * @return
	 */
	inline DOUBLE *getRegion(void) { return &region[0]; }

	/**
	 * @brief getFeatures
	 * @param inputImage
	 * @param boundingBox
	 * @return
	 */
	DOUBLE *getFeatures(cimg_library::CImg<unsigned char> *inputImage, int * const boundingBox, int * const configuration);

	/**
	 * @brief getFeatures_base
	 * @param nf
	 * @param configuration
	 * @return
	 */
	DOUBLE *getFeatures_base(cimg_library::CImg<unsigned char> *nf, int * const configuration);

	/**
	 * @brief getFeatures
	 * @param configuration
	 * @return
	 */
	DOUBLE *getFeatures(int * const configuration);

	/**
	 * @brief getPsiNodes_base
	 * @param nf
	 * @param configuration
	 * @return
	 */
	DOUBLE *getPsiNodes_base(cimg_library::CImg<unsigned char> *nf, int * const configuration);

	/**
	 * @brief getPsiNodes
	 * @param configuration
	 * @return
	 */
	DOUBLE *getPsiNodes(int * const configuration);

	/**
	 * @brief setNormalizationFactor
	 * @param factor
	 */
	inline void setNormalizationFactor(DOUBLE factor) {	normalizationFactor = factor; }

	/**
	 * @brief getName
	 * @return
	 */
	inline std::string getName(void) { return name; }

	/**
	 * @brief setName
	 * @param name_
	 */
	inline void setName(std::string name_) { name = name_; }

	/**
	 * @brief getLandmarks
	 * @return
	 */
	inline DOUBLE *getLandmarks(void) { return landmarksPositions; }

	/**
	 * @brief getLandmarksNF
	 * @return
	 */
	inline int *getLandmarksNF(void) { return landmarksPositionsNF; }

	/**
	 * @brief getLandmarksCount
	 * @return
	 */
	inline int getLandmarksCount(void) { return vertices.size(); }

	/**
	 * @brief getEdgesCount
	 * @return
	 */
	inline int getEdgesCount(void) { return kEdgesCount; }

	/**
	 * @brief computeWdimension
	 */
	void computeWdimension(void);

	/**
	 * @brief getWdimension
	 * @return
	 */
	inline int getWdimension(void) { return wDimension; }

	/**
	 * @brief getPsiNodesDimension
	 * @return
	 */
	inline int getPsiNodesDimension(void) { return psiNodesDimension; }

	/**
	 * @brief getPsiEdgesDimension
	 * @return
	 */
	inline int getPsiEdgesDimension(void) { return wDimension-psiNodesDimension; }

	/**
	 * @brief getNodesDimensions
	 * @return
	 */
	int *getNodesDimensions(void);

	/**
	 * @brief setScaleFactor
	 * @param sf_x
	 * @param sf_y
	 */
	inline void setScaleFactor(DOUBLE sf_x, DOUBLE sf_y)
	{
		scaleFactor[0] = sf_x;
		scaleFactor[1] = sf_y;
		scaleFactorInverse[0] = 1.0/sf_x;
		scaleFactorInverse[1] = 1.0/sf_y;
	}

	/**
	 * @brief updateScaleFactor
	 * @param boundingBox
	 */
	inline void updateScaleFactor(int * const boundingBox)
	{
		// Enlarge boundingBox by given baseWindowMargin
		region[2] = (boundingBox[2]-boundingBox[0]+1)*baseWindowMargin[0];
		region[3] = (boundingBox[3]-boundingBox[1]+1)*baseWindowMargin[1];
		region[0] = (boundingBox[2]+boundingBox[0])/2.0 - region[2]/2.0;
		region[1] = (boundingBox[3]+boundingBox[1])/2.0 - region[3]/2.0;

		// compute scaleFactor and scaleFactorInverse & update offset
		scaleFactor[0] = (DOUBLE)region[2]/(DOUBLE)baseWindow[0];
		scaleFactor[1] = (DOUBLE)region[3]/(DOUBLE)baseWindow[1];
		scaleFactorInverse[0] = (DOUBLE)baseWindow[0]/(DOUBLE)region[2];
		scaleFactorInverse[1] = (DOUBLE)baseWindow[1]/(DOUBLE)region[3];

		//updateOffset(boundingBox);
		offset[0] = region[0];
		offset[1] = region[1];
	}

	/**
	 * @brief setW
	 * @param input_w
	 */
	void setW(DOUBLE * const input_w);

	/**
	 * @brief setNodesW
	 * @param input_w
	 */
	void setNodesW(DOUBLE * const input_w);

	/**
	 * @brief getQvalues
	 * @return
	 */
	inline DOUBLE *getQvalues(void) { return Q; }

	/**
	 * @brief getGvalues
	 * @return
	 */
	inline DOUBLE *getGvalues(void) { return G; }

	/**
	 * @brief write
	 * @param filename
	 */
	void write(const char *filename, bool writeW=true);

	/**
	 * @brief getEdges
	 * @return
	 */
	int *getEdges(void);

	// DEBUG -----------------------------------------
	inline bool nodeHasLoss(int nodeID) { return vertices[nodeID].appearances[0]->hasLoss(); }
	// //DEBUG ---------------------------------------

protected:

	// update values of functions q and g
	/**
	 * @brief getQG
	 */
	void getQG(void);

	/**
	 * @brief getQGTableLoss
	 */
	void getQGTableLoss(void);

	/**
	 * @brief setGroundTruth
	 * @param ground_truth
	 */
	void setGroundTruth(DOUBLE *ground_truth);

	/**
	 * @brief setGroundTruth
	 * @param ground_truth
	 */
	void setGroundTruth(int *ground_truth);

	// crop & resize enlarged face box from input image
	/**
	 * @brief getNormalizedFrame
	 * @param inputImage
	 * @param boundingBox
	 */
	void getNormalizedFrame(cimg_library::CImg<unsigned char> *inputImage, int *boundingBox);

	// crop & resize enlarged face box from rotated input image
	/**
	 * @brief getNormalizedFrameGeneralBB
	 * @param inputImage					input image
	 * @param boundingBox					general bbox (format: )
	 * @return						angle of the in-plane rotation (roll)
	 */
	DOUBLE getNormalizedFrameGeneralBB(cimg_library::CImg<unsigned char> *inputImage, int *boundingBox);

	/**
	 * @brief applyNormalizationFactor
	 */
	void applyNormalizationFactor(void);

	/**
	 * @brief transformCoordinatesImage2NF
	 * @param input
	 * @param output
	 */
	void transformCoordinatesImage2NF(DOUBLE * const input, int *output);

	/**
	 * @brief transformCoordinatesNF2Image
	 * @param input
	 * @param output
	 */
	void transformCoordinatesNF2Image(int * const input, DOUBLE *output);

	/**
	 * @brief init
	 * @param landmarksCount
	 * @param edgesCount
	 * @param base_window_width
	 * @param base_window_height
	 * @param base_window_margin_x
	 * @param base_window_margin_y
	 */
	void init(int landmarksCount,
			  int edgesCount,
			  int base_window_width,
			  int base_window_height,
			  DOUBLE base_window_margin_x,
			  DOUBLE base_window_margin_y);

private:

	/**
	 * @brief CLandmark
	 */
	CLandmark(const CLandmark&)
	{}

public:

	Timings timings;									/**< */

protected:

	std::string name;									/**< */

	// internal parameters
	int kLandmarksCount;								/**< */
	int kEdgesCount;									/**< */

	// convention for size [width x height]
	int baseWindow[2];									/**< */
	DOUBLE baseWindowMargin[2];							/**< */

	// normalized image frame related variables
	cimg_library::CImg<unsigned char> *normalizedFrame;	/**< */
	cimg_library::CImg<unsigned char> *croppedImage;	/**< */
	DOUBLE region[4];									/**< */
	DOUBLE scaleFactor[2];								/**< */
	DOUBLE scaleFactorInverse[2];						/**< */
	DOUBLE offset[2];									/**< */

	// Values of functions q and g (passed to solver)
	std::vector< std::vector< DOUBLE* > > q;			/**< */
	DOUBLE **g;											/**< */

	// individual parts of weight vector
	std::vector< std::vector< DOUBLE* > > w;			/**< */

	// dimension of weight vector
	int wDimension;										/**< */

	// Output
	DOUBLE *landmarksPositions;							/**< */
	int *landmarksPositionsNF;							/**< */

	// Graph
	std::vector< Vertex > vertices;						/**< */
	std::vector< CDeformationCost* > edges;				/**< */

	// MaxSum Solver
	CMaxSumSolver *solver;								/**< */

	// Q & G response for the detected landmarks positions
	DOUBLE * Q;											/**< */
	DOUBLE * G;											/**< */

	// GroundTruth (for training only)
	DOUBLE *groundTruthPositions;						/**< */
	int *groundTruthPositionsNF;						/**< */

	DOUBLE normalizationFactor;							/**< */

	DOUBLE *psi;										/**< */

	int psiNodesDimension;								/**< */
};

} /* namespace clandmark */

#endif /* _CLANDMARK__H__ */
