#pragma once
#pragma once

#include "OpenGL.h"
#include "IDrawable.h"
#include <vector>

namespace Crawfis
{
	namespace Graphics
	{
		//
		// A concrete drawable representing a simple flat mesh.
		// The orientation is in the xz plane.
		//
		class DrawableBackground : public IDrawable
		{
		public:
			//
			// Constructor.
			//
			DrawableBackground()
			{

				created = false;
			}
			//
			// Draw the floor.
			//
			virtual void Draw()
			{
				InternalDraw();
			}
		private:
			void CreateBackground()
			{

				CreateOpenGLBuffers();
				created = true;
			}

			void CreateOpenGLBuffers()
			{
				glGenVertexArrays(1, &VAO);
				glGenBuffers(1, &VBO);
				glGenBuffers(1, &EBO);

				glBindVertexArray(VAO);

				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

				// position attribute
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);
				// color attribute
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
				glEnableVertexAttribArray(1);
				// texture coord attribute
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
				glEnableVertexAttribArray(2);


			}
			void InternalDraw()
			{
				if (!created)
				{
					CreateBackground();
				}
				//
				// Actually draw the background
				//
				glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);


				glBindVertexArray(VAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			}
		private:
			int numHorizontalSamples;
			int numVerticalSamples;
			float vertices[36] = {
				// positions          // colors           // texture coords
				0.0f,  0.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
				0.0f, -0.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
				-0.0f, -0.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
				-0.0f,  0.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
			};
			/*
			
			float vertices[36] = {
				// positions          // colors           // texture coords
				1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
				1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
				-1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
				-1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
			};
			*/
			unsigned int indices[6] = {
				0, 1, 3, // first triangle
				1, 2, 3  // second triangle
			};
			bool created;

			unsigned int VBO, VAO, EBO;
		};
	}
}