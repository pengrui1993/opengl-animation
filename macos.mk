INCLUDES=-I/Users/pengrui/learn/stb/ \
	 -I/Users/pengrui/learn/glad/include/ 
CXXFLAGS+=$(INCLUDES)
CFLAGS+=$(INCLUDES)
#LDFLAGS+= -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl
CXXFLAGS+= -std=c++17 -framework CoreVideo -framework OpenGL \
        -framework IOKit -framework Cocoa -framework Carbon
CXXFLAGS+=$(shell pkg-config --cflags glfw3 glm)
LDFLAGS+= $(shell pkg-config --libs glfw3 glm)
glad.o: glad.c
	gcc -c $(CFLAGS) $< -o $@
main: glad.o main.cpp Camera.o Loader.o AnimatedModel.o OpenglUtil.o 
	g++ $(CXXFLAGS) $^ $(LDFLAGS) -o $@
template: template.cpp glad.c 
	g++ $(CXXFLAGS) $^ $(LDFLAGS) -o $@

.PHONY:clean

ALL_CPP=$(wildcard *.cpp)
ALL_EXE=$(ALL_CPP:%.cpp=%)
ALL_OBJ=$(ALL_CPP:%.cpp=%.o)
clean:
	rm -f $(ALL_EXE) $(ALL_OBJ)

