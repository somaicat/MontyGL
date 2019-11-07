// MontyCompute.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//#include <GL/glew.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
//#include <GL/GLU.h>
//#include <gl/gl.h>
#include <stdio.h>

#include <stdlib.h>  
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
"out vec4 res;\n"\

"uniform sampler2D ourTexture;\n"\
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
"vec4 samp = texture(ourTexture,uvOut);\n"\
//"samp = samp+0.1;"
//"vec4 e = vec4(0.2f,0.2f,0.2f,1.0f);\n"\
//"e.a = s;\n"\
//"e.r = float(wang_hash((int(s))));\n"\
//"e.b = samp.a;\n"\

"res = samp+0.2;\n"\
/*"int num = int(s);\n"\
"int decision = (num & 0xff)%2;\n"\
"int correctdoor = ((num >> 8) & 0xff)%3;\n"\
"int chosendoor = ((num >> 16) & 0xff)%3;\n"\
"int r = 0;\n"\
"if (correctdoor == chosendoor) r = r + 1;\n"\
"if (decision == 1) r = r + 2;\n"\
"	res = r;\n"\*/
"}\n";
GLfloat g_vertex_buffer_data[] = {
   -1.0f, -1.0f, 0.0f, 100.0f,0.0f,0.0f,
	1.0f, -1.0f, 0.0f, 20000.0f,1.0f,0.0f,
	-1.0f,  1.0f, 0.0f, 34312.0f,0.0f,1.0f,

	-1.0f,  1.0f, 0.0f, 2653626.0f,0.0f,1.0f,
	1.0f, -1.0f, 0.0f, 12363.0f,1.0f,0.0f,
	1.0f,  1.0f, 0.0f, 1643.0f,1.0f,1.0f
};

GLuint renderedTexture[2];
GLuint framebuffers[2];
GLuint activeFBO=0;
void SetupFramebuffers() {

	glGenFramebuffers(2, framebuffers);

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[0]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture[0], 0);

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[1]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture[1], 0);

}
void FlipTextures() {
//	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderedTexture[activeFBO]);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, renderedTexture)
	if (activeFBO == 0)
		activeFBO = 1;
	else activeFBO = 0;

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture[activeFBO], 0);
	//glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[activeFBO]);
}

GLuint SetupShader(const GLchar* const* buf, GLenum type) {
	GLchar data[512];
	GLuint shader = glCreateShader(type);
	const char* typeStr = (type == GL_VERTEX_SHADER) ? "vertex" : "fragment"; // quick and dirty solution to display type of shader in output.
	glShaderSource(shader, 1, buf, nullptr);
	printf("Compiling %s shader...\n",typeStr);
	glCompileShader(shader);
	glGetShaderInfoLog(shader, sizeof(data), NULL, data);
	printf("%s\n", data);
	GLuint error;
	if ((error = glGetError()) != GL_NO_ERROR) 
		exit(-1);
	return shader;
}
GLuint LinkShaders(const GLuint program, const GLuint vertShader, const GLuint fragShader) {
	GLchar data[512];
	GLuint shader = program;
	glAttachShader(shader, vertShader);
	glAttachShader(shader, fragShader);
	glLinkProgram(shader);

	glGetProgramInfoLog(shader, sizeof(data), NULL, data);
	printf("%s\n", data);
	if (glGetError() != GL_NO_ERROR) exit(-1);
	return shader;
}
void SetupTexture(const GLuint tex) {
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GL_MAX_TEXTURE_SIZE, GL_MAX_TEXTURE_SIZE, 0, GL_RGBA, GL_FLOAT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
GLuint SetupVertexArray(GLfloat *buf, GLuint len) {
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

//	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	window = glfwCreateWindow(1024, 1024, "MontyCompute", NULL, NULL);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	//	if (glewInit() != GLEW_OK)
		//{
		//	printf("Glew failed to initalize\n");
		//	return -1;
		//}

		//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	VertexShader = SetupShader(&vertShader, GL_VERTEX_SHADER);
	FragmentShader = SetupShader(&fragShader, GL_FRAGMENT_SHADER);
	Shader = glCreateProgram();
	glBindAttribLocation(Shader, 0, "pos");
	glBindAttribLocation(Shader, 1, "iseed");
	glBindAttribLocation(Shader, 2, "uv");
	LinkShaders(Shader, VertexShader, FragmentShader);
	glUseProgram(Shader);
	GLint texSampler = glGetUniformLocation(Shader, "ourTexture");
	//auto error = glGetError();
	glUniform1i(texSampler, 0);
	
	glGenTextures(2, renderedTexture);

	SetupTexture(renderedTexture[0]);
	SetupTexture(renderedTexture[1]);
	glViewport(0, 0, GL_MAX_TEXTURE_SIZE, GL_MAX_TEXTURE_SIZE);
//	SetupFramebuffers();

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
	GLfloat* resultframe = new GLfloat[GL_MAX_TEXTURE_SIZE * GL_MAX_TEXTURE_SIZE*4];
	int r0 = 0;
	int r1 = 0;
	int r2 = 0;
	int r3 = 0;
	int l = 0;

	//auto error = glGetError();
	while (1) {
		FlipTextures();
	//	auto error = glGetError();
		//glUnmapBuffer(GL_ARRAY_BUFFER);
	//	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(g_vertex_buffer_data), g_vertex_buffer_data);
		//glClear(GL_COLOR_BUFFER_BIT);

		//glBindTexture(GL_TEXTURE_2D, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//glBindTexture(GL_TEXTURE_2D, 0);
		//glfwPollEvents();
		glReadPixels(0, 0, GL_MAX_TEXTURE_SIZE, GL_MAX_TEXTURE_SIZE, GL_RGBA, GL_FLOAT, resultframe);
		//	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, resultframe);
	//	for (int i = 0; i < (GL_MAX_TEXTURE_SIZE * GL_MAX_TEXTURE_SIZE*4); i=i+4) {
		int i = 0;
			printf("%f %f %f %f\n", resultframe[i], resultframe[i + 1], resultframe[i + 2], resultframe[i + 3]);
			getchar();
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
