#ifndef __CAMERA_H_
#define __CAMERA_H_
#include<glm/mat4x4.hpp>
class Camera{
public:
virtual ~Camera(){}
virtual void move() = 0;
virtual bool getProjectViewMatrixData(float[16]) = 0;
static Camera* create();
static void destroy(Camera*);
};
#endif