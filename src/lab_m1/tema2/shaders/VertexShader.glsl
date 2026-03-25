#version 330 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 3) in vec3 a_color;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

uniform float u_amp;    // >0 when broken
uniform float u_seed;   // per-rail seed for unique static offsets
uniform int   u_type;   // 0=standard,1=mountain,2=sea

out vec3 v_color;
out vec3 v_normal;

// Simple hash based on position for deterministic static offsets
float hash31(vec3 p)
{
    // Large primes for decorrelated components
    const vec3 dotDir = vec3(12.9898, 78.233, 45.164);
    return fract(sin(dot(p, dotDir)) * 43758.5453);
}

void main()
{
    vec4 wp = Model * vec4(a_position, 1.0);

    // Broken flag from amplitude (only broken rails set u_amp > 0)
    float broken = u_amp > 0.0001 ? 1.0 : 0.0;
    float mixAmt = clamp(u_amp * 10.0, 0.0, 1.0);
    vec3 brokenTint;
    if (u_type == 1)      brokenTint = vec3(0.35, 0.20, 0.12);
    else if (u_type == 2) brokenTint = vec3(0.08, 0.08, 0.18);
    else                  brokenTint = vec3(0.30, 0.05, 0.05);
    v_color = mix(a_color, brokenTint, mixAmt * broken);

    if (broken > 0.0)
    {
        // Static deformation derived from world position
        float h1 = hash31(wp.xyz + u_seed);
        float h2 = hash31(wp.yzx + u_seed + 13.37);
        float h3 = hash31(wp.zxy + u_seed + 37.13);

        // Type-specific deformation intensities
        float ampX = (u_type == 1) ? 0.4 : (u_type == 2 ? 0.8 : 0.6);
        float ampY = (u_type == 1) ? 3.5 : (u_type == 2 ? 2.5 : 3.0);
        float ampZ = (u_type == 1) ? 1.0 : (u_type == 2 ? 1.8 : 1.2);

        vec3 offset;
        offset.x = (h1 - 0.5) * u_amp * ampX * 2.0;
        offset.y = (h2 - 0.5) * u_amp * ampY;
        offset.z = (h3 - 0.5) * u_amp * ampZ;
        wp.xyz += offset;
    }

    mat3 normalM = transpose(inverse(mat3(Model)));
    v_normal = normalize(normalM * a_normal);

    gl_Position = Projection * View * wp;
}
