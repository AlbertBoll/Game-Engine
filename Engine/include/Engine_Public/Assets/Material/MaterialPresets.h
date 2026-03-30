#pragma once

#include "Assets/Material/MaterialDescBuilder.h"

// MaterialPresets provides convenient starting points for common material types.
//
// Important:
// 1) Presets create NEW material descriptions from standard defaults.
//    They do NOT preserve settings from an existing material.
// 2) If you want to keep an existing material's settings and only tweak a few fields,
//    use:
//
//        MaterialDescBuilder::From(existingDesc)
//
//    instead of starting from a preset.

namespace MaterialPresets
{
    // -------------------- Generic Empty --------------------
    // Minimal preset: only sets shader.
    inline MaterialDescBuilder Empty(ShaderHandle shader)
    {
        return MaterialDescBuilder{}
            .Shader(shader);
    }

    // -------------------- PBR --------------------
    // Reasonable default metallic/roughness workflow material.
    //
    // Defaults:
    // - Albedo    = white
    // - Emissive  = black
    // - Metallic  = 0.0
    // - Roughness = 1.0
    // - Exposure  = 1.0
    // - Flags     = None
    inline MaterialDescBuilder PBR(ShaderHandle shader)
    {
        return MaterialDescBuilder{}
            .Shader(shader)
            .Albedo({1.0f, 1.0f, 1.0f})
            .Emissive({0.0f, 0.0f, 0.0f})
            .Metallic(0.0f)
            .Roughness(1.0f)
            .Exposure(1.0f)
            .Flags(MaterialFlag::None);
    }

    // -------------------- Metallic PBR --------------------
    // Good starting point for metals.
    inline MaterialDescBuilder Metallic(ShaderHandle shader)
    {
        return MaterialDescBuilder{}
            .Shader(shader)
            .Albedo({1.0f, 1.0f, 1.0f})
            .Emissive({0.0f, 0.0f, 0.0f})
            .Metallic(1.0f)
            .Roughness(0.2f)
            .Exposure(1.0f)
            .Flags(MaterialFlag::None);
    }

    // -------------------- Dielectric PBR --------------------
    // Good starting point for non-metal materials like wood/plastic/stone.
    inline MaterialDescBuilder Dielectric(ShaderHandle shader)
    {
        return MaterialDescBuilder{}
            .Shader(shader)
            .Albedo({1.0f, 1.0f, 1.0f})
            .Emissive({0.0f, 0.0f, 0.0f})
            .Metallic(0.0f)
            .Roughness(0.8f)
            .Exposure(1.0f)
            .Flags(MaterialFlag::None);
    }

    // -------------------- Emissive --------------------
    // Useful for glowing surfaces / light cards / debug neon-like materials.
    inline MaterialDescBuilder Emissive(ShaderHandle shader)
    {
        return MaterialDescBuilder{}
            .Shader(shader)
            .Albedo({0.0f, 0.0f, 0.0f})
            .Emissive({1.0f, 1.0f, 1.0f})
            .Metallic(0.0f)
            .Roughness(1.0f)
            .Exposure(1.0f)
            .Flags(MaterialFlag::None);
    }

    // -------------------- Unlit --------------------
    // Assumes the shader itself is unlit.
    // Material flags remain None by default.
    inline MaterialDescBuilder Unlit(ShaderHandle shader)
    {
        return MaterialDescBuilder{}
            .Shader(shader)
            .Albedo({1.0f, 1.0f, 1.0f})
            .Emissive({0.0f, 0.0f, 0.0f})
            .Metallic(0.0f)
            .Roughness(1.0f)
            .Exposure(1.0f)
            .Flags(MaterialFlag::None);
    }

    // -------------------- Low-Roughness PBR --------------------
    // A safer replacement for the old "Transparent" preset name.
    // This is simply a glossy / smooth PBR starting point.
    // It does NOT imply blending, transparency, refraction, transmission, or render-pass behavior.
    inline MaterialDescBuilder LowRoughnessPBR(ShaderHandle shader)
    {
        return MaterialDescBuilder{}
            .Shader(shader)
            .Albedo({1.0f, 1.0f, 1.0f})
            .Emissive({0.0f, 0.0f, 0.0f})
            .Metallic(0.0f)
            .Roughness(0.1f)
            .Exposure(1.0f)
            .Flags(MaterialFlag::None);
    }

    // -------------------- Two-Sided PBR --------------------
    // Good for foliage, cloth, paper, thin shells, decals-on-quads, etc.
    //
    // Note:
    // The TwoSided flag is material data only.
    // Actual cull-state behavior should ultimately be handled by Renderer / pipeline state.
    inline MaterialDescBuilder TwoSidedPBR(ShaderHandle shader)
    {
        return MaterialDescBuilder{}
            .Shader(shader)
            .Albedo({1.0f, 1.0f, 1.0f})
            .Emissive({0.0f, 0.0f, 0.0f})
            .Metallic(0.0f)
            .Roughness(1.0f)
            .Exposure(1.0f)
            .SetFlag(MaterialFlag::TwoSided);
    }

   // -------------------- Cutout PBR --------------------
    // Good starting point for masked / cutout materials such as leaves, fences, paper cutouts.
    //
    // Important:
    // This only sets the material AlphaTest flag.
    // Actual cutout behavior still depends on:
    // - shader support
    // - alpha source in the texture/material
    // - renderer / pipeline decisions
    inline MaterialDescBuilder CutoutPBR(ShaderHandle shader)
    {
        return MaterialDescBuilder{}
            .Shader(shader)
            .Albedo({1.0f, 1.0f, 1.0f})
            .Emissive({0.0f, 0.0f, 0.0f})
            .Metallic(0.0f)
            .Roughness(1.0f)
            .Exposure(1.0f)
            .SetFlag(MaterialFlag::AlphaTest);
    }

    // -------------------- Textured PBR --------------------
    // Semantic entry point for "this will likely become a texture-driven PBR material".
    //
    // It does NOT automatically assign textures or force feature flags.
    // Use .AlbedoMap(...), .NormalMap(...), .MetallicRoughnessMap(...), etc. explicitly.
    inline MaterialDescBuilder TexturedPBR(ShaderHandle shader)
    {
        return PBR(shader);
    }

    // -------------------- Textured Unlit --------------------
    // Semantic entry point for "this will likely become a texture-driven unlit material".
    inline MaterialDescBuilder TexturedUnlit(ShaderHandle shader)
    {
        return Unlit(shader);
    }
}