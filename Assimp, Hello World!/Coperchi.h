#pragma once

#include"DrawableObjIstanced.h"

using namespace std;

class Coperchi: public DrawableObjIstanced {
	public:

		Coperchi(string dirFilePosizioni, string modelDirectory) : DrawableObjIstanced(dirFilePosizioni, modelDirectory) {
			setModel(modelDirectory);
			caricaPosizioniEOrientamento(dirFilePosizioni);
			creaBufferMatrix();
		}

		Coperchi() : DrawableObjIstanced() {

		}

		glm::vec3 getPosizioneIndice(int i) {
			return posizioni[i];
		}

		int getCoperchioToOpen(glm::vec3 pos, float raggio) {
			for (int i = 0; i < posizioni.size(); i++) {
				float distanza = sqrt(pow(pos.x - posizioni[i].x, 2) + pow(pos.y - posizioni[i].y, 2) + pow(pos.z - posizioni[i].z, 2));
				if (distanza <= raggio) {
					if (coperchiAperti[i] == false) {
						return i;
					}
				}
			}
			return -1;
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

				glEnableVertexAttribArray(3);
				glBindBuffer(GL_ARRAY_BUFFER, bufferCoperchio);
				glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
				glEnableVertexAttribArray(4);
				glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
				glEnableVertexAttribArray(5);
				glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
				glEnableVertexAttribArray(6);
				glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
				glBindBuffer(GL_ARRAY_BUFFER, 0);

				glVertexAttribDivisor(3, 1);
				glVertexAttribDivisor(4, 1);
				glVertexAttribDivisor(5, 1);
				glVertexAttribDivisor(6, 1);

				glDrawElementsInstanced(GL_TRIANGLES, model->meshes[i].indices.size(), GL_UNSIGNED_INT, 0, posizioni.size());

				// always good practice to set everything back to defaults once configured.
				glActiveTexture(GL_TEXTURE0);
				glBindVertexArray(0);
			}

		}

		void apriCassa(int cassaDaAprire, float angoloGradi, SmokeHendler* smokeHendler, float currentFrame) {
			
			//Gestisco l'eccezione sulla cassa da aprire
			if (cassaDaAprire == -1) {
				return;
			}

			// Calcolo 2 punti sulla cerniera della cassa (per farlo ho studiato il modello)
			glm::vec3 puntoD = glm::vec3(0.0f, 0.71f, 0.530828f);
			glm::vec3 puntoA = glm::vec3(0.33f, 0.0f, 0.0f) + puntoD;
			glm::vec3 puntoB = glm::vec3(-0.328179f, 0.0f, 0.0f) + puntoD;

			// Calcolo l'asse della cerniera nell'origine
			glm::vec3 asseCerniera = glm::normalize(puntoA - puntoB);
			
			// Calcolo una nuova matrice model per il coperchio
			glm::mat4 modelCoperchio = glm::mat4(1.0f);
			modelCoperchio = glm::translate(modelCoperchio, puntoD);
			modelCoperchio = glm::rotate(modelCoperchio, glm::radians(angoloGradi), asseCerniera);
			modelCoperchio = glm::translate(modelCoperchio, -puntoD);
			modelMatricesCoperchio[cassaDaAprire] = modelMatricesCoperchio[cassaDaAprire] * modelCoperchio;

			// Ricarico su GPU la model di tutti i coperchi
			glBindBuffer(GL_ARRAY_BUFFER, bufferCoperchio);
			glBufferSubData(GL_ARRAY_BUFFER, 0, 5 * sizeof(glm::mat4), &modelMatricesCoperchio[0]);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			coperchiAperti[cassaDaAprire] = true;
			
			//Aggiungo uno smoker e lo attivo
			smokeHendler->addSmokeGenerator(currentFrame, getPosizioneIndice(cassaDaAprire));
		}

		int contaCasse() {
			int counter = 0;
			for (int i = 0; i < coperchiAperti.size(); i++) {
				if (coperchiAperti[i]) {
					counter++;
				}
			}
			return 5 - counter;
		}

		void creaBufferMatrix() {
		    int amount = posizioni.size();
			for (int i = 0; i < amount; i++) {
				glm::mat4 modelCoperchio = glm::mat4(1.0f);
				modelCoperchio = glm::translate(modelCoperchio, posizioni[i]);
				modelCoperchio = glm::rotate(modelCoperchio, glm::radians(rotazioni[i].y), glm::vec3(0.0f, 0.0f, -1.0f));
				modelCoperchio = glm::rotate(modelCoperchio, glm::radians(rotazioni[i].z), glm::vec3(0.0f, 1.0f, 0.0f));
				modelCoperchio = glm::rotate(modelCoperchio, glm::radians(rotazioni[i].x), glm::vec3(1.0f, 0.0f, 0.0f));
				modelCoperchio = glm::translate(modelCoperchio, glm::vec3(0, 0, 0));
				modelCoperchio = glm::scale(modelCoperchio, glm::vec3(1.5, 1.5, 1.5));
				modelMatricesCoperchio[i] = modelCoperchio;
			}

			// store instance data in an array buffer
			// --------------------------------------
			glGenBuffers(1, &bufferCoperchio);
			glBindBuffer(GL_ARRAY_BUFFER, bufferCoperchio);
			glBufferData(GL_ARRAY_BUFFER, 5 * sizeof(glm::mat4), &modelMatricesCoperchio[0], GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

	private:
		vector<bool> coperchiAperti;
		glm::mat4 modelMatricesCoperchio[5];
		unsigned int bufferCoperchio;

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
				rotazioni.push_back(glm::vec3(alpha, beta, gamma));
				coperchiAperti.push_back(false);
			}
			fin.close();
		}

};