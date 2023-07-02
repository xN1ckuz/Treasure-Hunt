#pragma once

#include <iostream>
#include<fstream>
#include<vector>
#include "DrawableObj.h"
#include "kdtree.h"
#include "Punto3D.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace kd;

class DrawableObjIstanced: public DrawableObj {

	public:
		DrawableObjIstanced(string dirFilePosizioni, string modelDirectory) : DrawableObj (modelDirectory){
			setModel(modelDirectory);
			caricaPosizioniEOrientamento(dirFilePosizioni);
			creaBufferMatrix();
		}

		void Draw() {
			shader->use();
			for (int i = 0; i < model->meshes.size(); i++) {
				// bind appropriate textures
				unsigned int diffuseNr = 1;
				unsigned int specularNr = 1;
				unsigned int normalNr = 1;
				unsigned int heightNr = 1;
				vector<Texture> textures = model->meshes[i].textures;
				for (unsigned int j = 0; j < textures.size(); j++)
				{
					glActiveTexture(GL_TEXTURE0 + j); // active proper texture unit before binding
					// retrieve texture number (the N in diffuse_textureN)
					string number;
					string name = textures[j].type;
					if (name == "texture_diffuse")
						number = std::to_string(diffuseNr++);
					else if (name == "texture_specular")
						number = std::to_string(specularNr++); // transfer unsigned int to stream
					else if (name == "texture_normal")
						number = std::to_string(normalNr++); // transfer unsigned int to stream
					else if (name == "texture_height")
						number = std::to_string(heightNr++); // transfer unsigned int to stream

					// now set the sampler to the correct texture unit
					glUniform1i(glGetUniformLocation(shader->ID, (name + number).c_str()), j);
					// and finally bind the texture
					glBindTexture(GL_TEXTURE_2D, textures[j].id);
				}

				// draw mesh
				glBindVertexArray(model->meshes[i].VAO);
				glDrawElementsInstanced(GL_TRIANGLES, model->meshes[i].indices.size(), GL_UNSIGNED_INT, 0, posizioni.size());
				glBindVertexArray(0);

				// always good practice to set everything back to defaults once configured.
				glActiveTexture(GL_TEXTURE0);
			}
			
		}

		void aggiornaPosPerCollisione(glm::vec3* pos, glm::vec3 posVecchia, float raggio) {
			glm::vec3 puntoVicino(0.0);
			for (int i = 0; i < posizioni.size(); i++) {
				puntoVicino = posizioni[i];
				float distanza = sqrt(pow(pos->x - puntoVicino.x, 2) + pow(pos->y - puntoVicino.y, 2) + pow(pos->z - puntoVicino.z, 2));
				if (distanza <= raggio) {
					*pos = posVecchia;
					return;
				}
			}
		}

	protected:
		vector<glm::vec3> posizioni;
		vector<glm::vec3> rotazioni;

		void caricaPosizioniEOrientamento(string dirFilePosizioni) {

			//Apro il flusso da file
			ifstream fin(dirFilePosizioni);

			string var;
			while (true)
			{
				fin >> var;
				//cout << var << endl;
				if (var[0] == 'F') {
					break;
				}
				//fin.ignore(10, ' ');
				float x = 0, y = 0, z = 0, alpha = 0, beta = 0, gamma = 0;
				fin >> x;
				//cout << "x: " << x << endl;
				fin.ignore(2, ' ');
				fin >> y;
				//cout << "y: " << y << endl;
				fin.ignore(2, ' ');
				fin >> z;
				//cout << "z: " << z << endl;
				fin.ignore(2, '\n');
				fin >> alpha;
				//cout << "alpha: " << alpha << endl;
				fin.ignore(3, ' ');
				fin >> beta;
				//cout << "beta: " << beta << endl;
				fin.ignore(3, ' ');
				fin >> gamma;
				//cout << "gamma: " << gamma << endl;
				fin.ignore(2, '\n');

				posizioni.push_back(glm::vec3(x, z, -y));
				//posizioni.push_back(glm::vec3(x, z, y));
				rotazioni.push_back(glm::vec3(alpha, beta, gamma));
			}
			fin.close();
		}

		void creaBufferMatrix() {
			unsigned int amount = posizioni.size();
			glm::mat4* modelMatrices;
			modelMatrices = new glm::mat4[amount];
			for (unsigned int i = 0; i < amount; i++) {
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, posizioni[i]);
				model = glm::rotate(model, glm::radians(rotazioni[i].y), glm::vec3(0.0f, 0.0f, -1.0f));
				model = glm::rotate(model, glm::radians(rotazioni[i].z), glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, glm::radians(rotazioni[i].x), glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::translate(model, glm::vec3(0, 0, 0));
				model = glm::scale(model, glm::vec3(1.5, 1.5, 1.5));
				modelMatrices[i] = model;
			}

			// vertex buffer object
			unsigned int buffer;
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);


			for (unsigned int i = 0; i < model->meshes.size(); i++)
			{
				unsigned int VAO = model->meshes[i].VAO;
				glBindVertexArray(VAO);
				// set attribute pointers for matrix (4 times vec4)
				glEnableVertexAttribArray(3);
				glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
				glEnableVertexAttribArray(4);
				glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
				glEnableVertexAttribArray(5);
				glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
				glEnableVertexAttribArray(6);
				glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

				glVertexAttribDivisor(3, 1);
				glVertexAttribDivisor(4, 1);
				glVertexAttribDivisor(5, 1);
				glVertexAttribDivisor(6, 1);

				glBindVertexArray(0);
			}
		}

	private:

		unsigned int instanceVBO;
			
		
};
