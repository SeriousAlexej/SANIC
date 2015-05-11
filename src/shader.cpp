#include "shader.h"

#ifdef SANIC_DEBUG
int Shader::numberOfCreations = 0;
int Shader::numberOfDeletions = 0;
#endif // SANIC_DEBUG

Shader::Shader(std::string shaderPath)
{
    #ifdef SANIC_DEBUG
    numberOfCreations++;
    #endif // SANIC_DEBUG
	srcShaFnm = shaderPath;
	shaderID = 0;
	MatrixID = 0;
	mID = 0;
	CamPosID = 0;
	NormalTextureID = 0;
	DiffuseTextureID = 0;
	HeightTextureID = 0;
	normStrengthID = 0;
	parallaxScaleID = 0;
	parallaxOffsetID = 0;
	for(int i=0; i<4; i++)
    {
        LightPositionID[i] = 0;
        LightDiffuseID[i] = 0;
        LightFallOffID[i] = 0;
        LightHotSpotID[i] = 0;
        LightIntensityID[i] = 0;
    }
	loadShaders(shaderPath);
}

Shader::~Shader()
{
    #ifdef SANIC_DEBUG
    numberOfDeletions++;
    #endif // SANIC_DEBUG
	if(shaderID)
		glDeleteProgram(shaderID);
}

void Shader::loadShaders(std::string shaderPath)
{
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    std::string VertexShaderCode = "#define VERTEX_SHADER\n";
    std::string FragmentShaderCode = "#define FRAGMENT_SHADER\n";
	std::ifstream ShaderStream(shaderPath.c_str(), std::ios::in);
    if(ShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(ShaderStream, Line))
        {
            FragmentShaderCode += "\n" + Line;
            VertexShaderCode += "\n" + Line;
        }
        ShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
#ifdef SANIC_DEBUG
	printf("\nCompiling shader : %s\n", shaderPath.c_str());
	printf("Vertex shader... ");
#endif
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

    // Compile Fragment Shader
#ifdef SANIC_DEBUG
	printf("Fragment shader... ");
#endif
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
#ifdef SANIC_DEBUG
    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);
#endif

    // Link the program
#ifdef SANIC_DEBUG
    fprintf(stdout, "Linking program... ");
#endif
    shaderID = glCreateProgram();
    glAttachShader(shaderID, VertexShaderID);
    glAttachShader(shaderID, FragmentShaderID);
    glLinkProgram(shaderID);

    // Check the program
    glGetProgramiv(shaderID, GL_LINK_STATUS, &Result);
    glGetProgramiv(shaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( glm::max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(shaderID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
#ifdef SANIC_DEBUG
    fprintf(stdout, "%s\n\n", &ProgramErrorMessage[0]);
#endif

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

	MatrixID = glGetUniformLocation(shaderID, "MVP");
	mID = glGetUniformLocation(shaderID, "M");
	HeightTextureID = glGetUniformLocation(shaderID, "HeightTextureSampler");
	NormalTextureID  = glGetUniformLocation(shaderID, "NormalTextureSampler");
	DiffuseTextureID  = glGetUniformLocation(shaderID, "DiffuseTextureSampler");
	FrameProgressID = glGetUniformLocation(shaderID, "frameProgress");
	CamPosID = glGetUniformLocation(shaderID, "cameraPosition");
	normStrengthID = glGetUniformLocation(shaderID, "normalStrength");
	parallaxScaleID = glGetUniformLocation(shaderID, "parallaxScale");
	parallaxOffsetID = glGetUniformLocation(shaderID, "parallaxOffset");

	for(int i=0; i<4; i++)
    {
        LightPositionID[i] = glGetUniformLocation(shaderID, ("LightPosition_worldspace[" + std::to_string(i) + "]").c_str());
        LightDiffuseID[i] = glGetUniformLocation(shaderID, ("lightColorD[" + std::to_string(i) + "]").c_str());
        LightFallOffID[i] = glGetUniformLocation(shaderID, ("lightFallOff[" + std::to_string(i) + "]").c_str());
        LightHotSpotID[i] = glGetUniformLocation(shaderID, ("lightHotSpot[" + std::to_string(i) + "]").c_str());
        LightIntensityID[i] = glGetUniformLocation(shaderID, ("lightIntensity[" + std::to_string(i) + "]").c_str());
    }
}

void Shader::bind()
{
	glUseProgram(shaderID);
}
