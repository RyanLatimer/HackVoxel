#pragma once

#include "voxel_chunk.h"
#include <string>
#include <map>

class BlockDatabase {
public:
    static std::string getBlockName(BlockType blockType);
    static std::string getBlockDescription(BlockType blockType);
    static bool isBlockSolid(BlockType blockType);
    static bool isBlockTransparent(BlockType blockType);
    
private:
    static std::map<BlockType, std::string> blockNames;
    static std::map<BlockType, std::string> blockDescriptions;
    static bool initialized;
    static void initialize();
};
