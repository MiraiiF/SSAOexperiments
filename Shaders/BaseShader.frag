#version 420 core
out vec4 cores;
in vec3 fragpos;
in vec3 norm;

void main(){
    cores = vec4(norm, 1.0);
}