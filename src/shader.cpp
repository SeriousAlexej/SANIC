#include "shader.h"

Shader::Shader(std::string vertex_file_path, std::string fragment_file_path)
{
	srcVxShaFnm = vertex_file_path;
	srcFgShaFnm = fragment_file_path;
	shaderID = 0;
	MatrixID = 0;
	mID = 0;
	vID = 0;
	NormalTextureID = 0;
	DiffuseTextureID = 0;
	ModelView3x3MatrixID = 0;
	LightPositionID = 0;
	LightDiffuseID = 0;
	LightAmbientID = 0;
	LightFallOffID = 0;
	loadShaders(vertex_file_path, fragment_file_path);
}

Shader::~Shader()
{
	if(shaderID)
		glDeleteProgram(shaderID);
}

void Shader::loadShaders(std::string vertex_file_path, std::string fragment_file_path)
{
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
 
    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path.c_str(), std::ios::in);
    if(VertexShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }
 
    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path.c_str(), std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }
 
    GLint Result = GL_FALSE;
    int InfoLogLength;
 
    // Compile Vertex Shader
#ifdef SANIC_DEBUG
	printf("Compiling shader : %s\n", vertex_file_path.c_str());
#endif
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);
 
    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);
 
    // Compile Fragment Shader
#ifdef SANIC_DEBUG
	printf("Compiling shader : %s\n", fragment_file_path.c_str());
#endif
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
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
    fprintf(stdout, "Linking program\n");
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
    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);
#endif
 
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

	MatrixID = glGetUniformLocation(shaderID, "MVP");
	mID = glGetUniformLocation(shaderID, "M");
	vID = glGetUniformLocation(shaderID, "V");
	NormalTextureID  = glGetUniformLocation(shaderID, "NormalTextureSampler");
	DiffuseTextureID  = glGetUniformLocation(shaderID, "DiffuseTextureSampler");
	ModelView3x3MatrixID = glGetUniformLocation(shaderID, "MV3x3");
	FrameProgressID = glGetUniformLocation(shaderID, "frameProgress");
	LightPositionID = glGetUniformLocation(shaderID, "LightPosition_worldspace");
	LightDiffuseID = glGetUniformLocation(shaderID, "lightColorD");
	LightAmbientID = glGetUniformLocation(shaderID, "lightColorA");
	LightFallOffID = glGetUniformLocation(shaderID, "lightFallOff");
	LightHotSpotID = glGetUniformLocation(shaderID, "lightHotSpot");
}

void Shader::bind()
{
	glUseProgram(shaderID);
}