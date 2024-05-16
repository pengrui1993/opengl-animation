#include<vector>
#include<unordered_map>
#include<unordered_set>
#include<iostream>
#include<cmath>
#include<glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"
typedef glm::mat4x4 mat4;
typedef glm::vec3 vec3;
namespace{
using ID = int;
using Map = std::unordered_map<ID,Camera*>;
using List = std::vector<Camera*>;
using Set = std::unordered_set<ID>;
Map cameras;
Set ids;
constexpr float FOV = 70;
constexpr float NEAR_PLANE = 0.2f;
constexpr float FAR_PLANE = 400;
constexpr float PI = 3.1415926f;
constexpr int DISPLAY_WIDTH = 640;
constexpr int DISPLAY_HEIGHT = 480;
constexpr float rta(float f){
    return f*180/PI;
}
constexpr float atr(float f){
    return PI*f/180.f;
}
namespace{
    void mprint(const mat4 m){
        std::printf("[%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f]\n"
            ,m[0][0],m[1][0],m[2][0],m[3][0]
            ,m[0][1],m[1][1],m[2][1],m[3][1]
            ,m[0][2],m[1][2],m[2][2],m[3][2]
            ,m[0][3],m[1][3],m[2][3],m[3][3]
        );
    }
}
class CameraImpl: public Camera{
public:
    CameraImpl():Camera(){
        while(ids.count(id = ++nextId));
        ids.insert(id);
        cameras[id] = this;
        initMatrix();
        // std::cout << "camera id:" << id<< std::endl;
    }
    virtual ~CameraImpl()override{
        ids.erase(id);
        cameras[id] = nullptr;
    }
    virtual void move() override {
        if(!dirty)return;
        viewMatrix = glm::rotate(mat4(1),atr(pitch),vec3(1,0,0));
        viewMatrix = glm::rotate(viewMatrix,atr(yaw),vec3(0,1,0));
        viewMatrix = glm::translate(viewMatrix,vec3(-position.x,-position.y,-position.z));
        dirty = false;
    }
    virtual bool getProjectViewMatrixData(mat4& mat) override{
        mat = projectMatrix*viewMatrix;
        return true;
    };
    static Camera* create(){
        CameraImpl* c = new 
        // (std::nothrow)
        CameraImpl;
        return c;
    }
    static void destroy(Camera* c){
        if(!c)return;
        delete c;
    }
private:
    void initMatrix(){
        mat4& m = projectMatrix;
        float aspectRatio = static_cast<float>(DISPLAY_WIDTH)/static_cast<float>(DISPLAY_HEIGHT);
        auto y_scale = static_cast<float>(1.0f/tan(atr(FOV/2.f)));
        auto x_scale = y_scale/aspectRatio;
        float frustum_length = FAR_PLANE - NEAR_PLANE;
        m[0][0] = x_scale;
        m[1][1] = y_scale;
        m[2][2] = (-((FAR_PLANE + NEAR_PLANE) / frustum_length));
        m[2][3] = -1;
        m[3][2] = (-((2 * NEAR_PLANE * FAR_PLANE) / frustum_length));
        m[3][3] = 0;
    }
private:
    static ID nextId;
private:
    bool dirty{true};
    ID id;
    mat4 projectMatrix{1};
    mat4 viewMatrix{1};
    vec3 position{0,6.7f,9.8f};
    float yaw{0};
    float pitch{10};
    float angleAroundPlayer{0};
    float distanceFromPlayer{10};
};
ID CameraImpl::nextId = 0;
};

Camera* Camera::create(){
    return CameraImpl::create();
}

void Camera::destroy(Camera* c){
    CameraImpl::destroy(c);
}