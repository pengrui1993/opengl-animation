CXXFLAGS+=-I/home/pengrui/learn/glad/include/ \
	  -I/usr/local/include/ 
	  
LDFLAGS+=-L/usr/local/lib64/ \
	-lglm -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl
all:
	g++ -g -I/home/pengrui/learn/glad/include/ \
	-I/usr/local/include/ \
	-L/usr/local/lib64/ \
	glad.c main.cpp Camera.cpp Loader.cpp AnimatedModel.cpp OpenglUtil.cpp \
	-lglm -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl \
	-o main


.PHONY:clean

ALL_CPP=$(wildcard *.cpp)
ALL_EXE=$(ALL_CPP:%.cpp=%)

clean:
	rm -f $(ALL_EXE)

