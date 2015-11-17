#ifdef VERTEX_SHADER

in vec3 vertexPosition_modelspace;
in vec2 vertexUV;
in vec3 vertexNormal_modelspace;
in vec3 vertexPosition_interpolation;

out vec2 UV;
out vec3 Normal_worldspace;
out vec3 Position_worldspace;
//shadows
out vec4 ShadowCoord;
out vec4 ShadowCoord_LQ;
out float dirShadows;

uniform vec3 cameraPosition;
out float fade;
out vec3 cameraPos;

uniform mat4 MVP;
uniform mat4 M;
uniform float frameProgress;
uniform vec2 uvTilingD;
//shadows
uniform mat4 ShadowBiasMVP;
uniform mat4 ShadowBiasMVP_LQ;
uniform float directionalShadows;

void main(){
	vec4 posint = vec4(mix(vertexPosition_modelspace,vertexPosition_interpolation,(0.5-cos(frameProgress * 3.1415926535897932384626433832795)*0.5) ), 1);
	gl_Position = MVP * posint;
	Position_worldspace = vec3(M * posint);
	UV = vec2(vertexUV.x * uvTilingD.x, vertexUV.y * uvTilingD.y);

	cameraPos = cameraPosition;
	fade = length(Position_worldspace - cameraPos);
	
	Normal_worldspace = normalize((M * vec4(vertexNormal_modelspace,0)).xyz);
	
//shadows
    if(directionalShadows != 0.0)
	{
	    ShadowCoord = ShadowBiasMVP * posint;
		ShadowCoord_LQ = ShadowBiasMVP_LQ * posint;
	}
	dirShadows = directionalShadows;
}

#endif
#ifdef FRAGMENT_SHADER

out lowp vec4 color;

in lowp vec2 UV;
in lowp vec3 Normal_worldspace;
in lowp vec3 Position_worldspace;
//shadows
in lowp vec4 ShadowCoord;
in lowp vec4 ShadowCoord_LQ;
in lowp float dirShadows;

in lowp vec3 cameraPos;
in lowp float fade;

uniform lowp vec3 LightPosition_worldspace[4];
uniform sampler2D DiffuseTextureSampler;
uniform lowp vec3 lightColorD[4];
uniform lowp float lightFallOff[4];
uniform lowp float lightHotSpot[4];
uniform lowp float lightIntensity[4];
//shadows
uniform sampler2DShadow shadowMap;
uniform sampler2DShadow shadowMap_LQ;
uniform lowp float shadowBorder;
uniform lowp vec3 dirLightAmbient;
uniform lowp vec3 dirLightDiffuse;
uniform lowp vec3 DLightDir;

vec2 poissonDisk[4] =  vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 )
);

/*
lowp float clamp(lowp float f, int mn, int mx)
{
	if(f < float(mn)) return float(mn);
	if(f > float(mx)) return float(mx);
	return f;
}
*/

void main(){
	lowp vec4 MaterialDiffuseColora = texture( DiffuseTextureSampler, UV ).rgba;
	if(MaterialDiffuseColora.a < 0.5)
	{
		discard;
	}
	lowp vec3 MaterialDiffuseColor = MaterialDiffuseColora.rgb;
	lowp vec3 diffuseColor = vec3(0,0,0);
	
	//shadow
	lowp float dirCos = clamp(dot(Normal_worldspace, DLightDir), 0, 1);
	lowp float visibility=1.0;
	lowp float bias;
	bool HQShadow = length(cameraPos - Position_worldspace) < shadowBorder*0.5;
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
		
		//shadows
		if(dirShadows != 0.0)
		{
			if(HQShadow)
			{
				visibility-=0.25*(1.0-texture( shadowMap,  vec3(ShadowCoord.xy + poissonDisk[i]*0.0004,  (ShadowCoord.z-bias)/ShadowCoord.w) ));
			} else {
				visibility-=0.25*(1.0-texture( shadowMap_LQ,  vec3(ShadowCoord_LQ.xy + poissonDisk[i]*0.000666666666667,  (ShadowCoord_LQ.z-bias)/ShadowCoord_LQ.w) ));
			}
		}
	}
	
	if(dirShadows != 0.0)
	{
		diffuseColor +=  MaterialDiffuseColor * (dirCos*visibility*dirLightDiffuse + dirLightAmbient);
	}
		
	lowp float alpha = 1.0;
	if(fade > 100.0)
	{
		alpha = 0.0;
	}
	else
	if(fade > 80.0)
	{
		alpha = 0.5+0.5*cos(0.1570795*fade - 12.56636); // 0.5*(1.0+cos(3.14159*(1.0-(100.0 - fade)/20.0)));
	}
	color = vec4(diffuseColor, alpha);
}
#endif

