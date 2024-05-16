#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>
int main(){
    int width,height,nrChannels;
    auto* data = stbi_load("./resources/diffuse.png",&width,&height,&nrChannels,0);
    if(data){
        std::cout << "load ok,"
                << "width:"<<width
                << "height:" << height
                << "channels:" <<nrChannels
                << std::endl;
    }
    stbi_image_free(data);
    return 0;
}