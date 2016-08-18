#include "shader.h"
#include "global.h"

Shader::Shader(std::string shaderPath)
{
    srcShaFnm = shaderPath;
    shaderID = 0;
    MatrixID = 0;
    mID = 0;
    NormalTextureID = 0;
    DiffuseTextureID = 0;
    HeightTextureID = 0;
    FrameProgressID = 0;
    normStrengthID = 0;
    parallaxScaleID = 0;
    parallaxOffsetID = 0;
    CamPosID = 0;
    BiasMVP = 0;
    DLightAmbient = 0;
    DLightDiffuse = 0;
    DirShadowBool = 0;
    ShadowMap = 0;
    DLightDir = 0;
    ShadowMap_LQ = 0;
    BiasMVP_LQ = 0;
    ShadowBorder = 0;
    for(int i=0; i<4; i++)
    {
        LightPositionID[i] = 0;
        LightDiffuseID[i] = 0;
        LightFallOffID[i] = 0;
        LightHotSpotID[i] = 0;
        LightIntensityID[i] = 0;
    }
    std::string toUse = shaderPath;
    if(toUse!="")
    {
        toUse = egg::getInstance().g_WorkingDir + toUse.substr(1);
    }
    loadShaders(toUse);
}

Shader::~Shader()
{
    if(shaderID)
        glDeleteProgram(shaderID);
}

void Shader::loadShaders(std::string shaderPath)
{
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    std::string ShaderVersion = "#version 130\n";
    std::string VertexShaderCode = ShaderVersion + "#define VERTEX_SHADER\n";
    std::string FragmentShaderCode = ShaderVersion + "#define FRAGMENT_SHADER\n";
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
#ifdef SANIC_DEBUG
    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

    // Compile Fragment Shader
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
    glBindAttribLocation(shaderID, 0, "vertexPosition_modelspace");
    glBindAttribLocation(shaderID, 1, "vertexUV");
    glBindAttribLocation(shaderID, 2, "vertexNormal_modelspace");
    glBindAttribLocation(shaderID, 3, "vertexTangent_modelspace");
    glBindAttribLocation(shaderID, 4, "vertexBitangent_modelspace");
    glBindAttribLocation(shaderID, 5, "vertexPosition_interpolation");
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
    BiasMVP = glGetUniformLocation(shaderID, "ShadowBiasMVP");
    DLightAmbient = glGetUniformLocation(shaderID, "dirLightAmbient");
    DLightDiffuse = glGetUniformLocation(shaderID, "dirLightDiffuse");
    DirShadowBool = glGetUniformLocation(shaderID, "directionalShadows");
    ShadowMap = glGetUniformLocation(shaderID, "shadowMap");
    DLightDir = glGetUniformLocation(shaderID, "DLightDir");
    BiasMVP_LQ = glGetUniformLocation(shaderID, "ShadowBiasMVP_LQ");
    ShadowMap_LQ = glGetUniformLocation(shaderID, "shadowMap_LQ");
    ShadowBorder = glGetUniformLocation(shaderID, "shadowBorder");

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
