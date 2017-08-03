#include "stdafx.h"
#include "glad.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <time.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>
#include <list>
#include <iostream>

#define NUM_LIGHTS 4

bool isIntersect(glm::mat4 projection1, glm::mat4 view1, glm::mat4 projection2, glm::mat4 view2);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
void renderScene(const Shader &shader);
void renderCube();
void renderQuad();

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// meshes
unsigned int planeVAO;

unsigned int quadVAO = 0;
unsigned int quadVBO;

int temp = 1024;

GLenum retTexNumber(int i)
{
	if (i == 0)
		return GL_TEXTURE0;
	else if (i == 1)
		return GL_TEXTURE1;
	else if (i == 2)
		return GL_TEXTURE2;
	else if (i == 3)
		return GL_TEXTURE3;
	else if (i == 4)
		return GL_TEXTURE4;
	else if (i == 5)
		return GL_TEXTURE5;
	else if (i == 6)
		return GL_TEXTURE6;
	else if (i == 7)
		return GL_TEXTURE7;
	else if (i == 8)
		return GL_TEXTURE8;
	else if (i == 9)
		return GL_TEXTURE9;
	else if (i == 10)
		return GL_TEXTURE10;
}

class Graph
{
	int V;
	list<int> *adj;
public:

	Graph(int V) { this->V = V; adj = new list<int>[V]; }
	~Graph() { delete[] adj; }


	void addEdge(int v, int w);


	int greedyColoring(int layersAssigned[NUM_LIGHTS]);
};

void Graph::addEdge(int v, int w)
{
	adj[v].push_back(w);
	adj[w].push_back(v);
}


int Graph::greedyColoring(int layersAssigned[NUM_LIGHTS])
{
	int* result = new int[V];
	int assigned = 0;

	result[0] = 0;
	layersAssigned[0] = 0;

	for (int u = 1; u < V; u++)
		result[u] = -1;


	bool* available = new bool[V];
	for (int cr = 0; cr < V; cr++)
		available[cr] = false;

	for (int u = 1; u < V; u++)
	{
		list<int>::iterator i;
		for (i = adj[u].begin(); i != adj[u].end(); ++i)
			if (result[*i] != -1)
				available[result[*i]] = true;

		int cr;
		for (cr = 0; cr < V; cr++)
			if (available[cr] == false)
				break;

		result[u] = cr;
		layersAssigned[u] = cr;
		if (cr > assigned)
			assigned = cr;
		for (i = adj[u].begin(); i != adj[u].end(); ++i)
			if (result[*i] != -1)
				available[result[*i]] = false;
	}

	for (int u = 0; u < V; u++)
		cout << "Vertex " << u << " --->  Color "
		<< result[u] << endl;

	return assigned+1;

}


int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Screen Space Soft Shadows", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSwapInterval(0);

	// tell GLFW to capture our mouse
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	

	// build and compile shaders
	// -------------------------
	Shader shader("3.1.2.shadow_mapping.vs", "3.1.2.shadow_mapping.fs");
	Shader simpleDepthShader("3.1.2.shadow_mapping_depth.vs", "3.1.2.shadow_mapping_depth.fs");
	Shader debugDepthQuad("3.1.2.debug_quad.vs", "3.1.2.debug_quad_depth.fs");
	Shader dilatedShadowMap("dilated_shadow_map.vs", "dilated_shadow_map.fs");
	Shader calculateBuffers("calculate_buffers.vs", "calculate_buffers.fs");
	Shader GaussBlurShader("GaussianBlur.vs", "GaussianBlur.fs");
	Shader GaussFinalShader("GaussFinal.vs", "GaussFinal.fs");
	Shader CombineShadowsShader("addShadows.vs", "addShadows.fs");
	Shader penumbraSizeShader("calculatePenumbraSize.vs", "calculatePenumbraSize.fs");
	Shader deferredLightingShader("DeferredLighting.vs", "DeferredLighting.fs");
	Shader copyTextureShader("CopyTexture.vs", "CopyTexture.fs");

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float planeVertices[] = {
		// positions            // normals         // texcoords
		25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

		25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
		25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
	};
	// plane VAO
	unsigned int planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);

	// load textures
	// -------------
	unsigned int woodTexture = loadTexture("Dark Yellow.png");

	// configure depth map FBO
	// -----------------------
	const unsigned int SHADOW_WIDTH = SCR_WIDTH, SHADOW_HEIGHT = SCR_HEIGHT;
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, temp, temp, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	unsigned int dilatedDepthMapFBO;
	glGenFramebuffers(1, &dilatedDepthMapFBO);
	unsigned int singleDilatedDepthMap;
	glGenTextures(1, &singleDilatedDepthMap);
	glBindTexture(GL_TEXTURE_2D, singleDilatedDepthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, temp / 10, temp, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	
	unsigned int dilatedDepthMap;
	glGenTextures(1, &dilatedDepthMap);
	glBindTexture(GL_TEXTURE_2D, dilatedDepthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, temp / 10, temp / 10, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	unsigned int bufferFBO;
	glGenFramebuffers(1, &bufferFBO);

	unsigned int tempFBO;
	glGenFramebuffers(1, &tempFBO);

	unsigned int Gauss1FBO;
	glGenFramebuffers(1, &Gauss1FBO);

	unsigned int GaussFinalFBO;
	glGenFramebuffers(1, &GaussFinalFBO);

	unsigned int combineShadowsFBO;
	glGenFramebuffers(1, &combineShadowsFBO);

	unsigned int accumulateTexturesFBO;
	glGenFramebuffers(1, &accumulateTexturesFBO);

	unsigned int shadowBuffer;
	glGenTextures(1, &shadowBuffer);
	glBindTexture(GL_TEXTURE_2D, shadowBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, SCR_WIDTH, SCR_HEIGHT, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	unsigned int distanceMap;
	glGenTextures(1, &distanceMap);
	glBindTexture(GL_TEXTURE_2D, distanceMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	unsigned int normalDepthBuffer;
	glGenTextures(1, &normalDepthBuffer);
	glBindTexture(GL_TEXTURE_2D, normalDepthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	unsigned int Gauss1Result;
	glGenTextures(1, &Gauss1Result);
	glBindTexture(GL_TEXTURE_2D, Gauss1Result);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	unsigned int GaussFinalResult;
	glGenTextures(1, &GaussFinalResult);
	glBindTexture(GL_TEXTURE_2D, GaussFinalResult);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	unsigned int CombinedShadows;
	glGenTextures(1, &CombinedShadows);
	glBindTexture(GL_TEXTURE_2D, CombinedShadows);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	unsigned int accumulatedShadowMaps;
	glGenTextures(1, &accumulatedShadowMaps);
	glBindTexture(GL_TEXTURE_2D, accumulatedShadowMaps);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	unsigned int accumulatedPenumbraSizes;
	glGenTextures(1, &accumulatedPenumbraSizes);
	glBindTexture(GL_TEXTURE_2D, accumulatedPenumbraSizes);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	unsigned int GaussLayerResult[NUM_LIGHTS];
	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		glGenTextures(1, &GaussLayerResult[i]);
		glBindTexture(GL_TEXTURE_2D, GaussLayerResult[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadowBuffer, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, distanceMap, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, normalDepthBuffer, 0);

	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, Gauss1FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Gauss1Result, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, GaussFinalFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, GaussFinalResult, 0);
	GLuint depthrenderbuffer2;
	glGenRenderbuffers(1, &depthrenderbuffer2);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer2);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, accumulateTexturesFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumulateTexturesFBO, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, combineShadowsFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, CombinedShadows, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	unsigned int penumbraSizeFBO;
	glGenFramebuffers(1, &penumbraSizeFBO);


	Graph g(NUM_LIGHTS);
	glm::vec3 lightPos[NUM_LIGHTS];
	float lightSize[NUM_LIGHTS];
	glm::vec3 lightColor[NUM_LIGHTS];
	float linearAttn[NUM_LIGHTS];
	float quadraticAttn[NUM_LIGHTS];
	glm::mat4 lightProjection[NUM_LIGHTS], lightView[NUM_LIGHTS];
	int layersAssigned[NUM_LIGHTS];

	float near_plane = 1.0f, far_plane = 7.5f;
	//Define the lights below
	lightSize[0] = 0.1f;
	lightPos[0] = (glm::vec3(-18.0f, 4.0f, -17.0f));
	lightProjection[0] = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightView[0] = glm::lookAt(lightPos[0], glm::vec3(-15.0f, 0.0f, -15.0f), glm::vec3(0.0, 1.0, 0.0));
	lightColor[0] = glm::vec3(0.3f);
	linearAttn[0] = 0.025f;
	quadraticAttn[0] = 0.025f;

	lightSize[1] = 0.15f;
	lightPos[1] = (glm::vec3(20.0f, 4.0f, 10.0f));
	lightProjection[1] = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightView[1] = glm::lookAt(lightPos[1], glm::vec3(17.0f, 0.0f, 11.0f), glm::vec3(0.0, 1.0, 0.0));
	lightColor[1] = glm::vec3(0.4f, 0.0f, 0.3f);
	linearAttn[1] = 0.025f;;
	quadraticAttn[1] = 0.025f;

	lightSize[2] = 0.1f;
	lightPos[2] = (glm::vec3(2.0f, 5.0f, -1.0f));
	lightProjection[2] = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightView[2] = glm::lookAt(lightPos[2], glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
	lightColor[2] = glm::vec3(0.3f, 0.0f, 0.0f);
	linearAttn[2] = 0.025f;
	quadraticAttn[2] = 0.025f;

	lightSize[3] = 0.2f;
	lightPos[3] = (glm::vec3(2.0f, 4.0f, 1.0f));
	lightProjection[3] = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightView[3] = glm::lookAt(lightPos[3], glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
	lightColor[3] = glm::vec3(0.3f);
	linearAttn[3] = 0.025f;
	quadraticAttn[3] = 0.025f;

	/*lightSize[4] = 0.15f;
	lightPos[4] = (glm::vec3(-19.0f, 4.0f, -15.0f));
	lightProjection[4] = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightView[4] = glm::lookAt(lightPos[4], glm::vec3(-15.0f, 0.0f, -15.0f), glm::vec3(0.0, 1.0, 0.0));
	lightColor[4] = glm::vec3(0.3f);
	linearAttn[4] = 0.025f;
	quadraticAttn[4] = 0.025f;

	lightSize[5] = 0.05f;
	lightPos[5] = (glm::vec3(-150.0f, 4.0f, -17.0f));
	lightProjection[5] = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightView[5] = glm::lookAt(lightPos[5], glm::vec3(-15.0f, 0.0f, -15.0f), glm::vec3(0.0, 1.0, 0.0));
	lightColor[5] = glm::vec3(0.2f, 0.3f, 0.0f);
	linearAttn[5] = 0.025f;
	quadraticAttn[5] = 0.025f;

	lightSize[6] = 0.1f;
	lightPos[6] = (glm::vec3(-16.0f, 4.0f, -160.0f));
	lightProjection[6] = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightView[6] = glm::lookAt(lightPos[6], glm::vec3(-15.0f, 0.0f, -15.0f), glm::vec3(0.0, 1.0, 0.0));
	lightColor[6] = glm::vec3(0.7f);
	linearAttn[6] = 0.025f;
	quadraticAttn[6] = 0.025f;

	lightSize[7] = 0.1f;
	lightPos[7] = (glm::vec3(180.0f, 4.0f, 11.0f));
	lightProjection[7] = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightView[7] = glm::lookAt(lightPos[7], glm::vec3(17.0f, 0.0f, 11.0f), glm::vec3(0.0, 1.0, 0.0));
	lightColor[7] = glm::vec3(0.1f, 0.4f, 0.3f);
	linearAttn[7] = 0.025f;;
	quadraticAttn[7] = 0.025f;

	lightSize[8] = 0.2f;
	lightPos[8] = (glm::vec3(220.0f, 4.0f, 120.0f));
	lightProjection[8] = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightView[8] = glm::lookAt(lightPos[8], glm::vec3(17.0f, 0.0f, 11.0f), glm::vec3(0.0, 1.0, 0.0));
	lightColor[8] = glm::vec3(0.3f, 0.4f, 0.0f);
	linearAttn[8] = 0.025f;;
	quadraticAttn[8] = 0.025f;

	lightSize[9] = 0.1f;
	lightPos[9] = (glm::vec3(25.0f, 4.0f, 20.0f));
	lightProjection[9] = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightView[9] = glm::lookAt(lightPos[9], glm::vec3(17.0f, 0.0f, 11.0f), glm::vec3(0.0, 1.0, 0.0));
	lightColor[9] = glm::vec3(0.3f, 0.4f, 0.0f);
	linearAttn[9] = 0.025f;;
	quadraticAttn[9] = 0.025f;*/

	////lightSize[0] = 0.1f;
	////lightPos[0] = (glm::vec3(3.0f, 4.0f, 1.0f));
	////lightProjection[0] = glm::perspective(glm::radians(90.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, near_plane, far_plane);
	////lightView[0] = glm::lookAt(lightPos[0], lightPos[0] + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	////lightColor[0] = glm::vec3(0.3f);
	////linearAttn[0] = 0.025f;
	////quadraticAttn[0] = 0.025f;

	////lightSize[1] = 0.1f;
	////lightPos[1] = (glm::vec3(3.0f, 4.0f, 1.0f));
	////lightProjection[1] = glm::perspective(glm::radians(90.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, near_plane, far_plane);
	////lightView[1] = glm::lookAt(lightPos[1], lightPos[1] + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	////lightColor[1] = glm::vec3(0.3f);
	////linearAttn[1] = 0.025f;
	////quadraticAttn[1] = 0.025f;

	////lightSize[2] = 0.1f;
	////lightPos[2] = (glm::vec3(3.0f, 4.0f, 1.0f));
	////lightProjection[2] = glm::perspective(glm::radians(90.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, near_plane, far_plane);
	////lightView[2] = glm::lookAt(lightPos[2], lightPos[2] + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	////lightColor[2] = glm::vec3(0.3f);
	////linearAttn[2] = 0.025f;
	////quadraticAttn[2] = 0.025f;

	////lightSize[3] = 0.1f;
	////lightPos[3] = (glm::vec3(3.0f, 4.0f, 1.0f));
	////lightProjection[3] = glm::perspective(glm::radians(90.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, near_plane, far_plane);
	////lightView[3] = glm::lookAt(lightPos[3], lightPos[3] + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	////lightColor[3] = glm::vec3(0.3f);
	////linearAttn[3] = 0.025f;
	////quadraticAttn[3] = 0.025f;

	//lightSize[4] = 0.1f;
	//lightPos[4] = (glm::vec3(3.0f, 4.0f, 1.0f));
	//lightProjection[4] = glm::perspective(glm::radians(90.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, near_plane, far_plane);
	//lightView[4] = glm::lookAt(lightPos[4], lightPos[4] + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	//lightColor[4] = glm::vec3(0.3f);
	//linearAttn[4] = 0.025f;
	//quadraticAttn[4] = 0.025f;

	////lightSize[5] = 0.1f;
	////lightPos[5] = (glm::vec3(3.0f, 4.0f, 1.0f));
	////lightProjection[5] = glm::perspective(glm::radians(90.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, near_plane, far_plane);
	////lightView[5] = glm::lookAt(lightPos[5], lightPos[5] + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	////lightColor[5] = glm::vec3(0.3f);
	////linearAttn[5] = 0.025f;
	////quadraticAttn[5] = 0.025f;

	for (int l = 0; l < NUM_LIGHTS; l++)
	{
		printf("Projection matrix of light %d\n", l);

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				printf("%f, ", lightProjection[0][i][j]);
			}
			printf("\n");
		}
		printf("\n");

		printf("View matrix of light %d\n", l);


		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				printf("%f, ", lightView[0][i][j]);
			}
			printf("\n");
		}
		printf("\n");
	}
	

	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		for (int j = i + 1; j < NUM_LIGHTS; j++)
		{
			if (isIntersect(lightProjection[i], lightView[i], lightProjection[j], lightView[j]))
			{
				g.addEdge(i, j);
				printf("Lights %d and %d intersect\n", i, j);
			}
		}
	}
	int numLayers = g.greedyColoring(layersAssigned);
	printf("Total layers assigned: %d\n", numLayers);
	unsigned int penumbraSize;
	
	glGenTextures(1, &penumbraSize);
	glBindTexture(GL_TEXTURE_2D, penumbraSize);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	int cnt = 0;
	float timeElapsed = 0.0f;
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		timeElapsed += deltaTime;
		cnt++;
		if (cnt == 100)
		{
			float freq = 100.0 / timeElapsed;
			printf("Frequency: %f\n", freq);
			timeElapsed = 0.0f;
			cnt = 0;
		}

		processInput(window);
		glm::mat4 projection;
		glm::mat4 view;
		projection = glm::perspective(camera.Zoom, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		view = camera.GetViewMatrix();

		lightPos[2].x = sin(glfwGetTime()) * 3.0f;
		lightPos[2].z = cos(glfwGetTime()) * 2.0f;
		lightProjection[2] = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		lightView[2] = glm::lookAt(lightPos[2], glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));

		glm::mat4 lightSpaceMatrix[NUM_LIGHTS];
		for (int i = 0; i < NUM_LIGHTS; i++)
		{
			lightSpaceMatrix[i] = lightProjection[i] * lightView[i];
		}

		for (int i = 0; i < numLayers; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, accumulateTexturesFBO);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumulatedShadowMaps, 0);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glBindFramebuffer(GL_FRAMEBUFFER, accumulateTexturesFBO);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumulatedPenumbraSizes, 0);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glBindFramebuffer(GL_FRAMEBUFFER, penumbraSizeFBO);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			for (int j = 0; j < NUM_LIGHTS; j++)
			{
				if (layersAssigned[j] == i && isIntersect(lightProjection[j], lightView[j], projection, view))
				{
					//Calculate Depth Map
					glEnable(GL_DEPTH_TEST);
					glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
					simpleDepthShader.use();
					simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix[j]);
					glViewport(0, 0, temp, temp);
					glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
					glClear(GL_DEPTH_BUFFER_BIT);
					renderScene(simpleDepthShader);
					glBindFramebuffer(GL_FRAMEBUFFER, 0);

					//glDisable(GL_DEPTH_TEST);


					//Dilate the depth map
					debugDepthQuad.use();
					debugDepthQuad.setInt("depthMap", 0);

					glViewport(0, 0, temp / 10, temp);
					dilatedShadowMap.use();
					dilatedShadowMap.setInt("depth_map", 0);
					dilatedShadowMap.setFloat("lightSize",lightSize[j]);
					glBindFramebuffer(GL_FRAMEBUFFER, dilatedDepthMapFBO);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, singleDilatedDepthMap, 0);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, depthMap);
					dilatedShadowMap.setVec2("dir", 1, 0);
					renderQuad();
					glViewport(0, 0, temp / 10, temp / 10);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dilatedDepthMap, 0);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, singleDilatedDepthMap);
					dilatedShadowMap.setVec2("dir", 0, 1);
					renderQuad();
					glBindFramebuffer(GL_FRAMEBUFFER, 0);


					//Calculate Buffers
					//glEnablei(GL_BLEND, 0);
					//glBlendFunc(GL_ONE, GL_ONE);
					//glBlendEquation(GL_ADD);
					glEnable(GL_DEPTH_TEST);
					glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
					calculateBuffers.use();
					glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
					glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
					glDrawBuffers(3, buffers);
					calculateBuffers.setMat4("projection", projection);
					calculateBuffers.setMat4("view", view);
					calculateBuffers.setInt("shadowMap", 0);
					calculateBuffers.setInt("dilatedShadowMap", 1);
					calculateBuffers.setVec3("viewPos", camera.Position);
					calculateBuffers.setVec3("lightPos", lightPos[j]);
					calculateBuffers.setMat4("lightSpaceMatrix", lightSpaceMatrix[j]);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, depthMap);
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, dilatedDepthMap);
					renderScene(calculateBuffers);
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
					//glDisablei(GL_BLEND, 0);

					//Additively Blend the Shadow Map
					glEnable(GL_BLEND);
					glBlendFunc(GL_ONE, GL_ONE);
					glBlendEquation(GL_ADD);
					copyTextureShader.use();
					glBindFramebuffer(GL_FRAMEBUFFER, accumulateTexturesFBO);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumulatedShadowMaps, 0);
					copyTextureShader.setInt("inputTexture", 0);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, shadowBuffer);
					renderQuad();
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
					glDisable(GL_BLEND);

					//Calculate penumbra sizes
					glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
					glBindFramebuffer(GL_FRAMEBUFFER, penumbraSizeFBO);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, penumbraSize, 0);
					glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					penumbraSizeShader.use();
					penumbraSizeShader.setMat4("projection", projection);
					penumbraSizeShader.setMat4("view", view);
					penumbraSizeShader.setFloat("lightSize", lightSize[j]);
					penumbraSizeShader.setMat4("lightSpaceMatrix", lightSpaceMatrix[j]);
					penumbraSizeShader.setInt("NormalDepth", 0);
					penumbraSizeShader.setInt("DistanceMap", 1);
					penumbraSizeShader.setInt("DilatedDepthMap", 2);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, normalDepthBuffer);
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, distanceMap);
					glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_2D, dilatedDepthMap);
					renderScene(penumbraSizeShader);
					glBindFramebuffer(GL_FRAMEBUFFER, 0);

					//Additively Blend the Penumbra Size Maps
					glEnable(GL_BLEND);
					glBlendFunc(GL_ONE, GL_ONE);
					glBlendEquation(GL_ADD);
					copyTextureShader.use();
					glBindFramebuffer(GL_FRAMEBUFFER, accumulateTexturesFBO);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumulatedPenumbraSizes, 0);
					copyTextureShader.setInt("inputTexture", 0);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, penumbraSize);
					renderQuad();
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
					glDisable(GL_BLEND);
				}
			}
				
			//Blur shadow map
			//Penumbra Size Texture also contains step sizes
			//Shadow map also contains distance from viewer to pixel
			glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, Gauss1FBO);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Gauss1Result, 0);
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			GaussBlurShader.use();
			GaussBlurShader.setInt("ShadowMap", 0);
			GaussBlurShader.setInt("PenumbraSizeMap", 1);
			GaussBlurShader.setInt("DistanceMap", 2);
			GaussBlurShader.setMat4("projection", projection);
			GaussBlurShader.setMat4("view", view);
			GaussBlurShader.setMat4("lightSpaceMatrix", lightSpaceMatrix[i]);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, accumulatedShadowMaps);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, accumulatedPenumbraSizes);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, distanceMap);
			float x = 0.89442719082100156952748325334158 * 1.11803398875;
			float y = 0.44721359585778655717526397765935 * 1.11803398875;
			glm::vec2 direction = glm::vec2(x, y);
			GaussBlurShader.setVec2("direction", direction);
			renderScene(GaussBlurShader);
			
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, GaussLayerResult[i], 0);
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			float x2 = -0.44721359585778655717526397765935 * 1.11803398875;
			float y2 = 0.89442719082100156952748325334158 * 1.11803398875;
			glm::vec2 direction2 = glm::vec2(x2, y2);
			GaussBlurShader.setVec2("direction", direction2);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Gauss1Result);
			renderScene(GaussBlurShader);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);


		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		deferredLightingShader.use();
		deferredLightingShader.setInt("anumLights", NUM_LIGHTS);
		deferredLightingShader.setMat4("projection", projection);
		deferredLightingShader.setMat4("view", view);
		deferredLightingShader.setVec3("viewPos", camera.Position);
		for (int i = 0; i < NUM_LIGHTS; i++)
		{
			deferredLightingShader.setVec3(string("lightPositions[") + string(std::to_string(i)) + string("]"), lightPos[i]);
			deferredLightingShader.setFloat(string("linearAttn[") + string(std::to_string(i)) + string("]"), linearAttn[i]);
			deferredLightingShader.setFloat(string("quadraticAttn[") + string(std::to_string(i)) + string("]"), quadraticAttn[i]);
			deferredLightingShader.setMat4(string("lightSpaceMatrix[") + string(std::to_string(i)) + string("]"), lightSpaceMatrix[i]);
			deferredLightingShader.setVec3(string("lightColors[") + string(std::to_string(i)) + string("]"), lightColor[i]);
			deferredLightingShader.setInt(string("assignedLayer[") + string(std::to_string(i)) + string("]"), layersAssigned[i]);
		}

		//deferredLightingShader.setVec3("lightPositions[0]", lightPos[0]);
		//deferredLightingShader.setFloat("linearAttn[0]", linearAttn[0]);
		//deferredLightingShader.setFloat("quadraticAttn[0]", quadraticAttn[0]);
		//deferredLightingShader.setMat4("lightSpaceMatrix[0]", lightSpaceMatrix[0]);
		//deferredLightingShader.setVec3("lightColors[0]", lightColor[0]);
		//deferredLightingShader.setInt("assignedLayer[0]", layersAssigned[0]);

		for (int i = 0; i < numLayers; i++)
		{
			deferredLightingShader.setInt(string("layerShadowMaps[") + string(std::to_string(i)) + string("]"), i);
			glActiveTexture(retTexNumber(i));
			glBindTexture(GL_TEXTURE_2D, GaussLayerResult[i]);
		}
		deferredLightingShader.setInt("diffuseTexture", numLayers);
		glActiveTexture(retTexNumber(numLayers));
		glBindTexture(GL_TEXTURE_2D, woodTexture);

		//deferredLightingShader.setInt("layerShadowMaps[0]", 0);
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, GaussLayerResult[0]);

		//deferredLightingShader.setInt("diffuseTexture", 1);
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, woodTexture);

		renderScene(deferredLightingShader);
		
		//glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//// render Depth map to quad for visual debugging
		//// ---------------------------------------------
		//debugDepthQuad.use();
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, shadowBuffer);
		//renderQuad();
	//////glm::mat4 vp = lightProjection[0] * lightView[0];

	////for (int i = 0; i < 4; i++)
	////{
	////	for (int j = 0; j < 4; j++)
	////	{
	////		printf("%f, ", glm::inverse(lightView[0])[i][j]);
	////	}
	////	printf("\n");
	////}

	////glm::mat4 tmp = glm::inverse(lightView[0]) * glm::inverse(lightProjection[0]);

	////bool intr = isIntersect(lightProjection[0], lightView[0], lightProjection[0], lightView[0]);


	////glm::vec4 te = tmp * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	////printf("\n%f, %f, %f, %f\n", te.x, te.y, te.z, te.w);
	////te = glm::inverse(lightView[0]) * (glm::inverse(lightProjection[0]) * glm::vec4(1.0f, 1.0f, -1.0f, 1.0f));
	////printf("\n%f, %f, %f, %f\n", te.x, te.y, te.z, te.w);
	////te = glm::inverse(lightView[0]) * (glm::inverse(lightProjection[0]) * glm::vec4(1.0f, -1.0f, 1.0f, 1.0f));
	////printf("\n%f, %f, %f, %f\n", te.x, te.y, te.z, te.w);
	////te = tmp * glm::vec4(1.0f, -1.0f, -1.0f, 1.0f);
	////printf("\n%f, %f, %f, %f\n", te.x, te.y, te.z, te.w);
	////te = tmp * glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f);
	////printf("\n%f, %f, %f, %f\n", te.x, te.y, te.z, te.w);
	////te = glm::inverse(lightView[0]) * (glm::inverse(lightProjection[0]) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f));
	////printf("\n%f, %f, %f, %f\n", te.x, te.y, te.z, te.w);
	////te = tmp * glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f);
	////printf("\n%f, %f, %f, %f\n", te.x, te.y, te.z, te.w);
	////te = glm::inverse(lightView[0]) * (glm::inverse(lightProjection[0]) * glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f));
	////printf("\n%f, %f, %f, %f\n", te.x, te.y, te.z, te.w);
	//int firstTime = 1;
	//// render loop
	//// -----------



	//	for (int i = 0; i < NUM_LIGHTS; i++)
	//	{

	//		clock_t t = clock();

	//		double sec = t / (double)CLOCKS_PER_SEC;
	//		//glm::vec3 lightPos((float)sin(sec), (float)cos(sec), -1.0f);
	//		// per-frame time logic	
	//		// --------------------

	//		// input
	//		// -----
	//		// change light position over time

	//		////lightPos.x = -2.5f;
	//		////lightPos.z = 0.2f;
	//		//lightPos.y = 5.0 + cos(glfwGetTime()) * 1.0f;
	//		// shader configuration
	//		// --------------------

	//		//printf("%lf, %lf\n", lightPos.x, lightPos.z);
	//		shader.use();
	//		shader.setInt("diffuseTexture", 0);
	//		shader.setInt("shadowMap", 1);




	//		// render
	//		// ------
	//		//Apply Minfilter to depth map
	//		// reset viewport




	//		//// 2. render scene as normal using the generated depth/shadow map  
	//		//// --------------------------------------------------------------
	//		//glBindFramebuffer(GL_FRAMEBUFFER, tempFBO);
	//		//shader.use();

	//		//shader.setMat4("projection", projection);
	//		//shader.setMat4("view", view);
	//		//// set light uniforms
	//		//shader.setVec3("viewPos", camera.Position);
	//		//shader.setVec3("lightPos", lightPos[i]);
	//		//shader.setMat4("lightSpaceMatrix", lightSpaceMatrix[i]);
	//		//glActiveTexture(GL_TEXTURE0);
	//		//glBindTexture(GL_TEXTURE_2D, woodTexture);
	//		//glActiveTexture(GL_TEXTURE1);
	//		//glBindTexture(GL_TEXTURE_2D, depthMap);
	//		//renderScene(shader);
	//		//glBindFramebuffer(GL_FRAMEBUFFER, 0);




	//	}
	//	glBindFramebuffer(GL_FRAMEBUFFER, combineShadowsFBO);
	//	CombineShadowsShader.use();
	//	CombineShadowsShader.setInt("numLights", NUM_LIGHTS);
	//	for (int i = 0; i < NUM_LIGHTS; i++)
	//	{
	//		CombineShadowsShader.setInt(string("lightShadowMaps[") + string(std::to_string(i)) + string("]"), i);
	//		//CombineShadowsShader.setInt("lightShadowMaps[0]", 0);
	//		glActiveTexture(retTexNumber(i));
	//		glBindTexture(GL_TEXTURE_2D, GaussLayerResult[i]);
	//	}
	//	renderQuad();
	//	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//	glBindFramebuffer(GL_FRAMEBUFFER, GaussFinalFBO);
	//	//glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//	GaussFinalShader.use();
	//	GaussFinalShader.setMat4("projection", projection);
	//	GaussFinalShader.setMat4("view", view);
	//	GaussFinalShader.setVec3("viewPos", camera.Position);
	//	GaussFinalShader.setVec3("lightPos", lightPos[0]);
	//	GaussFinalShader.setInt("FinalShadowMap", 0);
	//	GaussFinalShader.setInt("diffuseTexture", 1);
	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, CombinedShadows);
	//	glActiveTexture(GL_TEXTURE1);
	//	glBindTexture(GL_TEXTURE_2D, woodTexture);
	//	renderScene(GaussFinalShader);
	//	glBindFramebuffer(GL_FRAMEBUFFER, 0);



		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &planeVBO);

	glfwTerminate();
	return 0;
}

float retDistance(glm::vec3 a, glm::vec3 b)
{
	return sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y) + (b.z - a.z) * (b.z - a.z));
}

bool isIntersect(glm::mat4 projection1, glm::mat4 view1, glm::mat4 projection2, glm::mat4 view2)
{
	glm::vec4 C1NearLowLeft, C1FarTopRight, C2NearLowLeft, C2FarTopRight;

	glm::mat4 C1InvVP = glm::inverse(view1) * glm::inverse(projection1);
	glm::mat4 C2InvVP = glm::inverse(view2) * glm::inverse(projection2);

	C1NearLowLeft = C1InvVP * glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f);
	C1FarTopRight = C1InvVP * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	C2NearLowLeft = C2InvVP * glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f);
	C2FarTopRight = C2InvVP * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	glm::vec3 C1Center = glm::vec3((C1NearLowLeft.x + C1FarTopRight.x)/2.0f, (C1NearLowLeft.y + C1FarTopRight.y) / 2.0f, (C1NearLowLeft.z + C1FarTopRight.z) / 2.0f);
	glm::vec3 C2Center = glm::vec3((C2NearLowLeft.x + C2FarTopRight.x) / 2.0f, (C2NearLowLeft.y + C2FarTopRight.y) / 2.0f, (C2NearLowLeft.z + C2FarTopRight.z) / 2.0f);

	float CenterToCenter = retDistance(C1Center, C2Center);

	float Radius1 = retDistance(C1Center, glm::vec3(C1NearLowLeft));

	float Radius2 = retDistance(C2Center, glm::vec3(C2NearLowLeft));
	//printf("Radius1 = %f, Radius2 = %f, C2C = %f\n", Radius1, Radius2, CenterToCenter);
	if (Radius1 + Radius2 < CenterToCenter)
		return false;
	else
		return true;
}

// renders the 3D scene
// --------------------
void renderScene(const Shader &shader)
{
	// floor
	glm::mat4 model;
	shader.setMat4("model", model);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	// cubes
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);
	renderCube();
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);
	renderCube();
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.25));
	shader.setMat4("model", model);
	renderCube();
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(18.0f, 0.0f, 15.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.25));
	shader.setMat4("model", model);
	renderCube();
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(-15.0f, 0.0f, -15.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.25));
	shader.setMat4("model", model);
	renderCube();
}


// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
																  // front face
																  -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
																  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
																  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
																  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
																  -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
																  -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
																														// left face
																														-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
																														-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
																														-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
																														-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
																														-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
																														-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
																																											  // right face
																																											  1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
																																											  1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
																																											  1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
																																											  1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
																																											  1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
																																											  1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
																																																								   // bottom face
																																																								   -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
																																																								   1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
																																																								   1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
																																																								   1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
																																																								   -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
																																																								   -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
																																																																						 // top face
																																																																						 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
																																																																						 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
																																																																						 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
																																																																						 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
																																																																						 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
																																																																						 -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------

void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float cameraSpeed = 2.5 * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
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
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
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

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
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

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
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