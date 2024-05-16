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
#include "Loader.h"
#include <memory>
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
    ,GLuint //texture unit
    ,GLuint // texture id
    ,std::vector<glm::mat4x4>* // joint transforms    
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
    using Vector3f = glm::vec3;
    using Quaternion = glm::quat;
    using mat4 = glm::mat4x4;
public:
    JointTransform() = default;
    JointTransform(const Vector3f&,const Quaternion&);
    JointTransform(const JointTransform&) = default;
    mat4 getLocalTransform()const;
    static JointTransform interpolate(JointTransform& l
        ,JointTransform& r,float progress);
    JointTransform& operator=(const JointTransform&) = default;

public:
    Vector3f position;
    Quaternion rotation;
};
class KeyFrame{
    using TransformsMap = std::unordered_map<std::string,JointTransform>;
    using Pose = std::unordered_map<std::string,glm::mat4x4>;
public:
    float time;//point
    TransformsMap pose;
public:
    KeyFrame()=default;
    KeyFrame(float t,TransformsMap&& p);
    static Pose interpolatePoses(KeyFrame& l
        ,KeyFrame& r,float progress);
};

class Animation{
using KeyFrames = std::vector<KeyFrame>;
public:
    float length;
    KeyFrames keyframes;
public:
    Animation(float len,KeyFrames&& kfs);
};
class Animations{
    using Index = uint32_t;
    using List = std::vector<Animation>;
public:
    Animations()=default;
    Animations(Loader*);
    Animation& get(Index index);
    Animation& getDefaultAnimation();
private:
    List animations;
};
class Joint{
    using String = std::string;
    using Chlid = std::shared_ptr<Joint>;
    using Children = std::vector<Chlid>;
    using mat4 = glm::mat4x4;
public:
    Joint()=default;
    Joint(int idx,String nid,const mat4& lbt)
        :index(idx),name(nid),children()
        ,localBindTransform(lbt),inverseBindTransform(1)
        ,animatedTransform(1){
 
        }
    void addChild(Chlid child);
    Joint& calcInverseBindTransform(const mat4& parentBindTransform);
    int index;
    String name;
    Children children;
    mat4 localBindTransform;
    mat4 inverseBindTransform;
    mat4 animatedTransform;
};
class AnimatedModel{
using TextureId = GLuint;
using VaoId = GLuint;
using mat4 = glm::mat4x4;
using Mat4List = std::vector<mat4>;
static const int vboSize{6};
using Vbo6 = GLuint [vboSize];
using Pose = std::unordered_map<std::string,glm::mat4x4>;
public:
    using TextureFilePath = std::string;
    using SharedJoint = std::shared_ptr<Joint>;
    AnimatedModel(Loader*loader,TextureFilePath path);
    ~AnimatedModel();
    void update();
    void draw();
    TextureId getTextureId();
    void getJointTransforms(Mat4List&);
private:
    void addJointsToArray(SharedJoint headJoint,Mat4List&jointMatrices);
    void applyPoseToJoints(Pose& currentPose,SharedJoint joint
        ,const mat4& parentTransform);
private:
    //mesh
    VaoId vaoid;
    int indexCount;
    TextureId texture;
    //animation
    Animations animations;
    Animation* currentAnimation;
    float animationTime;
    bool repeatAnimation;
    //joint
    SharedJoint rootJoint;
    int jointCount;
    Vbo6 vbos;
    //
    bool stop {false};
    int64_t lastModifyTime;
    static mat4 ROOT_TRANSFORM;
};
#endif
