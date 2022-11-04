#include "asset_manager.h"

AssetManager::AssetManager() {
}

AssetManager::~AssetManager() {
    for(auto& d : textureCache) delete d->sprite;
}

size_t AssetManager::allocateTexture(olc::Sprite* sprite, bool freemem, bool clamp) {
    textureCache.emplace_back(new olc::Decal(sprite, false, clamp));
    textureCache.back()->Update();      // this might not be needed
    if(freemem){
        sprite->pColData.clear();          // empty CPU texture cache
        sprite->pColData.shrink_to_fit();  // cleanup unused memory
    }

    return textureCache.size() - 1;
}

size_t AssetManager::CreateTexture(const std::string& path, bool cpubind) {
    olc::Sprite* texture = new olc::Sprite(32,32); // errored/default texture on failed to load

    if(texture->LoadFromFile(path) != olc::OK){
        std::cerr << "Failed to load texture: " << path << "\n";
    }

    return allocateTexture(texture, !cpubind);
}

size_t AssetManager::AddExistingTexture(olc::Sprite* sprite, bool cpubind, bool clamp) {
    return allocateTexture(sprite, !cpubind, clamp);
}

olc::Decal* AssetManager::GetTexture(size_t texid) {
    if(texid >= textureCache.size()) return nullptr;
    auto& tex = textureCache[texid];
    return !tex ? nullptr : tex.get();
}

olc::vi2d AssetManager::GetTextureSize(size_t texid) const {
    if(texid >= textureCache.size()) return {0,0};

    auto& tex = textureCache[texid];
    return !tex ? olc::vi2d(0, 0) : tex->sprite->Size();
}

void AssetManager::TruncateTexture(size_t texid) {
    if(texid >= textureCache.size()) return;
    auto& tex = textureCache[texid];
    if(tex){
        tex->sprite->pColData.clear();          // empty CPU texture cache
        tex->sprite->pColData.shrink_to_fit();  // cleanup unused memory
    }
}

void AssetManager::FreeTexture(size_t texid) {
    if(texid >= textureCache.size()) return;
    auto& tex = textureCache[texid];
    if(tex){
        delete tex->sprite;
        tex.reset();
    }
}