#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstring>
#include <cstdlib>


//shader
unsigned int vertexShader;
unsigned int fragmentShader;
unsigned int shaderProgram;
const char* vertexShaderSource = 
    "#version 330 core\n"
    "layout (location=0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position =  vec4(aPos,1.0);\n"
    "}\0"
;
const char* fragmentShaderSource=
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor =  vec4(1.0f,0.5f,0.2f,1.0f);\n"
    "}\0"
;

void prepareShader(){
    int success;
    
    char infoLog[512];
    if(shaderProgram>0){
        std::cout << "readly init the shader program";
        return;
    }
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader,1,&vertexShaderSource,nullptr);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader,GL_COMPILE_STATUS,&success);

    if(!success){
        std::memset(infoLog,0,512);
        glGetShaderInfoLog(vertexShader,512,nullptr,infoLog);
        std::cout << "error vertex compilation failed\n" << infoLog << std::endl;
        std::exit(-1);
    }
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader,1,&fragmentShaderSource,nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader,GL_COMPILE_STATUS,&success);
    if(!success){
        std::memset(infoLog,0,512);
        glGetShaderInfoLog(fragmentShader,512,nullptr,infoLog);
        std::cout << "error fragment compilation failed\n" << infoLog << std::endl;
        std::exit(-1);
    }
    shaderProgram = glCreateProgram();
    std::cout << "program id:" << shaderProgram << std::endl;
    glAttachShader(shaderProgram,vertexShader);
    glAttachShader(shaderProgram,fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram,GL_LINK_STATUS,&success);
    if(!success){
        glGetProgramInfoLog(shaderProgram,512,nullptr,infoLog);
        std::cout << "error link\n" << infoLog << std::endl;
        std::exit(-1);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

const auto z = 0.f;
const float vertices[]= {
    -0.5f,-0.5f,z
    ,0.5f,-0.5f,z
    ,0.f,0.5f,z
};
unsigned int VAO;
unsigned int VBO;
void prepareBuffer(){
    if(VBO>0){
        std::cout << "already init the buffer" << std::endl;
        return;
    }
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);
    std::cout << "vao:"<<VAO<< ",vbo:"<<VBO<< std::endl;
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);
}
  
void framebufferSizeCallback(GLFWwindow* window,int width,int height){

}
int vx=0,vy = 0;
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){

    if(GLFW_PRESS==glfwGetKey(window,GLFW_KEY_ESCAPE)){
        glfwSetWindowShouldClose(window,true);
    }
    if(GLFW_PRESS==glfwGetKey(window,GLFW_KEY_LEFT))    {
        vx--;
    }
    if(GLFW_PRESS==glfwGetKey(window,GLFW_KEY_RIGHT))    {
        vx++;
    }
    if(GLFW_PRESS==glfwGetKey(window,GLFW_KEY_UP))    {
        vy++;
    }
    if(GLFW_PRESS==glfwGetKey(window,GLFW_KEY_DOWN))    {
        vy--;
    }
    std::cout << "x:"<<vx<<",y"<<vy<<std::endl;
}
int main(void)
{
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
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS,&att);
    std::cout <<"maximum vertex attributes supported:" <<att << std::endl;
    prepareShader();
    prepareBuffer();

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        glViewport(vx,vy,200,200);
        glClearColor(0,1,0,1);
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES,0,3);
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
