#ifdef VERTEX_SHADER

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 5) in vec3 vertexPosition_interpolation;
layout(location = 1) in vec2 vertexUV;

out vec2 UV;

uniform mat4 MVP;
uniform float frameProgress;

void main(){
	vec3 dlt = (vertexPosition_interpolation - vertexPosition_modelspace) * ((1.0-cos(frameProgress * 3.1415926535897932384626433832795))*0.5);
	gl_Position =  MVP * vec4(vertexPosition_modelspace + dlt,1);
	UV = vertexUV;	
}

#endif
#ifdef FRAGMENT_SHADER

in lowp vec2 UV;

out lowp vec3 color;

uniform sampler2D DiffuseTextureSampler;

void main(){
	lowp vec3 MaterialDiffuseColor = texture2D( DiffuseTextureSampler, UV ).rgb;
	color = MaterialDiffuseColor;
}

#endif

