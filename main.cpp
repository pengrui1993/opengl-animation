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
#ifndef PRINT
#define PRINT(e) std::cout << (e) << std::endl;
#endif
namespace{
    void mprint(const mat4 m){
        std::printf("[%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f]\n"
            ,m[0][0],m[1][0],m[2][0],m[3][0]
            ,m[0][1],m[1][1],m[2][1],m[3][1]
            ,m[0][2],m[1][2],m[2][2],m[3][2]
            ,m[0][3],m[1][3],m[2][3],m[3][3]
        );
    }
    void qprint(const glm::quat q){
        std::printf("x:%f,y:%f,z:%f,w:%f\n"
            ,q.x,q.y,q.z,q.w
        );
    }    
}
namespace{

using Vector3f = glm::vec3;
void framebufferSizeCallback(GLFWwindow* window,int width,int height){}
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){}
}
int main(void)
{
    std::atexit([](){
        std::cout << "at exit function callback"<< std::endl;
    });
    namespace ns = std::chrono;
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;
    glfwDefaultWindowHints(); // optional, the current window hints are already the default
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // the window will stay hidden after creation
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // the window will be resizable
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    // glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    /* Create a windowed mode window and its OpenGL context */
    const int width = 640;		
    const int height = 480;
    window = glfwCreateWindow(width, height, "Hello World", NULL, NULL);
    if (!window){
	    std::cerr << "error on create window" << std::endl;
        glfwTerminate();
        return -1;
    }
    int wi,he;
    glfwGetWindowSize(window,&wi,&he);
    auto& vidmode = *glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowPos(window //center
                    ,(vidmode.width - wi) / 2,
					(vidmode.height - he) / 2);
    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwShowWindow(window);
    glfwSetFramebufferSizeCallback(window,framebufferSizeCallback);
    glfwSetKeyCallback(window,keyCallback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }
    int att;
    glCall(glGetIntegerv,GL_MAX_VERTEX_ATTRIBS,&att);
    std::cout <<"maximum vertex attributes supported:" <<att << std::endl;
    glCall(glClearColor,0.8f,0.8f,0.8f,1.0f);
    glCall(glViewport,0, 0, width, height);
    auto loader = Loader::create("./resources/model.dae");
    std::cout << "loader data \n"
        <<"animation time:"<<loader->getAnimationTime()
        <<"count of joint:"<<loader->getJointCount()
        << std::endl;
        ;
    Camera* camera = Camera::create();
    AnimatedModelShader shader("./resources/shader/animatedEntityVertex.glsl"
    ,"./resources/shader/animatedEntityFragment.glsl");
    AnimatedModel model(loader,"./resources/diffuse.png");
    Vector3f lightDirection(0,-1,0);
    using Pose = std::vector<mat4>;
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        OpenglUtil::clearError();
        Pose pose;
        mat4 mvp;
        /* Render here */
        glCall(glClear,GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glCall(glEnable,GL_DEPTH_TEST);
        glCall(glEnable,GL_MULTISAMPLE);
        glCall(glDisable,GL_BLEND);
        camera->move();
        model.update();
        shader.enable();
            model.getJointTransforms(pose);//ok
            camera->getProjectViewMatrixData(mvp);//ok
            constexpr GLuint textureUnit = 0;
            auto data = std::make_tuple(mvp,lightDirection
                ,textureUnit,model.getTextureId(),&pose);
            shader.passData(data);//ok?
            model.draw();
        shader.disable();
        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        /* Poll for and process events */
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    Camera::destroy(camera);
    Loader::destroy(loader);
    std::exit(0);
    return 0;
}
