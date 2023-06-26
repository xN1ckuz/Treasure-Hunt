#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include"camera.h"

class ShadowBox {
	public:

		ShadowBox(Camera* cam, glm::vec3 lightPos, const unsigned int SCR_WIDTH, const unsigned int SCR_HEIGHT, float NEAR_PLANE, float FAR_PLANE, float SHADOW_DISTANCE, float OFFSET) {
			this->lightPos = lightPos;
			this->camera = cam;
			this->SCR_WIDTH = SCR_WIDTH;
			this->SCR_HEIGHT = SCR_HEIGHT;
			this->FAR_PLANE = FAR_PLANE;
			this->NEAR_PLANE = NEAR_PLANE;
			this->SHADOW_DISTANCE = SHADOW_DISTANCE;
			this->OFFSET = OFFSET;
			boxMinMax = {};
			calculateWidthsAndHeights();
		}

		//Aggiorna i boxMinMax
		//Calcola Massimi e Minimi della Bounding Box che racchiude il Frustum
		void update() {
			glm::mat4 rotation = calculateCameraRotationMatrix();
			glm::vec3 forwardVector = glm::vec3(rotation * FORWARD);

			glm::vec3 toFar(forwardVector * SHADOW_DISTANCE);
			glm::vec3 toNear(forwardVector * NEAR_PLANE);
			glm::vec3 centerNear(toNear + camera->Position);
			glm::vec3 centerFar(toFar + camera->Position);

			vector<glm::vec4> points = calculateFrustumVerticesLightSpace(rotation, forwardVector, centerNear, centerFar);

			bool first = true;
			for (glm::vec4 point : points) {
				if (first) {
					boxMinMax.minX = point.x;
					boxMinMax.maxX = point.x;
					boxMinMax.minY = point.y;
					boxMinMax.maxY = point.y;
					boxMinMax.minZ = point.z;
					boxMinMax.maxZ = point.z;
					first = false;
					continue;
				}
				if (point.x > boxMinMax.maxX) {
					boxMinMax.maxX = point.x;
				}
				else if (point.x < boxMinMax.minX) {
					boxMinMax.minX = point.x;
				}
				if (point.y > boxMinMax.maxY) {
					boxMinMax.maxY = point.y;
				}
				else if (point.y < boxMinMax.minY) {
					boxMinMax.minY = point.y;
				}
				if (point.z > boxMinMax.maxZ) {
					boxMinMax.maxZ = point.z;
				}
				else if (point.z < boxMinMax.minZ) {
					boxMinMax.minZ = point.z;
				}
			}
			boxMinMax.maxZ += OFFSET;

			debugMinMaxValues();
		}

		//Restituisce la giusta lightView
		glm::mat4 getLightViewMatrix() {
			
			glm::vec3 direction = glm::normalize(glm::vec3(-lightPos.x, -lightPos.y, -lightPos.z));
			glm::vec3 center = - calculateCenter();
			glm::mat4 lightView(1.0);
			glm::vec2 somma(glm::vec2(direction.x, direction.z));
			float pitch = (float) acos(sqrt(pow(somma.x, 2) + pow(somma.y, 2)));
			lightView = glm::rotate(lightView, pitch, glm::vec3(1.0f, 0.0f, 0.0f));
			float yaw = glm::degrees(((float)atan(direction.x / direction.z)));
			yaw = direction.z > 0 ? yaw - 180 : yaw;
			lightView = glm::rotate(lightView, - glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
			lightView = glm::translate(lightView, center);

			//// stampa la matrice
			//cout << "Light view" << endl;
			//for (int i = 0; i < 4; ++i) {
			//	for (int j = 0; j < 4; ++j) {
			//		std::cout << lightView[i][j] << " ";
			//	}
			//	std::cout << std::endl;
			//}

			return lightView;
		}

		//Restituisce la giusta lightProjection
		glm::mat4 getOrthoProjectionMatrix() {
			glm::mat4 lightProjection(1.0);
			lightProjection[0][0] = 2.0f / calculateWidth();
			lightProjection[1][1] = 2.0f / calculateHeight();
			lightProjection[2][2] = -2.0f / calculateLength();
			lightProjection[3][3] = 1;

			//// stampa la matrice
			//cout << "lightProjection" << endl;
			//for (int i = 0; i < 4; ++i) {
			//	for (int j = 0; j < 4; ++j) {
			//		std::cout << lightProjection[i][j] << " ";
			//	}
			//	std::cout << std::endl;
			//}

			return lightProjection;
		}

		std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& projview)
		{
			const auto inv = glm::inverse(projview);

			std::vector<glm::vec4> frustumCorners;
			for (unsigned int x = 0; x < 2; ++x)
			{
				for (unsigned int y = 0; y < 2; ++y)
				{
					for (unsigned int z = 0; z < 2; ++z)
					{
						const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
						frustumCorners.push_back(pt / pt.w);
					}
				}
			}

			return frustumCorners;
		}

		void debugVerticesLight() {
			vector<glm::vec4> cornersFrustumLight = getFrustumCornersWorldSpace(this->getOrthoProjectionMatrix() * this->getLightViewMatrix());
			cout << cornersFrustumLight.size() << endl;
			for (const auto& v : cornersFrustumLight)
			{
				cout << "cornerL: (" << v.x << " , " << v.z << " , " << v.y << ")" << endl;
			}
		}

		void debugVerticesCamera() {
			const auto proj = glm::perspective(glm::radians(camera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, NEAR_PLANE, FAR_PLANE);
			const auto corners = getFrustumCornersWorldSpace(proj * camera->GetViewMatrix());

			cout << corners.size() << endl;
			for (const auto& v : corners)
			{
				cout << "corner: (" << v.x << " , " << v.z << " , " << v.y << ")" << endl;
			}
		}

		void debugMinMaxValues() {
			cout << "minX: " << boxMinMax.minX << endl;
			cout << "maxX: " << boxMinMax.maxX << endl;
			cout << "minY: " << boxMinMax.minY << endl;
			cout << "maxY: " << boxMinMax.maxY << endl;
			cout << "minZ: " << boxMinMax.minZ << endl;
			cout << "maxZ: " << boxMinMax.maxZ << endl;
		}


	private:

		struct MinMaxValues {
			float minX;
			float maxX;
			float minY;
			float maxY;
			float minZ;
			float maxZ;
		};

		Camera* camera;
		glm::vec4 UP = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
		glm::vec4 FORWARD = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

		// Una struct per mantenere i dati della Box
		MinMaxValues boxMinMax = {};
		
		//Parametro per modificare la lunghezza del frustum della camera
		float SHADOW_DISTANCE = 0;
		
		//Serve ad impostare una distanza della Bounding Box dalla camera
		float OFFSET = 0;

		//Camera Near e Far
		float NEAR_PLANE = 0;
		float FAR_PLANE = 0;

		//Definizione della finestra
		unsigned int SCR_WIDTH = 0;
		unsigned int SCR_HEIGHT = 0;

		//Altezze di PLANE del Camera Frustum
		float farHeight = 0, farWidth = 0, nearHeight = 0, nearWidth = 0;

		//Caratteristiche luce
		glm::vec3 lightPos;

		//Calcola l'Asperct Ratio della finestra di Render
		float getAspectRatio() {
			return (float)SCR_WIDTH / (float)SCR_HEIGHT;
		}

		//calcola l'altezza del NEAR PLANE e del FAR PLANE del frustum della camera
		void calculateWidthsAndHeights() {
			farWidth = (float)(SHADOW_DISTANCE * tan(glm::radians(camera->Zoom)));
			nearWidth = (float)(NEAR_PLANE * tan(glm::radians(camera->Zoom)));
			farHeight = farWidth / getAspectRatio();
			nearHeight = nearWidth / getAspectRatio();
		}

		glm::vec4 calculateLightSpaceFrustumCorner(glm::vec3 startPoint, glm::vec3 direction, float width) {
			glm::vec3 point(startPoint + glm::vec3(direction.x * width, direction.y * width, direction.z * width));
			glm::vec4 point4f(point.x, point.y, point.z, 1.0f);
			glm::mat4 lightView = glm::lookAt(lightPos, glm::normalize(glm::vec3(-lightPos.x, -lightPos.y, -lightPos.z)), glm::vec3(0.0, 1.0, 0.0));
			point4f = lightView * point4f;
			/*cout << lightView[0][0] << endl;
			cout << lightView[0][1] << endl;
			cout << lightView[0][2] << endl;
			cout << lightView[0][3] << endl;
			cout << "points4f: (" << point4f.x << " , " << point4f.z << " , " << point4f.y << ")" << endl;*/
			return point4f;
		}

		//Se vuoi rendere indipendente dalla rotazione l'ombra dovresti disattivarla...
		glm::mat4 calculateCameraRotationMatrix() {
			glm::mat4 rotation(1.0);
			rotation = glm::rotate(rotation, glm::radians(- camera->Yaw), glm::vec3(0.0f, 1.0f, 0.0f));
			rotation = glm::rotate(rotation, glm::radians(- camera->Pitch), glm::vec3(1.0f, 0.0f, 0.0f));
			return rotation;
		}

		//Calcola la posizione dei vertici del Camera Frustum nel LightSpace (da WordSpace)
		vector<glm::vec4> calculateFrustumVerticesLightSpace(glm::mat4 rotation, glm::vec3 forwardVector, glm::vec3 centerNear, glm::vec3 centerFar) {
			
			glm::vec3 upVector(rotation * UP);
			glm::vec3 rightVector = glm::cross(forwardVector, upVector);
			glm::vec3 downVector(-upVector.x, -upVector.y, -upVector.z);
			glm::vec3 leftVector(-rightVector.x, -rightVector.y, -rightVector.z);

			glm::vec3 farTop (centerFar + glm::vec3(upVector.x * farHeight, upVector.y * farHeight, upVector.z * farHeight));
			//cout << "farTop: (" << farTop.x << " , " << farTop.z << " , " << farTop.y << ")" << endl;
			glm::vec3 farBottom (centerFar + glm::vec3(downVector.x * farHeight, downVector.y * farHeight, downVector.z * farHeight));
			//cout << "farBottom: (" << farBottom.x << " , " << farBottom.z << " , " << farBottom.y << ")" << endl;
			glm::vec3 nearTop (centerNear + glm::vec3(upVector.x * nearHeight, upVector.y * nearHeight, upVector.z * nearHeight));
			//cout << "nearTop: (" << nearTop.x << " , " << nearTop.z << " , " << nearTop.y << ")" << endl;
			glm::vec3 nearBottom (centerNear + glm::vec3(downVector.x * nearHeight, downVector.y * nearHeight, downVector.z * nearHeight));
			//cout << "nearBottom: (" << nearBottom.x << " , " << nearBottom.z << " , " << nearBottom.y << ")" << endl;

			vector<glm::vec4> points;
			points.push_back(calculateLightSpaceFrustumCorner(farTop, rightVector, farWidth));
			points.push_back(calculateLightSpaceFrustumCorner(farTop, leftVector, farWidth));
			points.push_back(calculateLightSpaceFrustumCorner(farBottom, rightVector, farWidth));
			points.push_back(calculateLightSpaceFrustumCorner(farBottom, leftVector, farWidth));
			points.push_back(calculateLightSpaceFrustumCorner(nearTop, rightVector, nearWidth));
			points.push_back(calculateLightSpaceFrustumCorner(nearTop, leftVector, nearWidth));
			points.push_back(calculateLightSpaceFrustumCorner(nearBottom, rightVector, nearWidth));
			points.push_back(calculateLightSpaceFrustumCorner(nearBottom, leftVector, nearWidth));

			return points;
		}


		//Calcolo il centro della ShadowBox nel WorldSpace
		glm::vec3 calculateCenter() {
			float x = (boxMinMax.minX + boxMinMax.maxX) / 2.0f;
			float y = (boxMinMax.minY + boxMinMax.maxY) / 2.0f;
			float z = (boxMinMax.minZ + boxMinMax.maxZ) / 2.0f;
			//cout << "center: (" << x << " , " << z << " , " << y << ")" << endl;
			glm::vec4 cen(x, y, z, 1);
			glm::mat4 lightView = glm::lookAt(lightPos, glm::normalize(glm::vec3(-lightPos.x, -lightPos.y, -lightPos.z)), glm::vec3(0.0, 1.0, 0.0));
			glm::mat4 invertedLight = glm::inverse(lightView);
			glm::vec3 center(invertedLight * cen);
			//cout << "centerPost: (" << center.x << " , " << center.z << " , " << center.y << ")" << endl;
			return center;
		}

		//Calcolo la larghezza della ShadowBox
		float calculateWidth() {
			return boxMinMax.maxX - boxMinMax.minX;
		}

		//Calcolo l'altezza della ShadowBox
		float calculateHeight() {
			return boxMinMax.maxY - boxMinMax.minY;
		}

		//Calcolo la lunghezza della ShadowBox
		float calculateLength() {
			return boxMinMax.maxZ - boxMinMax.minZ;
		}

};