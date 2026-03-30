#pragma once

#include "Assets/Shader/Shader.h"
#include "Assets/Texture/TextureManager.h"

struct PassGlobalsBinderRef
{
    using BindFn = void(*)(const void*, Shader&, TextureManager&);

    const void* m_Object = nullptr;
    BindFn      m_BindFn = nullptr;

    void Bind(Shader& sh, TextureManager& textureMgr) const
    {
        if (m_BindFn)
            m_BindFn(m_Object, sh, textureMgr);
    }

    explicit operator bool() const
    {
        return m_BindFn != nullptr;
    }

    template<class T>
    static PassGlobalsBinderRef From(const T& obj)
    {
        return PassGlobalsBinderRef{
            .m_Object = &obj,
            .m_BindFn = [](const void* p, Shader& sh, TextureManager& textureMgr)
            {
                const T& self = *static_cast<const T*>(p);
                self.Bind(sh, textureMgr);
            }
        };
    }
};