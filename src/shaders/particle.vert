#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in float aLife;

out vec4 Color;
out float Life;

uniform mat4 projection;
uniform mat4 view;

void main() {
    gl_Position = projection * view * vec4(aPos, 1.0);
    Color = aColor;
    Life = aLife;
    gl_PointSize = 10.0;
}
