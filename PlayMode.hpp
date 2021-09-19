#include "Mode.hpp"

#include "Scene.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, w_key, a_key, s_key, d_key;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//hexapod leg to wobble:
	Scene::Transform *hip = nullptr;
	Scene::Transform *upper_leg = nullptr;
	Scene::Transform *lower_leg = nullptr;
	glm::quat hip_base_rotation;
	glm::quat upper_leg_base_rotation;
	glm::quat lower_leg_base_rotation;
	float wobble = 0.0f;

	Scene::Transform* fork = nullptr;
	Scene::Transform* cactus = nullptr;
	glm::vec3 init_position = glm::vec3(0.0f,0.0f,0.0f);
	glm::quat init_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	float fork_speed = 10.0f;
	glm::vec3 fork_velocity = glm::vec3(0.0f);
	glm::vec2 accumulate_rotate = glm::vec2(0.0f);
	uint8_t shooted = 0;
	uint8_t finished = 0;
	uint8_t hit = 0;
	float x_offset = 0.0f;
	float y_offset = 0.0f;
	float ai_offset_update = 0.0f;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
