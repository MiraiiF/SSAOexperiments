#version 430 core
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norms;
layout(location = 2) in vec2 texcoords;

out vec3 norm;
out vec3 fragpos;
uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;
void main(){
    gl_Position = proj* view * model * vec4(pos, 1.0);
    fragpos = pos;
    norm = normalize(mat3(transpose(inverse(model))) * norms);
}