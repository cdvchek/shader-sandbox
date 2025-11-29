#version 330 core

struct Material {
    float shininess;
    // sampler2D specular;
    // sampler2D diffuse;
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

#define NR_POINT_LIGHTS 4
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform DirLight dirLight;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_diffuse3;
uniform sampler2D texture_diffuse4;

uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;
uniform sampler2D texture_specular3;
uniform sampler2D texture_specular4;


uniform vec3 viewPos;
uniform Material material;

in vec2 TexCoords;
in vec3 Normal;  
in vec3 FragPos;  
out vec4 FragColor;

// Helper: Quantize a value to discrete steps for cel shading
float toonStep(float value, float levels) {
    return floor(value * levels) / levels;
}

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    
    // diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    // diff = max(toonStep(diff, 3.0), 0.2); // minimum diffuse contribution
    
    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // spec = max(toonStep(spec, 2.0), 0.0); // minimum specular contribution
    
    vec3 ambient  = light.ambient  * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(texture_specular1, TexCoords));
    
    return ambient + diffuse + specular;
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    // diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    // diff = max(toonStep(diff, 3.0), 0.2);
    
    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // spec = max(toonStep(spec, 2.0), 0.0);
    
    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                               light.quadratic * (distance * distance));    
    
    vec3 ambient  = light.ambient  * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(texture_specular1, TexCoords));
    
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    
    return ambient + diffuse + specular;
}

void main() 
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    vec3 result = calcDirLight(dirLight, norm, viewDir);
    
    for(int i = 0; i < NR_POINT_LIGHTS; i++) 
        result += calcPointLight(pointLights[i], norm, FragPos, viewDir);
    
    // Optional: simple outline based on angle to view
    float edge = dot(norm, viewDir);
    // if(edge < 0.15)
    //     FragColor = vec4(1.0); // black outline
    // else
        FragColor = vec4(result, 1.0);
}
