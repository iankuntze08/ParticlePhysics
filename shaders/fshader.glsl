
#version 330 core

in vec3 ourColor;
layout(origin_upper_left, pixel_center_integer) in vec4 gl_FragCoord;

uniform vec2 windowSize;
uniform vec2 cameraPos;
uniform float cameraZoom;

out vec4 fragColor;

void main()
{
   vec2 uv = gl_FragCoord.xy / windowSize;

   fragColor = vec4(ourColor, 1.0);
}