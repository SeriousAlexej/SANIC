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
//shadows
out vec4 ShadowCoord;
out vec4 ShadowCoord_LQ;
out float dirShadows;

uniform mat4 MVP;
uniform mat4 M;
uniform float frameProgress;
//shadows
uniform mat4 ShadowBiasMVP;
uniform mat4 ShadowBiasMVP_LQ;
uniform float directionalShadows;

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
//shadows
    if(directionalShadows != 0.0)
	{
	    ShadowCoord = ShadowBiasMVP * vec4(vertexPosition_modelspace + dlt, 1);
		ShadowCoord_LQ = ShadowBiasMVP_LQ * vec4(vertexPosition_modelspace + dlt, 1);
	}
	dirShadows = directionalShadows;
}

#endif
#ifdef FRAGMENT_SHADER

out lowp vec3 color;

in lowp vec2 UV;
in lowp mat3 TBN;
in lowp vec3 Normal_worldspace;
in lowp vec3 Position_worldspace;
//shadows
in lowp vec4 ShadowCoord;
in lowp vec4 ShadowCoord_LQ;
in lowp float dirShadows;

uniform lowp vec3 LightPosition_worldspace[4];
uniform sampler2D DiffuseTextureSampler;
uniform sampler2D NormalTextureSampler;
uniform lowp vec3 lightColorD[4];
uniform lowp float lightFallOff[4];
uniform lowp float lightHotSpot[4];
uniform lowp float lightIntensity[4];
uniform lowp vec3 cameraPosition;
uniform lowp float normalStrength;
//shadows
uniform sampler2DShadow shadowMap;
uniform sampler2DShadow shadowMap_LQ;
uniform lowp float shadowBorder;
uniform lowp vec3 dirLightAmbient;
uniform lowp vec3 dirLightDiffuse;
uniform lowp vec3 DLightDir;

lowp vec2 poissonDisk[4] = lowp vec2[]( 
  lowp vec2( -0.94201624, -0.39906216 ), 
  lowp vec2( 0.94558609, -0.76890725 ), 
  lowp vec2( -0.094184101, -0.92938870 ), 
  lowp vec2( 0.34495938, 0.29387760 )
);

lowp float clamp(lowp float f, int mn, int mx)
{
	if(f < float(mn)) return float(mn);
	if(f > float(mx)) return float(mx);
	return f;
}

void main(){
	lowp vec4 MaterialDiffuseColora = texture(DiffuseTextureSampler, UV).rgba;
	if(MaterialDiffuseColora.a < 0.5)
	{
		discard;
	}
	lowp vec3 MaterialDiffuseColor = MaterialDiffuseColora.rgb;
	lowp vec3 n = TBN * (255.0/128.0 * texture(NormalTextureSampler, UV).xyz - 1.0);
	n = normalize((1.0 + normalStrength) * Normal_worldspace - n * normalStrength);
	lowp vec3 diffuseColor = vec3(0,0,0);
	
	//shadow
	lowp float dirCos = clamp(dot(n, DLightDir), 0, 1);
	lowp float visibility=1.0;
	lowp float bias;
	bool HQShadow = length(cameraPosition - Position_worldspace) < shadowBorder*0.5;
	if(HQShadow)
	{
		bias = 0.0006*tan(acos(dirCos));
	} else {
		bias = 0.001*tan(acos(dirCos));
	}
	
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
	
		//shadows
		if(dirShadows != 0.0)
		{
			if(HQShadow)
			{
				visibility-=0.25*(1.0-texture( shadowMap, lowp vec3(ShadowCoord.xy + poissonDisk[i]/2500.0,  (ShadowCoord.z-bias)/ShadowCoord.w) ));
			} else {
				visibility-=0.25*(1.0-texture( shadowMap_LQ, lowp vec3(ShadowCoord_LQ.xy + poissonDisk[i]/1500.0,  (ShadowCoord_LQ.z-bias)/ShadowCoord_LQ.w) ));
			}
		}
	}
	
	if(dirShadows != 0.0)
	{
		diffuseColor +=  MaterialDiffuseColor * (dirCos*visibility*dirLightDiffuse + dirLightAmbient);
	}
	
	color = diffuseColor;
}

#endif

