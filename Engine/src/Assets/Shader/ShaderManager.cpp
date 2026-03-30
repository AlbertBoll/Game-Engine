#include"Assets/Shader/ShaderManager.h"
#include<glad/gl.h>
#include "Core/Base.h"
#include "Core/Hash.h"


#ifndef SHADERMGR_USE_TIMESTAMP
    #define SHADERMGR_USE_TIMESTAMP 0
#endif

// ---- hash helpers ----
u64 ShaderManager::HashString64(std::string_view s)
{
    u64 h = 1469598103934665603ull;
    for (unsigned char c : s)
    {
        h ^= (u64)c;
        h *= 1099511628211ull;
    }
    return h;
}

u64 ShaderManager::HashCombine64(u64 a, u64 b)
{
    a ^= b + 0x9e3779b97f4a7c15ull + (a << 6) + (a >> 2);
    return a;
}

u64 ShaderManager::HashStageItem(ShaderStage st, u64 payloadHash, u64 kindTag)
{
    u64 h = kindTag;
    h = HashCombine64(h, (u64)st + 0x100ull);
    h = HashCombine64(h, payloadHash);
    return h;
}

std::size_t ShaderKeyHash::operator()(const ShaderKey& k) const noexcept
{
    // u64 h = 0xcbf29ce484222325ull;
    // h = ShaderManager::HashCombine64(h, (u64)k.kind);
    // h = ShaderManager::HashCombine64(h, (u64)k.count);
    // for (u32 i = 0; i < (u32)k.count; ++i)
    //     h = ShaderManager::HashCombine64(h, k.items[i]);
    // return (std::size_t)h;
    std::size_t h = Combine(k.kind, k.count);

    for (u8 i = 0; i < k.count; ++i)
        AppendOne(h, k.items[i]);

    return h;

}

// ---- helpers: path normalize + timestamp ----
static inline std::string NormalizePath(std::string_view p)
{
    namespace fs = std::filesystem;
    fs::path pp{std::string(p)};
    return pp.lexically_normal().generic_string();
}


#if SHADERMGR_USE_TIMESTAMP
static inline u64 FileLastWriteTime64(std::string_view path)
{
    namespace fs = std::filesystem;
    std::error_code ec;
    auto ft = fs::last_write_time(fs::path{std::string(path)}, ec);
    if (ec) return 0;
    return (u64)ft.time_since_epoch().count();
}
#else
static inline u64 FileLastWriteTime64(std::string_view path)
{
    return 0;
}
#endif


// ---- ShaderManager ----
ShaderManager::~ShaderManager()
{
    Clear(ClearMode::FREE_MEMORY);
}

void ShaderManager::Reserve(u32 shaderCount, u32 cacheEntries)
{
    m_Slots.reserve(shaderCount);
    m_FreeList.reserve(shaderCount);
    if (cacheEntries) m_Cache.reserve(cacheEntries);
}

void ShaderManager::Reserve(const ShaderReserveDesc &desc)
{
    m_Slots.reserve(desc.m_ShaderSlots);
    m_FreeList.reserve(desc.m_ShaderSlots);
    if (desc.m_cacheEntries) m_Cache.reserve(desc.m_cacheEntries);
}

ShaderHandle ShaderManager::AllocateSlot()
{
    if (!m_FreeList.empty())
    {
        u32 idx = m_FreeList.back();
        m_FreeList.pop_back();
        return ShaderHandle{ idx + 1, m_Slots[idx].m_Generation };
    }

    m_Slots.push_back({});
    u32 idx = (u32)m_Slots.size() - 1;
    return ShaderHandle{ idx + 1, m_Slots[idx].m_Generation };
}

ShaderManager::Slot* ShaderManager::GetSlot(ShaderHandle h)
{
    if (!h) return nullptr;
    const u32 idx = h.m_Id - 1;
    if (idx >= m_Slots.size()) return nullptr;

    Slot& s = m_Slots[idx];
    if (!s.b_Alive) return nullptr;
    if (s.m_Generation != h.m_Generation) return nullptr;
    return &s;
}

const ShaderManager::Slot* ShaderManager::GetSlot(ShaderHandle h) const
{
    return const_cast<ShaderManager*>(this)->GetSlot(h);
}

bool ShaderManager::IsValid(ShaderHandle h) const
{
    return GetSlot(h) != nullptr;
}

ShaderHandle ShaderManager::TryGetFromCache(const ShaderKey& key)
{
    auto it = m_Cache.find(key);
    if (it == m_Cache.end()) return {};

    ShaderHandle h = it->second;
    Slot* s = GetSlot(h);
    if (!s)
    {
        m_Cache.erase(it);
        return {};
    }

    ++s->m_RefCount;
    return h;
}

ShaderHandle ShaderManager::StoreToCache(ShaderKey&& key, ShaderHandle h)
{
    Slot* s = GetSlot(h);
    if (!s) return {};

    // 如果 key 已存在并指向别的 handle，清掉旧 slot 的 b_HasKey，避免旧 slot Destroy 时误删
    if (auto it = m_Cache.find(key); it != m_Cache.end())
    {
        ShaderHandle oldH = it->second;
        if (oldH != h)
        {
            if (Slot* old = GetSlot(oldH))
            {
                old->b_HasKey = false;
                old->m_Key.count = 0;
            }
        }
    }

    s->b_HasKey = true;
    s->m_Key = std::move(key);
    if (s->m_RefCount == 0) s->m_RefCount = 1;

    m_Cache.insert_or_assign(s->m_Key, h);
    return h;
}

// ---------------- GetOrCreateFromSources ----------------
ShaderHandle ShaderManager::GetOrCreateFromSources(std::span<const ShaderStageSource> sources, bool useCache)
{
    CORE_ASSERT(!sources.empty(), "GetOrCreateFromSources: empty");
    CORE_ASSERT(sources.size() <= ShaderKey::kMaxItems, "Too many shader stages");

    // stable ordering: stage then srcHash
    struct E { ShaderStage st; u64 srcHash; const ShaderStageSource* src; };
    std::vector<E> entries;
    entries.reserve(sources.size());

    for (auto& s : sources)
        entries.push_back({ s.m_Stage, HashString64(s.m_Source), &s });

    std::sort(entries.begin(), entries.end(),
              [](const E& a, const E& b)
              {
                  if (a.st != b.st) return (int)a.st < (int)b.st;
                  return a.srcHash < b.srcHash;
              });

    for (size_t i = 1; i < entries.size(); ++i)
        CORE_ASSERT(entries[i - 1].st != entries[i].st, "Duplicate shader stage in sources list");

    ShaderKey key{};
    key.kind  = ShaderKeyKind::Sources;
    key.count = (u8)entries.size();

    constexpr u64 kindTag = 0x534F5552ull; // "SOUR"
    for (u8 i = 0; i < key.count; ++i)
        key.items[i] = HashStageItem(entries[i].st, entries[i].srcHash, kindTag);

    if (useCache)
        if (ShaderHandle cached = TryGetFromCache(key))
            return cached;

    ShaderHandle h = AllocateSlot();
    Slot& s = m_Slots[h.m_Id - 1];

    std::array<ShaderStageSource, ShaderKey::kMaxItems> sorted{};
    for (u8 i = 0; i < key.count; ++i)
        sorted[i] = *entries[i].src;   
    s.m_Shader.BuildFromSources(std::span<const ShaderStageSource>(sorted.data(), key.count));

    s.b_Alive     = true;
    s.m_RefCount  = 1;
    s.b_HasKey    = false;
    s.m_Key.count = 0;

    return useCache ? StoreToCache(std::move(key), h) : h;
}

// ---------------- Bind ----------------
void ShaderManager::Bind(ShaderHandle h) const
{
    const Slot* s = GetSlot(h);
    if (!s) return;

    const u32 prog = s->m_Shader.GetProgramHandle();
    if (m_BoundProgram == prog)
        return;

    s->m_Shader.Bind();
    m_BoundProgram = prog;
}

void ShaderManager::Unbind() const
{
    glUseProgram(0);
    m_BoundProgram = 0;
}

Shader& ShaderManager::Get(ShaderHandle h)
{
    Slot* s = GetSlot(h);
    CORE_ASSERT(s, "Invalid ShaderHandle");
    return s->m_Shader;
}

const Shader& ShaderManager::Get(ShaderHandle h) const
{
    const Slot* s = GetSlot(h);
    CORE_ASSERT(s, "Invalid ShaderHandle");
    return s->m_Shader;
}

// ---------------- Destroy ----------------
void ShaderManager::Destroy(ShaderHandle h)
{
    Slot* s = GetSlot(h);
    if (!s) return;

    if (s->m_RefCount > 1)
    {
        --s->m_RefCount;
        return;
    }

    const u32 oldProg = s->m_Shader.GetProgramHandle();

    if (s->b_HasKey)
    {
        auto it = m_Cache.find(s->m_Key);
        if (it != m_Cache.end() && it->second == h)
            m_Cache.erase(it);

        s->b_HasKey = false;
        s->m_Key.count = 0;
    }

    // 释放 GPU program（Shader RAII）
    s->m_Shader   = Shader{};
    s->m_RefCount = 0;

    if (m_BoundProgram == oldProg)
    {
        glUseProgram(0);
        m_BoundProgram = 0;
    }

    s->b_Alive = false;
    ++s->m_Generation;
    m_FreeList.push_back(h.m_Id - 1);
}

void ShaderManager::InvalidateBindCache() const
{
    m_BoundProgram = 0;
}

ShaderHandle ShaderManager::GetOrCreateFromFiles(std::initializer_list<const char *> files, ShaderCacheMode mode)
{
    CORE_ASSERT(files.size() > 0, "GetOrCreateFromFiles: empty");
    CORE_ASSERT(files.size() <= ShaderKey::kMaxItems, "Too many shader stages");

    std::vector<std::string_view> views;
    views.reserve(files.size());
    for (const char* p : files)
    {
        CORE_ASSERT(p && p[0] != '\0', "Null/empty shader path");
        views.emplace_back(p);
    }

    return GetOrCreateFromFiles(std::span<const std::string_view>(views.data(), views.size()), mode);
}

ShaderHandle ShaderManager::GetOrCreateFromFiles(std::span<const std::string_view> files, ShaderCacheMode mode)
{
    
    CORE_ASSERT(!files.empty(), "GetOrCreateFromFiles: empty");
    CORE_ASSERT(files.size() <= ShaderKey::kMaxItems, "Too many shader stages");

    // 1) Normalize paths first (own the strings)
    std::vector<std::string> norm;
    norm.reserve(files.size());
    for (auto p : files)
    {
        CORE_ASSERT(!p.empty(), "Empty shader path");
        norm.emplace_back(NormalizePath(p));
    }

    // 2) Infer stage + stable sort by (stage, path)
    struct E { ShaderStage st; std::string_view path; };
    std::vector<E> entries;
    entries.reserve(norm.size());

    for (auto& p : norm)
    {
        entries.emplace_back(InferStageFromPath(p), std::string_view(p));
    }

    std::sort(entries.begin(), entries.end(),
              [](const E& a, const E& b)
              {
                  if (a.st != b.st) return (int)a.st < (int)b.st;
                  return a.path < b.path;
              });

    for (size_t i = 1; i < entries.size(); ++i)
        CORE_ASSERT(entries[i-1].st != entries[i].st, "Duplicate shader stage in files list");

    // 2) Build key：pathHash + last_write_time
    ShaderKey key{};
    key.kind  = ShaderKeyKind::Files;
    key.count = (u8)entries.size();

    constexpr u64 kindTag = 0x46494C45ull; // "FILE"

    for (u8 i = 0; i < key.count; ++i)
    {
        const u64 pathHash = HashString64(entries[i].path);
        const u64 ts64     = FileLastWriteTime64(entries[i].path); // 改成接受 string_view
        const u64 payload  = HashCombine64(pathHash, ts64);
        key.items[i] = HashStageItem(entries[i].st, payload, kindTag);
    }

    // 4) Cache lookup
    if (mode == ShaderCacheMode::USE_CACHE)
        if (ShaderHandle cached = TryGetFromCache(key))
            return cached;

    // 5) Build from sorted paths (match key order)
    std::vector<std::string> sortedPaths;
    sortedPaths.reserve(entries.size());
    for (auto& e : entries)
        sortedPaths.emplace_back(e.path); // copy string_view -> string

    // Shader sh;
    // sh.BuildFromFiles(sortedPaths);

    // Allocate slot + commit
    ShaderHandle h = AllocateSlot();
    Slot& s = m_Slots[h.m_Id - 1];
    s.m_Shader.BuildFromFiles(sortedPaths);
    s.b_Alive = true;
    s.m_RefCount = 1;
    s.b_HasKey = false;
    s.m_Key.count = 0;

    return (mode == ShaderCacheMode::USE_CACHE) ? StoreToCache(std::move(key), h) : h;
}

ShaderStage ShaderManager::InferStageFromPath(std::string_view p)
{
    auto ends_with = [&](std::string_view suf)
    {
        return p.size() >= suf.size() && p.substr(p.size() - suf.size()) == suf;
    };

    if (ends_with(".vs") || ends_with(".vert") || ends_with(".vert.glsl") || ends_with("_vert.glsl"))
        return ShaderStage::VERTEX;
    if (ends_with(".fs") || ends_with(".frag") || ends_with(".frag.glsl") || ends_with("_frag.glsl"))
        return ShaderStage::FRAGMENT;
    if (ends_with(".gs") || ends_with(".geom") || ends_with(".geom.glsl"))
        return ShaderStage::GEOMETRY;
    if (ends_with(".tcs") || ends_with(".tcs.glsl"))
        return ShaderStage::TESS_CONTROL;
    if (ends_with(".tes") || ends_with(".tes.glsl"))
        return ShaderStage::TESS_EVAL;
    if (ends_with(".cs") || ends_with(".cs.glsl"))
        return ShaderStage::COMPUTE;

    CORE_ASSERT(false, "Unrecognized shader extension");
    return ShaderStage::VERTEX;
}

void ShaderManager::Clear(ClearMode mode)
{
    m_Cache.clear();
    m_BoundProgram = 0;

    for (auto& s : m_Slots)
    {
        if (s.b_Alive)
            s.m_Shader = Shader{};

        s.b_Alive = false;
        s.m_RefCount = 0;
        s.b_HasKey = false;
        s.m_Key.count = 0;
        ++s.m_Generation;
    }

    m_FreeList.clear();

    if (mode == ClearMode::KEEP_CAPACITY)
    {
        m_FreeList.reserve(m_Slots.size());
        for (u32 i = 0; i < (u32)m_Slots.size(); ++i)
            m_FreeList.push_back(i);
    }
    else
    {
        m_Slots.clear();
        m_FreeList.clear();
    }
}

ShaderHandle ShaderManager::GetOrCreateFromSources(std::span<const ShaderStageSource> sources, ShaderCacheMode mode)
{
   if(mode == ShaderCacheMode::USE_CACHE)
   {
        return GetOrCreateFromSources(sources, true); 
   }
   else
   {
        return GetOrCreateFromSources(sources, false); 
   }
}