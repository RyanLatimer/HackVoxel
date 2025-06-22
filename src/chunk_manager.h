#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "voxel_chunk.h"
#include "FastNoiseLite.h"

// Hash function for chunk coordinates
struct ChunkCoord {
    int x, z;
    
    ChunkCoord(int x, int z) : x(x), z(z) {}
    
    bool operator==(const ChunkCoord& other) const {
        return x == other.x && z == other.z;
    }
    
    // Distance calculation for sorting
    float distanceSquared(const ChunkCoord& other) const {
        float dx = x - other.x;
        float dz = z - other.z;
        return dx * dx + dz * dz;
    }
};

// Custom hash function for ChunkCoord
struct ChunkCoordHash {
    std::size_t operator()(const ChunkCoord& coord) const {
        return std::hash<int>()(coord.x) ^ (std::hash<int>()(coord.z) << 1);
    }
};

class ChunkManager {
public:
    // Configuration constants
    static const int RENDER_DISTANCE = 8;     // Chunks to render around player
    static const int LOAD_DISTANCE = 10;      // Chunks to keep loaded around player
    static const int UNLOAD_DISTANCE = 12;    // Distance at which to unload chunks
    
    ChunkManager();
    ~ChunkManager();
    
    // Core update function - call every frame
    void update(const glm::vec3& playerPosition);
    
    // Initialize chunks after OpenGL is ready
    void initialize(const glm::vec3& playerPosition);
    
    // Rendering
    void render(unsigned int shaderProgram, const glm::vec3& playerPosition, 
                const glm::mat4& view, const glm::mat4& projection);
    
    // World queries for collision detection
    bool isBlockSolid(const glm::vec3& worldPosition) const;
    BlockType getBlockType(const glm::vec3& worldPosition) const;
    
    // Get chunk containing world position
    VoxelChunk* getChunk(const glm::vec3& worldPosition) const;
    VoxelChunk* getChunkAt(int chunkX, int chunkZ) const;
    
    // Statistics
    int getLoadedChunkCount() const { return loadedChunks.size(); }
    int getRenderedChunkCount() const { return lastRenderedCount; }
    
    // Helper method to find surface height at world position
    int getSurfaceHeight(float worldX, float worldZ) const;
    
private:
    // Convert world position to chunk coordinates
    ChunkCoord worldToChunkCoord(const glm::vec3& worldPosition) const;
    ChunkCoord worldToChunkCoord(float x, float z) const;
    
    // Chunk loading/unloading
    void loadChunk(const ChunkCoord& coord);
    void unloadChunk(const ChunkCoord& coord);
    
    // Get chunks that should be loaded around a position
    std::vector<ChunkCoord> getChunksInRange(const ChunkCoord& center, int range) const;
    
    // Check if chunk should be rendered based on distance
    bool shouldRenderChunk(const ChunkCoord& coord, const glm::vec3& playerPosition) const;
    
private:
    // Chunk storage
    std::unordered_map<ChunkCoord, std::unique_ptr<VoxelChunk>, ChunkCoordHash> loadedChunks;
    
    // Tracking
    ChunkCoord lastPlayerChunk;
    mutable int lastRenderedCount;    // Enhanced terrain generation with realistic noise systems
    FastNoiseLite heightNoise;
    FastNoiseLite caveNoise;
    FastNoiseLite biomeNoise;
    FastNoiseLite temperatureNoise;
    FastNoiseLite humidityNoise;
    FastNoiseLite ridgeNoise;       // For mountain ridges
    FastNoiseLite erosionNoise;     // For erosion patterns
    FastNoiseLite vegetationNoise;  // For vegetation density
    
    // Cache for performance
    std::vector<ChunkCoord> chunksToLoad;
    std::vector<ChunkCoord> chunksToUnload;
    std::vector<std::pair<ChunkCoord, VoxelChunk*>> chunksToRender;
};
