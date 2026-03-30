#pragma once

#include "TextureDesc.h"
#include "Core/Hash.h"
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <span>
#include <cstddef>


template<>
struct CustomHash<SamplerDesc>
{
    std::size_t operator()(const SamplerDesc& s) const noexcept
    {
        return Combine(
            s.m_MinFilter,
            s.m_MagFilter,
            s.m_WrapS,
            s.m_WrapT,
            s.m_WrapR,
            s.b_EnableAniso,
            (int)std::lround(s.m_MaxAniso * 1000.0f)
        );
    }
};

struct TextureKeyHash
{
    std::size_t operator()(const TextureKey& k) const noexcept;
};

class TextureManager
{
public:
    TextureManager() = default;
    ~TextureManager();

    DELETE_COPY(TextureManager)

    TextureHandle CreateEmpty(const TextureDesc& desc, std::string_view debugName = {});
    TextureHandle CreateFromPixels2D(const TextureDesc& desc,
                                     std::span<const std::byte> pixels,
                                     u32 rowStrideBytes = 0,
                                     std::string_view debugName = {});

    TextureHandle CreateFromFile2D(const TextureFile2DLoadDesc& d);
    TextureHandle CreateFromFileCube(const TextureCubeLoadDesc& d);

    void Bind(TextureHandle h, u32 unit) const;
    void Destroy(TextureHandle h);

    void GenerateMips(TextureHandle h);
    
    void InitDefaultTextures();
    TextureHandle GetDefaultTexture(DefaultTextureKind kind)const;


    bool IsValid(TextureHandle h) const;
    const TextureDesc& GetDesc(TextureHandle h) const;

    //necessary for invoke if use raw openGL api to change state
    void InvalidateBindCache() const;

    TextureHandle LoadColor2D(std::string_view file,
                          const TextureLoadOptions& opt = {});

    TextureHandle LoadData2D(std::string_view file,
                            const TextureLoadOptions& opt = {});

    TextureHandle LoadHDR2D(std::string_view file,
                            const TextureLoadOptions& opt = {});

    TextureHandle LoadColorCube(std::string_view baseDir,
                                std::string_view extension = ".png",
                                const TextureLoadOptions& opt = {});

    TextureHandle LoadHDRCube(std::string_view baseDir,
                            std::string_view extension = ".hdr",
                            const TextureLoadOptions& opt = {});

    TextureHandle LoadAlbedoMap(std::string_view file,
                            const TextureLoadOptions& opt = {});

    TextureHandle LoadNormalMap(std::string_view file,
                                const TextureLoadOptions& opt = {});

    TextureHandle LoadMRMap(std::string_view file,
                            const TextureLoadOptions& opt = {});

    TextureHandle LoadEmissiveMap(std::string_view file,
                                const TextureLoadOptions& opt = {});

    u32 GetNativeTexture(TextureHandle h) const;

private:
    struct Slot
    {
        u32 m_GLTex     = 0; 
        u32 m_GLSampler = 0;

        TextureDesc m_Desc{};
        u32 m_Generation = 1;
        bool b_Alive = false;

        u32  m_RefCount = 0;
        bool b_HasKey   = false;
        TextureKey m_Key{};
    };

    struct SamplerCache; //implemented in cpp file

    std::vector<Slot> m_Slots;
    std::vector<u32>  m_FreeList;

    std::unordered_map<TextureKey, TextureHandle, TextureKeyHash> m_FileCache;

    SamplerCache* m_SamplerCache = nullptr;

    // Bind denote const，so cache must mutable
    mutable std::vector<u32> m_BoundTex;
    mutable std::vector<u32> m_BoundSampler;
    mutable u32 m_ActiveUnit = 0xFFFFFFFFu;

    mutable std::array<TextureHandle, 4> m_DefaultTextures{};
    mutable bool b_DefaultTexturesReady = false;

private:
    TextureHandle AllocateSlot();
    Slot*       GetSlot(TextureHandle h);
    const Slot* GetSlot(TextureHandle h) const;
    TextureHandle CreateEmptyInternal(const TextureDesc& inDesc,
                                      std::string_view debugName);
    TextureHandle CreateSolidRGBA8(u8 r, u8 g, u8 b, u8 a,
                                   std::string_view debugName);
    void CreateDefaultTextures();

    void EnsureUnitCacheInit() const;

    u32  GetOrCreateSampler(const SamplerDesc& s);
    void DestroySamplerCache();

    TextureHandle TryGetFromCache(const TextureKey& key);
    TextureHandle StoreToCache(const TextureKey& key, TextureHandle h);
};