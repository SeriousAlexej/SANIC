#ifdef VERTEX_SHADER

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal_modelspace;
layout(location = 3) in vec3 vertexTangent_modelspace;
layout(location = 4) in vec3 vertexBitangent_modelspace;
layout(location = 5) in vec3 vertexPosition_interpolation;

out vec2 UV;
out vec3 Normal_worldspace;
out vec3 Position_worldspace;
out mat3 TBN;

uniform mat4 MVP;
uniform mat4 M;
uniform float frameProgress;

void main(){
	vec3 dlt = (vertexPosition_interpolation - vertexPosition_modelspace) * ((1.0-cos(frameProgress * 3.1415926535897932384626433832795))*0.5);
	gl_Position =  MVP * vec4(vertexPosition_modelspace + dlt,1);
	Position_worldspace = (M * vec4(vertexPosition_modelspace + dlt,1)).xyz;
	UV = vertexUV;

	Normal_worldspace = (M * vec4(vertexNormal_modelspace,0)).xyz;

	vec3 t = normalize((M * vec4(vertexTangent_modelspace, 0.0)).xyz);
	vec3 b = normalize((M * vec4(vertexBitangent_modelspace, 0.0)).xyz);
	vec3 n = normalize((M * vec4(vertexNormal_modelspace, 0.0)).xyz);
	TBN = mat3(t,b,n);
}

#endif
#ifdef FRAGMENT_SHADER

out lowp vec3 color;

in lowp vec2 UV;
in lowp mat3 TBN;
in lowp vec3 Normal_worldspace;
in lowp vec3 Position_worldspace;

uniform lowp vec3 LightPosition_worldspace[4];
uniform sampler2D DiffuseTextureSampler;
uniform sampler2D NormalTextureSampler;
uniform sampler2D HeightTextureSampler;
uniform lowp vec3 lightColorD[4];
uniform lowp float lightFallOff[4];
uniform lowp float lightHotSpot[4];
uniform lowp float lightIntensity[4];
uniform lowp vec3 cameraPosition;
uniform lowp float normalStrength;
uniform lowp float parallaxOffset;
uniform lowp float parallaxScale;

lowp float clamp(lowp float f, int mn, int mx)
{
	if(f < float(mn)) return float(mn);
	if(f > float(mx)) return float(mx);
	return f;
}

void main(){
	lowp float parallaxBias = 0.5 * parallaxScale * (parallaxOffset - 1.0);
	lowp vec3 toViewer = normalize(TBN * (cameraPosition - Position_worldspace));
	toViewer.y *= -1.0;

	const lowp float minLayers = 8.0;
	const lowp float maxLayers = 20.0;
	lowp float numSteps = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), toViewer)));
	
	lowp float step = 1.0/numSteps;
	lowp vec2 dtex = toViewer.xy * parallaxScale / (numSteps * toViewer.z);
	lowp float height = 1.0;
	lowp vec2 parallaxUV = UV.xy - toViewer.xy*parallaxBias;
	lowp float h = texture2D(HeightTextureSampler, parallaxUV).r;

	while(h < height)
	{
		height -= step;
		parallaxUV += dtex;
		h = texture2D(HeightTextureSampler, parallaxUV).r;
	}

	lowp vec2 delta = 0.5 * dtex;
	lowp vec2 mid = parallaxUV - delta;

	for(int i=0; i<5; i++)
	{
		delta *= 0.5;
		
		if(texture2D(HeightTextureSampler, mid).r < height)
		{
			mid += delta;
		} else {
			mid -= delta;
		}
	}

	parallaxUV = mid;

	lowp vec3 MaterialDiffuseColor = texture2D(DiffuseTextureSampler, parallaxUV).xyz;
	lowp vec3 n = TBN * (255.0/128.0 * texture2D(NormalTextureSampler, parallaxUV).xyz - 1.0);
	n = normalize((1.0 + normalStrength) * Normal_worldspace - n * normalStrength);
	lowp vec3 diffuseColor = vec3(0,0,0);

	for(int i=0; i<4; i++)
	{
		lowp vec3 toLight = (LightPosition_worldspace[i] - Position_worldspace);
		lowp float lightDistance = length(toLight);
		lowp float cosTheta = clamp(dot(n, normalize(toLight)), 0, 1);
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

