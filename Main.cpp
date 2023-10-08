#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Shader.hpp"
#include "Mesh.hpp"
#include "Model.hpp"
#include "Camera.hpp"

// Global variables

// Screen size
const int SCR_WIDTH = 1280;
const int SCR_HEIGHT = 720;

// Frames
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Camera configs
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool firstMouse = true;
float lastX = 0.0f;
float lastY = 0.0f;
glm::vec3 mouseRay{};
glm::mat4 projection;
glm::mat4 view;
glm::mat4 model;
glm::vec3 modelPos;

// Point light controls
glm::vec3 lightPos{};

// Stencil Test Toggle
bool stencilTestToggle = false;
bool canEnableStencilTest = false;

// Callback and input handling functions
void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
glm::vec3 getMouseRayDirection();

int main() {
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Model Testing", NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	// Setting callbacks
	// Window resize
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	// Cursor position
	glfwSetCursorPosCallback(window, mouse_callback);
	// Scroll movement
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	stbi_set_flip_vertically_on_load(true);

	Model loadedModel = Model("assets/models/backpack.obj");

	Shader shader = Shader("assets\\shaders\\model_loading.vs", "assets\\shaders\\model_loading.fs");

	shader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.0f);

	shader.setVec3("dirLight.ambient", 0.2f, 0.2f, 0.2f);
	shader.setVec3("dirLight.diffuse", 0.5f, 0.5f, 0.5f);
	shader.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);

	Shader stencilShader("assets\\shaders\\stencil_shader.vs", "assets\\shaders\\stencil_shader.fs");

	while (!glfwWindowShouldClose(window)) {
		// per-frame time logic
		 // --------------------
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.setVec3("pointLight.position", lightPos);

		stencilShader.use();
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		view = camera.GetViewMatrix();
		stencilShader.setMat4("projection", projection);
		stencilShader.setMat4("view", view);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));

		shader.use();

		shader.setVec3("viewPos", camera.Position);
		shader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);

		// light properties
		shader.setVec3("dirLight.ambient", 0.2f, 0.2f, 0.2f);
		shader.setVec3("dirLight.diffuse", 0.5f, 0.5f, 0.5f);
		shader.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);

		shader.setMat4("projection", projection);
		shader.setMat4("view", view);

		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);

		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
		shader.setMat4("model", model);
		loadedModel.Draw(shader);

		if (stencilTestToggle) {
			// After drawting the object with the Stencil test on,
			// we disable the Stencil test and draw the outline.
			glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
			glStencilMask(0x00); // disable writing to the stencil buffer
			glDisable(GL_DEPTH_TEST);

			stencilShader.use();
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
			model = glm::scale(model, glm::vec3(1.1f, 1.1f, 1.1f));
			stencilShader.setMat4("model", model);
			loadedModel.Draw(stencilShader);
		}

		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 0, 0xFF);
		glEnable(GL_DEPTH_TEST);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}

glm::vec3 getMouseRayDirection() {
	// Normalize the mouse coordinates
	float normalizedX = (2 * lastX) / SCR_WIDTH - 1.0f;
	// TODO(Ruan): If this does not work, it's because some libraries calculate the mouse
	// coordinates from the bottom left to top right, meaning Y should be NEGATIVE and not positive.
	float normalizedY = (2 * lastY) / SCR_HEIGHT - 1.0f;

	// Clip space 
	glm::vec4 clipSpaceCoordinates = glm::vec4(normalizedX, -normalizedY, -1.0f, 1.0f);

	// Convert from clip space to eye space
	glm::mat4 inverseProjectionMatrix = glm::inverse(projection);

	// After obtaining the eye coordinates, we'll use only the x and y components of the vector (z: -1, w: 0)
	glm::vec4 eyeCoords = (inverseProjectionMatrix * clipSpaceCoordinates);
	eyeCoords.z = -1.0;
	eyeCoords.w = 0.0f;

	glm::mat4 inverseViewMatrix = glm::inverse(view);
	
	glm::vec4 worldCoords = inverseViewMatrix * eyeCoords;

	return glm::normalize(glm::vec3(worldCoords.x, worldCoords.y, worldCoords.z));
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		lightPos.y += 0.1f;

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		lightPos.y -= 0.1f;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
		if (canEnableStencilTest) {
			stencilTestToggle = stencilTestToggle ? false : true;
		}
		else
		{
			stencilTestToggle = false;
		}
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	//camera.ProcessMouseMovement(xoffset, yoffset);

	// Calculate mouse ray
	glm::vec3 mouseRayDirection = getMouseRayDirection();

	// std::cout << mouseRayDirection.x << ", " << mouseRayDirection.<< ", " << mouseRayDirection.z << std::endl;

	float dot = glm::dot(mouseRayDirection, glm::vec3(1.0f));

	std::cout << dot << std::endl;

    canEnableStencilTest = dot < -0.4f && dot > -1.2f;

}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}