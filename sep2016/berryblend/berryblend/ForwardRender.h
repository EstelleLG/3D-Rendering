#pragma once

#include "berryblend.h"
#include "Mesh.h"
#include "perlin.h"
#include "Scene.h"
namespace Berryblend {


	class ForwardRender {
		Scene* scene;
		GLProgram program;
		GLVertexArray vertexarray;
		GLVertexArray planeVertexArray;
		GLVertexArray australiaVAO;
		
		GLProgram shadowProgram;

		int mvpLoc;
		int lightColorLoc;
		int colorLoc;
		int shininessLoc;
		int lightDirLoc;
		int modelLoc;
		int camPosLoc;
		
		int shadowMVPLoc;
		int shadowBiasedMVPLoc;
		int shadowMapLoc;

		HeightMapMesh* heightMapMesh;

		Light dirLight;
		GLTexture2D shadowMap;
		GLuint frameBuffer;
		GLuint renderBuffer;
		glm::mat4 lightVP;
		glm::mat4 textureLightVP;


	public:
		ForwardRender() {
			scene = new Scene();
			GLShader shadowFrag = GLShader(GL_FRAGMENT_SHADER);
			GLShader shadowVert = GLShader(GL_VERTEX_SHADER);

			if (!shadowFrag.loadAndCompile("../rsc/shader/shadowFrag.glsl")) {
				std::string err = shadowFrag.getErrorString();
				printf(err.c_str());
			}
			if (!shadowVert.loadAndCompile("../rsc/shader/shadowVert.glsl")) {
				std::string err = shadowVert.getErrorString();
				printf(err.c_str());
			}

			shadowProgram.attachShader(shadowFrag);
			shadowProgram.attachShader(shadowVert);
			if (!shadowProgram.linkProgram()) {
				std::string error = shadowProgram.getErrorString();
				printf(error.c_str());
			}


			GLShader frag = GLShader(GL_FRAGMENT_SHADER);
			GLShader vertex = GLShader(GL_VERTEX_SHADER);
	
			if (!frag.loadAndCompile("../rsc/shader/blinnFrag.glsl")) {
				std::string err = frag.getErrorString();
				printf(err.c_str());
			}
			if (!vertex.loadAndCompile("../rsc/shader/blinnVert.glsl")) {
				std::string err = vertex.getErrorString();
				printf(err.c_str());
			}

			program.attachShader(frag);
			program.attachShader(vertex);
			if (!program.linkProgram()) {
				std::string error = program.getErrorString();
				printf(error.c_str());
			}


			//cube
			Mesh mesh = MeshFactory().createBox(3, 3, 3);
			mesh.loadToVertexArray(vertexarray);
			//plane
			Mesh meshPlane = MeshFactory().createPlane(5, 8);
			meshPlane.loadToVertexArray(planeVertexArray);
			//australia height map!
			unsigned char* img = Image().loadImage("../rsc/supertexture.data");		
			//heightMapMesh = new HeightMapMesh(img, 512, 512, glm::vec2(0.02), 0.3);
			//heightMapMesh->loadToVertexArray(australiaVAO);

			//roam
			auto heightRoamMapMesh = new RoamHeightMap(img, 512, 512, glm::vec2(0.02), 1);
			heightRoamMapMesh->computeFrame(glm::lookAt<float>(glm::vec3(4, 3, 4), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));
			heightRoamMapMesh->loadToVertexArray(australiaVAO);

			//save them to a node
			Node cube(vertexarray.getGLId(), glm::translate(glm::vec3(0, 3, 0)), mesh.indicesSize());
			Node plane(planeVertexArray.getGLId(), glm::translate(glm::vec3(0, -3, 0))*glm::rotate<float>(-M_PI / 2, glm::vec3(1, 0, 0)), meshPlane.indicesSize());
			//Node australia(australiaVAO.getGLId(), glm::mat4(1), heightMapMesh->indicesSize());
			Node australia(australiaVAO.getGLId(), glm::mat4(1), heightRoamMapMesh->indicesSize());

			//add to material
			Material mat;
			mat.color = glm::vec3(0, 0, 1);
			Material australiaMat;
			australiaMat.color = glm::vec3(0, 1, 0);
			
			mat.addNode(cube);
			mat.addNode(plane);
			australiaMat.addNode(australia);

			scene->addMaterial(mat);
			scene->addMaterial(australiaMat);
			

			auto test = glGetError();
			if (test != GL_NONE) {
				printf("sss");
			}
			shadowMap.bindTexture();
			shadowMap.loadData(GL_DEPTH_COMPONENT32, 1024, 1024, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			shadowMap.setParameter(GL_LINEAR, GL_LINEAR);
			shadowMap.setWrapST(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
			shadowMap.setCompareMode(GL_COMPARE_REF_TO_TEXTURE);   
			shadowMap.setCompareFcn(GL_LEQUAL); //return 1 if <= value stored in the depth map
			
			test = glGetError();
			if (test != GL_NONE) {
				printf("sss");
			}

			glm::mat4 NDCtoTexture(glm::vec4(0.5, 0, 0, 0), glm::vec4(0, 0.5, 0, 0), glm::vec4(0, 0, 0.5, 0), glm::vec4(0.5, 0.5, 0.5, 1));
			
			dirLight = Light::makeDirlight(glm::vec3(1, -1, -1), glm::vec3(1));
			glm::mat4 orthoMatrix = ortho(-10, 10, -10, 10, -10, 30);
			glm::mat4 viewMatrix = glm::lookAt(glm::vec3(-4, 3, 4), glm::vec3(0), glm::vec3(0, 1, 0));
			lightVP = orthoMatrix * viewMatrix;
			textureLightVP = NDCtoTexture * lightVP;

			
			
			glGenFramebuffers(1, &frameBuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap.getGLId(), 0);
			
			glDepthMask(GL_TRUE);
			auto check = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (check != GL_FRAMEBUFFER_COMPLETE) {
				printf("framebuffer not complete");
				switch (check) {
				case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
					printf("aaaa");
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
					printf("aaaa");
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
					printf("aaaa");
					break;
				case GL_FRAMEBUFFER_UNSUPPORTED:
					printf("aaaa");
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
					printf("aaaa");
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
					printf("aaaa");
					break;

				default:
					printf("sss");
					break;
				};
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			
			shadowMVPLoc = shadowProgram.getUniformLocation("mvp");
			mvpLoc = program.getUniformLocation("mvp");
			modelLoc = program.getUniformLocation("model");
			camPosLoc = program.getUniformLocation("camPos");
			lightDirLoc = program.getUniformLocation("lightDir");
			colorLoc = program.getUniformLocation("color");
			lightColorLoc = program.getUniformLocation("lightColor");
			shininessLoc = program.getUniformLocation("shininess");
			shadowBiasedMVPLoc = program.getUniformLocation("shadowMVP");
			shadowMapLoc = program.getUniformLocation("shadowMap");
			}

		//viewspace/ view matrix
		glm::mat4 lookAt(glm::vec3 cameraPos, glm::vec3 target, glm::vec3 up) {
			glm::vec3 forward = glm::normalize(target - cameraPos);
			glm::vec3 right = glm::normalize(glm::cross(forward, up));
			glm::vec3 upOrth = glm::cross(forward, right);
			
			glm::mat4 rotationTranspose(glm::vec4(right.x, up.x, -forward.x, 0), glm::vec4(right.y, up.y, -forward.y, 0),
				glm::vec4(right.z, up.z, -forward.z, 0), glm::vec4(-glm::dot(right, cameraPos), -glm::dot(up, cameraPos), glm::dot(forward, cameraPos), 1));
			return rotationTranspose;
			
		}

		//-1 to 1  --> (NDC coordinate)
		glm::mat4 ortho(float minx, float maxx, float miny, float maxy, float minz, float maxz) {
			float overWidth = 2 / (maxx - minx);
			float overHeight = 2 / (maxy - miny);
			float overLength = 2 / (maxz - minz);

			glm::mat4 map(glm::vec4(overWidth, 0, 0, 0), glm::vec4(0, overHeight, 0, 0), glm::vec4(0, 0, -overLength, 0),
				-glm::vec4(1 + minx*overWidth, 1 + miny*overHeight, 1 + minz*overLength, -1));
			return map;
		}

		void render() {
			drawShadow();
			drawScene();

		}


		void drawShadow() {
			glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
			glViewport(0, 0, 1024, 1024);
			glClear(GL_DEPTH_BUFFER_BIT);
			shadowProgram.useProgram();

			for (auto mat : scene->getMaterials()) {
				for (Node node : mat.getNodes()) {
					glBindVertexArray(node.getVao());
					glUniformMatrix4fv(shadowMVPLoc, 1, GL_FALSE, glm::value_ptr(lightVP*node.getModel()));
					glDrawElements(GL_TRIANGLES, node.getCount(), GL_UNSIGNED_INT, 0);
				}
			}
			
		}

		void drawScene() {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			program.useProgram();

			glActiveTexture(GL_TEXTURE0);
			shadowMap.bindTexture();
			glUniform1i(shadowMapLoc, 0);   //gl_texture0
			glUniformMatrix4fv(shadowBiasedMVPLoc, 1, GL_FALSE, glm::value_ptr(textureLightVP));
			
			glUniform3fv(camPosLoc, 1, glm::value_ptr(glm::vec3(4, 3, 4)));
			glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::normalize(glm::vec3(1, -1, -1))));
			glUniform3fv(lightColorLoc, 1, glm::value_ptr(glm::vec3(1)));
			glm::mat4 vp = ortho(-10, 10, -10, 10, 0, 15) * glm::lookAt<float>(glm::vec3(4, 3, 4), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

			for (auto mat : scene->getMaterials()) {
				glUniform3fv(colorLoc, 1, glm::value_ptr(mat.color));
				glUniform1i(shininessLoc, mat.glossiness);


				for (Node node : mat.getNodes()) {

					glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(vp*node.getModel()));
					glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(node.getModel()));
					glBindVertexArray(node.getVao());
					glDrawElements(GL_LINES, node.getCount(), GL_UNSIGNED_INT, 0);
				}
			}

		}


	};


	class ForwardWindow : public GLWindow{
		ForwardRender* rdr;
	public:
		ForwardWindow(const System& system, std::wstring classname, std::wstring caption, int x, int y, int width, int height, uint32_t style)
			: GLWindow(system, classname, caption, x, y, width, height, style) {
			rdr = new ForwardRender();
		}

		virtual void Paint(Surface& surface) {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			//create uniform...
			glEnable(GL_DEPTH_TEST);
			rdr->render();
			
			surface.SwapBuffers();

		}

	};
}