#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

struct Light {
	vec3 position;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	
	float linear;
	float constant;
	float quadratic;
};

//struct Material {
//    sampler2D diffuse;
//    sampler2D specular;
//	sampler2D height;
//    float     shininess;
//};

uniform Light pointLight;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
const float shininess = 32.0;

uniform vec3 viewPos;
// uniform Material material;

void main()
{
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 lightDir = normalize(pointLight.position - FragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // attenuation
    float distance = length(pointLight.position - FragPos);
    float attenuation = 1.0 / (pointLight.constant + pointLight.linear * distance + pointLight.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = pointLight.ambient * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse = pointLight.diffuse * diff * vec3(texture(texture_diffuse1, TexCoords));
    vec3 specular = pointLight.specular * spec * vec3(texture(texture_specular1, TexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}