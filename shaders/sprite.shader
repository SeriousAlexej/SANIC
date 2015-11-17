#ifdef VERTEX_SHADER

in vec3 vertexPosition_modelspace;
in vec3 vertexPosition_interpolation;
in vec2 vertexUV;

out vec2 UV;

uniform mat4 MVP;
uniform float frameProgress;
uniform vec2 uvTilingD;

uniform vec3 cameraPosition;
out float fade;
uniform mat4 M;

void main(){
	vec4 posint = vec4(mix(vertexPosition_modelspace,vertexPosition_interpolation,(0.5-cos(frameProgress * 3.1415926535897932384626433832795)*0.5) ), 1);
	gl_Position =  MVP * vec4(0.0,0.0,0.0,1.0) + posint;
	UV = vec2(vertexUV.x * uvTilingD.x, vertexUV.y * uvTilingD.y);
	fade = length(vec3(M * posint) - cameraPosition);
}

#endif
#ifdef FRAGMENT_SHADER

in lowp vec2 UV;
in lowp float fade;

out lowp vec4 color;

uniform sampler2D DiffuseTextureSampler;

void main(){
	lowp vec4 MaterialDiffuseColor = texture( DiffuseTextureSampler, UV ).rgba;
	if(MaterialDiffuseColor.a < 0.5)
	{
		discard;
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
	color = vec4(MaterialDiffuseColor.rgb, alpha);
}

#endif

