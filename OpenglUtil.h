#ifndef __OPENGL_UTIL_H_
#define __OPENGL_UTIL_H_

//https://gitee.com/lamora/SFML/blob/master/src/SFML/Graphics/GLCheck.hpp
//https://gitee.com/pengrui1993/hello-openal001/blob/master/myal.cpp
#include<string>
#include<filesystem>
struct OpenglUtil{
    // using Path = std::filesystem::path;
    using Path = std::string;
    using Line = std::uint_fast32_t;
OpenglUtil() = delete;
~OpenglUtil() = delete;
template <typename GlFunction, typename... Params>
static auto glCall(Path file,Line line,GlFunction function,Params...params)
    -> typename std::enable_if<std::is_same<void, decltype(function(params...))>::value, decltype(function(params...))>::type{
    function(std::forward<Params>(params)...);
    checkError(file,line);
}
template <typename GlFunction, typename... Params>
static auto glCall(Path file,Line line,GlFunction function,Params...params)
    ->typename std::enable_if<!std::is_same<void, decltype(function(params...))>::value
                                , decltype(function(params...))>::type{
    auto ret = function(std::forward<Params>(params)...);
    checkError(file,line);
    return ret;
}
template <typename GlFunction>
static auto glCall0(Path file,Line line,GlFunction function)
    -> typename std::enable_if<std::is_same<void
        , decltype(function())>::value
        , decltype(function())>::type{
    function();
    checkError(file,line);
}
template <typename GlFunction>
static auto glCall0(Path file,Line line,GlFunction function)
    -> typename std::enable_if<!std::is_same<void
        , decltype(function())>::value
        , decltype(function())>::type{
    auto ret = function();
    checkError(file,line);
    return ret;
}
static const int TRUE{1};
static const int FALSE{0};
static const int exitOnError{TRUE};

static void checkError(const Path& file,Line line);
};

#define DEBUG 1
#ifdef DEBUG

#endif
#if (DEBUG)
#define glCall(function,...) OpenglUtil::glCall(__FILE__,__LINE__,function,__VA_ARGS__)
#define glCall0(function) OpenglUtil::glCall0(__FILE__,__LINE__,function)
#else
#define glCall(function,...) function(__VA_ARGS__)
#define glCall0(function) function()
#endif



#endif