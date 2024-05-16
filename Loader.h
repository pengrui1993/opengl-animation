#ifndef __LOADER_H_
#define __LOADER_H_

#include<vector>
#include<string>
#include<glm/glm.hpp>
#include<iostream>
#include<cinttypes>
typedef glm::mat4x4 mat4;
using String = std::string;
class JointData{
using List = std::vector<JointData>;
public:
    JointData()=default;
    JointData(int idx,const String& nid,const mat4& trans)
    :index{idx},nameId{nid}
    ,bindLocalTransform{trans}
    ,children{}{
    }
    JointData(const JointData&) = default;
    void addChild(JointData child){
        children.push_back(child);
    }
    JointData& operator = (const JointData& o) = default;
public:
    int index;
    String nameId;
    mat4 bindLocalTransform;
    List children;
};
class MeshData{
typedef std::vector<float> FloatList;
using IntList = std::vector<int>;
public:
    FloatList vertices;
    FloatList textureCoords;
    FloatList normals;
    IntList indices;
    IntList jointIds;
    FloatList vertexWeights;
public:
    MeshData():vertices(),textureCoords{}
        ,normals{},indices{}
        ,jointIds{},vertexWeights(){
        }
    void swap(FloatList&& vs,FloatList&& ts
    ,FloatList&& ns,IntList&& ids,IntList&& js
    ,FloatList&& vws){
        vertices.swap(vs);
        textureCoords.swap(ts);
        normals.swap(ns);
        indices.swap(ids);
        jointIds.swap(js);
        vertexWeights.swap(vws);
    }
};

class JointTransformData{
public:
    JointTransformData(String jid, mat4 tran)
    :jointNameId{jid},jointLocalTransform{tran}{}
    JointTransformData() = delete;
    String jointNameId;
    mat4 jointLocalTransform;
};

class KeyFrameData{
using TimePoint = float;
using List = std::vector<JointTransformData>;
public:
    TimePoint time;
    List jointTransform;
public:
    KeyFrameData() = default;
    KeyFrameData(TimePoint t):time(t){}
    KeyFrameData& addJointTransform(const JointTransformData& data){
        jointTransform.push_back(data);
        return *this;
    }
};

using KeyFramesData = std::vector<KeyFrameData>;

class Loader{
    using FilePath = std::string;
public:
    enum class FileType:u_int8_t{
        COLLADA//dae
    };
    virtual ~Loader(){}
    virtual int getJointCount() = 0;
    virtual float getAnimationTime() = 0;
    virtual JointData& getRootJointData() = 0;
    virtual MeshData& getMeshData() = 0;
    virtual KeyFramesData& getKeyFramesData() = 0;
    static Loader* create(FilePath,FileType=FileType::COLLADA);
    static void destroy(Loader*);
};

#endif
