#ifdef VERTEX_SHADER

in vec3 vertexPosition_modelspace;
in vec3 vertexPosition_interpolation;
in vec2 vertexUV;

out vec2 UV;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform float frameProgress;
uniform vec2 uvTilingD;
uniform float currentTime;

uniform vec3 cameraPosition;
out float fade;

void main(){
    vec4 posint = vec4(mix(vertexPosition_modelspace,vertexPosition_interpolation,(0.5-cos(frameProgress * 3.1415926535897932384626433832795)*0.5) ), 1);
    mat4 VMsprite = V * M;
    //VMsprite[0] = vec4(length(M[0]), 0.0, 0.0, 0.0);
    //VMsprite[1] = vec4(0.0, length(M[1]), 0.0, 0.0);
    //VMsprite[2] = vec4(0.0, 0.0, length(M[2]), 0.0);
    vec3 frontV = normalize(vec3(VMsprite[3].x, VMsprite[3].y, VMsprite[3].z));
    vec3 rightV = cross(frontV, vec3(0.0, 1.0, 0.0));
    vec3 upV    = cross(rightV, frontV);
    frontV *= length(M[2]);
    rightV *= length(M[0]);
    upV *= length(M[1]);
    VMsprite[0] = vec4(rightV, 0.0);
    VMsprite[1] = vec4(upV, 0.0);
    VMsprite[2] = vec4(frontV, 0.0);
    
    
    float leavesAngle = 10.0 * (3.1415926535897932384626433832795 / 180.0) * cos(currentTime);    
    mat4 ZRotation;
    ZRotation[0] = vec4(cos(leavesAngle), sin(leavesAngle), 0.0, 0.0);
    ZRotation[1] = vec4(-sin(leavesAngle), cos(leavesAngle), 0.0, 0.0);
    ZRotation[2] = vec4(0.0, 0.0, 1.0, 0.0);
    ZRotation[3] = vec4(0.0, 0.0, 0.0, 1.0);
    
	gl_Position =  P * VMsprite * ZRotation * posint;
	UV = vec2(vertexUV.x * uvTilingD.x, vertexUV.y * uvTilingD.y);
	fade = length(vec3(M * posint) - cameraPosition);
}

#endif
#ifdef FRAGMENT_SHADER

in lowp vec2 UV;
in lowp float fade;

out lowp vec4 color;

uniform sampler2D DiffuseTextureSampler;
uniform bool overlayBool;
uniform lowp float translucencyAlpha;
uniform lowp float useImageAlpha;

void main(){
	lowp vec4 MaterialDiffuseColor = texture( DiffuseTextureSampler, UV ).rgba;
	lowp float desiredAlpha = translucencyAlpha * max(useImageAlpha, MaterialDiffuseColor.a);
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
	if(overlayBool)
	{
		MaterialDiffuseColor = MaterialDiffuseColor*0.5 + vec4(0.0625, 0.2115, 0.5, 0.0);
	}
	color = vec4(MaterialDiffuseColor.rgb, min(desiredAlpha, alpha));
}

#endif
