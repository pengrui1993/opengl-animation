#include"OpenglUtil.h"
#include<glad/glad.h>
#include<iostream>
#include<cstdlib>
namespace{
      auto err = []()->std::ostream&{ return std::cerr;};
}
void OpenglUtil::clearError(){
    int count = 0;
    while(GL_NO_ERROR != glGetError()) count++;
    if(count)err()<<"clear error count:" << count;
}
void OpenglUtil::checkError(const Path& file,Line line){
        // Get the last error
    const GLenum errorCode = glGetError();

    if (errorCode != GL_NO_ERROR)
    {
        std::string error       = "Unknown error";
        std::string description = "No description";

        // Decode the error code
        switch (errorCode)
        {
            case GL_INVALID_ENUM:
            {
                error       = "GL_INVALID_ENUM";
                description = "An unacceptable value has been specified for an enumerated argument.";
                break;
            }

            case GL_INVALID_VALUE:
            {
                error       = "GL_INVALID_VALUE";
                description = "A numeric argument is out of range.";
                break;
            }

            case GL_INVALID_OPERATION:
            {
                error       = "GL_INVALID_OPERATION";
                description = "The specified operation is not allowed in the current state.";
                break;
            }

            case GL_STACK_OVERFLOW:
            {
                error       = "GL_STACK_OVERFLOW";
                description = "This command would cause a stack overflow.";
                break;
            }

            case GL_STACK_UNDERFLOW:
            {
                error       = "GL_STACK_UNDERFLOW";
                description = "This command would cause a stack underflow.";
                break;
            }

            case GL_OUT_OF_MEMORY:
            {
                error       = "GL_OUT_OF_MEMORY";
                description = "There is not enough memory left to execute the command.";
                break;
            }

            case GL_INVALID_FRAMEBUFFER_OPERATION:
            {
                error       = "GL_INVALID_FRAMEBUFFER_OPERATION";
                description = "The object bound to FRAMEBUFFER_BINDING is not \"framebuffer complete\".";
                break;
            }
        }
        err() << "An internal OpenGL call failed in " << file << "(" << line << ")."
              << "\nExpression:\n   " 
              << "\nError description:\n   " << error << "\n   " << description << '\n'
              << std::endl;
        if(exitOnError)
            std::exit(2);
    }
}