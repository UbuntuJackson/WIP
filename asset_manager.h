#pragma once

#include "olcPixelGameEngine.h"

#include <vector>
#include <memory>

#define EMPTY_TEXTURE UINT64_MAX

class AssetManager {
    AssetManager();
    std::vector<std::unique_ptr<olc::Decal>> textureCache;

    size_t allocateTexture(olc::Sprite* sprite, bool freemem=true, bool clamp=true);

public:
    static inline AssetManager& Current() { static AssetManager self; return self; }
    ~AssetManager();

    // Do Not Move!
    AssetManager(AssetManager&&) = delete;
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;
    AssetManager& operator=(AssetManager&&) = delete;

    size_t CreateTexture(const std::string& path, bool cpubind=true);
    size_t AddExistingTexture(olc::Sprite* sprite, bool cpubind=true, bool clamp=true);
    olc::Decal* GetTexture(size_t texid);
    olc::vi2d GetTextureSize(size_t texid) const;

    void TruncateTexture(size_t texid);
    void FreeTexture(size_t texid);
};