/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 2013 Michal Uricar
 * Copyright (C) 2013 Michal Uricar
 */

#ifndef _APPEARANCEMODEL_H__
#define _APPEARANCEMODEL_H__

#include "msvc-compat.h"
#include "CLoss.h"

#define cimg_verbosity 1		// we don't need window output capabilities of CImg
#define cimg_display 0			// we don't need window output capabilities of CImg
#include "CImg.h"

#include "CXMLInOut.h"

#include <iostream>
#include <string>
#include <vector>

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
	SPARSE_LBP=1,
	EXTENDED_SPARSE_LBP=2,
	HOG=3
} EAppearanceModelType;

/**
 * @brief The CAppearanceModel class
 */
class CAppearanceModel {

public:

	/**
	 * @brief CAppearanceModel
	 * @param nodeID
	 * @param search_space_minx
	 * @param search_space_miny
	 * @param search_space_maxx
	 * @param search_space_maxy
	 * @param window_size_w
	 * @param window_size_h
	 * @param loss_
	 */
	CAppearanceModel(
		int nodeID,
		int search_space_minx,
		int search_space_miny,
		int search_space_maxx,
		int search_space_maxy,
		int window_size_w,
		int window_size_h,
		CLoss *loss_=0
	);

	/**
	 * @brief ~CAppearanceModel
	 */
	virtual ~CAppearanceModel();

	/**
	 * @brief update
	 * @param image_data
	 * @param w
	 * @param q
	 * @param groundTruth
	 */
	void update(cimg_library::CImg<unsigned char> *image_data, DOUBLE * const w, DOUBLE *q, int * const groundTruth=0);

	/**
	 * @brief getLength
	 * @return
	 */
	inline const int getLength(void) { return kLength; }

	/**
	 * @brief getSearchSpace
	 * @return
	 */
	inline const int* getSearchSpace(void) { return &searchSpace[0]; }

	/**
	 * @brief getSize
	 * @return
	 */
	inline const int* getSize(void) { return &size[0]; }

	/**
	 * @brief getFeatureDimension
	 * @return
	 */
	inline const int getFeatureDimension(void) { return kFeatureDimension; }

	/**
	 * @brief getNodeID
	 * @return
	 */
	inline const int getNodeID(void) { return kNodeID; }

	/**
	 * @brief getFeatureAt
	 * @param index
	 * @param feature_vector
	 */
	virtual void getFeatureAt(const int index, DOUBLE *feature_vector) = 0;

	/**
	 * @brief setLossNormalizationFactor
	 * @param factor
	 */
	inline void setLossNormalizationFactor(DOUBLE factor)
	{
		if (loss)
			loss->setNormalizationFactor(factor);
	}

	/**
	 * @brief getLossNormalizationFactor
	 * @return
	 */
	inline DOUBLE getLossNormalizationFactor(void)
	{
		if (loss)
			return loss->getNormalizationFactor();
		return 1.0;
	}

	/**
	 * @brief getLoss
	 * @return
	 */
	inline CLoss * getLoss(void) { return loss; }

	/**
	 * @brief getQvalue
	 * @param s_i
	 * @param w
	 * @return
	 */
	virtual DOUBLE getQvalue(int * const s_i, DOUBLE * const w) = 0;

	/**
	 * @brief write
	 * @param fs
	 */
	virtual void write(XmlStorage& fs, DOUBLE * const w, bool writeW=true) const = 0;

	///DEBUG
	inline void setName(std::string name) { this->name = name; }
	inline std::string getName() { return name; }
	///-----

	//=====
//	virtual int * returnInternalFeatureRep(int *dim, int *length) = 0;
	inline bool hasLoss(void) { return (loss == 0x0) ? false : true; }
	//=====

protected:

	/**
	 * @brief computeFeatures
	 */
	virtual void computeFeatures(void) = 0;

	/**
	 * @brief dotProductWithWq
	 * @param w
	 * @param q
	 */
	virtual void dotProductWithWq(DOUBLE * const w, DOUBLE *q) = 0;

	/**
	 * @brief computeWindowPositions
	 */
	void computeWindowPositions(void);

private:

	/**
	 * @brief init
	 */
	void init(void);

	/**
	 * @brief CAppearanceModel
	 */
	CAppearanceModel(const CAppearanceModel&) : kNodeID(-1), kLength(-1)
	{}

protected:

	const int kNodeID;
	const int kLength;		// number of feature vectors for node (component)

	int size[2];
	int searchSpace[4];
	int kFeatureDimension;	// dimensionality of features

	cimg_library::CImg<unsigned char> *imageData;
	int *windowPositions;
	int windowSize[2];

	// Loss function
	CLoss *loss;

	std::string name;

	// internal representation of features goes to specialized classes
};


/**
 * @brief The Vertex class
 */
class Vertex {

public:

	/**
	 * @brief Vertex
	 */
	Vertex()
	{
		best=0;
		appearances = std::vector< CAppearanceModel* >();
		ss[0] = 0;
		ss[1] = 0;
		ss[2] = 0;
		ss[3] = 0;
		winSize[0] = 0;
		winSize[1] = 0;
		nodeID = 0;
		name = "";
	}

	/**
	 * @brief Vertex
	 * @param node_id
	 * @param search_space_minx
	 * @param search_space_miny
	 * @param search_space_maxx
	 * @param search_space_maxy
	 * @param window_size_w
	 * @param window_size_h
	 */
	Vertex(int node_id,
		   std::string name_,
		   int search_space_minx,
		   int search_space_miny,
		   int search_space_maxx,
		   int search_space_maxy,
		   int window_size_w,
		   int window_size_h)
	{
		nodeID = node_id;
		name = name_;
		ss[0] = search_space_minx;
		ss[1] = search_space_miny;
		ss[2] = search_space_maxx;
		ss[3] = search_space_maxy;
		winSize[0] = window_size_w;
		winSize[1] = window_size_h;
		best = 0;
		appearances = std::vector< CAppearanceModel* >();
	}

	/**
	 *
	 */
	~Vertex() { }

	/**
	 * @brief write
	 * @param fs
	 */
	void write(XmlStorage &fs) const
	{
		fs << "Header";
		fs << "{"
		   << "NodeID" << nodeID
		   << "Name" << name
		   << "ss_minX" << ss[0]
		   << "ss_minY" << ss[1]
		   << "ss_maxX" << ss[2]
		   << "ss_maxY" << ss[3]
		   << "win_size_width" << winSize[0]
		   << "win_size_height" << winSize[1]
		   << "}";
	}

	int best;
	std::vector< CAppearanceModel* > appearances;
	int ss[4];
	int winSize[2];
	int nodeID;
	std::string name;
};

}

#endif // _APPEARANCEMODEL_H__
