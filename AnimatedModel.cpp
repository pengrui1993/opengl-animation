#include <glad/glad.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cinttypes>
#include <cstdarg>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/quaternion.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "AnimatedModel.h"
#include "OpenglUtil.h"
#include "Loader.h"
namespace{
    void mprint(const mat4 m){
        std::printf("[%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f]\n"
            ,m[0][0],m[1][0],m[2][0],m[3][0]
            ,m[0][1],m[1][1],m[2][1],m[3][1]
            ,m[0][2],m[1][2],m[2][2],m[3][2]
            ,m[0][3],m[1][3],m[2][3],m[3][3]
        );
    }
    void vprint(const glm::vec3 v){
         std::printf("x:%f,y:%f,z:%f\n"
            ,v.x,v.y,v.z
        );
    }
    void qprint(const glm::quat q){
        std::printf("x:%f,y:%f,z:%f,w:%f\n"
            ,q.x,q.y,q.z,q.w
        );
    }    
}
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
        GLsizei len;
        glCall(glGetShaderiv,vertexShader, GL_INFO_LOG_LENGTH, &len);
        std::cerr << "compile vertex shader error"<<",max log len:" << len << std::endl;
        GLchar msg[512];
        glCall(glGetShaderInfoLog,vertexShader,sizeof(msg),&len,msg);
        std::cout << msg<< std::endl;
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
        GLsizei len;
        GLchar msg[512];
        glCall(glGetShaderInfoLog,fragmentShader,sizeof(msg),&len,msg);
        std::cout << msg<< std::endl;
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
        jointTransforms[i] = glCall(glGetUniformLocation
            ,program,ss.str().c_str());
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
   auto&& transforms = *std::get<4>(data);
   glCall(glUniformMatrix4fv,projectionViewMatrix,1,GL_FALSE,glm::value_ptr(mvp));
   glCall(glUniform3f,lightDirection,light.r,light.g,light.b);

   glCall(glActiveTexture,GL_TEXTURE0+unit);
   glCall(glBindTexture,GL_TEXTURE_2D,tid);
   for(int i=0;i<transforms.size();i++){
        auto& m = transforms[i];
        const int location = jointTransforms[i];
        glCall(glUniformMatrix4fv,location,1,GL_FALSE,glm::value_ptr(m));
   }
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
namespace{
    struct MeshVaoCreator{
        static const int vboSize{6};
        using Vbo6 = unsigned int [vboSize];
        using Integers = std::vector<int>;
        using Floats = std::vector<float>;
        using VaoId = unsigned int;
        MeshVaoCreator(MeshData& meshData){
            glCall(glGenVertexArrays,1,&vaoid);
            glCall(glBindVertexArray,vaoid);
            auto& indices = meshData.indices;
            indexCount = indices.size();
            glCall(glGenBuffers,1,&vbos[5]);
            glCall(glBindBuffer,GL_ELEMENT_ARRAY_BUFFER,vbos[5]);
            glCall(glBufferData,GL_ELEMENT_ARRAY_BUFFER
                ,indices.size()*sizeof(std::remove_reference<decltype(indices)>::type::value_type)
                ,indices.data(),GL_STATIC_DRAW);
            createAttribute(0,meshData.vertices,3);
            createAttribute(1,meshData.textureCoords,2);
            createAttribute(2,meshData.normals,3);
            createIntAttribute(3,meshData.jointIds,3);
        }
        VaoId vaoid;
        int indexCount;
        Vbo6 vbos;
        static const int BYTES_PER_FLOAT = 4;
        static const int BYTES_PER_INT = 4;
        private:
            void createIntAttribute(int attribute,Integers& data,int attrSize){
                glCall(glGenBuffers,1,&vbos[attribute]);
                glCall(glBindBuffer,GL_ARRAY_BUFFER,vbos[attribute]);
                glCall(glBufferData,GL_ARRAY_BUFFER
                ,data.size()*sizeof(std::remove_reference<decltype(data)>::type::value_type)
                ,data.data(), GL_STATIC_DRAW);
                //注意这里是 IPinter 
                glCall(glVertexAttribIPointer,attribute, attrSize, GL_INT,attrSize * BYTES_PER_INT, nullptr);
                glCall(glBindBuffer,GL_ARRAY_BUFFER, 0);
            }
            void createAttribute(int attribute,Floats& data,int attrSize){
                glCall(glGenBuffers,1,&vbos[attribute]);
                glCall(glBindBuffer,GL_ARRAY_BUFFER,vbos[attribute]);
                glCall(glBufferData,GL_ARRAY_BUFFER
                ,data.size()*sizeof(std::remove_reference<decltype(data)>::type::value_type)
                ,data.data(), GL_STATIC_DRAW);
                glCall(glVertexAttribPointer,attribute, attrSize, GL_FLOAT,false,attrSize * BYTES_PER_FLOAT,nullptr);
                glCall(glBindBuffer,GL_ARRAY_BUFFER, 0);
            }
    };

   struct JointCreator{
    using mat4 = glm::mat4;
    using Child = std::shared_ptr<Joint>;
    explicit JointCreator(JointData& data):rootData(data){
        for(auto& child:data.children){

        }
    }
    Child createJoints(){
        return createJoints(rootData);
    }
    private:
        Child createJoints(JointData& data){
            auto joint = std::make_shared<Joint>(data.index,data.nameId,data.bindLocalTransform);
            for(auto& child:data.children){
                joint->addChild(createJoints(child));
            }
            return joint;
        }
    private:
        JointData& rootData;
   };
   struct TextureUtils{
    using ID = GLuint;
    using FilePath = std::string;
    ID decodeTextureFile(FilePath path){
        if(id!=DEFAULT)
            return id;
        int width,height,nrChannels;
        auto* data = stbi_load(path.c_str(),&width,&height,&nrChannels,0);
        if(!data){
            std::cerr << "load image :"<<path << " error" << std::endl;
            std::exit(-1);
        }
        glCall(glGenTextures,1,&id);
        glCall(glActiveTexture,GL_TEXTURE0);
        glCall(glBindTexture,GL_TEXTURE_2D,id);
        glCall(glPixelStorei,GL_UNPACK_ALIGNMENT,1);
        glCall(glTexImage2D,GL_TEXTURE_2D
            ,0,GL_RGBA,width,height,0,GL_BGRA,GL_UNSIGNED_BYTE,data);
        glCall(glGenerateMipmap,GL_TEXTURE_2D);
        glCall(glTexParameteri,GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glCall(glTexParameteri,GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        bool GL_EXT_texture_filter_anisotropic = false;//TODO
        if(GL_EXT_texture_filter_anisotropic){
            int GL_TEXTURE_MAX_ANISOTROPY_EXT = -1;
            glCall(glTexParameterf,GL_TEXTURE_2D,GL_TEXTURE_LOD_BIAS,0);
            glCall(glTexParameterf,GL_TEXTURE_2D,GL_TEXTURE_MAX_ANISOTROPY_EXT,4.0f);
        }
        glCall(glTexParameteri,GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glCall(glTexParameteri,GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
        glCall(glBindTexture,GL_TEXTURE_2D,0);
        stbi_image_free(data);
        return id;
    }
    private:
        static const ID DEFAULT{};
        ID id{DEFAULT};
   };
}
AnimatedModel::~AnimatedModel(){
    	glCall(glDeleteVertexArrays,1,&vaoid);
		glCall(glDeleteTextures,1,&texture);
		for(int i=0;i<vboSize;i++)
			glDeleteBuffers(1,&vbos[i]);
}
AnimatedModel::AnimatedModel(Loader*loader,TextureFilePath path){
    auto& mesh = loader->getMeshData();
    MeshVaoCreator holder(mesh);
    vaoid = holder.vaoid;
    std::memcpy(vbos,holder.vbos,vboSize);
    indexCount = holder.indexCount;
    TextureUtils util;
    texture = util.decodeTextureFile(path);
    JointCreator creator(loader->getRootJointData());
    rootJoint = creator.createJoints();
    rootJoint->calcInverseBindTransform(mat4(1));
    jointCount = loader->getJointCount();
    animations = Animations(loader);
    animationTime = 0;
    currentAnimation = &animations.getDefaultAnimation();
    repeatAnimation = true;
}
AnimatedModel::TextureId AnimatedModel::getTextureId(){
    return texture;
}
namespace{
    static int counter=0;
}
void AnimatedModel::getJointTransforms(Mat4List& jointMatrices){
    jointMatrices.resize(jointCount);
    addJointsToArray(rootJoint,jointMatrices);
}

void AnimatedModel::addJointsToArray(SharedJoint headJoint,Mat4List&jointMatrices){
    if(!headJoint)return;
	jointMatrices[headJoint->index] = headJoint->animatedTransform;
    for (auto childJoint : headJoint->children) {
        addJointsToArray(childJoint, jointMatrices);
    }
}
static int count = 0;
void AnimatedModel::applyPoseToJoints(Pose& currentPose
    ,SharedJoint joint ,const mat4& parentTransform){
    auto& curLocationTransform = currentPose[joint->name];
    auto currentTransform = parentTransform*curLocationTransform;
    joint->animatedTransform = currentTransform*joint->inverseBindTransform;
    for(auto childJoint:joint->children){
        applyPoseToJoints(currentPose,childJoint,currentTransform);
    }
}
void AnimatedModel::draw(){
    	glCall(glBindVertexArray,vaoid);
        for(int i=0;i<5;i++)
            glCall(glEnableVertexAttribArray,i);
    	//glCall(glPolygonMode,GL_FRONT_AND_BACK, GL_LINE);
        glCall(glDrawElements,GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
        for(int i=0;i<5;i++)
            glCall(glDisableVertexAttribArray,i);
        glCall(glBindVertexArray,0);
}
void AnimatedModel::update(){
        if(!currentAnimation||stop)return;
        animationTime+= (1.0f/50);
        if(repeatAnimation){
            if(animationTime>=currentAnimation->length)
                animationTime -= currentAnimation->length;
        }else{
            currentAnimation = &animations.getDefaultAnimation();
            animationTime = 0;
        }
        auto frams = currentAnimation->keyframes;
        auto* previousFrame = &frams[0];
        auto* nextFrame = previousFrame;
        for(int i=1;i<frams.size();i++) {
            nextFrame = &frams[i];
            if(nextFrame->time>animationTime){
                break;
            }
            previousFrame = nextFrame;
        }
        float progression = (animationTime-previousFrame->time)/(nextFrame->time-previousFrame->time);
        Pose currentPose = KeyFrame::interpolatePoses(*previousFrame, *nextFrame, progression);
        applyPoseToJoints(currentPose,rootJoint,mat4(1));
}
namespace{
    using Quaternion = glm::quat;
    using Vector3f = glm::vec3;
    using KeyFrames = std::vector<KeyFrame>;
    using TransformsMap = std::unordered_map<std::string,JointTransform>;
    using Pose = std::unordered_map<std::string,glm::mat4x4>;
    KeyFrames getKeyFrames(Loader* l){
        auto& fds = l->getKeyFramesData();
        KeyFrames keyframes(fds.size());
        for(int i=0;i<keyframes.size();i++){
            TransformsMap map;
            for(auto& jointData:fds[i].jointTransform){
                mat4& m = jointData.jointLocalTransform;
                Vector3f translation{m[3][0],m[3][1],m[3][2]};
                mat4 rot(m);
                rot[3][0]=rot[3][1]=rot[3][2]=0;
                Quaternion rotation(rot);
                map[jointData.jointNameId] = 
                    JointTransform(translation,rotation);
            }
            keyframes[i] = KeyFrame(fds[i].time,std::move(map));
        }

        return keyframes;
    }
}
KeyFrame::KeyFrame(float t,TransformsMap&& p){
    pose.swap(p);
    time = t;
}
Pose KeyFrame::interpolatePoses(KeyFrame& previousFrame
        ,KeyFrame& nextFrame,float progress){
    Pose currentPose;
    for (auto& entry: previousFrame.pose) {
        auto& jointName = entry.first;
        JointTransform previousTransform = previousFrame.pose[jointName];
        JointTransform nextTransform = nextFrame.pose[jointName];
        JointTransform currentTransform = 
                JointTransform::interpolate(previousTransform, nextTransform, progress);
        currentPose[jointName]=currentTransform.getLocalTransform();
  

    }
 
    return currentPose;
}
Animation::Animation(float len,KeyFrames&& kfs){
    length = len;
    keyframes.swap(kfs);
}
Animations::Animations(Loader* l){
    const auto len = l->getAnimationTime();
    animations.push_back(Animation(len,getKeyFrames(l)));
}

Animation& Animations::get(Index index){
    if(index>animations.size()){
        std::cerr << "invalid index of animation, max:"
                << animations.size()-1
                <<",but get:" << index
                << std::endl;
        std::exit(-1);
    }
    return animations[index];
}
Animation& Animations::getDefaultAnimation(){
    return get(0);
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
namespace{
    using mat4 = glm::mat4x4;
}
mat4 JointTransform::getLocalTransform()const{
    auto m = glm::translate(mat4(1),position)*mat4(rotation);
    return m;
}
namespace{
    Vector3f inter_v3f(const Vector3f& start
        ,const Vector3f& end,float progression){
        float x = start.x + (end.x - start.x) * progression;
		float y = start.y + (end.y - start.y) * progression;
		float z = start.z + (end.z - start.z) * progression;
        return Vector3f(x,y,z);
    }
    Quaternion inter_q4f(const Quaternion& a
        ,const Quaternion& b,float t){
	    float dot = a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
		float blend;
		float blendI;
		blend = t;
		blendI = 1.f - blend;
		if (dot < 0) {
			blend = -blend;
		}
		Quaternion result(1, 0, 0, 0);// w,x,y,z
		result.w = blendI * a.w + blend * b.w;
		result.x = blendI * a.x + blend * b.x;
		result.y = blendI * a.y + blend * b.y;
		result.z = blendI * a.z + blend * b.z;
		return glm::normalize(result);
    }
}
JointTransform JointTransform::interpolate(JointTransform& l
        ,JointTransform& r,float progress){
    Vector3f pos = inter_v3f(l.position, r.position, progress);
    Quaternion rot = inter_q4f(l.rotation, r.rotation, progress);
    return JointTransform(pos,rot);
}

Joint& Joint::calcInverseBindTransform(const mat4& parentBindTransform){
    mat4 bindTransform = parentBindTransform*localBindTransform;
    inverseBindTransform = glm::inverse(bindTransform);
    for(auto child:children){
        child->calcInverseBindTransform(bindTransform);
    }
    return *this;
}

  

void Joint::addChild(Chlid child){
    children.push_back(child);
}
