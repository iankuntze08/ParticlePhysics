
#version 430 core

struct Particle
{
    vec4 position;
    vec4 velocity;
    float radius;
    float forceStrength;
    float forceDist;
    int type;
};

layout(local_size_x = 128) in;

layout(std430, binding = 0) readonly buffer curPart
{
    Particle particlesCurrent[];
};

layout(std430, binding = 1) writeonly buffer nextPart
{
    Particle particlesNext[];
};

uniform uint count;
uniform float dt;

// built in variables
// in uvec3 gl_NumWorkGroups;
// in uvec3 gl_WorkGroupID;
// in uvec3 gl_LocalInvocationID;
// in uvec3 gl_GlobalInvocationID;
// in uint  gl_LocalInvocationIndex;

float gFunc(float dist2)
{
    return -1.0 / ((5.0 * dist2) + 0.1);
}

// vec4 RHS(vec2 v)
// {
//     return vec2(v.x + (dt / 6.0));
// }

void main()
{
    uint idx = gl_GlobalInvocationID.x;
    if (idx >= count)
        return;
    Particle p = particlesCurrent[idx];

    vec4 g = vec4(0.0);
    // for (uint i = 0; i < count; i++)
    // {
    //     if (i == idx) continue;
    //     vec4 dif = particlesCurrent[i].position - particlesCurrent[idx].position;
    //     float dist2 = dot(dif, dif);
    //     float result = gFunc(dist2);
    //     g += normalize(dif) * result;
    // }
    vec4 dif = vec4(0.0, 0.0, 0.0, 0.0) - particlesCurrent[idx].position;
    g += normalize(dif) * (1.0 / (dot(dif, dif) + 0.001));

    vec4 dif2 = vec4(0.5, 0.5, 0.0, 0.0) - particlesCurrent[idx].position;
    g += normalize(dif2) * (1.0 / (dot(dif2, dif2) + 0.001));

    vec4 dif3 = vec4(-0.8, -0.2, 0.0, 0.0) - particlesCurrent[idx].position;
    g += normalize(dif3) * (1.0 / (dot(dif3, dif3) + 0.001));

    p.velocity = (particlesCurrent[idx].velocity + (g * dt));
    p.position = particlesCurrent[idx].position + (p.velocity * dt);

    if (p.position.x > 0.98 || p.position.x < -0.98)
    {
        p.position.x = clamp(p.position.x, -0.975, 0.975);
        p.velocity.x = p.velocity.x * -1.0;
    }
    if (p.position.y > 0.98 || p.position.y < -0.98)
    {
        p.position.y = clamp(p.position.y, -0.975, 0.975);
        p.velocity.y = p.velocity.y * -1.0;
    }

    particlesNext[idx] = p;
}