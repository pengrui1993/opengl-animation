#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <glm/glm.hpp>
#include <chrono>

#include "Camera.h"
#include "Loader.h"
#include "AnimatedModel.h"
#include "OpenglUtil.h"
void framebufferSizeCallback(GLFWwindow* window,int width,int height){}
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){}
int main(void)
{
    namespace ns = std::chrono;
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
	std::cout << "error on create window" << std::endl;
        glfwTerminate();
        return -1;
    }

    std::cout <<"pre make context" << std::endl;
    /* Make the window's context current */
    glfwMakeContextCurrent(window);
  
    glfwSetFramebufferSizeCallback(window,framebufferSizeCallback);
    glfwSetKeyCallback(window,keyCallback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }
    int att;
    glCall(glGetIntegerv,GL_MAX_VERTEX_ATTRIBS,&att);
    std::cout <<"maximum vertex attributes supported:" <<att << std::endl;
    //std::cout << glm::vec4(1,0,0,1).length() << std::endl;
    auto loader = Loader::create("./resources/model.dae");
    std::cout << "loader data \n"
        <<"animation time:"<<loader->getAnimationTime()
        <<"count of joint:"<<loader->getJointCount()
        << std::endl;
        ;
    Camera* camera = Camera::create();
    /* Loop until the user closes the window */
    AnimatedModelShader shader("./resources/shader/animatedEntityVertex.glsl"
    ,"./resources/shader/animatedEntityFragment.glsl");
    while (!glfwWindowShouldClose(window))
    {
        glCall(glClearColor,0,1,0,1);
        /* Render here */
        glCall(glClear,GL_COLOR_BUFFER_BIT);
        camera->move();
        shader.enable();

        shader.disable();
        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    Camera::destroy(camera);
    Loader::destroy(loader);
    return 0;
}