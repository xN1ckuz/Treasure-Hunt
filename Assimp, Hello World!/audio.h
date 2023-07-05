#pragma once
#include "SFML/Audio.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

using namespace std;
using namespace sf;
using namespace glm;

class Audio {
	
	public:
		Music backgroundAudio;
		SoundBuffer bufferOpenChest;
		Sound openChestSound;

		Audio() {

		}

		void loadAudioFiles(string path, int type) {
			switch (type) {
				case 0:
					backgroundAudio.openFromFile(path);
					setBackGroundAudio();
				case 1:
					bufferOpenChest.loadFromFile(path);
					setOpenChestSound();
			}
		}
	
		void backgroundAudioPlay() {
			backgroundAudio.play();
		}

		void openChestSoundPlay(vec3 pos) {
			openChestSound.setPosition(pos.x, pos.y, pos.z);
			openChestSound.play();
		}

		void closeAllBuffers() {
			backgroundAudio.~Music();
			bufferOpenChest.~SoundBuffer();
			openChestSound.~Sound();
		}

	private:
		void setBackGroundAudio() {
			Listener::setPosition(0.0f, 0.0f, 0.0f);
			backgroundAudio.setPosition(0.0f, 0.0f, 0.0f);
			backgroundAudio.setPitch(1.0f);
			backgroundAudio.setVolume(4.0f);
			backgroundAudio.setMinDistance(5.0);
			backgroundAudio.setLoop(true);
		}

		void setOpenChestSound() {
			openChestSound.setPosition(0.0f, 0.0f, 0.0f);
			openChestSound.setPitch(1.0f);
			openChestSound.setVolume(15.0f);
			openChestSound.setBuffer(bufferOpenChest);
			openChestSound.setMinDistance(5.0);
		}
		
};