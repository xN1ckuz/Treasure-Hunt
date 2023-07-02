#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Particle {
	public:
		float life;
		float tempoInziale;
		glm::vec3 posizioneInziale;
		float a, b;
		float alpha;

		Particle(float life, float tempoIniziale, glm::vec3 posIniziale, float a, float b, float alpha) {
			this->life = life;
			this->tempoInziale = tempoIniziale;
			this->posizioneInziale = posIniziale;
			this->a = a;
			this->b = b;
			this->alpha = alpha;
		}
};

class SmokeGenerator
{

	public:
	
		SmokeGenerator(string modelDir, int amount, bool attivo = false) {
			modello = Model(modelDir);
			this->amount = amount;
			this->attivo = attivo;
		}

		void generaParticle(float tempoInit, glm::vec3 posInit, glm::vec3 posFinal, float raggio, float life) {
			this->posInit = posInit;
			this->posFinal = posFinal;
			v = posFinal - posInit;
			u = glm::vec3(0.0);
			if (v.z != 0) {
				u.x = v.z;
				u.z = -v.x;
			}
			else {
				u.x = v.y;
				u.y = -v.x;
			}
		    w = glm::cross(v,u);
			for (int i = 0; i < amount; i++) {
				float tempoInziale = tempoInit + RandomNumber(-life/1.3, 0);
				//float tempoInziale = tempoInit;
				float a = RandomNumber(-raggio, raggio);
				float b = RandomNumber(-sqrt(pow(raggio, 2)-pow(a, 2)), sqrt(pow(raggio, 2) - pow(a, 2)));
				float alpha = 2.0;
				particles.push_back(new Particle(life, tempoInziale, posInit, a, b, alpha));
			}
			this->attivo = true;
		}

		void setShaders(Shader* shaderS) {
			shader = shaderS;
		}

		Shader* getShader() {
			return shader;
		}

		void Draw(float tempoCorrente) {
			float normaV = sqrt(pow(v.x, 2) + pow(v.y, 2) + pow(v.z, 2));
			float normaU = sqrt(pow(u.x, 2) + pow(u.y, 2) + pow(u.z, 2));
			float normaW = sqrt(pow(w.x, 2) + pow(w.y, 2) + pow(w.z, 2));
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			for (int i = 0; i < amount; i++) {
				float dt = tempoCorrente - this->particles[i]->tempoInziale;
				if (dt < this->particles[i]->life) {
					float decellerazione = (this->particles[i]->life) / normaV;
					glm::vec3 pos = this->particles[i]->posizioneInziale + ((dt) / decellerazione) * ((v / normaV) + this->particles[i]->a * (u / normaU) + this->particles[i]->b * (w / normaW));
					float alpha = this->particles[i]->alpha *  pow((1 - (dt / particles[i]->life)),1);
					glm::mat4 model(1.0);
					model = glm::translate(model, pos);
					model = glm::scale(model, glm::vec3(0.7,0.7,0.7));
					shader->use();
					shader->setFloat("alpha", alpha);
					shader->setMat4("model", model);
					modello.Draw(*shader);
				}
			}
			glDisable(GL_BLEND);
		}

	private:

		glm::vec3 posInit;
		glm::vec3 posFinal;
		Model modello;
		Shader* shader;
		vector<Particle*> particles;
		int amount;
		//vettori lungo cui muoversi
		glm::vec3 v, u, w;
		bool attivo;

		float RandomNumber(float Min, float Max)
		{
			return ((float(rand()) / float(RAND_MAX)) * (Max - Min)) + Min;
		}

};