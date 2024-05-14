#include<vector>
#include<unordered_map>
#include<unordered_set>
#include<iostream>
#include<cmath>
#include<glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
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
    return 180/PI*f;
}
class CameraImpl: public Camera{
public:
    CameraImpl():Camera(){
        while(ids.count(id = ++nextId));
        ids.insert(id);
        cameras[id] = this;
        initMatrix();
    }
    virtual ~CameraImpl()override{
        ids.erase(id);
        cameras[id] = nullptr;
    }
    virtual void move() override {
        if(!dirty)return;
        std::cout << "camera id:" << id<< std::endl;
        viewMatrix = glm::translate(mat4(1),vec3(-position.x,-position.y,-position.z));
        dirty = false;
    }
    virtual bool getProjectViewMatrixData(float array[16]) override{
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
        float aspectRatio = DISPLAY_WIDTH/DISPLAY_HEIGHT;
        auto y_scale = 1.0f/tan(rta(FOV/2));
        auto x_scale = y_scale/aspectRatio;
        float frustum_length = FAR_PLANE - NEAR_PLANE;
        m[0][0] = x_scale;
        m[1][1] = y_scale;
        m[2][2] = (-((FAR_PLANE + NEAR_PLANE) / frustum_length));
        m[2][3] = -1;
        m[3][2] = (-((2 * NEAR_PLANE * FAR_PLANE) / frustum_length));
        m[3][3] = 0;

        for(int i=0;i<4;i++)viewMatrix[i][i] = 1;
    }
private:
    static ID nextId;
private:
    bool dirty{true};
    ID id;
    mat4 projectMatrix{0};
    mat4 viewMatrix{0};
    vec3 position{0};
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