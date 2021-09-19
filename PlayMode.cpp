#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint hexapod_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("hexapod.pnct"));
	hexapod_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > hexapod_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("hexapod.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = hexapod_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

PlayMode::PlayMode() : scene(*hexapod_scene) {
	//get pointers to leg for convenience:
	//for (auto &transform : scene.transforms) {
	//	if (transform.name == "Hip.FL") hip = &transform;
	//	else if (transform.name == "UpperLeg.FL") upper_leg = &transform;
	//	else if (transform.name == "LowerLeg.FL") lower_leg = &transform;
	//}
	//if (hip == nullptr) throw std::runtime_error("Hip not found.");
	//if (upper_leg == nullptr) throw std::runtime_error("Upper leg not found.");
	//if (lower_leg == nullptr) throw std::runtime_error("Lower leg not found.");

	//hip_base_rotation = hip->rotation;
	//upper_leg_base_rotation = upper_leg->rotation;
	//lower_leg_base_rotation = lower_leg->rotation;

	for (auto& transform : scene.transforms) {
		if (transform.name == "Fork") fork = &transform;
		if (transform.name == "cactus") cactus = &transform;
	}
	if (fork == nullptr) throw std::runtime_error("Fork not found.");
	if (cactus == nullptr) throw std::runtime_error("Cactus not found.");

	init_position = fork->position;
	init_rotation = fork->rotation;


	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (finished == 1) {
		if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_r) {
			fork->position = init_position;
			fork->rotation = init_rotation;
			fork_velocity = glm::vec3(0.0f);
			accumulate_rotate = glm::vec2(0.0f);
			shooted = 0;
			finished = 0;
			hit = 0;
			return true;
		} 
		else if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} 
		else {
			return false;
		}
	}


	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_a) {
			a_key.downs += 1;
			a_key.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_d) {
			d_key.downs += 1;
			d_key.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_w) {
			w_key.downs += 1;
			w_key.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_s) {
			s_key.downs += 1;
			s_key.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_SPACE && shooted == 0) {
			shooted = 1;
			fork_velocity.x = fork_speed * glm::cos(accumulate_rotate.y) * glm::sin(accumulate_rotate.x);
			fork_velocity.y = fork_speed * glm::cos(accumulate_rotate.y) * glm::cos(accumulate_rotate.x);
			fork_velocity.z = fork_speed * glm::sin(accumulate_rotate.y);
			return true;
		}
	}
	else if (evt.type == SDL_KEYUP) {
	if (evt.key.keysym.sym == SDLK_a) {
		a_key.pressed = false;
		return true;
	}
	else if (evt.key.keysym.sym == SDLK_d) {
		d_key.pressed = false;
		return true;
	}
	else if (evt.key.keysym.sym == SDLK_w) {
		w_key.pressed = false;
		return true;
	}
	else if (evt.key.keysym.sym == SDLK_s) {
		s_key.pressed = false;
		return true;
	}
	else if (evt.key.keysym.sym == SDLK_LEFT) {
		left.pressed = false;
		return true;
	}
	else if (evt.key.keysym.sym == SDLK_RIGHT) {
		right.pressed = false;
		return true;
	}
	else if (evt.key.keysym.sym == SDLK_UP) {
		up.pressed = false;
		return true;
	}
	else if (evt.key.keysym.sym == SDLK_DOWN) {
		down.pressed = false;
		return true;
	}
	}
	else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	}
	else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);
			camera->transform->rotation = glm::normalize(
				camera->transform->rotation
				* glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
			);
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
	static std::mt19937 mt;
	if (finished == 1)
		return;
	if (shooted == 1) {
		fork_velocity.z -= 9.8f * elapsed;
		fork->position.x += 3* fork_velocity.x * elapsed;
		fork->position.y += 3* fork_velocity.y * elapsed;
		fork->position.z += 3* fork_velocity.z * elapsed;
		if (fork->position.z <= 0.0f || (float)glm::distance(fork->position,cactus->position) <= 1.0f) {
			shooted = 0;
			finished = 1;
			if ((float)glm::distance(fork->position, cactus->position) <= 1.0f) {
				hit = 1;
			}
			fork_velocity = glm::vec3(0.0f);
		}
	}

	

	//move fork direction:
	if(shooted == 0)
	{

		//combine inputs into a move:
		constexpr float PlayerSpeed = 1.0f;
		glm::vec3 move = glm::vec3(0.0f);
		glm::vec2 rotate = glm::vec2(0.0f);
		if (left.pressed && !right.pressed && fork->position.x >= -10.0f) move.x =-1.0f;
		if (!left.pressed && right.pressed && fork->position.x <= 10.0f) move.x = 1.0f;
		if (down.pressed && !up.pressed && fork->position.z >= 0.1f) move.z =-1.0f;
		if (!down.pressed && up.pressed && fork->position.z <= 100.0f) move.z = 1.0f;
		if (a_key.pressed && !d_key.pressed) rotate.x = -1.0f;
		if (!a_key.pressed && d_key.pressed) rotate.x = 1.0f;
		if (s_key.pressed && !w_key.pressed) rotate.y = -1.0f;
		if (!s_key.pressed && w_key.pressed) rotate.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		move *= PlayerSpeed * elapsed;
		rotate *= PlayerSpeed * elapsed;
		accumulate_rotate += rotate;

		fork->position += move;

		fork->rotation = fork->rotation
			* glm::angleAxis(-rotate.x, glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::angleAxis(rotate.y, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	GL_ERRORS(); //print any errors produced by this setup code

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		if (finished == 0) {
			constexpr float H = 0.09f;
			lines.draw_text("Arrow keys moves; WASD rotate; press space to shoot; escape ungrabs mouse; Q to quit",
				glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0x00, 0x00, 0x00, 0x00));
			float ofs = 2.0f / drawable_size.y;
			lines.draw_text("Arrow keys moves; WASD rotate; press space to shoot; escape ungrabs mouse; Q to quit",
				glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + +0.1f * H + ofs, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0xff, 0xff, 0xff, 0x00));
		}
		else {
			constexpr float H = 0.09f;
			if (hit == 1) {
				lines.draw_text("You hit the target! Press R to reset, press Q to quit",
					glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
					glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
					glm::u8vec4(0xff, 0x00, 0x00, 0x00));
				float ofs = 2.0f / drawable_size.y;
				lines.draw_text("You hit the target! Press R to reset, press Q to quit",
					glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + +0.1f * H + ofs, 0.0),
					glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
					glm::u8vec4(0xff, 0xff, 0xff, 0x00));
			}
			else {
				lines.draw_text("Game over! Press R to reset, press Q to quit",
					glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
					glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
					glm::u8vec4(0x00, 0x00, 0x00, 0x00));
				float ofs = 2.0f / drawable_size.y;
				lines.draw_text("Game over! Press R to reset, press Q to quit",
					glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + +0.1f * H + ofs, 0.0),
					glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
					glm::u8vec4(0xff, 0xff, 0xff, 0x00));
			}


		}

	}
}
