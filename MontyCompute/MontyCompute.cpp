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
//const GLchar* fragShader =

#define GL_MAX_TEXTURE_SIZE 1024
const GLchar * vertShader =
"#version 140\n"\
"in vec3 pos;\n"\
"in float iseed;\n"\
"in vec2 uv;\n"\
"out float s;\n"\
"out vec2 uvOut;\n"\

"void main()\n"\
"{\n"\
"	gl_Position.xyz = pos;\n"\
"   gl_Position.w = 1.0;\n"\
"   uvOut = uv;\n"\
"	s = iseed;\n"\
"}\n";
const GLchar* fragShader =
"#version 140\n"\
"in float s;\n"\
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
/*
"int xorsh(int rnd)\n"\
"{\n"\
"	rnd ^= (rnd << 13);\n"\
"	rnd ^= (rnd >> 17);\n"\
"	rnd ^= (rnd << 5);\n"\
"	return rnd;\n"\
"}\n"\
*/
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
"	int rand2 = rand1 >> 8;\n"\
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

//"res = samp;\n"\

/*"int num = int(s);\n"\
"int decision = (num & 0xff)%2;\n"\
"int correctdoor = ((num >> 8) & 0xff)%3;\n"\
"int chosendoor = ((num >> 16) & 0xff)%3;\n"\
"int r = 0;\n"\
"if (correctdoor == chosendoor) r = r + 1;\n"\
"if (decision == 1) r = r + 2;\n"\
"	res = r;\n"\*/
GLfloat g_vertex_buffer_data[] = {
   -1.0f, -1.0f, 0.0f, 100.0f,0.0f,0.0f,
	1.0f, -1.0f, 0.0f, 20000.0f,1.0f,0.0f,
	-1.0f,  1.0f, 0.0f, 34312.0f,0.0f,1.0f,

	-1.0f,  1.0f, 0.0f, 2653626.0f,0.0f,1.0f,
	1.0f, -1.0f, 0.0f, 12363.0f,1.0f,0.0f,
	1.0f,  1.0f, 0.0f, 1643.0f,1.0f,1.0f
};

GLuint renderedTexture[2][4];
GLuint randTex[2];
GLuint framebuffers[2];
GLuint activeFBO = 0;
GLint* GenRandTex() {
	static GLint buf[GL_MAX_TEXTURE_SIZE * GL_MAX_TEXTURE_SIZE * sizeof(GLint)];
	static bool firstcall = true;
	if (!firstcall) return buf;
	printf("Generating blank texture\n");
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
	printf("Generating random texture\n");
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
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, renderedTexture)
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
	GLchar data[512];
	GLuint shader = glCreateShader(type);
	const char* typeStr = (type == GL_VERTEX_SHADER) ? "vertex" : "fragment"; // quick and dirty solution to display type of shader in output.
	glShaderSource(shader, 1, buf, nullptr);
	printf("Compiling %s shader...\n", typeStr);
	glCompileShader(shader);
	glGetShaderInfoLog(shader, sizeof(data), NULL, data);
	printf("%s\n", data);
	GLuint error;
	if ((error = glGetError()) != GL_NO_ERROR)
		exit(-1);
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
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		6 * sizeof(float),                  // stride
		(GLvoid*)(0 * sizeof(GL_FLOAT))            // array buffer offset
	);
	glVertexAttribPointer(
		1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		1,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		6 * sizeof(float),                  // stride
		(GLvoid*)(3 * sizeof(GL_FLOAT))           // array buffer offset
	);
	glVertexAttribPointer(
		2,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		2,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		6 * sizeof(float),                  // stride
		(GLvoid*)(4 * sizeof(GL_FLOAT))           // array buffer offset
	);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	printf("Uploaded vertex data to GPU\n");
	return VertexArray;
}
int main()
{
	GLFWwindow* window;
	GLuint framebuffer = 0;
	GLuint VertexArray;
	GLuint VertexShader;
	GLuint FragmentShader;
	GLuint Shader;
	int texNum = 0;

	if (!glfwInit())
	{
		printf("GLFW3 failed to initalize\n");
		return -1;
	}


	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	window = glfwCreateWindow(1024, 1024, "MontyCompute", NULL, NULL);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	//	if (glewInit() != GLEW_OK)
		//{
		//	printf("Glew failed to initalize\n");
		//	return -1;
		//}

		//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	printf("OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	VertexShader = SetupShader(&vertShader, GL_VERTEX_SHADER);
	FragmentShader = SetupShader(&fragShader, GL_FRAGMENT_SHADER);
	GLchar data[512];
	GLuint shader = glCreateProgram();
	glAttachShader(shader, VertexShader);
	glAttachShader(shader, FragmentShader);
	glBindAttribLocation(shader, 0, "pos");
	glBindAttribLocation(shader, 1, "iseed");
	glBindAttribLocation(shader, 2, "uv");
	glBindFragDataLocation(shader, 0, "c0");
	glBindFragDataLocation(shader, 1, "c1");
	glBindFragDataLocation(shader, 2, "c2");
	glBindFragDataLocation(shader, 3, "c3");
	glBindFragDataLocation(shader, 4, "randOut");
	printf("Linking shaders\n");
	glLinkProgram(shader);

	glGetProgramInfoLog(shader, sizeof(data), NULL, data);
	printf("%s\n", data);
	if (glGetError() != GL_NO_ERROR) exit(-1);

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
	//glGenFramebuffers(1, &framebuffer);
	//glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture[0], 0);
	//if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
	//	return 0;
	//}
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//auto errorone = glGetError();
	//auto error = glGetError();
//	glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), 0);
	/*float* p = (float*)g_vertex_buffer_data;
	p[3] = (float)rand();
	p[7] = (float)rand();
	p[11] = (float)rand(); THESE ARE WRONG
	p[15] = (float)rand();
	p[19] = (float)rand();
	p[23] = (float)rand();
	*/
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
	time_t startTime = time(NULL);
	int  r0 = 0;
	int r1 = 0;
	int r2 = 0;
	int r3 = 0;
	//	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2,GL_COLOR_ATTACHMENT3 };
		//glDrawBuffers(4, DrawBuffers);
	auto error = glGetError();
	int i = 0;
	int l;
	int k;
	time_t timeElapsed;
	printf("Entering render loop\n\n");
	while (1) {
		FlipTextures();
		//	auto error = glGetError();
			//glUnmapBuffer(GL_ARRAY_BUFFER);
		//	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(g_vertex_buffer_data), g_vertex_buffer_data);
			//glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		i++;
		totalDoorsWonKept = 0;
		totalDoorsWonChanged = 0;
		totalDoorsLostKept = 0;
		totalDoorsLostChanged = 0;
		if ((i % 200) == 0) {
			timeElapsed = time(NULL) - startTime;
			printf("Rendered %d frames (%d fps), downloading current results from gpu...\n", i, i / timeElapsed);
			//glBindTexture(GL_TEXTURE_2D, 0);
			//glfwPollEvents();
			//	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, resultframe);
		//	for (int i = 0; i < (GL_MAX_TEXTURE_SIZE * GL_MAX_TEXTURE_SIZE*4); i=i+4) {
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
			printf("Results downloaded from vram, parsing...\n");

			for (l = 0; l < GL_MAX_TEXTURE_SIZE * GL_MAX_TEXTURE_SIZE; l++)
				totalDoorsWonKept += doorsWonKept[l];
			for (l = 0; l < GL_MAX_TEXTURE_SIZE * GL_MAX_TEXTURE_SIZE; l++)
				totalDoorsWonChanged += doorsWonChanged[l];
			for (l = 0; l < GL_MAX_TEXTURE_SIZE * GL_MAX_TEXTURE_SIZE; l++)
				totalDoorsLostKept += doorsLostKept[l];
			for (l = 0; l < GL_MAX_TEXTURE_SIZE * GL_MAX_TEXTURE_SIZE; l++)
				totalDoorsLostChanged += doorsLostChanged[l];
			
			printf("Total doors won without changing: %lu\n", totalDoorsWonKept);
			printf("Total doors won with changing: %lu\n", totalDoorsWonChanged);
			printf("Total doors lost without changing: %lu\n", totalDoorsLostKept);
			printf("Total doors lost with changing: %lu\n", totalDoorsLostChanged);

			printf("Total games played: %lu\n", totalDoorsWonKept + totalDoorsWonChanged + totalDoorsLostKept + totalDoorsLostChanged);
			printf("Games per second: %d\n", (totalDoorsWonKept + totalDoorsWonChanged + totalDoorsLostKept + totalDoorsLostChanged) / timeElapsed);
			printf("Debug data (first 10 pixels):\n");
			printf("doorsWonKept:\t\t");
			for (k = 0; k < 10; k++) printf("%d ", doorsWonKept[k]);
			printf("\ndoorsWonChanged:\t");
			for (k = 0; k < 10; k++) printf("%d ", doorsWonChanged[k]);
			printf("\ndoorsLostKept:\t\t");
			for (k = 0; k < 10; k++) printf("%d ", doorsLostKept[k]);
			printf("\ndoorsLostChanged:\t");
			for (k = 0; k < 10; k++) printf("%d ", doorsLostChanged[k]);
			printf("\nrandTex: ");
			for (k = 0; k < 10; k++) printf("%d ", randTex[k]);
			printf("\n\n");
		}
	//	}
		/*	glReadBuffer(GL_COLOR_ATTACHMENT4);
			glReadPixels(0, 0, GL_MAX_TEXTURE_SIZE, GL_MAX_TEXTURE_SIZE, GL_RED_INTEGER, GL_INT, resultframe);
			printf("rand: %u %u %u %u\n", resultframe[i], resultframe[i + 1], resultframe[i + 2], resultframe[i + 3]);*/
			//getchar();
			//if (resultframe[i] == 0) r0++;
			//if (resultframe[i] == 1) r1++;
		//	if (resultframe[i] == 2) r2++;
			//if (resultframe[i] == 3) r3++;
		//	if (resultframe[i] != 77)
			//	printf("fail\n");
	//	}

		glfwSwapBuffers(window);
		//if ((++l % 1000) == 0)
	//	printf("Res won no change: %d  won change: %d total %d\n", r1, r2, r1 + r2);

		//		printf("Res = %.8x\n", resultframe[i]);
	}

}
/*
	if (device->GetFeatureLevel() < D3D_FEATURE_LEVEL_11_0)
	{
		D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS hwopts = { 0 };
		(void)device->CheckFeatureSupport(D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &hwopts, sizeof(hwopts));
		if (!hwopts.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x)
		{
			printf("DirectCompute is not supported by this device\n");
			return -1;
		}
	}*/



	// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
	// Debug program: F5 or Debug > Start Debugging menu

	// Tips for Getting Started: 
	//   1. Use the Solution Explorer window to add/manage files
	//   2. Use the Team Explorer window to connect to source control
	//   3. Use the Output window to see build output and other messages
	//   4. Use the Error List window to view errors
	//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
	//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
