// MontyCompute.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/GLU.h>
#include <gl/gl.h>
#include <stdio.h>

//const GLchar* fragShader =


const GLchar* vertShader = 
"#version 140\n"\
"in vec3 pos;\n"\
"in float iseed;\n"\
"flat out int s;\n"\
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
"	gl_Position.xyz = pos;\n"\
"   gl_Position.w = 1.0;\n"\
"	s = wang_hash(int(iseed));\n"\
"}\n";
const GLchar* fragShader =
"#version 140\n"\
"flat in int s;\n"\
"out int res;\n"\

"void main()\n"\
"{\n"\
"float sf = float(s);"\
"	res=74;\n"\
"}\n";
static const GLfloat g_vertex_buffer_data[] = {
	-1.0f, -1.0f, 0.0f, 1.0f,
	 1.0f, -1.0f, 0.0f, 2.0f,
	 -1.0f,  1.0f, 0.0f, 3.0f,

	 -1.0f,  1.0f, 0.0f, 4.0f,
	 1.0f, -1.0f, 0.0f, 5.0f,
	 1.0f,  1.0f, 0.0f, 6.0f
};
int main()
{
	GLuint VertexBuffer;
	GLuint renderedTexture;
	GLFWwindow* window;
	GLuint framebuffer=0;
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
	if (glewInit() != GLEW_OK)
	{
		printf("Glew failed to initalize\n");
		return -1;
	}

	//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(VertexShaderID, 1, &vertShader, nullptr);
	glShaderSource(FragmentShaderID, 1, &fragShader, nullptr);
	glCompileShader(VertexShaderID);
	printf("Compiling vshader\n");
	GLchar data[512];

	glGetShaderInfoLog(VertexShaderID, sizeof(data), NULL, data);
	printf("%s\n",data );
	printf("Compiling fshader\n");
	glCompileShader(FragmentShaderID);
	glGetShaderInfoLog(FragmentShaderID, sizeof(data), NULL, data);
	printf("%s\n", data);
	printf("Linking shader\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	glGetProgramInfoLog(ProgramID, sizeof(data), NULL, data);
	printf("%s\n", data);


	glGenTextures(1, &renderedTexture);
	glBindTexture(GL_TEXTURE_2D, renderedTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, 1024, 1024, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, 0);

	glViewport(0, 0, 1024, 1024);

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		return 0;
	}
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//auto errorone = glGetError();
	//auto error = glGetError();
	//auto error = glGetError();


	glUseProgram(ProgramID);
	glGenBuffers(1, &VertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		4*sizeof(float),                  // stride
		(GLvoid*)(0 * sizeof(GL_FLOAT))            // array buffer offset
	);
	glVertexAttribPointer(
		1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		1,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		4 * sizeof(float),                  // stride
		(GLvoid*)(3 * sizeof(GL_FLOAT))           // array buffer offset
	);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	GLuint *resultframe = new GLuint[1024 * 1024];
	while (1) {
		//glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);

		glfwPollEvents();
		glReadPixels(0, 0, 1024, 1024, GL_RED_INTEGER, GL_UNSIGNED_BYTE, resultframe);
	//	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, resultframe);
		printf("Res = %.8x\n", resultframe[0]);
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
