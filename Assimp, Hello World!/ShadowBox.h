#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include"camera.h"

class ShadowBox {

	public:

		ShadowBox(float nearDist, float farDist, const unsigned int SCR_WIDTH, const unsigned int SCR_HEIGHT, Camera* camera, glm::vec3 lightPos) {
			this->nearDist = nearDist;
			this->farDist = farDist;
			this->SCR_WIDTH = SCR_WIDTH;
			this->SCR_HEIGHT = SCR_HEIGHT;
			this->camera = camera;
			this->lightPos = lightPos;
		}

		glm::mat4 getlightSpaceMatrix() {

			float Hnear = 0; float Wnear = 0; float Hfar = 0; float Wfar = 0;

			calculateWidthsAndHeights(&Hnear, &Wnear, &Hfar, &Wfar);
			glm::mat4 lightView = glm::lookAt(normalize(lightPos), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
			std::array<glm::vec3, 8> frustumToLightView = calculateFrustumVerticesLightSpace(Hnear, Wnear, Hfar, Wfar,lightView);
			
			// find max and min points to define a ortho matrix around
			glm::vec3 min{ INFINITY, INFINITY, INFINITY };
			glm::vec3 max{ -INFINITY, -INFINITY, -INFINITY };
			for (unsigned int i = 0; i < frustumToLightView.size(); i++) {
				if (frustumToLightView[i].x < min.x)
					min.x = frustumToLightView[i].x;
				if (frustumToLightView[i].y < min.y)
					min.y = frustumToLightView[i].y;
				if (frustumToLightView[i].z < min.z)
					min.z = frustumToLightView[i].z;

				if (frustumToLightView[i].x > max.x)
					max.x = frustumToLightView[i].x;
				if (frustumToLightView[i].y > max.y)
					max.y = frustumToLightView[i].y;
				if (frustumToLightView[i].z > max.z)
					max.z = frustumToLightView[i].z;
			}

			float l = min.x;
			float r = max.x;
			float b = min.y;
			float t = max.y;
			// because max.z is positive and in NDC the positive z axis is 
			// towards us so need to set it as the near plane flipped same for min.z.
			float n = -max.z;
			float f = -min.z;

			// finally, set our ortho projection
			// and create the light space view-projection matrix
			glm::mat4 lightProjection = glm::ortho(l, r, b, t, n, f);

			return lightProjection * lightView;
		}

	private:


		float nearDist = 0.001f;
		float farDist = 80.0f;

		//Definizione della finestra
		unsigned int SCR_WIDTH = 0;
		unsigned int SCR_HEIGHT = 0;

		glm::vec3 lightPos;
		Camera* camera;


		//Calcola l'Asperct Ratio della finestra di Render
		float getAspectRatio() {
			return (float)SCR_WIDTH / (float)SCR_HEIGHT;
		}

		//calcola l'altezza del NEAR PLANE e del FAR PLANE del frustum camera
		void calculateWidthsAndHeights(float* Hnear, float* Wnear, float* Hfar, float* Wfar) {
			//float fov = glm::radians(camera->Zoom);
			float fov = glm::radians(80.0f);
			*Hnear = 2 * tan(fov / 2) * nearDist;
			*Wnear = *Hnear * getAspectRatio();
			*Hfar = 2 * tan(fov / 2) * farDist;
			*Wfar = *Hfar * getAspectRatio();
		}

		//Calcola la posizione dei vertici del Camera Frustum nel LightSpace (da WordSpace)
		std::array<glm::vec3, 8> calculateFrustumVerticesLightSpace(float Hnear, float Wnear, float Hfar, float Wfar, glm::mat4 lightView) {
			glm::vec3 centerFar = camera->Position + camera->Front * farDist;
			glm::vec3 topLeftFar = centerFar + (camera->Up * Hfar / 2.0f) - (camera->Right * Wfar / 2.0f);
			glm::vec3 topRightFar = centerFar + (camera->Up * Hfar / 2.0f) + (camera->Right * Wfar / 2.0f);
			glm::vec3 bottomLeftFar = centerFar - (camera->Up * Hfar / 2.0f) - (camera->Right * Wfar / 2.0f);
			glm::vec3 bottomRightFar = centerFar - (camera->Up * Hfar / 2.0f) + (camera->Right * Wfar / 2.0f);

			glm::vec3 centerNear = camera->Position + camera->Front * nearDist;
			glm::vec3 topLeftNear = centerNear + (camera->Up * Hnear / 2.0f) - (camera->Right * Wnear / 2.0f);
			glm::vec3 topRightNear = centerNear + (camera->Up * Hnear / 2.0f) + (camera->Right * Wnear / 2.0f);
			glm::vec3 bottomLeftNear = centerNear - (camera->Up * Hnear / 2.0f) - (camera->Right * Wnear / 2.0f);
			glm::vec3 bottomRightNear = centerNear - (camera->Up * Hnear / 2.0f) + (camera->Right * Wnear / 2.0f);

			std::array<glm::vec3, 8> frustumToLightView{
				lightView * glm::vec4(bottomRightNear, 1.0f),
				lightView * glm::vec4(topRightNear, 1.0f),
				lightView * glm::vec4(bottomLeftNear, 1.0f),
				lightView * glm::vec4(topLeftNear, 1.0f),
				lightView * glm::vec4(bottomRightFar, 1.0f),
				lightView * glm::vec4(topRightFar, 1.0f),
				lightView * glm::vec4(bottomLeftFar, 1.0f),
				lightView * glm::vec4(topLeftFar, 1.0f)
			};

			return frustumToLightView;
		}

};