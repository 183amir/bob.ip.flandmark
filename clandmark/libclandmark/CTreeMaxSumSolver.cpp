#include "CTreeMaxSumSolver.h"

#include <cstring>
#include <cfloat>
#include <stdexcept>

using namespace clandmark;

CTreeMaxSumSolver::CTreeMaxSumSolver(std::vector< Vertex > * const vertices, std::vector< CDeformationCost* > * const edges)
	: CMaxSumSolver(vertices, edges)
{
	// initialization
	Node tempNode;
	std::vector< int > tempInDegrees;
	int id;
	DOUBLE *p_double = 0;
	int *p_int = 0;

	kLandmarks = (int)vertices->size();
	kEdges = (int)edges->size();

	for (int i=0; i < kLandmarks; ++i)
	{
		int best = vertices->at(i).best;
		int length = vertices->at(i).appearances[best]->getLength();

		p_double = new DOUBLE[length];
		memset(p_double, 0, sizeof(DOUBLE)*length);
		intermediateResults.push_back(p_double);

		argmaxIndices.push_back(int(0));
		inDegrees.push_back(int(0));
		tempNode.ID = i;
		graph.push_back(tempNode);
	}

	// fill in in-degrees
	for (int i=0; i < kEdges; ++i)
	{
		inDegrees[edges->at(i)->getParent()->getNodeID()]++;
		graph[edges->at(i)->getParent()->getNodeID()].incidentEdgeIDs.push_back(i);
		graph[edges->at(i)->getParent()->getNodeID()].children.push_back(edges->at(i)->getChild()->getNodeID());
		graph[edges->at(i)->getChild()->getNodeID()].ancestors.push_back(edges->at(i)->getParent()->getNodeID());
	}

	// Topological sort
	// 1. Find leaves and put them to queue
	for (unsigned int i=0; i < inDegrees.size(); ++i)
	{
		tempInDegrees.push_back(inDegrees[i]);

		if (inDegrees[i]==0)
		{
			topsortQueue.push(i);
		}
	}

	// 2. Go through queue and update it
	while (!topsortQueue.empty())
	{
		int top = topsortQueue.front();
		topsortQueue.pop();
		order.push_back(top);

		for (unsigned int i=0; i < graph[top].ancestors.size(); ++i)
		{
			id = graph[top].ancestors[i];
			if (--tempInDegrees[id] == 0)
			{
				topsortQueue.push(id);
			}
		}
	}

	// prepare intermediate arrays for indices
	for (int i=0; i < kLandmarks; ++i)
	{
		int best = 0, length = 0;
		if (graph[i].ancestors.size() > 0)
		{
			best = vertices->at(graph[i].ancestors.at(0)).best;
			length = vertices->at(graph[i].ancestors.at(0)).appearances[best]->getLength();
		} else {
			best = vertices->at(i).best;
			length = vertices->at(i).appearances[best]->getLength();
		}

		p_int = new int[length];
		memset(p_int, 0, sizeof(int)*length);
		intermediateIndices.push_back(p_int);
	}
}

CTreeMaxSumSolver::~CTreeMaxSumSolver()
{
	inDegrees.clear();

	for (unsigned int i=0; i < intermediateResults.size(); ++i)
	{
		delete [] intermediateResults[i];
		delete [] intermediateIndices[i];
	}

	intermediateResults.clear();
	intermediateIndices.clear();
	argmaxIndices.clear();
	order.clear();
	graph.clear();
}

void CTreeMaxSumSolver::maximizeSum(DOUBLE * const q, DOUBLE * const g, const int length, DOUBLE * maximum, int *index)
{
#if DOUBLE_PRECISION==1
	DOUBLE tmp_maximum = -DBL_MAX;
#else
	DOUBLE tmp_maximum = -FLT_MAX;
#endif
	int tmp_index = 0;
	DOUBLE sum = 0;

	for (int i=0; i < length; ++i)
	{
		sum = q[i]+g[i];
		if (sum > tmp_maximum)
		{
			tmp_maximum = sum;
			tmp_index = i;
		}
	}

	*maximum += tmp_maximum;
	*index = tmp_index;
}

void CTreeMaxSumSolver::solve(const std::vector<std::vector<DOUBLE *> > &w, const std::vector<std::vector<DOUBLE *> > &q, DOUBLE **g, int *output)
{
	int parentID = 0;
	int childID = 0;
	int edgeID = 0;
#if DOUBLE_PRECISION==1
	DOUBLE maximum = -DBL_MAX;
#else
	DOUBLE maximum = -FLT_MAX;
#endif

	// fill in taskQueue based on fixed order of Nodes (created in the constructor)
	for (unsigned int i=0; i < order.size(); ++i)
	{
		taskQueue.push(order[i]);
	}

	// While TaskQueue is not empty
	while (!taskQueue.empty())
	{
		parentID = taskQueue.front();
		taskQueue.pop();

		// B_i[x_i] = D_i(x_i) + \sum_{v_j \in C_i} max_{x_j} (B_j[x_j] + V_{ij}(x_i, x_j)
		memcpy(intermediateResults[parentID],
			q[parentID][vertices->at(parentID).best],
			vertices->at(parentID).appearances[vertices->at(parentID).best]->getLength()*sizeof(DOUBLE));

		// B_i[x_i] += \sum_{v_j \in C_i} max_{x_j} (B_j[x_j] + V_{ij}(x_i, x_j)
		if (graph[parentID].children.size())
		{
			int length = vertices->at(parentID).appearances[vertices->at(parentID).best]->getLength();

			for (int i=0; i < length; ++i)
			{
				for (unsigned int child=0; child < graph.at(parentID).children.size(); ++child)
				{
					childID = graph.at(parentID).children.at(child);
					edgeID = graph.at(parentID).incidentEdgeIDs.at(child);

					// get g[edgeID]
					edges->at(edgeID)->dotProductWithWg(w[kLandmarks+edgeID][0], g[edgeID], i);

					// max+argmax (q_childID + g_edgeID)
					maximizeSum(intermediateResults.at(childID),
								g[edgeID],
								vertices->at(childID).appearances[vertices->at(parentID).best]->getLength(),
								&intermediateResults.at(parentID)[i],
								&intermediateIndices.at(childID)[i]);
				}
			}
		}
	}

	// Find max and argmax over the root node
	for (int i=0; i < vertices->at(order.back()).appearances[vertices->at(order.back()).best]->getLength(); ++i)
	{
		if (intermediateResults.at(order.back())[i] > maximum)
		{
			maximum = intermediateResults.at(order.back())[i];
			argmaxIndices.at(order.back()) = i;
		}
	}

	// Trace back the argmax indices (from root to leaves)
	for (unsigned int i=order.size()-1; i > 0; --i)
	{
		for (unsigned int child=0; child < graph.at(order.at(i)).children.size(); ++child)
		{
			int childID = graph.at(order.at(i)).children[child];
			argmaxIndices.at(childID) = intermediateIndices.at(childID)[argmaxIndices.at(order.at(i))];
		}
	}

	// Transform the argmax indices to 2D coordinates (=output)
	for (unsigned int i=0; i < argmaxIndices.size(); ++i)
	{
		const int *size = vertices->at(i).appearances[vertices->at(i).best]->getSize();
		int offset[2] = { vertices->at(i).appearances[vertices->at(i).best]->getSearchSpace()[0],
						  vertices->at(i).appearances[vertices->at(i).best]->getSearchSpace()[1] };

		output[INDEX(0, i, 2)] = argmaxIndices[i] / size[1] + offset[0];	// x-coordinate
		output[INDEX(1, i, 2)] = argmaxIndices[i] % size[1] + offset[1];	// y-coordinate
	}
}
