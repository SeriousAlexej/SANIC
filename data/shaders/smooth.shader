#ifdef VERTEX_SHADER

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal_modelspace;
layout(location = 5) in vec3 vertexPosition_interpolation;

out vec2 UV;
out vec3 Normal_worldspace;
out vec3 Position_worldspace;

uniform mat4 MVP;
uniform mat4 M;
uniform float frameProgress;

void main(){
	vec3 dlt = (vertexPosition_interpolation - vertexPosition_modelspace) * ((1.0-cos(frameProgress * 3.1415926535897932384626433832795))*0.5);
	gl_Position = MVP * vec4(vertexPosition_modelspace + dlt, 1);
	Position_worldspace = vec3(M * vec4(vertexPosition_modelspace + dlt, 1));
	UV = vertexUV;
	
	Normal_worldspace = normalize((M * vec4(vertexNormal_modelspace,0)).xyz);
}

#endif
#ifdef FRAGMENT_SHADER

out lowp vec3 color;

in lowp vec2 UV;
in lowp vec3 Normal_worldspace;
in lowp vec3 Position_worldspace;

uniform lowp vec3 LightPosition_worldspace[4];
uniform sampler2D DiffuseTextureSampler;
uniform lowp vec3 lightColorD[4];
uniform lowp float lightFallOff[4];
uniform lowp float lightHotSpot[4];
uniform lowp float lightIntensity[4];

lowp float clamp(lowp float f, int mn, int mx)
{
	if(f < float(mn)) return float(mn);
	if(f > float(mx)) return float(mx);
	return f;
}

void main(){
	lowp vec3 MaterialDiffuseColor = texture( DiffuseTextureSampler, UV ).rgb;
	lowp vec3 diffuseColor = vec3(0,0,0);

	for(int i=0; i<4; i++)
	{
		lowp vec3 toLight = (LightPosition_worldspace[i] - Position_worldspace);
		lowp float lightDistance = length(toLight);
		lowp float cosTheta = clamp(dot(Normal_worldspace, normalize(toLight)), 0, 1);
		lowp float diffuseComponent = 0.0;

		if(lightDistance < lightHotSpot[i])
		{
			diffuseComponent = 1.0;
		}
		else
		if(lightDistance < lightFallOff[i])
		{
			diffuseComponent = (lightFallOff[i] - lightDistance) / (lightFallOff[i] - lightHotSpot[i]);
		}

		diffuseColor += MaterialDiffuseColor * lightColorD[i] * diffuseComponent * cosTheta * lightIntensity[i];
	}
		
	color = diffuseColor;
}
#endif

