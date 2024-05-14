#include"AnimatedModel.h"
#include <glad/glad.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cinttypes>
#include <cstdarg>
#include <glm/gtc/type_ptr.hpp>
#include"OpenglUtil.h"
bool AnimatedModelShader::checkShaderCompileStatus(ShaderId id){
    GLint result;
    glCall(glGetShaderiv,id,GL_COMPILE_STATUS,&result);
    return result != GL_FALSE;
}
bool AnimatedModelShader::getSource(FilePath path,String& data){
    std::ifstream is;
    is.open(path,std::ifstream::in);
    if(!is){
        std::cerr << "open file error:" << path << std::endl;
        return false;
    }
    is.seekg(0,std::ios::end);
    data.resize(is.tellg());
    is.seekg(0,std::ios::beg);
    is.read(&data[0],data.size());
    is.close();
    return true;
}
bool AnimatedModelShader::checkProgramLinkStatus(){
    GLint result;
    glCall(glGetProgramiv,program,GL_LINK_STATUS,&result);
    if(GL_FALSE==result)
        return false;
    glCall(glValidateProgram,program);
    return true;
}
bool AnimatedModelShader::checkStoreUniformLocation(Location l){
    return NOT_FOUND !=l;
}
AnimatedModelShader::~AnimatedModelShader(){
    destroy();
}
AnimatedModelShader::AnimatedModelShader(FilePath vs,FilePath fs){
  
    const char* ptr;
    String source;
    ShaderId vertexShader;
    ShaderId fragmentShader;
    vertexShader = glCall(glCreateShader,GL_VERTEX_SHADER);
    if(!getSource(vs,source)){
        std::cerr << "get source from " << vs << " failed" << std::endl;
        std::exit(-1);
    }
    ptr = source.c_str();
    glCall(glShaderSource,vertexShader,1,&ptr,nullptr);
    glCall(glCompileShader,vertexShader);
    if(!checkShaderCompileStatus(vertexShader)){
        std::cerr << "compile vertex shader error" << std::endl;
        std::exit(-1);
    }
    fragmentShader = glCall(glCreateShader,GL_FRAGMENT_SHADER);
    if(!getSource(fs,source)){
        std::cerr << "get source from " << fs << " failed" << std::endl;
        std::exit(-1);
    }
    ptr = source.c_str();
    glCall(glShaderSource,fragmentShader,1,&ptr,nullptr);
    glCall(glCompileShader,fragmentShader);
    if(!checkShaderCompileStatus(fragmentShader)){
        std::cerr << "compile fragment shader error" << std::endl;
        std::exit(-1);
    }
    program = glCall0(glCreateProgram);
    glCall(glAttachShader,program,vertexShader);
    glCall(glAttachShader,program,fragmentShader);
    glCall(glBindAttribLocation,program,0,"in_position");
    glCall(glBindAttribLocation,program,1,"in_textureCoords");
    glCall(glBindAttribLocation,program,2,"in_normal");
    glCall(glBindAttribLocation,program,3,"in_jointIndices");
    glCall(glBindAttribLocation,program,4,"in_weights");
    glCall(glLinkProgram,program);
    glCall(glDetachShader,program,vertexShader);
    glCall(glDetachShader,program,fragmentShader);
    glCall(glDeleteShader,vertexShader);
    glCall(glDeleteShader,fragmentShader);
    checkProgramLinkStatus();
    projectionViewMatrix = glCall(glGetUniformLocation,program,"projectionViewMatrix");
    if(!checkStoreUniformLocation(projectionViewMatrix)){
        std::cerr << "projection view matrix location error" << std::endl;
        std::exit(-1);
    }
    lightDirection = glCall(glGetUniformLocation,program,"lightDirection");
    if(!checkStoreUniformLocation(lightDirection)){
        std::cerr << "light direction location error" << std::endl;
        std::exit(-1);
    }
    diffuseMap = glCall(glGetUniformLocation,program,"diffuseMap");
    if(!checkStoreUniformLocation(diffuseMap)){
        std::cerr << "diffuse map location error" << std::endl;
        std::exit(-1);
    }
    jointTransforms.resize(MAX_JOINTS);
    for(int i=0;i<MAX_JOINTS;i++){
        std::stringstream ss;
        ss << "jointTransforms[" << i << "]";
        jointTransforms[i] = glCall(glGetUniformLocation,program,ss.str().c_str());
        if(!checkStoreUniformLocation(jointTransforms[i])){
            std::cerr << ss.str()<<" location error" << std::endl;
            std::exit(-1);
        }
    }
    glCall(glUseProgram,program);
    glCall(glUniform1i,diffuseMap,DIFFUSE_TEX_UNIT);
    glCall(glUseProgram,0);

}
void AnimatedModelShader::enable(){
    glCall(glUseProgram,program);
}
void AnimatedModelShader::disable(){
    glCall(glUseProgram,0);
}
void AnimatedModelShader::passData(PassedData& data){
    auto f = [&](){
        glm::vec3 v;
        std::tie(std::ignore,v
        ,std::ignore
        ,std::ignore
        ,std::ignore
        ) = data;
    };
   auto&& mvp = std::get<0>(data);
   auto&& light = std::get<1>(data);
   auto unit = std::get<2>(data);
   auto tid = std::get<3>(data);
   auto&& transforms = std::get<4>(data);
   glCall(glUniformMatrix4fv,projectionViewMatrix,1,GL_FALSE,glm::value_ptr(mvp));
   glCall(glUniform3f,lightDirection,light.r,light.g,light.b);
   glCall(glActiveTexture,GL_TEXTURE0+unit);
   glCall(glBindTexture,GL_TEXTURE_2D,tid);
   for(int i=0;i<transforms.size();i++){
        auto& m = transforms[i];
        const int location = jointTransforms[i];
        glCall(glUniformMatrix4fv,location,1,GL_FALSE,glm::value_ptr(m));
   }
   return;
}   
void AnimatedModelShader::destroy(){
    auto f = [&](){
        std::cout << "projectionViewMatrix location:" << projectionViewMatrix
            << "\nlightDirection location:" << lightDirection
            << "\ndiffuseMap location:" << diffuseMap
            << "\njointTransforms location:" << jointTransforms
            << std::endl;
    };
    // f();
    glCall(glDeleteProgram,program);
}
std::ostream& operator <<(std::ostream& o,const std::vector<int>& ls){
    if(ls.size()==0) return o<<"[]";
    if(1==ls.size()) return o<<"["<< ls[0] << "]";
    for(int i=0;i<ls.size();i++){
        switch(i){
            case 0:{
            o<<"["<<ls[i];
            }break;
            default:
            o<<","<<ls[i];
        }
    }
    return o<<"]";
};

JointTransform::JointTransform(const Vector3f& v,const Quaternion& q)
:position(v)
,rotation(q)
{}

mat4 JointTransform::getLocalTransform()const{
    return mat4();//TODO
}
JointTransform JointTransform::interpolate(const JointTransform& l
        ,const JointTransform& r,float progress){
            return JointTransform(Vector3f(),Quaternion());//TODO
}