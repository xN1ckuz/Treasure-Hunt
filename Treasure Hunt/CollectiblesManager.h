#pragma once

#include<vector>
#include"DrawableObj.h"
#include "Coperchi.h"
#include "effects.h"

using namespace std;

class CollectiblesManager {
	public:

		struct distEindex {
			int i;
			float dist;
			float tipo;
		}distEindex_default = { -1,10000,-1 };

		CollectiblesManager() {
			
		}

		void addDrawableObj(DrawableObj* collectible) {
			collectibles.push_back(collectible);
		}

		void addCoperchi(Coperchi* collectible) {
			coperchiCasse = collectible;
		}

		distEindex isNearcollectibles(glm::vec3 pos, float raggio) {
			distEindex collectibles = distEindex_default;
			if (caricamento) {
				collectibles = cercaMinimaDist(posizioniCollect, trovatiCollectibles, pos, raggio, 1);
			}
			distEindex coperchi = cercaMinimaDist(coperchiCasse->getPosizioni(), coperchiCasse->getBoolAperti(), pos, raggio, 0);
			//Caso in cui non ha trovato nulla per entrambi
			if (collectibles.dist == coperchi.dist) {
				return distEindex_default;
			}
			else {
				if (collectibles.dist < coperchi.dist) {
					return collectibles;
				}
				else {
					return coperchi;
				}
			}
		}

		void raccogliCollectibles(glm::vec3 pos,float raggio, SmokeHendler* smokeHendler, float currentFrame, Audio* audioHandler, float *tempoMax, bool aperturaCollectible) {
			distEindex distIndex = isNearcollectibles(pos, raggio);
			if (distIndex.tipo == -1) {
				return;
			}
			if (distIndex.tipo == 0) {
				coperchiCasse->apriCassa(distIndex.i, 90, smokeHendler, currentFrame, audioHandler);
				return;
			}
			if (distIndex.tipo == 1) {
				trovatiCollectibles[distIndex.i] = true;
				if (aperturaCollectible) {
					*tempoMax += 15;
				}
			}
		}

		void drawCollectibles(bool ombra) {
			//Draw casse
			if (ombra) {
				coperchiCasse->getShader()->use();
				coperchiCasse->getShader()->setFloat("soglia", 0.5);
			}
			coperchiCasse->Draw();

			//Draw collectionable
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			if (ombra) {
				if (caricamento) {
					for (int i = 0; i < posizioniCollect.size(); i++) {
						if (trovatiCollectibles[i] == false) {
							collectibles[i]->getShader()->use();
							collectibles[i]->getShader()->setFloat("soglia", 0.5);
						}
						else {
							continue;
						}
					}
				}
			}
			if (caricamento) {
				for (int i = 0; i < posizioniCollect.size(); i++) {
					if (trovatiCollectibles[i] == false) {
						collectibles[i]->traslate(posizioniCollect[i]);
						collectibles[i]->rotate(glm::vec3(0.0f, 0.0f, -1.0f), glm::radians(rotazioniCollect[i].y));
						collectibles[i]->rotate(glm::vec3(0.0f, 1.0f, 0.0f), glm::radians(rotazioniCollect[i].z));
						collectibles[i]->rotate(glm::vec3(1.0f, 0.0f, 0.0f), glm::radians(rotazioniCollect[i].x));
						collectibles[i]->traslate(glm::vec3(0.0f, 0.0f, 0.0f));
						collectibles[i]->Draw();
					}
					else {
						continue;
					}
				}
			}
			glDisable(GL_BLEND);
		}

		void aggiornaPosPerCollisione(glm::vec3* pos, glm::vec3 posVecchia, float raggio) {
			glm::vec3 puntoVicino(0.0);
			for (int i = 0; i < posizioniCollect.size(); i++) {
				if (trovatiCollectibles[i] == false) {
					puntoVicino = posizioniCollect[i];
					float distanza = sqrt(pow(pos->x - puntoVicino.x, 2) + pow(pos->y - puntoVicino.y, 2) + pow(pos->z - puntoVicino.z, 2));
					if (distanza <= raggio ) {
						*pos = posVecchia;
						return;
					}
				}
				else {
					continue;
				}
			}
		}

		Coperchi* getCoperchi() {
			return coperchiCasse;
		}


		//cacrico posizioni e orientamento dei drawable
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

				posizioniCollect.push_back(glm::vec3(x, z, -y));
				rotazioniCollect.push_back(glm::vec3(alpha, beta, gamma));
				trovatiCollectibles.push_back(false);
			}
			fin.close();
			caricamento = true;
		}

	private:
		vector<glm::vec3> posizioniCollect;
		vector<glm::vec3> rotazioniCollect;
		vector<bool> trovatiCollectibles;
		vector<DrawableObj*> collectibles;
		Coperchi* coperchiCasse = nullptr;
		bool caricamento = false;
		
		distEindex cercaMinimaDist(vector<glm::vec3> posizioni, vector<bool> boolCollected, glm::vec3 pos, float raggio, float tipo) {
			distEindex distIndex = distEindex_default;
			for (int i = 0; i < boolCollected.size(); i++) {
				float distanza = sqrt(pow(pos.x - posizioni[i].x, 2) + pow(pos.y - posizioni[i].y, 2) + pow(pos.z - posizioni[i].z, 2));
				if (distanza <= raggio) {
					if (boolCollected[i] == false) {
						distIndex.i = i;
						distIndex.dist = distanza;
						distIndex.tipo = tipo;
						return distIndex;
					}
				}
			}
			return distIndex;
		}

};
