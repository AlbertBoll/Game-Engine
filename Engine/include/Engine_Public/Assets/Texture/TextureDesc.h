#pragma once

#include "Core/Base.h"

enum class TextureType : u8 { Tex2D, Cube };

enum class DefaultTextureKind : u8
{
    White,      //Albedo and Emissive
    FlatNormal, //Normal
    WhiteR,     //Metallic/Roughness/AO
    WhiteORM    //ORM
};

enum class TextureFormat : u8
{
    // LDR
    R8,
    RG8,
    RGB8,
    RGBA8,
    SRGB8,
    SRGB8A8,

    // HDR
    R16F,
    RG16F,
    RGB16F,
    RGBA16F,

    // Depth/Stencil
    Depth24Stencil8,
    Depth32F
};

enum class FlipY: u8
{
    YES,
    NO
};

enum class TextureCacheMode : u8
{
    UseCache,
    NoCache
};

enum class MinFilter : u8
{
    Nearest,
    Linear,
    NearestMipmapNearest,
    LinearMipmapLinear
};

enum class MagFilter : u8
{
    Nearest,
    Linear
};

enum class Wrap : u8 { Repeat, ClampToEdge, MirroredRepeat };

struct SamplerDesc
{
    MinFilter m_MinFilter = MinFilter::LinearMipmapLinear;
    MagFilter m_MagFilter = MagFilter::Linear;
    Wrap   m_WrapS     = Wrap::Repeat;
    Wrap   m_WrapT     = Wrap::Repeat;
    Wrap   m_WrapR     = Wrap::Repeat;

    float m_MaxAniso   = 8.0f;
    bool  b_EnableAniso = true;

    friend bool operator==(const SamplerDesc& a, const SamplerDesc& b) = default;
};

struct TextureDesc
{
    TextureType   m_Type   = TextureType::Tex2D;
    TextureFormat m_Format = TextureFormat::RGBA8;

    u32 m_Width  = 1;
    u32 m_Height = 1;

    u8   m_MipLevels = 0;    // 0 = auto full chain
    bool b_GenerateMips = true; // 上传后是否生成 mip（CreateEmpty 不会空生成）

    SamplerDesc m_Sampler{};

    friend bool operator==(const TextureDesc& a, const TextureDesc& b) = default;
};

struct TextureHandle
{
    u32 m_Id         = 0; // slot index + 1
    u32 m_Generation = 0; // stale handle protection

    explicit operator bool() const { return m_Id != 0; }
    friend bool operator==(const TextureHandle& a, const TextureHandle& b) = default;
};

// -------- 强类型选项（替代 bool） --------
enum class ColorSpace : u8
{
    Auto,        // LDR 3/4 channel -> sRGB；1/2 channel -> Linear
    ForceSRGB,   // force sRGB（only for LDR 3/4）
    ForceLinear  // force linear（normal/data texture）
};

enum class ImageRange : u8
{
    LDR,
    HDR
};

struct TextureFile2DLoadDesc
{
    std::string_view m_File;

    ImageRange  m_Range      = ImageRange::LDR;
    ColorSpace  m_ColorSpace = ColorSpace::Auto;

    bool b_FlipY    = true;
    bool b_UseCache = true;

    SamplerDesc      m_Sampler{};
    std::string_view m_DebugName{};
};

struct TextureCubeLoadDesc
{
    std::string_view m_BaseDir;
    std::string_view m_Extension = ".png";

    ImageRange m_Range      = ImageRange::LDR;
    ColorSpace m_ColorSpace = ColorSpace::Auto;

    bool b_UseCache = true;

    SamplerDesc      m_Sampler{};
    std::string_view m_DebugName{};
};

// -------- Cache Key：
struct TextureKey
{
    std::string m_Path;   // 2D: file；Cube: baseDir + "|" + extension
    bool        b_IsCube = false;

    ImageRange m_Range      = ImageRange::LDR;
    ColorSpace m_ColorSpace = ColorSpace::Auto;
    bool       b_FlipY      = false; // 2D ；Cube is false

    SamplerDesc m_Sampler{};

    friend bool operator==(const TextureKey& a, const TextureKey& b) = default;
};

struct TextureLoadOptions
{
    SamplerDesc m_Sampler{};
    std::string_view m_DebugName{};

    FlipY m_FlipY = FlipY::YES;
    TextureCacheMode m_CacheMode = TextureCacheMode::UseCache;
};
