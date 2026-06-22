
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec4 aOffset;

uniform vec2 windowSize;
uniform vec2 cameraPos;
uniform float cameraZoom;

out vec3 ourColor;

void main()
{
   gl_Position = vec4(cameraZoom * (aPos.xy + aOffset.xy - cameraPos), 0.0, 1.0);
   ourColor = aColor;
}