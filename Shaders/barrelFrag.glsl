#version 330 core
uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
//uniform sampler2D metalnessTex;
uniform vec4 lightColour;
uniform vec3 lightPos;
uniform vec3 cameraPos;
uniform float lightRadius;

in Vertex {
	vec3 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
} IN;

out vec4 fragColour;
void main (void) {
vec3 incident = normalize(lightPos - IN.worldPos);
vec3 viewDir = normalize(cameraPos - IN.worldPos);
vec3 halfDir = normalize(incident + viewDir);

mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));
vec4 diffuse = texture(diffuseTex, IN.texCoord);
vec3 normal = texture(bumpTex, IN.texCoord).rgb;
//vec3 metalness = texture(metalnessTex, IN.texCoord).rgb;
normal = normalize(TBN * normal * 2.0 - 1.0);
//metalness = normalize(TBN * metalness * 2.0 - 1.0);

float lambert = max(dot(incident, normal), 0.0f);
float distance = length(lightPos - IN.worldPos);
float attenuation = 1.0f - clamp(distance/lightRadius, 0.0, 1.0);

float specFactor = clamp(dot(halfDir, normal) ,0.0 ,1.0);
specFactor = pow (specFactor, 60.0);

vec3 surface = (diffuse.rgb * lightColour.rgb); // Base colour
fragColour.rgb = surface * attenuation * lambert; // Diffuse
fragColour.rgb +=(lightColour.rgb * attenuation * specFactor)*0.33;
fragColour.rgb += surface * 0.1f; // Ambient
fragColour.a = diffuse.a; // Alpha
}