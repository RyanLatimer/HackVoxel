#include "block_database.h"

std::map<BlockType, std::string> BlockDatabase::blockNames;
std::map<BlockType, std::string> BlockDatabase::blockDescriptions;
bool BlockDatabase::initialized = false;

void BlockDatabase::initialize() {
    if (initialized) return;
    
    // Initialize block names
    blockNames[BlockType::AIR] = "Air";
    blockNames[BlockType::GRASS] = "Grass Block";
    blockNames[BlockType::DIRT] = "Dirt";
    blockNames[BlockType::STONE] = "Stone";
    blockNames[BlockType::COBBLESTONE] = "Cobblestone";
    blockNames[BlockType::WOOD_PLANK] = "Wood Planks";
    blockNames[BlockType::WOOD_LOG] = "Wood Log";
    blockNames[BlockType::LEAVES] = "Leaves";
    blockNames[BlockType::SAND] = "Sand";
    blockNames[BlockType::WATER] = "Water";
    blockNames[BlockType::BEDROCK] = "Bedrock";
    blockNames[BlockType::SNOW] = "Snow";
    blockNames[BlockType::ICE] = "Ice";
    blockNames[BlockType::GLOWSTONE] = "Glowstone";
    blockNames[BlockType::OBSIDIAN] = "Obsidian";
    blockNames[BlockType::BRICK] = "Brick";
    blockNames[BlockType::MOSSY_STONE] = "Mossy Stone";
    blockNames[BlockType::GRAVEL] = "Gravel";
    blockNames[BlockType::GOLD_ORE] = "Gold Ore";
    blockNames[BlockType::IRON_ORE] = "Iron Ore";
    blockNames[BlockType::DIAMOND_ORE] = "Diamond Ore";
    blockNames[BlockType::EMERALD_ORE] = "Emerald Ore";
    blockNames[BlockType::REDSTONE_ORE] = "Redstone Ore";
    
    // Initialize block descriptions
    blockDescriptions[BlockType::GRASS] = "Natural grass-covered dirt block";
    blockDescriptions[BlockType::DIRT] = "Basic earth material";
    blockDescriptions[BlockType::STONE] = "Common stone found underground";
    blockDescriptions[BlockType::COBBLESTONE] = "Rough stone blocks";
    blockDescriptions[BlockType::WOOD_PLANK] = "Processed wood planks";
    blockDescriptions[BlockType::WOOD_LOG] = "Tree trunk material";
    blockDescriptions[BlockType::SAND] = "Fine granular material";
    blockDescriptions[BlockType::BRICK] = "Fired clay bricks";
    blockDescriptions[BlockType::OBSIDIAN] = "Dark volcanic glass";
    
    initialized = true;
}

std::string BlockDatabase::getBlockName(BlockType blockType) {
    initialize();
    auto it = blockNames.find(blockType);
    if (it != blockNames.end()) {
        return it->second;
    }
    return "Unknown Block";
}

std::string BlockDatabase::getBlockDescription(BlockType blockType) {
    initialize();
    auto it = blockDescriptions.find(blockType);
    if (it != blockDescriptions.end()) {
        return it->second;
    }
    return "No description available";
}

bool BlockDatabase::isBlockSolid(BlockType blockType) {
    return blockType != BlockType::AIR && blockType != BlockType::WATER;
}

bool BlockDatabase::isBlockTransparent(BlockType blockType) {
    return blockType == BlockType::AIR || blockType == BlockType::WATER || 
           blockType == BlockType::ICE || blockType == BlockType::LEAVES;
}
