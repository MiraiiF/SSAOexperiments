#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_diffuse2;
    sampler2D texture_diffuse3;
    sampler2D texture_specular1;
    sampler2D texture_specular2;
    float shininess;
}; 

struct DirLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};
  
in vec2 texcoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform mat4 lightSpaceMatrix;

uniform vec3 viewPos;
uniform DirLight light;
uniform Material material;

uniform sampler2D shadowMap;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, float s, vec3 Albedo, float Specular);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 Albedo, float Specular);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 Albedo, float Specular);
float ShadowCalculation(vec4 fragPosLightSpace);

void main(){             
    // retrieve data from G-buffer
    vec3 FragPos = texture(gPosition, texcoords).rgb;
    vec3 Normal = texture(gNormal, texcoords).rgb;
    vec3 Albedo = texture(gAlbedoSpec, texcoords).rgb;
    float Specular = texture(gAlbedoSpec, texcoords).a;
    vec4 FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    float shadow = ShadowCalculation(FragPosLightSpace);

    vec3 result = CalcDirLight(light, norm, viewDir, shadow, Albedo, Specular);   

    FragColor = vec4(result, 1.0);
}  

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, float s, vec3 Albedo, float Specular)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * Albedo;
    vec3 diffuse = light.diffuse * diff * Albedo;
    ambient += light.ambient * Albedo;
    diffuse += light.diffuse * diff * Albedo;
    ambient += light.ambient * Albedo;
    diffuse += light.diffuse * diff * Albedo;
    vec3 specular = light.specular * spec * Specular;
    specular += light.specular * spec * Specular;
    return (ambient + (1.0 - s) * (diffuse + specular));
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 Albedo, float Specular)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 1.0; //pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient * Albedo;
    vec3 diffuse = light.diffuse * diff * Albedo;
    ambient += light.ambient * Albedo;
    diffuse += light.diffuse * diff * Albedo;
    ambient += light.ambient * Albedo;
    diffuse += light.diffuse * diff * Albedo;
    vec3 specular = light.specular * spec * Specular;
    specular += light.specular * spec * Specular;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 Albedo, float Specular)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * Albedo;
    vec3 diffuse = light.diffuse * diff * Albedo;
    ambient += light.ambient * Albedo;
    diffuse += light.diffuse * diff * Albedo;
    ambient += light.ambient * Albedo;
    diffuse += light.diffuse * diff * Albedo;
    vec3 specular = light.specular * spec * Specular;
    specular += light.specular * spec * Specular;
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}

float ShadowCalculation(vec4 fragPosLightSpace){
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float bias = 0.005;
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

    return shadow;
}