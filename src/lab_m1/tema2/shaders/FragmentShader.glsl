#version 330 core

in vec3 v_color;
in vec3 v_normal;

layout(location = 0) out vec4 out_color;

void main()
{
    vec3 n = normalize(v_normal);
    vec3 lightDir = normalize(vec3(0.4, 1.0, 0.3));
    float diff = clamp(dot(n, lightDir), 0.1, 1.0);
    vec3 rgb = v_color * diff;
    out_color = vec4(rgb, 1.0);
}
