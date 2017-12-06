#pragma once

#include "OpenGL.h"
#include "IDrawable.h"
#include "Color.h"
#include <vector>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
using namespace Crawfis::Graphics;
using namespace Crawfis::Math;
using namespace std;

namespace Crawfis
{
	namespace Graphics
	{
		class DrawableShadowMapping : public IDrawable
		{
		public:
			DrawableShadowMapping()
			{

			}
			virtual void Draw()
			{
				if (!created)
					CreateOBJ();
				int error = glGetError();
				glBindVertexArray(vaoObject);
				if (!wired) {
					glDrawArrays(GL_TRIANGLES, 0, vertices.size());
				}
				else
				{
					for (int i = 0; i < vertices.size(); i += 3) {
						glDrawArrays(GL_LINE_LOOP, i, 3);
					}
				}
				error = glGetError();
			}
		private:
			bool LoadOBJ(const char* path) {
				std::vector< unsigned int > vertexIndices, uvIndices;
				std::vector< vec3<float> > temp_vertices;
				std::vector< vec2<float> > temp_uvs;
				FILE * file = fopen("..\\Media\\Objects\\blub_triangulated.obj", "r");
				if (file == NULL) {
					printf("Cannot open OBJ file.\n");
					return false;
				}

				while (1) {

					char lineHeader[128];
					// read the first word of the line
					int res = fscanf(file, "%s", lineHeader);
					if (res == EOF)
						break;

					if (strcmp(lineHeader, "v") == 0) {
						vec3<float> vertex;
						fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
						temp_vertices.push_back(vertex);

					}
					else if (strcmp(lineHeader, "vt") == 0) {
						vec2<float> uv;
						fscanf(file, "%f %f\n", &uv.x, &uv.y);
						temp_uvs.push_back(uv);
					}
					else if (strcmp(lineHeader, "f") == 0) {
						std::string vertex1, vertex2, vertex3;
						unsigned int vertexIndex[3], uvIndex[3];
						int matches = fscanf(file, "%d/%d %d/%d %d/%d\n", &vertexIndex[0], &uvIndex[0], &vertexIndex[1], &uvIndex[1], &vertexIndex[2], &uvIndex[2]);
						if (matches != 6) {
							printf("File can't be read by our simple parser : ( Try exporting with other options\n");
							return false;
						}
						vertexIndices.push_back(vertexIndex[0]);
						vertexIndices.push_back(vertexIndex[1]);
						vertexIndices.push_back(vertexIndex[2]);
						uvIndices.push_back(uvIndex[0]);
						uvIndices.push_back(uvIndex[1]);
						uvIndices.push_back(uvIndex[2]);
					}
				}

				for (unsigned int i = 0; i < vertexIndices.size(); i++) {
					unsigned int vertexIndex = vertexIndices[i];
					vec3<float> vertex = temp_vertices[vertexIndex - 1];
					vertices01.push_back(vertex);
				}

				file = fopen("..\\Media\\Objects\\spot_triangulated.obj", "r");
				if (file == NULL) {
					printf("Cannot open OBJ file.\n");
					return false;
				}

				while (1) {

					char lineHeader[128];
					// read the first word of the line
					int res = fscanf(file, "%s", lineHeader);
					if (res == EOF)
						break;

					if (strcmp(lineHeader, "v") == 0) {
						vec3<float> vertex;
						fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
						temp_vertices.push_back(vertex);

					}
					else if (strcmp(lineHeader, "vt") == 0) {
						vec2<float> uv;
						fscanf(file, "%f %f\n", &uv.x, &uv.y);
						temp_uvs.push_back(uv);
					}
					else if (strcmp(lineHeader, "f") == 0) {
						std::string vertex1, vertex2, vertex3;
						unsigned int vertexIndex[3], uvIndex[3];
						int matches = fscanf(file, "%d/%d %d/%d %d/%d\n", &vertexIndex[0], &uvIndex[0], &vertexIndex[1], &uvIndex[1], &vertexIndex[2], &uvIndex[2]);
						if (matches != 6) {
							printf("File can't be read by our simple parser : ( Try exporting with other options\n");
							return false;
						}
						vertexIndices.push_back(vertexIndex[0]);
						vertexIndices.push_back(vertexIndex[1]);
						vertexIndices.push_back(vertexIndex[2]);
						uvIndices.push_back(uvIndex[0]);
						uvIndices.push_back(uvIndex[1]);
						uvIndices.push_back(uvIndex[2]);
					}
				}

				for (unsigned int i = 0; i < vertexIndices.size(); i++) {
					unsigned int vertexIndex = vertexIndices[i];
					vec3<float> vertex = temp_vertices[vertexIndex - 1];
					vertices02.push_back(vertex);
				}

				stripSize = 2 * numHorizontalSamples;
				//vertices = new float*[numVerticalSamples-1];
				int index = 0;
				float y = 0.0f;
				float x = -1;
				float z = -1;
				float deltaX = 2.0f / (float)(numHorizontalSamples - 1);
				float deltaZ = 2.0f / (float)(numVerticalSamples - 1);
				float texOffset = 1;
				float texScale = 0.5f;
				for (int j = 0; j < (numVerticalSamples - 1); j++)
				{
					x = -1;
					//vertices[j] = new float[3*stripSize];
					index = 0;
					for (int i = 0; i < numHorizontalSamples; i++)
					{
						vec3<float> temp = vec3<float>(x,y,z);
						vertices03.push_back(temp);

						temp = vec3<float>(x, y, z + deltaZ);
						vertices03.push_back(temp);

						x += deltaX;
					}
					z += deltaZ;
				}

				vertices.insert(vertices.end(),vertices01.begin(),vertices01.end());
				vertices.insert(vertices.end(), vertices02.begin(), vertices02.end());
				vertices.insert(vertices.end(), vertices03.begin(), vertices03.end());

			}

			void CreateOBJ()
			{

				glGenFramebuffers(1, &FramebufferName);
				glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

				// Depth texture. Slower than a depth buffer, but you can sample it later in your shader

				glGenTextures(1, &depthTexture);
				glBindTexture(GL_TEXTURE_2D, depthTexture);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);

				glDrawBuffer(GL_NONE); // No color buffer is drawn to.

									   // Always check that our framebuffer is ok
				if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
					return;


				created = true;
			}
		private:
			bool created = false;
			bool wired = false;
			int numHorizontalSamples;
			int numVerticalSamples;
			int stripSize;
			GLuint depthTexture;
			unsigned int vaoObject, vboVertices, vboUV, vboIndices, vboNorm;
			std::vector< vec3<float> > vertices;
			std::vector< vec3<float> > vertices01;
			std::vector< vec3<float> > vertices02;
			std::vector< vec3<float> > vertices03;
			GLuint FramebufferName = 0;
		};
	}
}