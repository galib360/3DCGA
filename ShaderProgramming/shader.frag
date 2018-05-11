#version 430

// Global variables for lighting calculations
layout(location = 1) uniform vec3 viewPos;
layout(location = 2) uniform sampler2D texToon;
layout(location = 3) uniform float time;

// Output for on-screen color
layout(location = 0) out vec4 outColor;

// Interpolated output data from vertex shader
in vec3 fragPos; // World-space position
in vec3 fragNormal; // World-space normal

//const vec3 lightPos = vec3(-1, 1, 1);
const vec3 lightPos = vec3(sin(time), 1.0, cos(time));
const vec3 lightColor = vec3(0.5, 0.5, 0.5);
const float lightPower = 30.0;
const vec3 ambColor = vec3(0.1, 0.1, 0.1);
const vec3 diffuseColor = vec3(0.4, 0.4, 0.4);
const vec3 specColor = vec3(1.0, 1.0, 1.0);
const float shininess = 16.0;



void toonNoTex(float testColor) {
	// Toon shading (no texture)
	if(testColor > 0.875) {
		outColor = vec4(vec3(1,1,1), 1.0);
	} else if (testColor > 0.625) {
		outColor = vec4(vec3(0.75,0.75,0.75), 1.0);
	} else if (testColor > 0.375) {
		outColor = vec4(vec3(0.5,0.5,0.5), 1.0);
	} else if (testColor > 0.125) {
		outColor = vec4(vec3(0.25,0.25,0.25), 1.0);
	} else {
		outColor = vec4(vec3(0,0,0), 1.0);
	}
}

void xtoontex(float testColor){
	if(testColor> 0.75){
		outColor = texture(texToon, vec2(1.0, 0.0));
	}
	else if (testColor > 0.5){
		outColor = texture(texToon, vec2(0.75, 0.0));
	}
	else if (testColor > 0.25){
		outColor = texture(texToon, vec2(0.50, 0.0));
	}
	else if (testColor > 0){
		outColor = texture(texToon, vec2(0.25, 0.0));
	}
	else {
		outColor = texture(texToon, vec2(0.0, 0.0));
	}
}

void main() {
    // Output the normal as color
	vec3 normal = normalize(fragNormal);
	vec3 lightDir = lightPos - fragPos;
	float distance = length(lightDir);
	distance = distance*distance;
	lightDir = normalize(lightDir);
	
	float lambertian = max(dot(lightDir, normal), 0.0);
	float specular = 0.0;

	if(lambertian> 0.0){
		vec3 viewDir = normalize(-fragPos);

		vec3 halfDir = normalize(lightDir + viewDir);
		float specAngle = max(dot(halfDir, normal), 0.0);
		specular = pow(specAngle, shininess);
	}

	vec3 finalColor = ambColor + (diffuseColor * lambertian * lightColor * lightPower / distance) + (specColor * specular * lightColor * lightPower / distance);

	//toonNoTex(lambertian);

	xtoontex(lambertian);

	//outColor = vec4(finalColor, 1.0); 

    //outColor = vec4(abs(fragNormal)*cos(time), 1.0);

	//outColor = vec4(abs(fragNormal), 1.0);
}