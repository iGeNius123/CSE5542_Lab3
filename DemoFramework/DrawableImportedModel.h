#pragma once

#include "OpenGL.h"
#include "IDrawable.h"
#include "Color.h"
#include <vector>

using namespace Crawfis::Graphics;
using namespace Crawfis::Math;
using namespace std;

namespace Crawfis
{
	namespace Graphics
	{
		class DrawableImportedModel : public IDrawable
		{
		public:
			DrawableImportedModel(const char* path, bool wireframe)
			{
				bool load = LoadOBJ(path);
				if (!load) {
					printf("Cannot load OBJ file.");

				}
				wired = wireframe;
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
				FILE * file = fopen(path, "r");
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
					vertices.push_back(vertex);
				}

				for (unsigned int i = 0; i < uvIndices.size(); i++) {
					unsigned int uvIndex = uvIndices[i];
					vec2<float> uv = temp_uvs[uvIndex - 1];
					uvs.push_back(uv);
				}

				// Calculate surface normal
				for (int i = 0; i < vertices.size(); i += 3) {
					vec3<float> v1 = vertices[i];
					vec3<float> v2 = vertices[i + 1];
					vec3<float> v3 = vertices[i + 2];
					vec3<float> u = v1 - v2;
					vec3<float> v = v1 - v3;
					vec3<float> normal;
					normal.x = u.y*v.z - u.z*v.y;
					normal.y = -(u.x*v.z - v.x*u.z);
					normal.z = u.x*v.y - v.x*v.y;
					float length = normal.x*normal.x + normal.y*normal.y + normal.z*normal.z;
					length = sqrt(length);
					normal = normal / length;
					normals.push_back(normal);
					normals.push_back(normal);
					normals.push_back(normal);
				}


			}

			void CreateOBJ()
			{
				// Create a Vertex Array Object to organize all of the bindings.
				glGenVertexArrays(1, &vaoObject);
				glBindVertexArray(vaoObject);
				// Steps:
				//  1) Create buffer for vertex positions
				//  2) Set vertex positions to be sent to the shaders at slot 0.
				//  3) Create buffer for colors
				//  4) Set colors to be sent to the shaders at slot 1.
				//  5) Create buffer for indices of the cube.
				//
				// Vertex positions
				// Allocate Vertex Buffer Object (get a handle or ID)
				glGenBuffers(1, &vboVertices);
				// VBO for vertex data
				glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
				// Set the model data into the VBO.
				glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3<float>), &vertices[0], GL_STATIC_DRAW);
				// Define the layout of the vertex data.
				// This also set's the vertex array's location for slot 0.
				glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(0);

				// Uvcoords
				//// Allocate Vertex Buffer Object (get a handle or ID)
				glGenBuffers(1, &vboUV);
				glBindBuffer(GL_ARRAY_BUFFER, vboUV);
				//// Fill the buffer with the uvCoords.
				glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec2<float>), &uvs[0], GL_STATIC_DRAW);
				//// Define the layout of the vertex data.
				//// This also set's the color array's location for slot 1.
				glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(1);

				//Normals
				glGenBuffers(1, &vboNorm);
				glBindBuffer(GL_ARRAY_BUFFER, vboNorm);
				//// Fill the buffer with the uvCoords.
				glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3<float>), &normals[0], GL_STATIC_DRAW);
				//// Define the layout of the vertex data.
				//// This also set's the color array's location for slot 1.
				glVertexAttribPointer((GLuint)2, 3, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(2);
				glBindVertexArray(0);
				created = true;
			}
		private:
			bool created = false;
			bool wired = false;
			unsigned int vaoObject, vboVertices, vboUV, vboIndices, vboNorm;
			std::vector< vec3<float> > vertices;
			std::vector<vec3<float>> normals;
			std::vector<vec2<float> > uvs;
		};
	}
}