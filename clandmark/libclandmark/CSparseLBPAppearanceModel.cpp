#include "CSparseLBPAppearanceModel.h"

#include <cstring>

using namespace clandmark;

CSparseLBPAppearanceModel::CSparseLBPAppearanceModel(
		int nodeID,
		int search_space_min_x,	int search_space_min_y,	int search_space_max_x,	int search_space_max_y,
		int window_size_w,	int window_size_h,
		int height_of_pyramid,
		CLoss *loss_
		) : CAppearanceModel(
				nodeID,
				search_space_min_x,	search_space_min_y,	search_space_max_x,	search_space_max_y,
				window_size_w, window_size_h,
				loss_
			), kHeightOfPyramid(height_of_pyramid)
{
	kFeatureDimension = getDim(window_size_w, window_size_h, height_of_pyramid);
	kSparseFeatureDimension = kFeatureDimension/256;

	// allocate features
	LBPFeatures = new int[kLength*kSparseFeatureDimension];
	memset(LBPFeatures, 0, sizeof(int)*kLength*kSparseFeatureDimension);

	// prepare window
	window = new int[window_size_w*window_size_h];
	memset(window, 0, window_size_w*window_size_h*sizeof(int));
}

CSparseLBPAppearanceModel::~CSparseLBPAppearanceModel()
{
	if (LBPFeatures)
		delete [] LBPFeatures;
	if (window)
		delete [] window;
}

void CSparseLBPAppearanceModel::getFeatureAt(const int index, DOUBLE *feature_vector)
{
	// zero-out feature vector
	memset(feature_vector, 0, kFeatureDimension*sizeof(DOUBLE));

	for (int i=0; i < kSparseFeatureDimension; ++i)
	{
		feature_vector[LBPFeatures[kSparseFeatureDimension*index+i]] = 1.0;
	}
}

void CSparseLBPAppearanceModel::computeFeatures(void)
{
	int subWin[4] = {0, 0, windowSize[0], windowSize[1]};

	for (int i=0; i < kLength; ++i)
	{
		subWin[0] = windowPositions[INDEX(0, i, 2)];
		subWin[1] = windowPositions[INDEX(1, i, 2)];

		// copy subwindow of inputImage to window (content of window is destroyed during computation of features!!!)
		// TODO: check this !!!
		int index = 0;
		for (int x=subWin[0]; x < subWin[0]+subWin[2]; ++x)
		{
			for (int y=subWin[1]; y < subWin[1]+subWin[3]; ++y)
			{
				window[index++] = (int)(*imageData)(x, y);
			}
		}

		featuresSparse(&LBPFeatures[kSparseFeatureDimension*i]);
	}
}

int CSparseLBPAppearanceModel::getDim(int img_nRows, int img_nCols, int nPyramids)
{
	int w = img_nCols;
	int h = img_nRows;
	int N=0;

	for(int i=0; (i < nPyramids) && (MIN(w,h) >= 3); ++i)
	{
		N += (w-2)*(h-2);

		if (w % 2)
			w--;
		if (h % 2)
			h--;
		w >>= 1;
		h >>= 1;
	}

	return N << 8;
}

void CSparseLBPAppearanceModel::featuresSparse(int *vec)
{
	int offset, ww, hh, x, y, idx, j;
	unsigned char pattern;
	int center;

	idx = 0;
	offset = 0;
	ww = windowSize[0];
	hh = windowSize[1];

	while (1)
	{
		for (x = 1; x < ww-1; ++x)
		{
			for (y = 1; y < hh-1; ++y)
			{
				pattern = 0;
				center = window[INDEX(y,x,windowSize[1])];
				if(window[INDEX(y-1,x-1,windowSize[1])] < center) pattern = pattern | 0x01;
				if(window[INDEX(y-1,x  ,windowSize[1])] < center) pattern = pattern | 0x02;
				if(window[INDEX(y-1,x+1,windowSize[1])] < center) pattern = pattern | 0x04;
				if(window[INDEX(y  ,x-1,windowSize[1])] < center) pattern = pattern | 0x08;
				if(window[INDEX(y  ,x+1,windowSize[1])] < center) pattern = pattern | 0x10;
				if(window[INDEX(y+1,x-1,windowSize[1])] < center) pattern = pattern | 0x20;
				if(window[INDEX(y+1,x  ,windowSize[1])] < center) pattern = pattern | 0x40;
				if(window[INDEX(y+1,x+1,windowSize[1])] < center) pattern = pattern | 0x80;

				vec[idx++] = offset + pattern;
				offset += 256;
			}
		}

		if (kSparseFeatureDimension <= idx)
			return;

		if (ww % 2 == 1)
			--ww;
		if (hh % 2 == 1)
			--hh;

		ww >>= 1;

		for (x=0; x < ww; ++x)
			for (j=0; j < hh; ++j)
				window[INDEX(j,x,windowSize[1])] = window[INDEX(j,2*x,windowSize[1])] + window[INDEX(j,2*x+1,windowSize[1])];

		hh >>= 1;

		for (y=0; y < hh; ++y)
			for (j=0; j < ww; ++j)
				window[INDEX(y,j,windowSize[1])] = window[INDEX(2*y,j,windowSize[1])] + window[INDEX(2*y+1,j,windowSize[1])];
	}

	return;
}

void CSparseLBPAppearanceModel::dotProductWithWq(DOUBLE *const w, DOUBLE *q)
{
	int temp_index = 0;
	DOUBLE dotProduct = 0.0;

	// sparse dot product of LBP features and vector w
	for (int i=0; i < kLength; ++i)
	{
		dotProduct = 0.0;
		for (int j=0; j < kSparseFeatureDimension; ++j)
		{
			temp_index = LBPFeatures[INDEX(j, i, kSparseFeatureDimension)];
			dotProduct += w[temp_index];
		}
		q[i] = dotProduct;
	}
}

DOUBLE CSparseLBPAppearanceModel::getQvalue(int *const s_i, DOUBLE *const w)
{
	int temp_index = 0;
	DOUBLE dotProduct = 0.0;

	int offset[2] = {searchSpace[0], searchSpace[1]};
	int i = INDEX(s_i[1]-offset[1], s_i[0]-offset[0], size[1]);

	for (int j=0; j < kSparseFeatureDimension; ++j)
	{
//		temp_index = LBPFeatures[INDEX(s_i[1], s_i[0], kSparseFeatureDimension)];
		temp_index = LBPFeatures[INDEX(j, i, kSparseFeatureDimension)];
		dotProduct += w[temp_index];
	}

	return dotProduct;
}

void CSparseLBPAppearanceModel::write(XmlStorage &fs, DOUBLE * const w, bool writeW) const
{
	fs << "Appearance";
	fs << "{"
	   << "Type" << "SPARSE_LBP"
	   << "hop" << kHeightOfPyramid;

	if (loss)
		fs << "LossType" << loss->getName();
//		fs << "LossType" << loss->getType();

	if (writeW)
	{
		fs << "w";
		fs.writeRaw(w, sizeof(DOUBLE)*kFeatureDimension);
	}

	fs << "}";
}
