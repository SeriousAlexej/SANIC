#include "decoration.h"
#include "../dialogs/tinyfiledialogs.h"

extern std::string g_WorkingDir;

Decoration::Decoration()
{
	setClass("Decoration");
}

Decoration::~Decoration()
{
}

void Decoration::initialize()
{
    shaderPath     = "./shaders/fullbright.shader";
    modelPath       = "./models/axis/axis.obj";
    dTexturePath    = "./models/uv_checker.jpg";
    nTexturePath    = "";
    hTexturePath    = "";
    setupModel(shaderPath,
               modelPath,
               dTexturePath,
               nTexturePath,
               hTexturePath);
    setupCollision(0.0f, glm::vec3(0.5f, 0.5f, 0.5f));
    switchToModel();
	pushState(main);
}

static std::string relativePath(std::string absPath)
{
    std::replace(absPath.begin(), absPath.end(), '\\', '/');
    if(absPath.find(g_WorkingDir)==0)
    {
        absPath = "." + absPath.substr(g_WorkingDir.length());
    }
    if(absPath[0] != '.')
    {
        absPath = "." + absPath;
    }
    return absPath;
}

static void TW_CALL pickShader(void *stringPtr)
{
    std::string &path = *(std::string*)stringPtr;
    const char* filter[] = { "*.shader" };
    const char * result = tinyfd_openFileDialog("Select shader","./",1,filter,0);
    if(result) {
        path = relativePath(result);
    }
}
static void TW_CALL pickTexture(void *stringPtr)
{
    std::string &path = *(std::string*)stringPtr;
    const char* filter[] = { "*.jpg", "*.png", "*.tga" };
    const char * result = tinyfd_openFileDialog("Select Texture","./",3,filter,0);
    if(result) {
        path = relativePath(result);
    }
}
static void TW_CALL pickModel(void *stringPtr)
{
    std::string &path = *(std::string*)stringPtr;
    const char* filter[] = { "*.obj" };
    const char * result = tinyfd_openFileDialog("Select Model","./",1,filter,0);
    if(result) {
        path = relativePath(result);
    }
}

void TW_CALL updateParameters(void *decorPtr)
{
    Decoration &decor = *(Decoration*)decorPtr;
    decor.updateParamsInternal();
}

void Decoration::addShaderMutators()
{
    TwAddVarRW(entityBar, "NStrength", TW_TYPE_FLOAT, &model->normalStrength, "label='Normal strength' step=0.01 group='Filenames'");
    TwAddVarRW(entityBar, "HScale", TW_TYPE_FLOAT, &model->parallaxScale, "label='Parallax scale' step=0.001 group='Filenames'");
    TwAddVarRW(entityBar, "HOffset", TW_TYPE_FLOAT, &model->parallaxOffset, "label='Parallax offset' step=0.01 group='Filenames'");
}

void Decoration::removeShaderMutators()
{
    TwRemoveVar(entityBar, "NStrength");
    TwRemoveVar(entityBar, "HScale");
    TwRemoveVar(entityBar, "HOffset");
}

void Decoration::updateParamsInternal()
{
    glm::vec3 mutValues(model->normalStrength, model->parallaxOffset, model->parallaxScale);
    removeShaderMutators();
    setupModel(shaderPath,
               modelPath,
               dTexturePath,
               nTexturePath,
               hTexturePath);
    model->normalStrength = mutValues.x;
    model->parallaxOffset = mutValues.y;
    model->parallaxScale = mutValues.z;
    addShaderMutators();
}

void Decoration::editorSelect()
{
    Entity::editorSelect();
    TwAddVarRW(entityBar, "ModelPath", TW_TYPE_STDSTRING, &modelPath, "label='Model' group='Filenames'");
    TwAddButton(entityBar, "PickModel", pickModel, &modelPath, "label='    Pick' group='Filenames'");

    TwAddSeparator(entityBar, "sep05", "group='Filenames'");

    TwAddVarRW(entityBar, "DTextPath", TW_TYPE_STDSTRING, &dTexturePath, "label='Diffuse map' group='Filenames'");
    TwAddButton(entityBar, "PickDText", pickTexture, &dTexturePath, "label='    Pick' group='Filenames'");

    TwAddSeparator(entityBar, "sep06", "group='Filenames'");

    TwAddVarRW(entityBar, "NTextPath", TW_TYPE_STDSTRING, &nTexturePath, "label='Normal map' group='Filenames'");
    TwAddButton(entityBar, "PickNText", pickTexture, &nTexturePath, "label='    Pick' group='Filenames'");

    TwAddSeparator(entityBar, "sep07", "group='Filenames'");

    TwAddVarRW(entityBar, "HTextPath", TW_TYPE_STDSTRING, &hTexturePath, "label='Displacement map' group='Filenames'");
    TwAddButton(entityBar, "PickHText", pickTexture, &hTexturePath, "label='    Pick' group='Filenames'");

    TwAddSeparator(entityBar, "sep08", "group='Filenames'");

    TwAddVarRW(entityBar, "ShaPath", TW_TYPE_STDSTRING, &shaderPath, "label='Shader' group='Filenames'");
    TwAddButton(entityBar, "PickSha", pickShader, &shaderPath, "label='    Pick' group='Filenames'");

    TwAddSeparator(entityBar, "sep09", "group='Filenames'");

    addShaderMutators();

    TwAddSeparator(entityBar, "sep04", "");

    TwAddButton(entityBar, "UpdateButton", updateParameters, this, "label='Update parameters'");
}

STATE Decoration::main(EntityEvent* ee, Entity* caller)
{
	return;
}
