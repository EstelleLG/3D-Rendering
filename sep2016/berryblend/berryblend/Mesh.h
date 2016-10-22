#pragma once

#include "berryblend.h"
#include <queue>
#include <utility>

namespace Berryblend {
	typedef unsigned int uint32;
	class Mesh {
		protected: 
			std::vector<Vertex> vertices;
			std::vector<unsigned int> indices;

	public:
		void addTriangle(unsigned int a, unsigned int b, unsigned int c) {
			indices.push_back(a);
			indices.push_back(b);
			indices.push_back(c);
		}

		void addVertices(Vertex a) {
			vertices.push_back(a);
		}

		void loadToVertexArray(GLVertexArray& vertexArray) {

			GLArrayBuffer arrayBuffer;
			GLElementBuffer elementBuffer;

			arrayBuffer.bindBuffer();
			arrayBuffer.bufferData(sizeof(Vertex)* vertices.size(), &vertices[0], GL_STATIC_DRAW);

			vertexArray.bindVertexArray();
			vertexArray.vertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), 0);
			vertexArray.vertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(Vertex), (void*)12);
			vertexArray.vertexAttribPointer(2, 3, GL_FLOAT, false, sizeof(Vertex), (void*)20);

			elementBuffer.bindBuffer();
			elementBuffer.bufferData(4 * indices.size(), &indices[0], GL_STATIC_DRAW);
			vertexArray.unbindVertexArray();

		}
		int indicesSize() {
			return indices.size();
		}
	};

	class MeshFactory {
	public:
		Mesh createBox(float width, float height, float length) {
			Mesh mesh;

			//counterclockwise front??

			Vertex a = { glm::vec3(-0.5, 0.5, 0.5), glm::vec2(0,0), glm::vec3(0,0,1) };
			Vertex b = { glm::vec3(-0.5, -0.5, 0.5), glm::vec2(0,0), glm::vec3(0,0,1) };
			Vertex c = { glm::vec3(0.5, -0.5, 0.5), glm::vec2(0,0), glm::vec3(0,0,1) };
			Vertex d = { glm::vec3(0.5, 0.5, 0.5), glm::vec2(0,0), glm::vec3(0,0,1) };

			glm::mat4 scale = glm::scale(glm::vec3(width, height, length));
			glm::mat4 rotate = glm::rotate(float(M_PI), glm::vec3(0, 1, 0));
			glm::mat4 rs = rotate * scale;

			//front
			mesh.addTriangle(0, 1, 2);
			mesh.addTriangle(0, 2, 3);
			mesh.addVertices(scale * a);
			mesh.addVertices(scale * b);
			mesh.addVertices(scale * c);
			mesh.addVertices(scale * d);

			//back
			mesh.addTriangle(4, 5, 6);
			mesh.addTriangle(4, 6, 7);
			mesh.addVertices(rs * a);
			mesh.addVertices(rs * b);
			mesh.addVertices(rs * c);
			mesh.addVertices(rs * d);

			rotate = glm::rotate(float(M_PI / 2), glm::vec3(1, 0, 0));
			rs = scale * rotate;  //first rotate then scale
								  //top
			mesh.addTriangle(8, 9, 10);
			mesh.addTriangle(8, 10, 11);
			mesh.addVertices(rs * a);
			mesh.addVertices(rs * b);
			mesh.addVertices(rs * c);
			mesh.addVertices(rs * d);

			rotate = glm::rotate(float(-M_PI / 2), glm::vec3(1, 0, 0));
			rs = scale * rotate;
			//bottom
			mesh.addTriangle(12, 13, 14);
			mesh.addTriangle(12, 14, 15);
			mesh.addVertices(rs * a);
			mesh.addVertices(rs * b);
			mesh.addVertices(rs * c);
			mesh.addVertices(rs * d);

			rotate = glm::rotate(float(M_PI / 2), glm::vec3(0, 1, 0));
			rs = scale * rotate;
			//right
			mesh.addTriangle(16, 17, 18);
			mesh.addTriangle(16, 18, 19);
			mesh.addVertices(rs * a);
			mesh.addVertices(rs * b);
			mesh.addVertices(rs * c);
			mesh.addVertices(rs * d);

			rotate = glm::rotate(float(-M_PI / 2), glm::vec3(0, 1, 0));
			rs = scale * rotate;
			//left
			mesh.addTriangle(20, 21, 22);
			mesh.addTriangle(20, 22, 23);
			mesh.addVertices(rs * a);
			mesh.addVertices(rs * b);
			mesh.addVertices(rs * c);
			mesh.addVertices(rs * d);


			return mesh;
		}

		Mesh createPlane(float width, float height) {
			Mesh mesh;
			float xmin = -width / 2;
			float ymin = -height / 2;
			float xmax = width / 2;
			float ymax = height / 2;
			Vertex a = Vertex(glm::vec3(xmin, ymin, 0), glm::vec2(0, 0), glm::vec3(0, 0, 1));
			Vertex b = Vertex(glm::vec3(xmin, ymax, 0), glm::vec2(0, 1), glm::vec3(0, 0, 1));
			Vertex c = Vertex(glm::vec3(xmax, ymax, 0), glm::vec2(1, 1), glm::vec3(0, 0, 1));
			Vertex d = Vertex(glm::vec3(xmax, ymin, 0), glm::vec2(1, 0), glm::vec3(0, 0, 1));


			mesh.addVertices(a);
			mesh.addVertices(b); 
			mesh.addVertices(c);
			mesh.addVertices(d);
			

			mesh.addTriangle(0, 3, 1);
			mesh.addTriangle(1, 3, 2);

			return mesh;

		}
	};


#define LMAX 10
	class RoamHeightMap : public Mesh {
		class BinNode {
		public:
			uint32 index;  //*3
			BinNode* child = NULL;
			BinNode* leftNeighbor = NULL;
			BinNode* rightNeighbor = NULL;
			BinNode* baseNeighbor = NULL;
			int coarsity = 0;

			BinNode* getLeft() { return child; }
			BinNode* getRight() { return child + 1; }
		};

		struct PriorityBN {
			BinNode* binnode;
			double priority;
	
			PriorityBN(BinNode* bn, double priority) : binnode(bn), priority(priority) {}
		};

		struct compPriority {
			bool operator()(const PriorityBN& a, const PriorityBN& b) {
				return a.priority < b.priority;
			}
		};

		class Allocator {
		public:
			std::vector<Vertex>& vertices;
			std::vector<unsigned int>& indices;
			std::vector<BinNode*> binNodes;
			std::vector<float> wedgieTree;

			unsigned char* img;
			uint32 height;
			uint32 width;
			float maxHeight;
			glm::vec2 pixelSize;
		
			Vertex sampleVertex(Vertex A, Vertex B) {
				glm::vec3 midPosition = (A.position + B.position) / 2.0f;

				float startx = -(width / 2 * pixelSize.x);
				float starty = -(height / 2 * pixelSize.y);
				int midX = (midPosition[0] - startx) / pixelSize.x;
				int midZ = (midPosition[2] - starty) / pixelSize.y;
				float midY = img[(midX + width* midZ) * 3] / 255.0 * maxHeight;

				return Vertex(midPosition.x, midY, midPosition.z);

			}
			unsigned int wedgieTreeSize() {
				unsigned int size = 2;
				int prev = 2;
				for (int i = 0; i < LMAX - 1; i++) {
					prev *= 2;
					size += prev;
				}
				return size;
			}

			float initWedgie(float* index, Vertex vp, Vertex vl, Vertex vr) {
				
				
				float* leftChild = index + (index - &wedgieTree[0] + 1) * 2;
				if (leftChild - &wedgieTree[0] >= wedgieTree.size()) {
					
					float y = sampleVertex(vl, vr).position.y;
					float yt = (vl.position.y + vr.position.y) / 2.0;

					*index = std::abs(y - yt);
					return std::abs(y - yt);
				}

				float* rightChild = index + (index - &wedgieTree[0]+1) * 2 + 1;

				Vertex vc = sampleVertex(vl, vr);
				float leftError = initWedgie(leftChild, vc, vp, vl);
				float rightError = initWedgie(rightChild, vc, vr, vp);

				float et = std::max(leftError, rightError);
				float y = vc.position.y;
				float yt = (vl.position.y + vr.position.y) / 2.0;

				*index = et + std::abs(y - yt);
				return et + std::abs(y - yt);


			}

			Allocator(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, unsigned char* img, uint32 height, uint32 width, 
				glm::vec2 pixelSize, float maxHeight) : 
				vertices(vertices), indices(indices), img(img), height(height), width(width), pixelSize(pixelSize), maxHeight(maxHeight) {



				float va1x = -(width / 2 * pixelSize.x);
				float va1z = -(height / 2 * pixelSize.y);
				float va1y = img[0] / 255.0 * maxHeight;

				float v11x = va1x;
				float v11z = -va1z;
				float v11y = img[((height - 1)*width) * 3] / 255.0 * maxHeight;

				float v21x = -va1x;
				float v21z = va1z;
				float v21y = img[(width - 1) * 3] / 255.0 * maxHeight;

				float va2x = -va1x;
				float va2z = -va1z;
				float va2y = img[(width - 1 + (height - 1)*width) * 3] / 255.0 * maxHeight;

				
				unsigned int size = wedgieTreeSize();
				wedgieTree.resize(size);
				initWedgie(&wedgieTree[0], Vertex(va1x, va1y, va1z), Vertex(v21x, v21y, v21z), Vertex(v11x, v11y, v11z));
				initWedgie(&wedgieTree[1], Vertex(va2x, va2y, va2z), Vertex(v11x, v11y, v11z), Vertex(v21x, v21y, v21z));


			}

			void deleteTri(uint32 index) {	
				unsigned int length = indices.size();
				std::swap(indices[length - 6], indices[index * 6]);
				std::swap(indices[length - 5], indices[index * 6+1]);
				std::swap(indices[length - 4], indices[index * 6+2]);
				std::swap(indices[length - 3], indices[index * 6+3]);
				std::swap(indices[length - 2], indices[index * 6+4]);
				std::swap(indices[length - 1], indices[index * 6+5]);
				indices.pop_back();
				indices.pop_back();
				indices.pop_back();
				indices.pop_back();
				indices.pop_back();
				indices.pop_back();
				

				unsigned int lastNodeIndex = (length / 6) - 1;
				binNodes[lastNodeIndex]->index = index;
				std::swap(binNodes[lastNodeIndex], binNodes[index]);
				
				binNodes.pop_back();
				
			}

			uint32 allocateTri(BinNode* binNode, uint32 va, uint32 v1, uint32 v2) {
				binNode->index = binNodes.size();
				binNodes.push_back(binNode);
					
				indices.push_back(va);
				indices.push_back(v1);
				indices.push_back(v1);
				indices.push_back(v2);
				indices.push_back(v2);
				indices.push_back(va);


				return binNode->index;
			}

			void deleteVertex(uint32 va) {

			}

			uint32 allocateVertex(uint32 a, uint32 b) {
				//a b = index of vertices
				Vertex A = vertices[a];
				Vertex B = vertices[b];
				
				glm::vec3 midPosition = (A.position + B.position) / 2.0f;

				float startx = -(width / 2 * pixelSize.x);
				float starty = -(height / 2 * pixelSize.y);
				int midX = (midPosition[0] - startx) / pixelSize.x;
				int midZ = (midPosition[2] - starty) / pixelSize.y;
				float midY = img[(midX + width* midZ)*3] / 255.0 * maxHeight;


				vertices.push_back(Vertex(midPosition.x, midY, midPosition.z));
				return vertices.size() - 1;
			}

			glm::uvec3 getTri(uint32 index) {
				return glm::uvec3(indices[index * 6], indices[index * 6 + 1], indices[index * 6 + 3]);
			}

	
		} allocator;


		BinNode root1;
		BinNode root2;



	public:
		RoamHeightMap(unsigned char* img, int width, int height, glm::vec2 pixelSize, float maxHeight = 1) : allocator(vertices, indices, 
			img, height, width, pixelSize, maxHeight) {
		
			root1.coarsity = 0;
			root2.coarsity = 1;
			float va1x = -(width / 2 * pixelSize.x);
			float va1z = -(height / 2 * pixelSize.y);
			float va1y = img[0] / 255.0 * maxHeight;

			float v11x = va1x;
			float v11z = -va1z;
			float v11y = img[((height-1)*width)*3] / 255.0 * maxHeight;

			float v21x = -va1x;
			float v21z = va1z;
			float v21y = img[(width-1)*3] / 255.0 * maxHeight;

			float va2x = -va1x;
			float va2z = -va1z;
			float va2y = img[(width-1 + (height-1)*width)*3] / 255.0 * maxHeight;

			vertices.push_back(Vertex(va1x, va1y, va1z));
			vertices.push_back(Vertex(v11x, v11y, v11z));
			vertices.push_back(Vertex(v21x, v21y, v21z));
			vertices.push_back(Vertex(va2x, va2y, va2z));

			allocator.allocateTri(&root1, 0, 2, 1);
			allocator.allocateTri(&root2, 3, 1, 2);

			
			//split(&root1);
			//split(&root2);
			//root1.baseNeighbor = &root2;
			//root2.baseNeighbor = &root1;
			//std::priority_queue<PriorityBN, std::vector<PriorityBN>, compPriority> queueRoot1, queueRoot2;
			//buildSplitQueue(&root1, queueRoot1);
			//buildSplitQueue(&root2, queueRoot2);
			//recursiveSplit(&root1, 0);



		}
		
		void deleteBinNodeChild(BinNode* node) {
			//must make sure node has 0 or 1 children.

			glm::uvec3 leftChildInd = allocator.getTri(node->child->index);
			glm::uvec3 rightChildInd = allocator.getTri((node->child+1)->index);
			allocator.deleteTri(node->child->index);
			allocator.deleteTri((node->child+1)->index);

			allocator.allocateTri(node, rightChildInd[1], rightChildInd[2], leftChildInd[1]);

			delete[] node->child;
			if (node->baseNeighbor) {
				leftChildInd = allocator.getTri(node->baseNeighbor->child->index);
				rightChildInd = allocator.getTri((node->baseNeighbor->child + 1)->index);
				allocator.deleteTri(node->baseNeighbor->child->index);
				allocator.deleteTri((node->baseNeighbor->child + 1)->index);

				allocator.allocateTri(node->baseNeighbor, rightChildInd[1], rightChildInd[2], leftChildInd[1]);

				delete[] node->baseNeighbor->child;
			}
			

		}

		void allocateChildrenTri(BinNode* node, uint32 midIndex) {
			glm::uvec3 triIndices = allocator.getTri(node->index);
			node->child = new BinNode[2];
			node->child->coarsity = (node->coarsity + 1 )*2;
			node->child[1].coarsity = (node->coarsity + 1)*2 + 1;
			allocator.allocateTri(node->getLeft(), midIndex, triIndices[0], triIndices[1]);
			allocator.allocateTri(node->getRight(), midIndex, triIndices[2], triIndices[0]);
			allocator.deleteTri(node->index);
		}
		
		void allocateChildrenDiamond(BinNode* node) {
			glm::uvec3 triIndices = allocator.getTri(node->index);
			uint32 midIndex = allocator.allocateVertex(triIndices[1], triIndices[2]);
			allocateChildrenTri(node, midIndex);
			allocateChildrenTri(node->baseNeighbor, midIndex);
		}

		void updateNeighbor(BinNode* node) {
			BinNode* left = node->leftNeighbor;
			if (left != NULL) {
				if (left->leftNeighbor == node) {
					left->leftNeighbor = node->child;
				}
				else if (left->rightNeighbor == node) {
					left->rightNeighbor = node->child;
				}
				else left->baseNeighbor = node->child;
			}
			node->child->baseNeighbor = left;


			BinNode* right = node->rightNeighbor;
			if (right != NULL) {
				if (right->leftNeighbor == node) {
					right->leftNeighbor = node->child + 1;
				}
				else if (right->rightNeighbor == node) {
					right->rightNeighbor = node->child + 1;
				}
				else right->baseNeighbor = node->child + 1;
			}
			(node->child+1)->baseNeighbor = right;

			node->child->leftNeighbor = (node->child + 1);
			(node->child + 1)->rightNeighbor = node->child;


		}

		void splitDiamond(BinNode* node) {
			allocateChildrenDiamond(node);
			BinNode* base = node->baseNeighbor;
			node->child->rightNeighbor = (base->child + 1);
			(node->child + 1)->leftNeighbor = base->child;
			base->child->rightNeighbor = node->child + 1;
			(base->child + 1)->leftNeighbor = node->child;
			updateNeighbor(node);
			updateNeighbor(base);
		}

		void splitTri(BinNode* node) {
			glm::uvec3 triIndices = allocator.getTri(node->index);
			uint32 midIndex = allocator.allocateVertex(triIndices[1], triIndices[2]);
			allocateChildrenTri(node, midIndex);
			updateNeighbor(node);
		}

		void forceSplit(BinNode* node, std::priority_queue<PriorityBN, std::vector<PriorityBN>, compPriority>& splitQueue, BinNode* origin) {
			BinNode* base = node->baseNeighbor;
			if (base == NULL) {
				splitTri(node);
			}
			else if (base->baseNeighbor == node) {
				splitDiamond(node);
				splitQueue.push(PriorityBN(base->child, computeError(base->child)));
				splitQueue.push(PriorityBN(base->child + 1, computeError(base->child + 1)));
			}
			else {
				forceSplit(base, splitQueue, node);
				splitDiamond(node);
				
				splitQueue.push(PriorityBN(base->child, computeError(base->child)));
				splitQueue.push(PriorityBN(base->child + 1, computeError(base->child + 1)));
			}
			if (origin->baseNeighbor == node->child) {
				splitQueue.push(PriorityBN(node->child + 1, computeError(node->child + 1)));
			}
			else splitQueue.push(PriorityBN(node->child, computeError(node->child)));


		}

		void recursiveSplit(BinNode* node, int depth) {
			std::priority_queue<PriorityBN, std::vector<PriorityBN>, compPriority> splitQueue;
			if (depth < 4) {
				if (!node->child) {
					split(node, splitQueue);
				}
				recursiveSplit(node->child, depth + 1);
				recursiveSplit(node->child+1, depth + 1);
			}
		}

		void split(BinNode* node, std::priority_queue<PriorityBN, std::vector<PriorityBN>, compPriority>& splitQueue) {
			BinNode* base = node->baseNeighbor;
			if (base == NULL) {
				splitTri(node);
			}
			else if (base->baseNeighbor == node) {
				splitDiamond(node);
				splitQueue.push(PriorityBN(base->child, computeError(base->child)));
				splitQueue.push(PriorityBN(base->child + 1, computeError(base->child + 1)));
			}
			else {
				forceSplit(base, splitQueue, node);
				splitDiamond(node);
				splitQueue.push(PriorityBN(base->child, computeError(base->child)));
				splitQueue.push(PriorityBN(base->child + 1, computeError(base->child + 1)));
			}
			splitQueue.push(PriorityBN(node->child, computeError(node->child)));
			splitQueue.push(PriorityBN(node->child+1, computeError(node->child+1)));
		}


		std::pair<double, glm::uvec3> computeWedgie(BinNode* node) {
			if (!node->child) {
				if (node->coarsity < LMAX) {
					
				}
				else return std::make_pair(0, allocator.getTri(node->index));
			}
			else {
				std::pair<double, glm::uvec3> leftTri = computeWedgie(node->child);
				std::pair<double, glm::uvec3> rightTri = computeWedgie(node->child+1);
				double et = std::max(leftTri.first, rightTri.first);
				int vc = leftTri.second[0];
				int vl = leftTri.second[2];
				int vr = rightTri.second[1];
				double zt = (vertices[vl].position.z + vertices[vr].position.z) / 2;
				double z = vertices[vc].position.z;
				double error = et + std::abs(z - zt);
				return std::make_pair(error, glm::uvec3(leftTri.second[1], leftTri.second[2], rightTri.second[1]));
			}

		}

		//compute error for a tri. Get its priority
		double computeError(BinNode* node) {
			if (node->child) {
				return (computeError(node->child) + computeError(node->child+1))/2;
			}
			else {
				return 1.0 - std::log2(node->coarsity)*0.1;
			}
		}

		float computeGeoDistortion(BinNode* node, glm::mat4 lookAt) {
			//only for leave node;
			float et = 0;
			if (node->coarsity < allocator.wedgieTree.size()) {
				et = allocator.wedgieTree[node->coarsity];
			}

			//look at matrix
			glm::vec4 abc = lookAt * glm::vec4(0, 0, et, 1);
			glm::vec3 indices = allocator.getTri(node->index);

			float distMax = 0;
			for (int i = 0; i < 3; i++) {
				Vertex v = vertices[indices[i]];
				glm::vec4 pqr = lookAt * glm::vec4(v.position, 1);
				float rc = pqr[2]*pqr[2] - abc[2]* abc[2];
				float temp1 = abc[0] * pqr[2] - abc[2] * pqr[0];
				float temp2 = abc[1] * pqr[2] - abc[2] * pqr[1];
				float sqr = temp1*temp1 + temp2*temp2;
				float dist = 2*glm::sqrt(sqr) / rc;
				if (dist > distMax) distMax = dist;
			}
			return distMax;
		}

		void fillSplitQueue(BinNode* node, std::priority_queue<PriorityBN, std::vector<PriorityBN>, compPriority>& splitQueue) {
			if (!node->child) {
				splitQueue.push(PriorityBN(node, computeError(node)));
			}
			else {
				fillSplitQueue(node->child, splitQueue);
				fillSplitQueue(node->child + 1, splitQueue);
			}
		}

		void buildSplitQueue(BinNode* root, std::priority_queue<PriorityBN, std::vector<PriorityBN>, compPriority>& splitQueue) {
			fillSplitQueue(root, splitQueue);
			while (computeError(root) > 0.01) {
				PriorityBN t = splitQueue.top();
				while (t.binnode->child) {
					splitQueue.pop();
					t = splitQueue.top();
				}
				split(t.binnode, splitQueue);
			}

		}

		//use geodistortion priority;
		void fillSplitQueue(BinNode* node, std::priority_queue<PriorityBN, std::vector<PriorityBN>, compPriority>& splitQueue, glm::mat4 lookAt) {
			if (!node->child) {
				splitQueue.push(PriorityBN(node, computeGeoDistortion(node, lookAt)));
			}
			else {
				fillSplitQueue(node->child, splitQueue);
				fillSplitQueue(node->child + 1, splitQueue);
			}
		}

		void computeFrame(glm::mat4 lookAt) {
			std::priority_queue<PriorityBN, std::vector<PriorityBN>, compPriority> queue;
			fillSplitQueue(&root1, queue, lookAt);

			while (!queue.empty() && (queue.top().priority > 0.2)) {
				//tessellate;
				PriorityBN t = queue.top();
				queue.pop();
				if (t.binnode->child) {
				}
				else {
					split(t.binnode, queue);
				}
			}

			std::priority_queue<PriorityBN, std::vector<PriorityBN>, compPriority> queue2;
			fillSplitQueue(&root2, queue2, lookAt);
			while (!queue2.empty() && (queue2.top().priority > 0.2)) {
				//tessellate;
				PriorityBN t = queue2.top();
				queue2.pop();
				if (t.binnode->child) {
				}
				else {
					split(t.binnode, queue2);
				}
			}
				
		}


		//tessellation level
		//merge

	};

	class HeightMapMesh : public Mesh{


	public:
		HeightMapMesh(unsigned char* img, int width, int height, glm::vec2 pixelSize, float maxHeight = 1) {
			
			float startx = -width / 2 * pixelSize.x;
			float starty = -height / 2 * pixelSize.y;
			for (int y = 0; y < height; ++y) {
				float positionY = starty + y*pixelSize.y;
				for (int x = 0; x < width; ++x) {
					float positionX = startx + x*pixelSize.x;
					float heightValue = float(img[(x + width* y)*3]) / 255.0 * maxHeight;
					addVertices(Vertex(positionX, heightValue, positionY));
				}
			}

			for (int y = 0; y < height - 1; y++) {
				for (int x = 0; x < width - 1; x++) {
					int currentX = x + y*width;
					addTriangle(currentX, currentX + 1, currentX + width + 1);
					addTriangle(currentX, currentX + width + 1, currentX + width);
				}
			}

			std::vector<float> triNum(vertices.size(), 0);
			for (int i = 0; i < indices.size(); i = i + 3) {
				Vertex& a = vertices[indices[i]];
				Vertex& b = vertices[indices[i + 1]];
				Vertex& c = vertices[indices[i + 2]];
				triNum[indices[i]] += 1;
				triNum[indices[i+1]] += 1;
				triNum[indices[i+2]] += 1;

				glm::vec3 normal = glm::normalize(glm::cross((c.position - b.position), (a.position - b.position)));
				a.normal += normal;
				b.normal += normal;
				c.normal += normal;
			}

			for (int i = 0; i < vertices.size(); i++) {
				Vertex& a = vertices[i];
				a.normal /= triNum[i];
				a.normal = -a.normal;
			}

			
			
			
		}
	
	};

	class Image {
		unsigned char* img;
	public:
		Image() {

		}
		unsigned char* loadImage(char* filename) {
			
			std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);

			if (file.is_open()) {
				std::streampos size = file.tellg();
				img = new unsigned char[size];

				file.seekg(0, std::ios::beg);
				file.read((char*)img, size);
				file.close();

			}
			else {
				throw std::runtime_error("load image error");
			}

			
			return img;
		}
	};
}