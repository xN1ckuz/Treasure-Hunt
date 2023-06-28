#pragma once

#include <iostream>
#include "DrawableObj.h"
#include "kdtree.h"
#include "TriangoloPt.h"

using namespace std;
using namespace kd;

class Terrain: public DrawableObj {
	public:

		Terrain (string modelDirectory, string textureDir) : DrawableObj(modelDirectory) {
			setModel(modelDirectory);
			setIdentityTrasf();
			loadModel(modelDirectory, VAOterreno, &IndexTerreno);
			texture = loadtextureRepeat(textureDir.c_str());
			alberoKD.costruisciAlbero(convertMapToTrPt());
			minMaxValues = getMinMaxXZFromMap();
		}

		void Draw() {
			shader->use();
			shader->setMat4("model", trasfMatrix);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			shader->setInt("texture_diffuse1", 0);
			glBindVertexArray(VAOterreno);
			glDrawElements(GL_TRIANGLES, IndexTerreno, GL_UNSIGNED_INT, 0);
			setIdentityTrasf();
		}

		vector<TriangoloPt> convertMapToTrPt() {
			vector<TriangoloPt> listaTrangoloPt;
			for (int i = 0; i < mappaTriangoloVertici.size(); i++) {
				std::vector<Vertex> listaVertTriangolo = mappaTriangoloVertici[i];
				TriangoloPt triangoloPt = TriangoloPt(listaVertTriangolo[0].Position, listaVertTriangolo[1].Position, listaVertTriangolo[2].Position);
				listaTrangoloPt.push_back(triangoloPt);
			}
			return listaTrangoloPt;
		}

		glm::vec3 updateCameraPositionOnMap(glm::vec3 posCamera, float offset, bool tutti) {
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

			TriangoloPt puntoCamera(glm::vec3 (posCamera.x, posCamera.y - offset, posCamera.z));

			vector<TriangoloPt> vicino;
			if (tutti == true) {
				vicino = alberoKD.getPuntiEntroRaggio(puntoCamera, 8);
			}
			else {
				vicino = alberoKD.getPuntiEntroRaggio(puntoCamera, 2.2);
			}
			for (int i = 0; i < vicino.size(); i++) {

				//cout << "Triangolo " << i + 1 << endl;
				float y = getYOnPlaneFromXZ(vicino[i].listaVertici[0], vicino[i].listaVertici[1], vicino[i].listaVertici[2], posCamera.x, posCamera.z);

				if (isPointInsideTriangle(glm::vec3(posCamera.x, y, posCamera.z), vicino[i].listaVertici[0], vicino[i].listaVertici[1], vicino[i].listaVertici[2])) {
					posCamera.y = y + offset;
				}
			}
			/*cout << "punto: (" << posCamera.x << ", " << posCamera.y << ", " << posCamera.z << ")" << endl;*/
			return posCamera;
		}

	private:

		struct MinMaxValues {
			float minX;
			float maxX;
			float minZ;
			float maxZ;
		};

		unsigned int VAOterreno, IndexTerreno, texture;
		map<int, vector<Vertex>> mappaTriangoloVertici;
		AlberoKD <TriangoloPt> alberoKD = AlberoKD <TriangoloPt>();
		MinMaxValues minMaxValues;

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

		void loadModel(const std::string& filename, GLuint& VAO, unsigned int* sizeIndex) {

			// Caricamento del modelloAssimp::Importer importer;
			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs);

			std::vector<GLuint> indices;

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
						indices.push_back(indices.size());
					}
					mappaTriangoloVertici[j] = triangoloVertici;
				}
			}

			std::vector<Vertex> vertices = getVerticesFromMap();

			// Crea i VBO e i VAO
			GLuint VBO, EBO;
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);
			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));
			glBindVertexArray(0);

			// Cleanup
			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			*sizeIndex = indices.size();
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

		unsigned int loadtextureRepeat(char const* path)
		{
			unsigned int textureID;
			glGenTextures(1, &textureID);

			int width, height, nrComponents;
			unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
			if (data)
			{
				GLenum format;
				if (nrComponents == 1)
					format = GL_RED;
				else if (nrComponents == 3)
					format = GL_RGB;
				else if (nrComponents == 4)
					format = GL_RGBA;

				glBindTexture(GL_TEXTURE_2D, textureID);
				glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				stbi_image_free(data);
			}
			else
			{
				std::cout << "Texture failed to load at path: " << path << std::endl;
				stbi_image_free(data);
			}

			return textureID;
		}

};