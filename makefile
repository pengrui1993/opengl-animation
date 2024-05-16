CXXFLAGS+= -g -I/home/pengrui/learn/glad/include/ \
	  -I/home/pengrui/learn/stb/ \
	  -I/usr/local/include/ \
	  -L/usr/local/lib64/ 
	  
LDFLAGS+=-lglm -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl
main: glad.c main.cpp Camera.o Loader.o AnimatedModel.o OpenglUtil.o 
	g++ $(CXXFLAGS) $^ $(LDFLAGS) -o $@
template: template.cpp glad.c 
	g++ $(CXXFLAGS) $^ $(LDFLAGS) -o $@

.PHONY:clean

ALL_CPP=$(wildcard *.cpp)
ALL_EXE=$(ALL_CPP:%.cpp=%)
ALL_OBJ=$(ALL_CPP:%.cpp=%.o)
clean:
	rm -f $(ALL_EXE) $(ALL_OBJ)

