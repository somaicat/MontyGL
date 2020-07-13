// MontyCompute.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//#include <GL/glew.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
//#include <GL/GLU.h>
//#include <gl/gl.h>
#include <stdio.h>

#include <stdlib.h>  
#include <time.h>

#define FRAGSHADER fragShader

#define CLEARSCR "\033[2J"
#define ZEROCURSOR "\033[H"

#define GL_MAX_TEXTURE_SIZE 1024
const GLchar * vertShader =
"#version 140\n"\
"in vec2 pos;\n"\
"in vec2 uv;\n"\
"out vec2 uvOut;\n"\

"void main()\n"\
"{\n"\
"	gl_Position.xy = pos;\n"\
"   gl_Position.zw = vec2(0.0,1.0);\n"\
"   uvOut = uv;\n"\
"}\n";
const GLchar* randOnlyShader = // Provided for future benchmarking setting where only the randomization is done but with no games played.
"#version 140\n"\
"in vec2 uvOut;\n"
"out uint c0;\n"\
"out uint c1;\n"\
"out uint c2;\n"\
"out uint c3;\n"\
"out int randOut;\n"\

"uniform usampler2D t0;\n"\
"uniform usampler2D t1;\n"\
"uniform usampler2D t2;\n"\
"uniform usampler2D t3;\n"\
"uniform isampler2D randTex;\n"\

"int wang_hash(int seed)\n"\
"{\n"\
"	seed = (seed ^ 61) ^ (seed >> 16);\n"\
"	seed *= 9;\n"\
"	seed = seed ^ (seed >> 4);\n"\
"	seed *= 0x27d4eb2d;\n"\
"	seed = seed ^ (seed >> 15);\n"\
"	return seed;\n"\
"}\n"\

"void main()\n"\
"{\n"\
"	int rand1 = wang_hash(texture(randTex,uvOut).r);\n"\

"	c0 = 0u;\n"\
"	c1 = 0u;\n"\
"	c2 = 0u;\n"\
"	c3 = 0u;\n"\

"	randOut = rand1;\n"\
"}\n";
const GLchar* fragShader = // The code that plays the actual games on the graphics chip
"#version 140\n"\
"in vec2 uvOut;\n"
"out uint c0;\n"\
"out uint c1;\n"\
"out uint c2;\n"\
"out uint c3;\n"\
"out int randOut;\n"\

"uniform usampler2D t0;\n"\
"uniform usampler2D t1;\n"\
"uniform usampler2D t2;\n"\
"uniform usampler2D t3;\n"\
"uniform isampler2D randTex;\n"\

"int wang_hash(int seed)\n"\
"{\n"\
"	seed = (seed ^ 61) ^ (seed >> 16);\n"\
"	seed *= 9;\n"\
"	seed = seed ^ (seed >> 4);\n"\
"	seed *= 0x27d4eb2d;\n"\
"	seed = seed ^ (seed >> 15);\n"\
"	return seed;\n"\
"}\n"\

"void main()\n"\
"{\n"\

"	uint doorsWonKept = texture(t0,uvOut).r;\n"\
"	uint doorsWonChanged = texture(t1,uvOut).r;\n"\
"	uint doorsLostKept = texture(t2,uvOut).r;\n"\
"	uint doorsLostChanged = texture(t3,uvOut).r;\n"\
"	int rand1 = wang_hash(texture(randTex,uvOut).r);\n"\
"	int rand2 = rand1 >> 8; // Derive random numbers from portion of the first hashed value... hopefully a bit faster than getting multiple hashes\n"\
"	int rand3 = rand2 >> 8;\n"\
/*
"	int rand2 = wang_hash(rand1);\n"\
"	int rand3 = wang_hash(rand2);\n"\
*/
"	int chosenDoor = rand1 % 3;\n"\
"	int correctDoor = rand2 % 3;\n"\
"	int decision = rand3 % 2;\n"\
"	int altDoor=0;\n"\
"	int excludedDoor=0;\n"\

"	if (decision == 0) { // Chose not to switch\n"\
"		if (chosenDoor == correctDoor){ doorsWonKept++; }\n"\
"		else { doorsLostKept++; }\n"\
"	}\n"\
"	else { // Chose to switch\n"\

"		if (excludedDoor==correctDoor || excludedDoor == chosenDoor) {excludedDoor++;}\n"\
"		if (excludedDoor==correctDoor || excludedDoor == chosenDoor) {excludedDoor++;}\n"\
"		if (altDoor == chosenDoor || altDoor == excludedDoor) {altDoor++;}\n"\
"		if (altDoor == chosenDoor || altDoor == excludedDoor) {altDoor++;}\n"\

"		if (altDoor == correctDoor){ doorsWonChanged++; }\n"\
"		else { doorsLostChanged++; }\n"\
"	\n"\
"	\n"\
"	}\n"\


"	c0 = doorsWonKept;\n"\
"	c1 = doorsWonChanged;\n"\
"	c2 = doorsLostKept;\n"\
"	c3 = doorsLostChanged;\n"\

"	randOut = rand1;\n"\
"}\n";

/*        Quad Definition
		X Y U V       X Y U V
	   -1,1,0,1       1,1,1,1
			  2*-----*4
			   |\    |
			   | \   |
			   |  \  |
			   |   \ |
			   |    \|
			  1*-----*3
	  -1,-1,0,0       1,-1,1,0
	   X  Y U V       X  Y U V
*/
// Simple vertex data defining a single full size quad in normalized device coordinates. Along with the appropriate texture UV values for sampling.
GLfloat g_vertex_buffer_data[] = {
   -1.0f, -1.0f, 0.0f, 0.0f,
   -1.0f,  1.0f, 0.0f, 1.0f,
	1.0f, -1.0f, 1.0f, 0.0f,
	1.0f,  1.0f, 1.0f, 1.0f
};

GLuint renderedTexture[2][4];
GLuint randTex[2];
GLuint framebuffers[2];
GLuint activeFBO = 0;
GLint* GenRandTex() {
	static GLint buf[GL_MAX_TEXTURE_SIZE * GL_MAX_TEXTURE_SIZE * sizeof(GLint)];
	static bool firstcall = true;
	if (!firstcall) return buf;
	printf("Generating random texture\n");
	for (int i = 0; i < GL_MAX_TEXTURE_SIZE * GL_MAX_TEXTURE_SIZE; i++) {
		buf[i] = rand();
	}
	firstcall = false;
	return buf; // NOTE: as a static, this is legal, although obviously not thread safe
}
GLuint* GenBlankTex() {
	static GLuint buf[GL_MAX_TEXTURE_SIZE * GL_MAX_TEXTURE_SIZE * sizeof(GLuint)];
	static bool firstcall = true;
	if (!firstcall) return buf;
	printf("Generating blank texture\n");
	for (int i = 0; i < GL_MAX_TEXTURE_SIZE * GL_MAX_TEXTURE_SIZE; i++) {
		buf[i] = 0;
	}
	firstcall = false;
	return buf; // NOTE: as a static, this is legal, although obviously not thread safe
}
void SetupFramebuffers() {
	printf("Configuring framebuffers\n");
	glGenFramebuffers(2, framebuffers);

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[0]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture[0][0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, renderedTexture[0][1], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, renderedTexture[0][2], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, renderedTexture[0][3], 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, randTex[0], 0);

	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2,GL_COLOR_ATTACHMENT3,GL_COLOR_ATTACHMENT4 };
	glDrawBuffers(5, DrawBuffers);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[1]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture[1][0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, renderedTexture[1][1], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, renderedTexture[1][2], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, renderedTexture[1][3], 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, randTex[1], 0);
	glDrawBuffers(5, DrawBuffers);
}
void FlipTextures() {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderedTexture[activeFBO][0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, renderedTexture[activeFBO][1]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, renderedTexture[activeFBO][2]);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, renderedTexture[activeFBO][3]);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, randTex[activeFBO]);
	//printf("Bound textures %d\n", activeFBO);
	if (activeFBO == 0)
		activeFBO = 1;
	else activeFBO = 0;
	//printf("Bound Framebuffer %d\n", activeFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[activeFBO]);
	/* Doesn't seem to be needed
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture[activeFBO][0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, renderedTexture[activeFBO][1], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, renderedTexture[activeFBO][2], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, renderedTexture[activeFBO][3], 0);*/
}

GLuint SetupShader(const GLchar* const* buf, GLenum type) {
	GLuint failnum = 0;
	GLchar data[512];
	GLuint shader = glCreateShader(type);
	const char* typeStr = (type == GL_VERTEX_SHADER) ? "vertex" : "fragment"; // quick and dirty solution to display type of shader in output.
	glShaderSource(shader, 1, buf, nullptr);
	printf("Compiling %s shader...\n", typeStr);
	glCompileShader(shader);
	glGetShaderInfoLog(shader, sizeof(data), NULL, data);
	printf("%s\n", data);
	GLuint error;
	if ((error = glGetError()) != GL_NO_ERROR) {

		printf("Shader compile failed: %x\n", failnum);
		getchar();
		exit(-1);
	}
	return shader;
}
void SetupTextures() {
	GenRandTex();
	glActiveTexture(GL_TEXTURE0);
	for (int i = 0; i < 2; i++) {
		glBindTexture(GL_TEXTURE_2D, randTex[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, GL_MAX_TEXTURE_SIZE, GL_MAX_TEXTURE_SIZE, 0, GL_RED_INTEGER, GL_INT, GenRandTex());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		for (int v = 0; v < 4; v++) {
			glBindTexture(GL_TEXTURE_2D, renderedTexture[i][v]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, GL_MAX_TEXTURE_SIZE, GL_MAX_TEXTURE_SIZE, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, GenBlankTex());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	printf("Uploaded texture data to GPU\n");
}
GLuint SetupVertexArray(GLfloat* buf, GLuint len) {
	GLuint VertexBuffer;
	GLuint VertexArray;
	glGenBuffers(1, &VertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, len, buf, GL_STATIC_DRAW);
	glGenVertexArrays(1, &VertexArray);
	glBindVertexArray(VertexArray);
	glVertexAttribPointer(
		0,                  // attribute 0
		2,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized
		4 * sizeof(float),  // stride
		(GLvoid*)(0 * sizeof(GL_FLOAT)) // offset
	);
	glVertexAttribPointer(
		1,                  // attribute 1
		2,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized
		4 * sizeof(float), // stride
		(GLvoid*)(2 * sizeof(GL_FLOAT)) // offset
	);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	printf("Uploaded vertex data to GPU\n");
	return VertexArray;
}
int main(int argc, char *argv[])
{
	GLFWwindow* window;
	GLuint framebuffer = 0;
	GLuint VertexArray;
	GLuint VertexShader;
	GLuint FragmentShader;
	GLuint Shader;
	int texNum = 0;
	GLuint failnum = 0;
	if (!glfwInit())
	{
		printf("GLFW3 failed to initalize\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE); // Double buffering will double the amount of video memory used... and since we're rendering off screen it's totally wasted. Attempt to disable.

	window = glfwCreateWindow(1024, 1024, "MontyCompute", NULL, NULL);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	//	if (glewInit() != GLEW_OK)
		//{
		//	printf("Glew failed to initalize\n");
		//	return -1;
		//}

	printf("OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	VertexShader = SetupShader(&vertShader, GL_VERTEX_SHADER);
	FragmentShader = SetupShader(&FRAGSHADER, GL_FRAGMENT_SHADER);
	GLchar data[512];
	GLuint shader = glCreateProgram();
	glAttachShader(shader, VertexShader);
	glAttachShader(shader, FragmentShader);
	glBindAttribLocation(shader, 0, "pos");
	glBindAttribLocation(shader, 1, "uv");
	glBindFragDataLocation(shader, 0, "c0");
	glBindFragDataLocation(shader, 1, "c1");
	glBindFragDataLocation(shader, 2, "c2");
	glBindFragDataLocation(shader, 3, "c3");
	glBindFragDataLocation(shader, 4, "randOut");
	printf("Linking shaders\n");
	glLinkProgram(shader);

	glGetProgramInfoLog(shader, sizeof(data), NULL, data);
	printf("%s\n", data);
	if ((failnum = glGetError()) != GL_NO_ERROR) {
		printf("Linking shaders failed %x\n",failnum);
		getchar();
		exit(-1); }

	glUseProgram(shader);
	printf("Configuring shader samplers\n");
	GLint texSampler = glGetUniformLocation(shader, "t0");
	glUniform1i(texSampler, 0);
	texSampler = glGetUniformLocation(shader, "t1");
	glUniform1i(texSampler, 1);
	texSampler = glGetUniformLocation(shader, "t2");
	glUniform1i(texSampler, 2);
	texSampler = glGetUniformLocation(shader, "t3");
	glUniform1i(texSampler, 3);

	texSampler = glGetUniformLocation(shader, "randTex");
	glUniform1i(texSampler, 4);


	glGenTextures(2, randTex);
	glGenTextures(4, renderedTexture[0]);
	glGenTextures(4, renderedTexture[1]);
	SetupTextures();
	glViewport(0, 0, GL_MAX_TEXTURE_SIZE, GL_MAX_TEXTURE_SIZE);
	SetupFramebuffers();

	VertexArray = SetupVertexArray(g_vertex_buffer_data, sizeof(g_vertex_buffer_data));
	GLuint* doorsWonKept = new GLuint[GL_MAX_TEXTURE_SIZE * GL_MAX_TEXTURE_SIZE * sizeof(GLuint)];
	GLuint* doorsWonChanged = new GLuint[GL_MAX_TEXTURE_SIZE * GL_MAX_TEXTURE_SIZE * sizeof(GLuint)];
	GLuint* doorsLostKept = new GLuint[GL_MAX_TEXTURE_SIZE * GL_MAX_TEXTURE_SIZE * sizeof(GLuint)];
	GLuint* doorsLostChanged = new GLuint[GL_MAX_TEXTURE_SIZE * GL_MAX_TEXTURE_SIZE * sizeof(GLuint)];
	GLint* randTex = new GLint[GL_MAX_TEXTURE_SIZE * GL_MAX_TEXTURE_SIZE * sizeof(GLint)];
	unsigned long totalDoorsWonKept = 0;
	unsigned long totalDoorsWonChanged = 0;
	unsigned long totalDoorsLostKept = 0;
	unsigned long totalDoorsLostChanged = 0;
	unsigned long totalGames = 0;
	time_t startTime = time(NULL);

	auto error = glGetError();
	int i = 0;
	int l;
	int k;
	time_t timeElapsed;
	printf("Entering render loop\n\n");
	while (1) {
		FlipTextures();
			//glUnmapBuffer(GL_ARRAY_BUFFER);
		//	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(g_vertex_buffer_data), g_vertex_buffer_data);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		i++;
		totalDoorsWonKept = 0;
		totalDoorsWonChanged = 0;
		totalDoorsLostKept = 0;
		totalDoorsLostChanged = 0;
		totalGames = 0;
		if ((i % 200) == 0) {
			timeElapsed = time(NULL) - startTime;
			//printf("%s%s", CLEARSCR, ZEROCURSOR);
			printf("%lld s: Rendered %d frames (%d fps), downloading current results from gpu...\n",timeElapsed, i, (timeElapsed != 0) ? i / timeElapsed : 0);

			//glfwPollEvents();

			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glReadPixels(0, 0, GL_MAX_TEXTURE_SIZE, GL_MAX_TEXTURE_SIZE, GL_RED_INTEGER, GL_UNSIGNED_INT, doorsWonKept);
			glReadBuffer(GL_COLOR_ATTACHMENT1);
			glReadPixels(0, 0, GL_MAX_TEXTURE_SIZE, GL_MAX_TEXTURE_SIZE, GL_RED_INTEGER, GL_UNSIGNED_INT, doorsWonChanged);
			glReadBuffer(GL_COLOR_ATTACHMENT2);
			glReadPixels(0, 0, GL_MAX_TEXTURE_SIZE, GL_MAX_TEXTURE_SIZE, GL_RED_INTEGER, GL_UNSIGNED_INT, doorsLostKept);
			glReadBuffer(GL_COLOR_ATTACHMENT3);
			glReadPixels(0, 0, GL_MAX_TEXTURE_SIZE, GL_MAX_TEXTURE_SIZE, GL_RED_INTEGER, GL_UNSIGNED_INT, doorsLostChanged);
			glReadBuffer(GL_COLOR_ATTACHMENT4);
			glReadPixels(0, 0, GL_MAX_TEXTURE_SIZE, GL_MAX_TEXTURE_SIZE, GL_RED_INTEGER, GL_INT, randTex);
			printf("Results downloaded from vram, parsing...\n\n");

			for (l = 0; l < GL_MAX_TEXTURE_SIZE * GL_MAX_TEXTURE_SIZE; l++)
				totalDoorsWonKept += doorsWonKept[l];
			for (l = 0; l < GL_MAX_TEXTURE_SIZE * GL_MAX_TEXTURE_SIZE; l++)
				totalDoorsWonChanged += doorsWonChanged[l];
			for (l = 0; l < GL_MAX_TEXTURE_SIZE * GL_MAX_TEXTURE_SIZE; l++)
				totalDoorsLostKept += doorsLostKept[l];
			for (l = 0; l < GL_MAX_TEXTURE_SIZE * GL_MAX_TEXTURE_SIZE; l++)
				totalDoorsLostChanged += doorsLostChanged[l];
			printf("Games keeping: %lu wins %lu lose\n", totalDoorsWonKept, totalDoorsLostKept);
			printf("Games switching: %lu wins %lu lose\n", totalDoorsWonChanged, totalDoorsLostChanged);
			printf("Odds keeping: %f\n", ((double) totalDoorsWonKept / ((double)totalDoorsWonKept + (double)totalDoorsLostKept)) * 100.0f);
			printf("Odds switching: %f\n", ((double) totalDoorsWonChanged / ((double)totalDoorsWonChanged + (double)totalDoorsLostChanged)) * 100.0f);
			totalGames = (totalDoorsWonKept + totalDoorsWonChanged + totalDoorsLostKept + totalDoorsLostChanged);
			printf("Total games played: %lu\n", totalGames);
			printf("Games per second: %lld\n", (time_t) totalGames / timeElapsed);
			printf("\nDebug data (first 10 pixels):\n");
			printf("WonKeptPixels:\t\t");
			for (k = 0; k < 10; k++) printf("%d ", doorsWonKept[k]);
			printf("\nWonSwitchedPixels:\t");
			for (k = 0; k < 10; k++) printf("%d ", doorsWonChanged[k]);
			printf("\nLostKeptPixels:\t\t");
			for (k = 0; k < 10; k++) printf("%d ", doorsLostKept[k]);
			printf("\nLostSwitchedPixels:\t");
			for (k = 0; k < 10; k++) printf("%d ", doorsLostChanged[k]);
			printf("\nRandTexPixels: ");
			for (k = 0; k < 10; k++) printf("%d ", randTex[k]);
			printf("\n\n");
		}

		glfwSwapBuffers(window);
	}

}
