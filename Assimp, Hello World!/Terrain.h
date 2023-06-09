#pragma once

#include <iostream>
#include "DrawableObj.h"

using namespace std;

class Terrain: public DrawableObj {
	public:

		Terrain (string modelDirectory) : DrawableObj(modelDirectory) {
			setModel(modelDirectory);
			setIdentityTrasf();
			loadVerticesForWalk(modelDirectory);
		}

		glm::vec3 updateCameraPositionOnMap(glm::vec3 posCamera, float offset) {
			MinMaxValues minMaxValues = getMinMaxXZFromMap();
			if (posCamera.x < minMaxValues.minX) {
				posCamera.x = minMaxValues.minX + 0.3;
			}
			if (posCamera.x > minMaxValues.maxX) {
				posCamera.x = minMaxValues.maxX - 0.3;
			}
			if (posCamera.z < minMaxValues.minZ) {
				posCamera.z = minMaxValues.minZ + 0.3;
			}
			if (posCamera.z > minMaxValues.maxZ) {
				posCamera.z = minMaxValues.maxZ - 0.3;
			}

			for (int i = 0; i < mappaTriangoloVertici.size(); i++) {
				std::vector<Vertex> listaVertTriangolo = mappaTriangoloVertici[i];

				//conversione in 2D
				Vertex a = listaVertTriangolo[0];
				a.Position.y = 0;
				Vertex b = listaVertTriangolo[1];
				b.Position.y = 0;
				Vertex c = listaVertTriangolo[2];
				c.Position.y = 0;
				glm::vec3 cam2d(posCamera.x, 0.0, posCamera.z);

				if (isPointInsideTriangle(cam2d, a.Position, b.Position, c.Position)) {
					/*cout << "Sono in un triangolo" << endl;*/

					float y = getYOnPlaneFromXZ(listaVertTriangolo[0].Position, listaVertTriangolo[1].Position, listaVertTriangolo[2].Position, posCamera.x, posCamera.z);
					posCamera.y = y + offset;
				}
			}
			return posCamera;
		}

		//Restituisce la lista di tutti i vertici della mesh
		std::vector<Vertex> getVerticesFromMap() {
			std::vector<Vertex> listaVertici;
			for (int i = 0; i < mappaTriangoloVertici.size(); i++) {
				std::vector<Vertex> listaVertTriangolo = mappaTriangoloVertici[i];
				listaVertici.push_back(listaVertTriangolo[0]);
				listaVertici.push_back(listaVertTriangolo[1]);
				listaVertici.push_back(listaVertTriangolo[2]);
			}
			return listaVertici;
		}

	private:
		map<int, vector<Vertex>> mappaTriangoloVertici;

		struct MinMaxValues {
			float minX;
			float maxX;
			float minZ;
			float maxZ;
		};

		void loadVerticesForWalk(string modelDir) {

			// Caricamento del modelloAssimp::Importer importer;
			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(modelDir, aiProcess_Triangulate | aiProcess_FlipUVs);

			// Estrae i vertici e gli indici dal modello
			for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
				const aiMesh* mesh = scene->mMeshes[i];
				for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
					const aiFace& face = mesh->mFaces[j];
					vector<Vertex> triangoloVertici;
					for (unsigned int k = 0; k < face.mNumIndices; k++) {
						unsigned int index = face.mIndices[k];
						Vertex vertex;
						vertex.Position = glm::vec3(mesh->mVertices[index].x, mesh->mVertices[index].y, mesh->mVertices[index].z);
						vertex.Normal = glm::vec3(mesh->mNormals[index].x, mesh->mNormals[index].y, mesh->mNormals[index].z);
						if (mesh->mTextureCoords[0]) {
							vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][index].x, mesh->mTextureCoords[0][index].y);
						}
						triangoloVertici.push_back(vertex);
					}
					mappaTriangoloVertici[j] = triangoloVertici;
				}
			}
		}

		
		// Dati un punto e un triangolo rappresentato dai suoi tre vertici, questa funzione restituisce true se il punto è all'interno del triangolo, false altrimenti
		bool isPointInsideTriangle(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c) {
			// Calcoliamo le normali dei tre sottotriangoli che si ottengono aggiungendo il punto p ai tre vertici del triangolo
			glm::vec3 normal1 = glm::cross(b - a, p - a);
			glm::vec3 normal2 = glm::cross(c - b, p - b);
			glm::vec3 normal3 = glm::cross(a - c, p - c);

			// Se i prodotti scalari tra le normali e la normale del triangolo hanno lo stesso segno, allora il punto è all'interno del triangolo
			return (glm::dot(normal1, glm::cross(b - a, c - a)) >= 0.0f) &&
				(glm::dot(normal2, glm::cross(c - b, a - b)) >= 0.0f) &&
				(glm::dot(normal3, glm::cross(a - c, b - c)) >= 0.0f);
		}

		// Dati i vertici di un triangolo e le coordinate x e z di un punto P sul piano del triangolo,
		// questa funzione restituisce la coordinata y di P.
		float getYOnPlaneFromXZ(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, float x, float z) {
			// Calcoliamo il vettore normale al piano del triangolo
			glm::vec3 n = glm::cross(v2 - v1, v3 - v1);

			// Calcoliamo l'equazione del piano che contiene il triangolo
			float a = n.x;
			float b = n.y;
			float c = n.z;
			float d = -glm::dot(n, v1);

			// Calcoliamo la coordinata y del punto P
			float y = (-a * x - c * z - d) / b;

			return y;
		}

		//Trova la minima X e Z dei vertici della mesh
		MinMaxValues getMinMaxXZFromMap() {
			std::vector<Vertex> vertices = getVerticesFromMap();

			float minX = vertices[0].Position.x;
			float maxX = vertices[0].Position.x;
			float minZ = vertices[0].Position.z;
			float maxZ = vertices[0].Position.z;

			for (const auto& vertex : vertices) {
				if (vertex.Position.x < minX) {
					minX = vertex.Position.x;
				}
				if (vertex.Position.x > maxX) {
					maxX = vertex.Position.x;
				}
				if (vertex.Position.z < minZ) {
					minZ = vertex.Position.z;
				}
				if (vertex.Position.z > maxZ) {
					maxZ = vertex.Position.z;
				}
			}
			return { minX, maxX, minZ, maxZ };
		}

};