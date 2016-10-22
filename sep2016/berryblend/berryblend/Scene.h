#pragma once

#include "Mesh.h"
namespace Berryblend {
	typedef unsigned int uint32;
	class Node {
		GLuint vao;
		glm::mat4 model;
		uint32 count;
	public:
		Node(const GLuint& vao, const glm::mat4& model, const uint32& count) : vao(vao), model(model), count(count) {}

		GLuint getVao() { return vao; }
		glm::mat4 getModel() { return model; }
		uint32 getCount() { return count; }

	};

	class Light {
		enum Type {
			LT_SPOTLIGHT,
			LT_DIRLIGHT,
			LT_POINTLIGHT
		};

		glm::vec3 color;
		glm::vec3 atten;
		glm::vec3 value1; //dir / pos 
		glm::vec3 value2;
		Type type;

	public:
		static inline Light makeSpotlight(glm::vec3 pos, glm::vec3 dir, glm::vec3 color, glm::vec3 atten) {
			Light l;
			l.type = LT_SPOTLIGHT;
			l.atten = atten;
			l.color = color;
			l.value1 = pos;
			l.value2 = dir;
			return l;
		};
		static inline Light makeDirlight(glm::vec3 dir, glm::vec3 color) {
			Light l;
			l.type = LT_DIRLIGHT;
			l.color = color;
			l.value1 = dir;
			return l;

		};
		static inline Light makePointlight(glm::vec3 pos, glm::vec3 color, glm::vec3 atten) {
			Light l;
			l.type = LT_POINTLIGHT;
			l.color = color;
			l.value1 = pos;
			l.atten = atten;
			return l;
		};




	};

	class Material {
		std::vector<Node> nodes;
	public:
		glm::vec3 color = glm::vec3(1);
		glm::vec3 ambient = glm::vec3(0.3);
		uint32 glossiness = 5;
		float specularStrength = 1;
		float diffuseStrength = 1;
		GLuint colorTexture = 0;
		GLuint normalMap = 0;

		void addNode(Node node) {
			nodes.push_back(node);
		}

		const std::vector<Node>& getNodes() {  //use const reference  no copy..
			return nodes;
		}
	
	};

	class Scene {
		std::vector<Material> materials;
		std::vector<Light> lights;
	public:
		const std::vector<Material>& getMaterials() {
			return materials;
		}

		const std::vector<Light>& getLights() {
			return lights;
		}

		void addMaterial(Material mat) {
			materials.push_back(mat);
		}

		void addLight(Light light) {
			lights.push_back(light);
		}
	};
}