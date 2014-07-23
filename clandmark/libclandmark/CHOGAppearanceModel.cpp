#include "CHOGAppearanceModel.h"

using namespace clandmark;

#define PI 3.14159265358979323846
//const DOUBLE PI = 3.14159265358979323846;

CHOGAppearanceModel::CHOGAppearanceModel(
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
		CLoss* loss_
		) : CAppearanceModel(
				nodeID,
				search_space_min_x,	search_space_min_y,	search_space_max_x,	search_space_max_y,
				window_size_w, window_size_h,
				loss_
			), kNbins(n_bins), kCellSize(cell_size), kBlockSize(block_size)
{
	kFeatureDimension = getDim(window_size_w, window_size_h, n_bins, cell_size, block_size);

	// allocate features
	HOGFeatures = new DOUBLE[kLength*kFeatureDimension];
	memset(HOGFeatures, 0, sizeof(DOUBLE)*kLength*kFeatureDimension);

	// prepare window
	window = new int[window_size_w*window_size_h];
	memset(window, 0, window_size_w*window_size_h*sizeof(int));

	// helper dimensions
	cH = ceil( ((float)(window_size_h - 2))/cell_size );
	cW = ceil( ((float)(window_size_w - 2))/cell_size );
	cDim = cH*cW;

	// allocate helper arrays
	derivx = new DOUBLE[(window_size_w-2)*(window_size_h-2)];
	derivy = new DOUBLE[(window_size_w-2)*(window_size_h-2)];
	angles = new DOUBLE[(window_size_w-2)*(window_size_h-2)];
	magnitudes = new DOUBLE[(window_size_w-2)*(window_size_h-2)];
	hist = new DOUBLE[cDim*kNbins];
}

CHOGAppearanceModel::~CHOGAppearanceModel()
{
	if (HOGFeatures)
		delete [] HOGFeatures;
	if (window)
		delete [] window;
	if (derivx)
		delete [] derivx;
	if (derivy)
		delete [] derivy;
	if (angles)
		delete [] angles;
	if (magnitudes)
		delete [] magnitudes;
	if (hist)
		delete [] hist;
}

void CHOGAppearanceModel::getFeatureAt(const int index, DOUBLE *feature_vector)
{
	memcpy(feature_vector, &HOGFeatures[index*kFeatureDimension], sizeof(DOUBLE)*kFeatureDimension);
}

void CHOGAppearanceModel::computeFeatures(void)
{
	int subWin[4] = {0, 0, windowSize[0], windowSize[1]};

	for (int i=0; i < kLength; ++i)
	{
		subWin[0] = windowPositions[INDEX(0, i, 2)];
		subWin[1] = windowPositions[INDEX(1, i, 2)];

		// copy subwindow of inputImage to window (content of window is destroyed during computation of features!!!)
		int index = 0;
		for (int x=subWin[0]; x < subWin[0]+subWin[2]; ++x)
		{
			for (int y=subWin[1]; y < subWin[1]+subWin[3]; ++y)
			{
				window[index++] = (int)(*imageData)(x, y);
			}
		}

		hogFeatures(&HOGFeatures[kFeatureDimension*i]);
	}
}

void CHOGAppearanceModel::dotProductWithWq(DOUBLE *const w, DOUBLE *q)
{
	DOUBLE dotProduct = 0.0;

	// dense dot product of HOG features and vector w
	for (int i=0; i < kLength; ++i)
	{
		dotProduct = 0.0;
		for (int j=0; j < kFeatureDimension; ++j)
		{
			dotProduct += w[j]*HOGFeatures[INDEX(j, i, kFeatureDimension)];
		}
		q[i] = dotProduct;
	}
}

DOUBLE CHOGAppearanceModel::getQvalue(int *const s_i, DOUBLE *const w)
{
	DOUBLE dotProduct = 0.0;

	int offset[2] = {searchSpace[0], searchSpace[1]};
	int i = INDEX(s_i[1]-offset[1], s_i[0]-offset[0], size[1]);

	for (int j=0; j < kFeatureDimension; ++j)
	{
		//dotProduct += w[j]*HOGFeatures[INDEX(s_i[1], s_i[0], kFeatureDimension)];
		dotProduct += w[j]*HOGFeatures[INDEX(j, i, kFeatureDimension)];
	}

	return dotProduct;
}

void CHOGAppearanceModel::write(XmlStorage &fs, DOUBLE *const w, bool writeW) const
{
	fs	<< "Appearance";
	fs	<< "{"
		<< "Type" << "HOG"
		<< "n_bins" << kNbins
		<< "cell_size" << kCellSize
		<< "block_size" << kBlockSize;

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

void CHOGAppearanceModel::hogFeatures(DOUBLE *vec)
{
	// TODO - write HOG implementation here...
	DOUBLE weight = 0, sum = 0;
	DOUBLE *last;
	int lhid, cid;
	int ww = windowSize[0];
	int hh = windowSize[1];

	// compute image derivatives
	DOUBLE dx, dy;
	for (int x=1; x < ww-1; ++x)
	{
		for (int y=1; y < hh-1; ++y)
		{
			dx = (DOUBLE)(window[INDEX(y, x+1, hh)] - window[INDEX(y, x-1, hh)]);
			dy = (DOUBLE)(window[INDEX(y+1, x, hh)] - window[INDEX(y-1, x, hh)]);

			derivx[INDEX(y-1, x-1, hh-2)] = dx;
			derivy[INDEX(y-1, x-1, hh-2)] = dy;

			//tmp_angles[INDEX(y-1, x-1, win_H-2)] = abs(atan2(dy, dx) * (180.0 / PI));
			angles[INDEX(y-1, x-1, hh-2)] = (((dx == 0) ? 0 : atan(dy/dx))/PI+0.5)*kNbins;
			//angles[INDEX(y-1, x-1, hh-2)] = (((dx == 0) ? 0 : atan(dy/dx))/(cimg_library::cimg::PI)+0.5)*kNbins;
			magnitudes[INDEX(y-1, x-1, hh-2)] = sqrt(dx*dx + dy*dy);
		}
	}

	// compute histograms for cells
	for(int j = 0; j< cDim*kNbins; ++j)
	{
		hist[j] = 0;
	}
	for (int x=0; x < ww-2; ++x)
	{
		for (int y=0; y < hh-2; ++y)
		{
			cid = (x/kCellSize)*cH + y/kCellSize;
			lhid = (int)floor(angles[INDEX(y, x, hh-2)]);
			weight = 1 - (angles[INDEX(y, x, hh-2)] - lhid);
			hist[INDEX(cid, lhid, cDim)] += weight*magnitudes[INDEX(y, x, hh-2)];
			hist[INDEX(cid, (lhid+1) % kNbins, cDim)] += (1-weight)*magnitudes[INDEX(y, x, hh-2)];
		}
	}

	//
	for(int bi=0; bi < cW - (kBlockSize >> 1); bi += kBlockSize >> 1)
	{
		for(int bj=0; bj < cH - (kBlockSize >> 1); bj += kBlockSize >> 1)
		{
			last=vec;
			sum=0;
			for(int x=bi; x < bi + kBlockSize; ++x)
			{
				for(int y=bj; y < bj + kBlockSize; ++y)
				{
					if( y >= cH || x >= cW)
						continue;
					for(lhid=0; lhid < kNbins; ++lhid)
					{
						*(vec++) = hist[lhid*cDim + x*cH + y];
						sum += pow(hist[lhid*cDim + x*cH + y], 2);
					}
				}
			}
			if(sum == 0)
			{
				sum = 1.0/sqrt((DOUBLE)(vec-last));
				while(last != vec)
					*(last++) = sum;
			} else {
				sum = sqrt(sum);
				while(last != vec)
				{
					*(last++) /= sum;
				}
			}
		}
	}
}

int CHOGAppearanceModel::getDim(int img_nRows, int img_nCols, int nBins, int cellSize, int blockSize)
{
	// count of cells
	int cH = (int)ceil( (DOUBLE)(img_nRows - 2.0)/(DOUBLE)cellSize );
	int cW = (int)ceil( (DOUBLE)(img_nCols - 2.0)/(DOUBLE)cellSize );

	// count of blocks
//	int bH = (int)ceil( (DOUBLE)(cH)/(DOUBLE)( blockSize >> 1 ) ) - 1;
//	int bW = (int)ceil( (DOUBLE)(cW)/(DOUBLE)( blockSize >> 1 ) ) - 1;

	int result = 0;

	for (int bi=0; bi < cW - (blockSize >> 1); bi += (blockSize >> 1))
	{
		for (int bj=0; bj < cH - (blockSize >> 1); bj += (blockSize >> 1))
		{
			for (int x=bi; x < bi + blockSize; ++x)
				for (int y=bj; y < bj + blockSize; ++y)
				{
					if (y >= cH || x >= cW)
						continue;
					++result;
				}
		}
	}

	// size of the descriptor
	result *= nBins;

	return result;
}

