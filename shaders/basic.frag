#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fPosLightSpace;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;

//lighting
uniform vec3 lightDir;
uniform vec3 spotLightDir;
uniform vec3 lightColor;
uniform vec3 pointLightPosEye;
uniform vec3 spotLightPosEye;

// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

uniform vec3 night;

//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;

float constant = 1.0f;
float linear = 0.09f;
float quadratic =  0.032f;

float cutOff = cos(radians(12.5f));
float outerCutOff = cos(radians(15.0f));
   
vec3 color;

float computeShadows(){
	vec3 normalizedCoords = fPosLightSpace.xyz / fPosLightSpace.w;
	normalizedCoords = normalizedCoords * 0.5 + 0.5;
	float shadow;
	if (normalizedCoords.z > 1.0f) {
		shadow = 0.0f;
	}
	else {
	
		float bias = max(0.05f * (1.0f - dot(fNormal, lightDir)), 0.005f);
		
		float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
		float currentDepth = normalizedCoords.z;

		if(currentDepth - bias > closestDepth)
			shadow = 1.0f;
		else shadow = 0.0f;
	}
    
	return shadow;
}

vec3 computeDirLight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;
		
	float shadow = computeShadows();  

    vec3 color = min((ambient + (1.0f - shadow)*diffuse) * texture(diffuseTexture, fTexCoords).rgb + (1.0f - shadow)*specular * texture(specularTexture, fTexCoords).rgb, 1.0f);

    return color;
}

vec3 computePointLight(){
    
	vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    
	//transform normal
	vec3 normalEye = normalize(fNormal);	
	
	//compute light direction
	vec3 lightDirN = normalize(pointLightPosEye - fPosEye.xyz);
	
	//compute view direction 
	vec3 viewDir = normalize(- fPosEye.xyz);

	//compute half vector
	vec3 halfVector = normalize(lightDirN + viewDir);
	
	//compute distance to light
	float dist = length(pointLightPosEye - fPosEye.xyz);
	
	//compute attenuation
	float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));

	//compute ambient light
	ambient = att * ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = att * max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	ambient *= texture(diffuseTexture, fTexCoords);
	diffuse *= texture(diffuseTexture, fTexCoords);

	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), 32);
	specular = att * specularStrength * specCoeff * lightColor;
	specular *= texture(specularTexture, fTexCoords);

	vec3 color = min((ambient + diffuse) * texture(diffuseTexture, fTexCoords).rgb + specular * texture(specularTexture, fTexCoords).rgb, 1.0f);

	return color;
}

vec3 computeSpotLight(){
	vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    
	//transform normal
	vec3 normalEye = normalize(fNormal);	
	
	//compute light direction
	vec3 lightDirN = normalize(spotLightPosEye - fPosEye.xyz);
	
	//compute view direction 
	vec3 viewDir = normalize(- fPosEye.xyz);

	//compute half vector
	vec3 halfVector = normalize(lightDirN + viewDir);
	
	//compute distance to light
	float dist = length(spotLightPosEye - fPosEye.xyz);
	
	//compute attenuation
	float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));
	
	float theta = dot(lightDirN, normalize(-spotLightDir)); 

    float epsilon = cutOff - outerCutOff;
    float intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);
	 
	//compute ambient light
	ambient = intensity * att * ambientStrength * lightColor;

	//compute diffuse light
	diffuse = intensity * att * max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	ambient *= texture(diffuseTexture, fTexCoords);
	diffuse *= texture(diffuseTexture, fTexCoords);

	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), 32);

	specular = intensity * att * specularStrength * specCoeff * lightColor;
	specular *= texture(specularTexture, fTexCoords);
	
	vec3 color = min((ambient + diffuse) * texture(diffuseTexture, fTexCoords).rgb + specular * texture(specularTexture, fTexCoords).rgb, 1.0f);

	return color;
}

void main() 
{
	if( night.x == 1.0f && night.y == 1.0f && night.x == 1.0f){
	color = computeSpotLight() +  computePointLight();
	}
	
	if( night.x == 0.0f && night.y == 0.0f && night.x == 0.0f){
	color =  computeDirLight();
	}

    //compute final vertex color
    fColor = vec4(color, 1.0f);
}
