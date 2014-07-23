#include "CDisplacementDeformationCost.h"

#include <cstring>

using namespace clandmark;

CDisplacementDeformationCost::CDisplacementDeformationCost(CAppearanceModel * const parent, CAppearanceModel * const child, int dimension)
	: CDeformationCost(parent, child)
{
	kDimension = dimension;

	parentLength = parent->getLength();
	childLength = child->getLength();

	deformationCosts = new int*[parentLength];
	for (int i = 0; i < parentLength; ++i)
	{
		deformationCosts[i] = new int[dimension*childLength];
		// TEST
		memset(deformationCosts[i], 0, sizeof(int)*dimension*childLength);
	}

	computeDeformationCosts();
}

CDisplacementDeformationCost::~CDisplacementDeformationCost()
{
	if (deformationCosts)
	{
		for (int i = 0; i < parentLength; ++i)
			delete [] deformationCosts[i];
		delete [] deformationCosts;
	}
}

void CDisplacementDeformationCost::getDeformationCostAt(int * const s_i, int * const s_j, int *deformation_cost)
{
	deformation_cost[0]	= s_j[0] - s_i[0];							// dx (= Sj_x - Si_x)
	deformation_cost[1] = s_j[1] - s_i[1];							// dy
	deformation_cost[2] = deformation_cost[0]*deformation_cost[0];	// dx^2
	deformation_cost[3] = deformation_cost[1]*deformation_cost[1];	// dy^2
	if (kDimension == 5)
		deformation_cost[4] = 1;
}

void CDisplacementDeformationCost::computeDeformationCosts()
{
	const int *i_size = parent->getSize();
	const int *j_size = child->getSize();
	int SiOffset[2] = { parent->getSearchSpace()[0], parent->getSearchSpace()[1] };
	int SjOffset[2] = { child->getSearchSpace()[0], child->getSearchSpace()[1] };
	int Si[2] = {0, 0};
	int Sj[2] = {0, 0};
	int index = 0;

	for (int i = 0; i < parentLength; ++i)
	{
		Si[0] = i / i_size[1] + SiOffset[0];		// x-coordinate
		Si[1] = i % i_size[1] + SiOffset[1];		// y-coordinate

		index = 0;

		for (int j = 0; j < childLength; ++j)
		{
			Sj[0]=j / j_size[1] + SjOffset[0];		// x-coordinate
			Sj[1]=j % j_size[1] + SjOffset[1];		// y-coordinate
			getDeformationCostAt(Si, Sj, &deformationCosts[i][index]);
			index+=kDimension;
		}
	}
}

void CDisplacementDeformationCost::dotProductWithWg(DOUBLE *const w, DOUBLE *g, const int index)
{
	DOUBLE dotProduct = 0.0;

	for (int i=0; i < childLength; ++i)
	{
		dotProduct = 0.0;

		for (int j=0; j < kDimension; ++j)
		{
			dotProduct += w[j]*deformationCosts[index][INDEX(j, i, kDimension)];
		}

		g[i] = dotProduct;
	}
}

void CDisplacementDeformationCost::update(DOUBLE * const w, DOUBLE *g, cimg_library::CImg<unsigned char> *image_data, int * const ground_truth)
{
	// Not needed for this type of deformation cost!!!
}

DOUBLE CDisplacementDeformationCost::getGvalue(int *const s_i, int *const s_j, DOUBLE *const w)
{
	DOUBLE dotProduct = 0.0;
	int * defCost = new int[kDimension];
//	int defCost[4];

	getDeformationCostAt(s_i, s_j, &defCost[0]);

	for (int j=0; j < kDimension; ++j)
	{
		dotProduct += w[j]*defCost[j];
	}

	delete [] defCost;

	return dotProduct;
}

void CDisplacementDeformationCost::write(XmlStorage &fs, DOUBLE * const w, bool writeW)
{
	fs << "Edge";
	fs << "{"
	   << "ParentID" << parent->getNodeID()
	   << "ChildID" << child->getNodeID()
	   << "Type" << DISPLACEMENT_VECTOR
	   << "Dims" << kDimension;

	if (loss)
		fs << "LossType" << loss->getName();
//		fs << "LossType" << loss->getType();

	if (writeW)
	{
		fs << "w";
		fs.writeRaw(w, sizeof(DOUBLE)*kDimension);
	}

	fs << "}";
}

DOUBLE * CDisplacementDeformationCost::distanceTransform1D(DOUBLE *f, int n)
{
	/*
	DOUBLE * d = new DOUBLE[n];
	int * v = new int[n];
	DOUBLE * z = new DOUBLE[n+1];
	int k = 0;
	v[0] = 0;
	z[0] = -INFINITY;
	z[1] = +INFINITY;


	for (int q = 1; q <= n-1; ++q)
	{
		DOUBLE s = ((f[q]+square(q)-q)-(f[v[k]]+square(v[k])-v[k]))/(2*q - 2*v[k]);

		while (s <= z[k])
		{
			--k;
			s = ((f[q]+square(q)-q)-(f[v[k]]+square(v[k])-v[k]))/(2*q - 2*v[k]);
		}

		++k;
		v[k] = q;
		z[k] = s;
		z[k+1] = +INFINITY;
	}

	k = 0;
	for (int q = 0; q <= n-1; ++q)
	{
		while (z[k+1] < q)
			++k;
		d[q] = square(q-v[k]) + (q-v[k]) + f[v[k]];
	}

	delete [] v;
	delete [] z;
	return d;
	*/
	return 0x0;
}

void CDisplacementDeformationCost::distanceTransform2D(DOUBLE *rectangle, int width, int height)
{
	DOUBLE * f = new DOUBLE[MAX(width, height)];

	// transform along columns
	for (int x = 0; x < width; ++x)
	{
		for (int y = 0; y < height; ++y)
		{
			f[y] = rectangle[INDEX(y, x, height)];
		}

		DOUBLE *d = distanceTransform1D(f, height);

		for (int y = 0; y < height; ++y)
		{
			rectangle[INDEX(y, x, height)] = d[y];
		}

		delete [] d;
	}

	// transform along rows
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			f[x] = rectangle[INDEX(y, x, height)];
		}

		DOUBLE *d = distanceTransform1D(f, width);

		for (int x = 0; x < width; ++x)
		{
			rectangle[INDEX(y, x, height)] = d[x];
		}

		delete [] d;
	}

	delete [] f;
}

