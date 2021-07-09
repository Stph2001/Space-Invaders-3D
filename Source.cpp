#include <math.h>
#include <vector>
#include <time.h>

#include <glutil.h>
#include <figures.h>
#include <camera.h>

#include <files.hpp>
#include <model.hpp>

const u32 FSIZE = sizeof(f32);
const u32 ISIZE = sizeof(u32);
const u32 SCRWIDTH = 1280;
const u32 SCRHEIGHT = 720;
const f32 ASPECT = (f32)SCRWIDTH / (f32)SCRHEIGHT;

glm::vec3 lightPos(50.0f, 20.0f, 100.0f);
glm::vec3 posBarra = glm::vec3(1.0f, -5.0f, 0.0);
glm::vec3 posBola = glm::vec3(1.0f, -4.50f, 0.0f);

Cam* cam;

f32 lastx;
f32 lasty;
f32 deltaTime = 0.0f;
f32 lastFrame = 0.0f;
bool firstmouse = true;
bool wireframe = false;

// estados
bool shoot = false;
float tiempo = 400;
bool win = false;
bool lose = false;
bool frase = false;

/**
 * keyboard input processing
 **/
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	// Mover barra
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		if (posBarra.x - deltaTime * 5 >= -2) {
			posBarra.x -= deltaTime * 5;
			if (!shoot) {
				posBola.x -= deltaTime * 5;
			}
		}
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		if (posBarra.x + deltaTime * 5 < 19) {
			posBarra.x += deltaTime * 5;
			if (!shoot) {
				posBola.x += deltaTime * 5;
			}
		}
	}
	// Disparo
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		if (!shoot) {
			shoot = true;
		}
	}
}

i32 main() {
	srand(time(NULL));
	GLFWwindow* window = glutilInit(3, 3, SCRWIDTH, SCRHEIGHT, "Breakout");
	Shader* lightingShader = new Shader("lightingmaps.vert", "lightingmaps.frag");
	Shader* lightCubeShader = new Shader("lightcube.vert", "lightcube.frag");

	cam = new Cam(-5.08283f, 0.225046f, 14.7233f);
	cam->setLook(glm::vec3(0.564965f, 0.00174536f, -0.825113f));
	glm::vec3 lightColor = glm::vec3(1.0f);

	Cube* cubex = new Cube(1.0f, 1.0f, 1.0f);

	Files* files = new Files("bin", "resources/textures", "resources/objects");
	Shader2* shaderModel = new Shader2(files, "shader.vert", "shader.frag");
	Model* ball = new Model(files, "ball/planet.obj");
	Model* alien = new Model(files, "alien/alien.obj");
	Model* nave = new Model(files, "nave/nave.obj");

	int count = 0;
	std::vector<glm::vec3> positions(4*15);
	std::vector<glm::vec3> estadosInter(4*15);
	std::vector<bool> estado(4*15);
	for (u32 i = 0; i < 4; ++i) {
		for (u32 j = 0; j < 15; ++j) {
			positions[count] = glm::vec3(j, i + 4.00, 0.0f);
			estadosInter[count] = glm::vec3(0.0);
			estado[count] = true;
			count++;
		}
	}

	glm::vec3 velocidad = glm::vec3(0.25f, 0.25f, 0.25f);
	glm::vec3 velocidad2 = glm::vec3(0.05f, 0.05f, 0.05f);
	f32 radio = 0.71;

	u32 cubeVao, lightCubeVao, vbo, ebo;
	glGenVertexArrays(1, &cubeVao);
	glGenVertexArrays(1, &lightCubeVao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(cubeVao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	glBufferData(GL_ARRAY_BUFFER, cubex->getVSize() * FSIZE,
		cubex->getVertices(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubex->getISize() * ISIZE,
		cubex->getIndices(), GL_STATIC_DRAW);

	// posiciones
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, cubex->len(), cubex->skip(0));
	glEnableVertexAttribArray(0);
	// normales
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, cubex->len(), cubex->skip(6));
	glEnableVertexAttribArray(1);
	// textures
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, cubex->len(), cubex->skip(9));
	glEnableVertexAttribArray(2);

	glBindVertexArray(lightCubeVao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, cubex->len(), cubex->skip(0));
	glEnableVertexAttribArray(0);

	glEnable(GL_DEPTH_TEST);

	unsigned int texture3 = lightingShader->loadTexture("oro.jpg");
	unsigned int texture2 = lightingShader->loadTexture("brick.jpg");
	unsigned int texture1 = lightingShader->loadTexture("waterEscamas.png");
	unsigned int texGameOver = lightingShader->loadTexture("gameover.jpg");

	while (!glfwWindowShouldClose(window)) {
		f32 currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 proj = glm::perspective(cam->getZoom(), ASPECT, 0.1f, 100.0f);
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 model3 = glm::mat4(1.0f);
		glBindVertexArray(cubeVao);

		for (u32 i = 0; i < positions.size(); ++i) {
			if (!estado[i]) continue;

			lightingShader->useProgram();
			glBindTexture(GL_TEXTURE_2D, texture3);
			lightingShader->setVec3("xyzMat.specular", 0.5f, 0.5f, 0.5f);
			lightingShader->setF32("xyzMat.shininess", 64.0f);

			lightingShader->setVec3("xyzLht.position", lightPos);
			lightingShader->setVec3("xyz", cam->getPos());

			lightingShader->setVec3("xyzLht.ambient", 0.2f, 0.2f, 0.2f);
			lightingShader->setVec3("xyzLht.diffuse", 0.5f, 0.5f, 0.5f);
			lightingShader->setVec3("xyzLht.specular", 1.0f, 1.0f, 1.0f);

			lightingShader->setMat4("proj", proj);
			lightingShader->setMat4("view", cam->getViewM4());

			model = glm::mat4(1.0f);
			model = glm::translate(model, positions[i]);
			model = glm::scale(model, glm::vec3(0.0005));
			lightingShader->setMat4("model", model);
			glDrawElements(GL_TRIANGLES, cubex->getISize(), GL_UNSIGNED_INT, 0);

			positions[i].x += velocidad2.x;
			if (positions[i].x >= 18 || positions[i].x <= -1) {
				velocidad2 *= -1;
			}

			bool colisionx = (positions[i].x + 0.5 > posBola.x - 0.025 && positions[i].x - 0.5 < posBola.x + 0.025);
			bool colisiony = (positions[i].y + 0.5 > posBola.y - 0.025 && positions[i].y - 0.5 < posBola.y + 0.025);
			bool colisionz = (positions[i].z + 0.5 > posBola.z - 0.025 && positions[i].z - 0.5 < posBola.z + 0.025);

			if (colisionx && colisiony && colisionz) {
				estado[i] = false;
				posBola.y = -4.50f;
				posBola.x = posBarra.x;
				shoot = false;
			}

			if (positions[i].y <= -4.00f) {
				lose = true;
			}
		}

		if (lose) {
			glBindTexture(GL_TEXTURE_2D, texGameOver);
			lightingShader->setVec3("xyzMat.specular", 0.5f, 0.5f, 0.5f);
			lightingShader->setF32("xyzMat.shininess", 64.0f);

			lightingShader->setVec3("xyzLht.position", lightPos);
			lightingShader->setVec3("xyz", cam->getPos());

			lightingShader->setVec3("xyzLht.ambient", 0.2f, 0.2f, 0.2f);
			lightingShader->setVec3("xyzLht.diffuse", 0.5f, 0.5f, 0.5f);
			lightingShader->setVec3("xyzLht.specular", 1.0f, 1.0f, 1.0f);

			lightingShader->setMat4("proj", proj);
			lightingShader->setMat4("view", cam->getViewM4());

			glm::mat4 modelGameOver = glm::mat4(1.0f);
			modelGameOver = glm::translate(modelGameOver, glm::vec3(5.0f, 0.0f, 0.0f));
			modelGameOver = glm::scale(modelGameOver, glm::vec3(7.0f));
			float theta = glfwGetTime();
			modelGameOver = glm::rotate(modelGameOver, theta, glm::vec3(1.0f, 1.0f, 1.0f));
			lightingShader->setMat4("model", modelGameOver);
			glDrawElements(GL_TRIANGLES, cubex->getISize(), GL_UNSIGNED_INT, 0);
		}

		glBindTexture(GL_TEXTURE_2D, texture2);
		lightingShader->setVec3("xyzMat.specular", 0.5f, 0.5f, 0.5f);
		lightingShader->setF32("xyzMat.shininess", 64.0f);

		lightingShader->setVec3("xyzLht.position", lightPos);
		lightingShader->setVec3("xyz", cam->getPos());

		lightingShader->setVec3("xyzLht.ambient", 0.2f, 0.2f, 0.2f);
		lightingShader->setVec3("xyzLht.diffuse", 0.5f, 0.5f, 0.5f);
		lightingShader->setVec3("xyzLht.specular", 1.0f, 1.0f, 1.0f);

		lightingShader->setMat4("proj", proj);
		lightingShader->setMat4("view", cam->getViewM4());

		model = glm::mat4(1.0f);
		model = glm::translate(model, posBarra);
		model = glm::scale(model, glm::vec3(0.05));
		lightingShader->setMat4("model", model);
		glDrawElements(GL_TRIANGLES, cubex->getISize(), GL_UNSIGNED_INT, 0);

		if (shoot) {
			posBola.y += velocidad.y;
			if (posBola.y >= 10.0f) {
				posBola.y = -4.50f;
				posBola.x = posBarra.x;
				shoot = false;
			}
		}

		glBindTexture(GL_TEXTURE_2D, texture1);
		lightingShader->setVec3("xyzMat.specular", 0.5f, 0.5f, 0.5f);
		lightingShader->setF32("xyzMat.shininess", 64.0f);

		lightingShader->setVec3("xyzLht.position", lightPos);
		lightingShader->setVec3("xyz", cam->getPos());

		lightingShader->setVec3("xyzLht.ambient", 0.2f, 0.2f, 0.2f);
		lightingShader->setVec3("xyzLht.diffuse", 0.5f, 0.5f, 0.5f);
		lightingShader->setVec3("xyzLht.specular", 1.0f, 1.0f, 1.0f);

		lightingShader->setMat4("proj", proj);
		lightingShader->setMat4("view", cam->getViewM4());

		model = glm::mat4(1.0f);
		model = glm::translate(model, posBola);
		model = glm::scale(model, glm::vec3(0.05));
		lightingShader->setMat4("model", model);
		glDrawElements(GL_TRIANGLES, cubex->getISize(), GL_UNSIGNED_INT, 0);

		lightCubeShader->useProgram();
		lightCubeShader->setMat4("proj", proj);
		lightCubeShader->setMat4("view", cam->getViewM4());
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.05));
		lightCubeShader->setMat4("model", model);

		glBindVertexArray(lightCubeVao);
		glDrawElements(GL_TRIANGLES, cubex->getISize(), GL_UNSIGNED_INT, 0);

		//model nave
		shaderModel->use();
		shaderModel->setVec3("xyz", lightPos);
		shaderModel->setVec3("xyzColor", lightColor);
		shaderModel->setVec3("xyzView", cam->getPos());
		shaderModel->setMat4("proj", proj);
		shaderModel->setMat4("view", cam->getViewM4());

		if (!win) {
			glm::mat4 modelNave = glm::mat4(1.0f);
			modelNave = translate(modelNave, posBarra);
			modelNave = glm::scale(modelNave, glm::vec3(0.25f));
			modelNave = glm::rotate(modelNave, 30.0f, glm::vec3(1.0f, 0.0f, 0.0f));
			shaderModel->setMat4("model", modelNave);
			nave->Draw(shaderModel);
		}

		//ball model
		if (!win) {
			glm::mat4 model2 = glm::mat4(1.0f);
			model2 = translate(model2, posBola);
			model2 = glm::scale(model2, glm::vec3(0.05f));
			shaderModel->setMat4("model", model2);
			ball->Draw(shaderModel);
		}

		// alien model
		for (u32 i = 0; i < positions.size(); ++i) {
			if (!estado[i]) continue;

			model3 = glm::mat4(1.0f);
			model3 = translate(model3, positions[i] - 0.5f);
			model3 = glm::scale(model3, glm::vec3(0.001f));
			shaderModel->setMat4("model", model3);
			alien->Draw(shaderModel);
		}

		// Baja aliens
		if (tiempo <= 0) {
			for (u32 i = 0; i < positions.size(); ++i) {
				if (!estado[i]) continue;
				positions[i].y = positions[i].y - 1.0;
			}
			tiempo = 400;
		}

		//tiempo
		tiempo--;

		if (win && !lose) {
			if (!frase) {
				std::cout << "WIN!!!!" << std::endl;
				frase = true;
			}

			glm::mat4 modelNave = glm::mat4(1.0f);
			modelNave = translate(modelNave, glm::vec3(1.0f));
			modelNave = glm::scale(modelNave, glm::vec3(5.0f,1.0f, 1.0f));
			float theta = glfwGetTime();
			modelNave = glm::rotate(modelNave, 30.0f, glm::vec3(1.0f, 0.0f, 0.0f));
			modelNave = glm::rotate(modelNave, theta, glm::vec3(0.0f, 0.0f, 1.0f));
			shaderModel->setMat4("model", modelNave);
			nave->Draw(shaderModel);

			lightColor.x = sin(5 * currentFrame * 2.0f);
			lightColor.y = sin(5 * currentFrame * 0.7f);
			lightColor.z = sin(5 * currentFrame * 1.3f);
		}

		// WIN
		int contAliens = 0;
		for (u32 i = 0; i < positions.size(); ++i) {
			if (!estado[i]) {
				contAliens++;
			}
			if (contAliens == 60) {
				win = true;
			}
		}
		contAliens = 0;

		// LOSE
		if (lose) {
			if (!frase) {
				std::cout << "GAME OVER" << std::endl;
				frase = true;
			}

			for (u32 i = 0; i < positions.size(); ++i) {
				if (estado[i]) {
					estado[i] = false;
				}
			}
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	};

	glDeleteVertexArrays(1, &cubeVao);
	glDeleteVertexArrays(1, &lightCubeVao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);

	delete lightingShader;
	delete lightCubeShader;
	delete cubex;
	delete cam;

	return 0;
}