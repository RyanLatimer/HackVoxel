#include "voxel_chunk.h"
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../libs/FastNoiseLite/FastNoiseLite.h"

VoxelChunk::VoxelChunk() {

    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise.SetFrequency(0.03f);


    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            float heightValue = noise.GetNoise((float)x, (float)y);
            int height = (int)((heightValue + 1.0f) * 0.5f * CHUNK_SIZE);

            for (int z = 0; z < CHUNK_SIZE; z++) {
                blocks[x][y][z] = (y <= height) ? 1 : 0;
            }
        }
    }
}

void VoxelChunk::render(unsigned int shaderID) {
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                if (blocks[x][y][z] == 0) continue;

                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(x, y, z));
                glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, glm::value_ptr(model));
                glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            }
        }
    }
}
