#include"Renderer/Buffer.h"
#include <glad/gl.h>

unsigned int ToGLUsage(BufferUsage u)
{
    switch (u)
    {
    case BufferUsage::StaticDraw:  return GL_STATIC_DRAW;
    case BufferUsage::DynamicDraw: return GL_DYNAMIC_DRAW;
    case BufferUsage::StreamDraw:  return GL_STREAM_DRAW;
    }
    return GL_STATIC_DRAW;
}