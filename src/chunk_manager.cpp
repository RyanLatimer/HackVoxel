#include "chunk_manager.h"
#include <algorithm>
#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

ChunkManager::ChunkManager() 
    : lastPlayerChunk(0, 0)
    , lastRenderedCount(0)
{
    // Initialize noise generators for terrain
    heightNoise.SetSeed(12345);
    heightNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    heightNoise.SetFrequency(0.01f);
    heightNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    heightNoise.SetFractalOctaves(4);
    heightNoise.SetFractalLacunarity(2.0f);
    heightNoise.SetFractalGain(0.5f);
    
    // Cave generation noise
    caveNoise.SetSeed(54321);
    caveNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    caveNoise.SetFrequency(0.05f);
    
    // Biome noise for variation
    biomeNoise.SetSeed(99999);
    biomeNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);    biomeNoise.SetFrequency(0.005f);
    
    std::cout << "ChunkManager initialized with procedural terrain generation" << std::endl;
}

ChunkManager::~ChunkManager() {
    loadedChunks.clear();    std::cout << "ChunkManager destroyed" << std::endl;
}

void ChunkManager::initialize(const glm::vec3& playerPosition) {
    // Load initial chunks around player spawn position
    ChunkCoord playerChunk = worldToChunkCoord(playerPosition);
    std::cout << "Loading initial chunks around player position (" << playerChunk.x << ", " << playerChunk.z << ")..." << std::endl;
    
    std::vector<ChunkCoord> initialChunks = getChunksInRange(playerChunk, LOAD_DISTANCE);
    for (const auto& coord : initialChunks) {
        loadChunk(coord);
    }
      lastPlayerChunk = playerChunk;
    std::cout << "Loaded " << initialChunks.size() << " initial chunks" << std::endl;
}

void ChunkManager::update(const glm::vec3& playerPosition) {
    ChunkCoord currentPlayerChunk = worldToChunkCoord(playerPosition);
    
    // Only update chunks if player moved to a different chunk
    if (!(currentPlayerChunk == lastPlayerChunk)) {
        std::cout << "Player moved to chunk (" << currentPlayerChunk.x << ", " << currentPlayerChunk.z << ")" << std::endl;
        
        // Clear temporary arrays
        chunksToLoad.clear();
        chunksToUnload.clear();
        
        // Find chunks that should be loaded
        std::vector<ChunkCoord> requiredChunks = getChunksInRange(currentPlayerChunk, LOAD_DISTANCE);
        
        // Find chunks to load (required but not loaded)
        for (const auto& coord : requiredChunks) {
            if (loadedChunks.find(coord) == loadedChunks.end()) {
                chunksToLoad.push_back(coord);
            }
        }
        
        // Find chunks to unload (too far from player)
        for (const auto& pair : loadedChunks) {
            const ChunkCoord& coord = pair.first;
            float distSq = coord.distanceSquared(currentPlayerChunk);
            if (distSq > UNLOAD_DISTANCE * UNLOAD_DISTANCE) {
                chunksToUnload.push_back(coord);
            }
        }
        
        // Load new chunks
        for (const auto& coord : chunksToLoad) {
            loadChunk(coord);
        }
        
        // Unload distant chunks
        for (const auto& coord : chunksToUnload) {
            unloadChunk(coord);
        }
        
        if (!chunksToLoad.empty() || !chunksToUnload.empty()) {
            std::cout << "Loaded " << chunksToLoad.size() << " chunks, unloaded " 
                      << chunksToUnload.size() << " chunks. Total: " << loadedChunks.size() << std::endl;
        }
        
        lastPlayerChunk = currentPlayerChunk;
    }
}

void ChunkManager::render(unsigned int shaderProgram, const glm::vec3& playerPosition,
                         const glm::mat4& view, const glm::mat4& projection) {
    ChunkCoord playerChunk = worldToChunkCoord(playerPosition);
    
    // Set up matrices
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
    // Clear render list
    chunksToRender.clear();
    
    // Collect chunks to render and sort by distance
    for (const auto& pair : loadedChunks) {
        const ChunkCoord& coord = pair.first;
        if (shouldRenderChunk(coord, playerPosition)) {
            chunksToRender.emplace_back(coord, pair.second.get());
        }
    }
    
    // Sort chunks by distance to player (closest first for better depth testing)
    std::sort(chunksToRender.begin(), chunksToRender.end(),
              [&playerChunk](const auto& a, const auto& b) {
                  return a.first.distanceSquared(playerChunk) < b.first.distanceSquared(playerChunk);
              });
    
    // Render sorted chunks
    for (const auto& pair : chunksToRender) {
        pair.second->render(shaderProgram);
    }
    
    lastRenderedCount = chunksToRender.size();
}

bool ChunkManager::isBlockSolid(const glm::vec3& worldPosition) const {
    // Convert world position to chunk coordinates
    int chunkX = static_cast<int>(std::floor(worldPosition.x / VoxelChunk::CHUNK_SIZE));
    int chunkZ = static_cast<int>(std::floor(worldPosition.z / VoxelChunk::CHUNK_SIZE));
    
    VoxelChunk* chunk = getChunkAt(chunkX, chunkZ);
    if (!chunk) return false;
    
    // Convert world position to local chunk coordinates using floor
    int localX = static_cast<int>(std::floor(worldPosition.x)) - (chunkX * VoxelChunk::CHUNK_SIZE);
    int localY = static_cast<int>(std::floor(worldPosition.y));
    int localZ = static_cast<int>(std::floor(worldPosition.z)) - (chunkZ * VoxelChunk::CHUNK_SIZE);
    
    // Ensure coordinates are within chunk bounds
    if (localX < 0 || localX >= VoxelChunk::CHUNK_SIZE ||
        localY < 0 || localY >= VoxelChunk::CHUNK_SIZE ||
        localZ < 0 || localZ >= VoxelChunk::CHUNK_SIZE) {
        return false;
    }
    
    return chunk->isBlockSolid(localX, localY, localZ);
}

BlockType ChunkManager::getBlockType(const glm::vec3& worldPosition) const {
    // Convert world position to chunk coordinates
    int chunkX = static_cast<int>(std::floor(worldPosition.x / VoxelChunk::CHUNK_SIZE));
    int chunkZ = static_cast<int>(std::floor(worldPosition.z / VoxelChunk::CHUNK_SIZE));
    
    VoxelChunk* chunk = getChunkAt(chunkX, chunkZ);
    if (!chunk) return BlockType::AIR;
    
    // Convert world position to local chunk coordinates using floor
    int localX = static_cast<int>(std::floor(worldPosition.x)) - (chunkX * VoxelChunk::CHUNK_SIZE);
    int localY = static_cast<int>(std::floor(worldPosition.y));
    int localZ = static_cast<int>(std::floor(worldPosition.z)) - (chunkZ * VoxelChunk::CHUNK_SIZE);
    
    if (localX < 0 || localX >= VoxelChunk::CHUNK_SIZE ||
        localY < 0 || localY >= VoxelChunk::CHUNK_SIZE ||
        localZ < 0 || localZ >= VoxelChunk::CHUNK_SIZE) {
        return BlockType::AIR;
    }
    
    return chunk->getBlockType(localX, localY, localZ);
}

VoxelChunk* ChunkManager::getChunk(const glm::vec3& worldPosition) const {
    ChunkCoord coord = worldToChunkCoord(worldPosition);
    return getChunkAt(coord.x, coord.z);
}

VoxelChunk* ChunkManager::getChunkAt(int chunkX, int chunkZ) const {
    ChunkCoord coord(chunkX, chunkZ);
    auto it = loadedChunks.find(coord);
    return (it != loadedChunks.end()) ? it->second.get() : nullptr;
}

ChunkCoord ChunkManager::worldToChunkCoord(const glm::vec3& worldPosition) const {
    return worldToChunkCoord(worldPosition.x, worldPosition.z);
}

ChunkCoord ChunkManager::worldToChunkCoord(float x, float z) const {
    int chunkX = static_cast<int>(std::floor(x / VoxelChunk::CHUNK_SIZE));
    int chunkZ = static_cast<int>(std::floor(z / VoxelChunk::CHUNK_SIZE));
    return ChunkCoord(chunkX, chunkZ);
}

void ChunkManager::loadChunk(const ChunkCoord& coord) {
    // Don't load if already exists
    if (loadedChunks.find(coord) != loadedChunks.end()) {
        return;
    }
    
    // Create new chunk
    auto chunk = std::make_unique<VoxelChunk>(coord.x, coord.z);
    
    // Generate terrain using noise
    for (int x = 0; x < VoxelChunk::CHUNK_SIZE; x++) {
        for (int z = 0; z < VoxelChunk::CHUNK_SIZE; z++) {
            float worldX = coord.x * VoxelChunk::CHUNK_SIZE + x;
            float worldZ = coord.z * VoxelChunk::CHUNK_SIZE + z;            // Get height from noise - improve height range
            float heightValue = heightNoise.GetNoise(worldX, worldZ);
            float biomeValue = biomeNoise.GetNoise(worldX, worldZ);
            
            // Convert to height with better range (6-12 for more realistic terrain)
            int surfaceHeight = static_cast<int>((heightValue * 0.5f + 0.5f) * 6 + 6);
            surfaceHeight = std::clamp(surfaceHeight, 3, 13);
            
            // Generate terrain layers
            for (int y = 0; y < VoxelChunk::CHUNK_SIZE; y++) {
                BlockType blockType = BlockType::AIR;
                
                if (y == 0) {
                    // Bedrock layer
                    blockType = BlockType::BEDROCK;
                } else if (y <= surfaceHeight) {
                    // Check for caves
                    float caveValue = caveNoise.GetNoise(worldX, y * 2.0f, worldZ);
                    if (caveValue > 0.4f && y > 1 && y < surfaceHeight - 1) {
                        blockType = BlockType::AIR; // Cave
                    } else {
                        // Surface block
                        if (y == surfaceHeight) {
                            // Biome-based surface blocks
                            if (biomeValue > 0.3f) {
                                blockType = BlockType::SAND;
                            } else if (surfaceHeight < 6) {
                                blockType = BlockType::SAND;
                            } else {
                                blockType = BlockType::GRASS;
                            }
                        } else if (y > surfaceHeight - 3) {
                            blockType = BlockType::DIRT;
                        } else {
                            blockType = BlockType::STONE;
                        }
                    }
                }
                
                // Set the block
                chunk->setBlock(x, y, z, blockType);
            }
        }
    }
    
    // Generate mesh after setting all blocks
    chunk->regenerateMesh();
    
    loadedChunks[coord] = std::move(chunk);
}

void ChunkManager::unloadChunk(const ChunkCoord& coord) {
    auto it = loadedChunks.find(coord);
    if (it != loadedChunks.end()) {
        loadedChunks.erase(it);
    }
}

std::vector<ChunkCoord> ChunkManager::getChunksInRange(const ChunkCoord& center, int range) const {
    std::vector<ChunkCoord> chunks;
    chunks.reserve((2 * range + 1) * (2 * range + 1));
    
    for (int x = center.x - range; x <= center.x + range; x++) {
        for (int z = center.z - range; z <= center.z + range; z++) {
            chunks.emplace_back(x, z);
        }
    }
    
    return chunks;
}

bool ChunkManager::shouldRenderChunk(const ChunkCoord& coord, const glm::vec3& playerPosition) const {
    ChunkCoord playerChunk = worldToChunkCoord(playerPosition);
    float distSq = coord.distanceSquared(playerChunk);
    return distSq <= RENDER_DISTANCE * RENDER_DISTANCE;
}

int ChunkManager::getSurfaceHeight(float worldX, float worldZ) const {
    VoxelChunk* chunk = getChunk(glm::vec3(worldX, 0, worldZ));
    if (!chunk) return -1;
    
    // Find the highest solid block at this X,Z position
    for (int y = VoxelChunk::CHUNK_SIZE - 1; y >= 0; y--) {
        if (isBlockSolid(glm::vec3(worldX, y, worldZ))) {
            return y;
        }
    }
    return -1; // No solid blocks found
}
