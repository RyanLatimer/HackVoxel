#include "chunk_manager.h"
#include <algorithm>
#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

ChunkManager::ChunkManager() 
    : lastPlayerChunk(0, 0)
    , lastRenderedCount(0)
{    // Initialize noise generators for more beautiful terrain
    heightNoise.SetSeed(12345);
    heightNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    heightNoise.SetFrequency(0.008f);  // Smoother terrain
    heightNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    heightNoise.SetFractalOctaves(5);  // More detail
    heightNoise.SetFractalLacunarity(2.2f);
    heightNoise.SetFractalGain(0.45f);
    
    // Cave generation noise - more organic caves
    caveNoise.SetSeed(54321);
    caveNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    caveNoise.SetFrequency(0.04f);
    caveNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    caveNoise.SetFractalOctaves(3);
    
    // Enhanced biome noise for smoother transitions
    biomeNoise.SetSeed(99999);
    biomeNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    biomeNoise.SetFrequency(0.003f);  // Larger biome regions
    
    // Temperature and humidity for better biome generation
    temperatureNoise.SetSeed(11111);
    temperatureNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    temperatureNoise.SetFrequency(0.004f);
    
    humidityNoise.SetSeed(22222);
    humidityNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    humidityNoise.SetFrequency(0.0035f);
    
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
    auto chunk = std::make_unique<VoxelChunk>(coord.x, coord.z);    // Generate enhanced terrain using improved noise
    for (int x = 0; x < VoxelChunk::CHUNK_SIZE; x++) {
        for (int z = 0; z < VoxelChunk::CHUNK_SIZE; z++) {
            float worldX = coord.x * VoxelChunk::CHUNK_SIZE + x;
            float worldZ = coord.z * VoxelChunk::CHUNK_SIZE + z;
            
            // Enhanced terrain generation with multiple noise layers
            float heightValue = heightNoise.GetNoise(worldX, worldZ);
            float temperatureValue = temperatureNoise.GetNoise(worldX, worldZ);
            float humidityValue = humidityNoise.GetNoise(worldX, worldZ);
            float mountainValue = heightNoise.GetNoise(worldX * 0.2f, worldZ * 0.2f);
            float detailValue = heightNoise.GetNoise(worldX * 3.0f, worldZ * 3.0f);
            
            // Advanced biome determination based on temperature and humidity
            enum BiomeType { 
                TUNDRA, TAIGA, PLAINS, DESERT, SAVANNA, 
                TEMPERATE_FOREST, TROPICAL_FOREST, SWAMP, 
                MOUNTAINS, SNOW_PEAKS, BEACH 
            };
            
            BiomeType biome;
            
            // Biome selection based on temperature and humidity
            if (temperatureValue < -0.4f) {
                if (humidityValue < -0.2f) {
                    biome = TUNDRA;
                } else {
                    biome = TAIGA;
                }
            } else if (temperatureValue > 0.4f) {
                if (humidityValue < -0.3f) {
                    biome = DESERT;
                } else if (humidityValue > 0.3f) {
                    biome = TROPICAL_FOREST;
                } else {
                    biome = SAVANNA;
                }
            } else {
                if (humidityValue < -0.2f) {
                    biome = PLAINS;
                } else if (humidityValue > 0.4f) {
                    biome = SWAMP;
                } else {
                    biome = TEMPERATE_FOREST;
                }
            }
            
            // Mountain biomes override based on elevation
            if (mountainValue > 0.3f) {
                if (temperatureValue < 0.0f) {
                    biome = SNOW_PEAKS;
                } else {
                    biome = MOUNTAINS;
                }
            }
            
            // Calculate height based on biome with more variation
            int surfaceHeight;
            switch (biome) {
                case SNOW_PEAKS:
                    surfaceHeight = static_cast<int>((heightValue * 0.3f + 0.7f) * 10 + mountainValue * 8 + 12);
                    break;
                case MOUNTAINS:
                    surfaceHeight = static_cast<int>((heightValue * 0.4f + 0.6f) * 8 + mountainValue * 6 + 10);
                    break;
                case DESERT:
                    surfaceHeight = static_cast<int>((heightValue * 0.3f + 0.7f) * 4 + detailValue * 3 + 6);
                    break;
                case SWAMP:
                    surfaceHeight = static_cast<int>((heightValue * 0.2f + 0.8f) * 2 + 4);
                    break;
                case BEACH:
                    surfaceHeight = static_cast<int>((heightValue * 0.1f + 0.9f) * 1 + 3);
                    break;
                case TUNDRA:
                    surfaceHeight = static_cast<int>((heightValue * 0.3f + 0.7f) * 5 + 5);
                    break;
                case TAIGA:
                    surfaceHeight = static_cast<int>((heightValue * 0.4f + 0.6f) * 6 + 6);
                    break;
                default:
                    surfaceHeight = static_cast<int>((heightValue * 0.5f + 0.5f) * 7 + 7);
                    break;
            }
            surfaceHeight = std::clamp(surfaceHeight, 4, 20);
            
            // Generate terrain layers
            for (int y = 0; y < VoxelChunk::CHUNK_SIZE; y++) {
                BlockType blockType = BlockType::AIR;
                
                if (y == 0) {
                    // Bedrock layer
                    blockType = BlockType::BEDROCK;
                } else if (y <= surfaceHeight) {
                    // Check for caves
                    float caveValue = caveNoise.GetNoise(worldX, y * 2.0f, worldZ);
                    if (caveValue > 0.45f && y > 1 && y < surfaceHeight - 1) {
                        blockType = BlockType::AIR; // Cave
                    } else {
                        // Generate ore deposits
                        float oreNoise = caveNoise.GetNoise(worldX * 3.0f, y * 3.0f, worldZ * 3.0f);
                        bool isOre = false;
                        
                        if (y < 4 && oreNoise > 0.85f) {
                            // Deep ores
                            if (oreNoise > 0.98f) {
                                blockType = BlockType::DIAMOND_ORE;
                                isOre = true;
                            } else if (oreNoise > 0.95f) {
                                blockType = BlockType::EMERALD_ORE;
                                isOre = true;
                            } else if (oreNoise > 0.90f) {
                                blockType = BlockType::GOLD_ORE;
                                isOre = true;
                            }
                        } else if (y < 8 && oreNoise > 0.80f) {
                            // Mid-level ores
                            if (oreNoise > 0.92f) {
                                blockType = BlockType::IRON_ORE;
                                isOre = true;
                            } else if (oreNoise > 0.88f) {
                                blockType = BlockType::REDSTONE_ORE;
                                isOre = true;
                            }
                        }
                          if (!isOre) {
                            // Enhanced terrain generation based on biome
                            if (y == surfaceHeight) {
                                // Surface block based on biome
                                switch (biome) {
                                    case DESERT:
                                        blockType = BlockType::SAND;
                                        break;
                                    case BEACH:
                                        blockType = BlockType::SAND;
                                        break;
                                    case SNOW_PEAKS:
                                    case TUNDRA:
                                        blockType = BlockType::SNOW;
                                        break;
                                    case MOUNTAINS:
                                        if (surfaceHeight > 15) {
                                            blockType = BlockType::SNOW;
                                        } else if (surfaceHeight > 12) {
                                            blockType = BlockType::STONE;
                                        } else {
                                            blockType = BlockType::GRASS;
                                        }
                                        break;
                                    case SWAMP:
                                        // Swampy grass
                                        blockType = BlockType::GRASS;
                                        break;
                                    case TAIGA:
                                    case TEMPERATE_FOREST:
                                    case TROPICAL_FOREST:
                                        blockType = BlockType::GRASS;
                                        break;
                                    case SAVANNA:
                                        // Dry grass
                                        blockType = BlockType::GRASS;
                                        break;
                                    default: // PLAINS
                                        blockType = BlockType::GRASS;
                                        break;
                                }
                            } else if (y > surfaceHeight - 3) {
                                // Sub-surface layers based on biome
                                switch (biome) {
                                    case DESERT:
                                    case BEACH:
                                        if (y > surfaceHeight - 2) {
                                            blockType = BlockType::SAND;
                                        } else {
                                            blockType = BlockType::DIRT;
                                        }
                                        break;
                                    case SNOW_PEAKS:
                                    case TUNDRA:
                                        blockType = BlockType::DIRT;
                                        break;
                                    case MOUNTAINS:
                                        if (y > surfaceHeight - 2) {
                                            blockType = BlockType::DIRT;
                                        } else {
                                            blockType = BlockType::STONE;
                                        }
                                        break;
                                    case SWAMP:
                                        // Rich, dark soil
                                        blockType = BlockType::DIRT;
                                        break;
                                    default:
                                        blockType = BlockType::DIRT;
                                        break;
                                }
                            } else if (y > surfaceHeight - 8) {
                                // Deeper layers with biome variation
                                if ((biome == MOUNTAINS || biome == SNOW_PEAKS) && 
                                    heightNoise.GetNoise(worldX * 4.0f, y * 4.0f, worldZ * 4.0f) > 0.2f) {
                                    blockType = BlockType::COBBLESTONE;
                                } else {
                                    blockType = BlockType::STONE;
                                }
                            } else {
                                // Deep stone layer
                                blockType = BlockType::STONE;
                            }
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
