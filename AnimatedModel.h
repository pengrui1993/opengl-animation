#ifndef __ANIMATED_MODEL_H_
#define __ANIMATED_MODEL_H_
#include<vector>
#include<string>
#include<iostream>
#include<tuple>
#include<glad/glad.h>
#include<glm/glm.hpp>
#include<glm/gtc/quaternion.hpp>
#include<unordered_map>
using Vector3f = glm::vec3;
using Quaternion = glm::quat;
using mat4 = glm::mat4x4;
class AnimatedModelShader{
    using FilePath = std::string;
    using String = std::string;
    using Locations = std::vector<int>;
    using Location = int;
    using ShaderId = int;
    using ProgramId = int;
    const GLint NOT_FOUND = -1;
    const int MAX_JOINTS = 50;
    const int DIFFUSE_TEX_UNIT = 0;
    using PassedData = std::tuple<glm::mat4x4 //mvp
    ,glm::vec3  //light
    ,int //texture unit
    ,int // texture id
    ,std::vector<glm::mat4x4>// joint transforms    
    >;
private:
    ProgramId program;
    Location projectionViewMatrix;
    Location lightDirection;
    Locations jointTransforms;
    Location diffuseMap;
public:
    AnimatedModelShader(FilePath vs,FilePath fs);
    ~AnimatedModelShader();
    void enable();
    void disable();
    void passData(PassedData&);
private:
    void destroy();
    bool checkStoreUniformLocation(Location location);
    bool checkProgramLinkStatus();
    bool checkShaderCompileStatus(ShaderId id);
    bool getSource(FilePath file,String& data);
};
std::ostream& operator <<(std::ostream& o,const std::vector<int>& ls);

class JointTransform{
public:
    JointTransform(const Vector3f&,const Quaternion&);
    JointTransform(const JointTransform&) = default;
    mat4 getLocalTransform()const;
    static JointTransform interpolate(const JointTransform& l
        ,const JointTransform& r,float progress);
    JointTransform& operator=(const JointTransform&) = default;

private:
    Vector3f position;
    Quaternion rotation;
};
class KeyFrame{
    using TransformsMap = std::unordered_map<std::string,JointTransform>;
    using Pose = std::unordered_map<std::string,glm::mat4x4>;
private:
    const float time;//point
    const TransformsMap pose;
public:
    KeyFrame(float t,const Pose& p);
    static Pose interpolate(const KeyFrame& l
        ,const KeyFrame& r,float progress);
};

class Animation{
using KeyFrames = std::vector<KeyFrame>;
private:
    float length;
    KeyFrames keyframes;
};
class Animations{
    using List = std::vector<Animation>;
private:
    List animations;
};
class Joint{
    using String = std::string;
    using List = std::vector<Joint>;
    using mat4 = glm::mat4x4;
public:
    void addChild(Joint child);
private:
    int index;
    String name;
    List children;
    mat4 localBindTransform;
    mat4 inverseBindTransform;
    mat4 animatedTransform;
};
class AnimatedModel{
using TextureId = int;
using VaoId = int;
using mat4 = glm::mat4x4;
private:
    //mesh
    VaoId vao;
    int indexCount;
    TextureId texture;
    //animation
    Animations animations;
    Animation* currentAnimation;
    float animationTime;
    bool repeatAnimation;
    //joint
    Joint rootJoint;
    int jointCount;
    static mat4 ROOT_TRANSFORM;
};
#endif
