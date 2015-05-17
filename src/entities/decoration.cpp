#include "decoration.h"
#include "../dialogs/tinyfiledialogs.h"

extern std::string g_WorkingDir;

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

Decoration::Decoration()
{
	setClass("Decoration");
}

Decoration::~Decoration()
{
}

void Decoration::addProperties()
{
    Entity::addProperties();

    registerProperties(
                "ModelPath",     &modelPath,
                "DTextPath",     &dTexturePath,
                "NTextPath",     &nTexturePath,
                "HTextPath",     &hTexturePath,
                "ShaPath",       &shaderPath,
                "NStrength",     &model->normalStrength,
                "HScale",        &model->parallaxScale,
                "HOffset",       &model->parallaxOffset
    );

    addDrawableElements(
        {
            {
                DrawableElement{DrawableElement::PT_STRING, "ModelPath", "label='Model' "},
                DrawableElement{DrawableElement::PT_BUTTON, "PickModel", "label='    Pick' ", &modelPath, pickModel}
            },
            {
                DrawableElement{DrawableElement::PT_STRING, "DTextPath", "label='Diffuse map' "},
                DrawableElement{DrawableElement::PT_BUTTON, "PickDText", "label='    Pick' ", &dTexturePath, pickTexture}
            },
            {
                DrawableElement{DrawableElement::PT_STRING, "NTextPath", "label='Normal map' "},
                DrawableElement{DrawableElement::PT_BUTTON, "PickNText", "label='    Pick' ", &nTexturePath, pickTexture}
            },
            {
                DrawableElement{DrawableElement::PT_STRING, "HTextPath", "label='Height map' "},
                DrawableElement{DrawableElement::PT_BUTTON, "PickHText", "label='    Pick' ", &hTexturePath, pickTexture}
            },
            {
                DrawableElement{DrawableElement::PT_STRING, "ShaPath", "label='Shader' "},
                DrawableElement{DrawableElement::PT_BUTTON, "PickSha", "label='    Pick' ", &shaderPath, pickShader}
            },
            {
                DrawableElement{DrawableElement::PT_FLOAT, "NStrength", "label='Normal strength' step=0.01 "},
                DrawableElement{DrawableElement::PT_FLOAT, "HScale", "label='Parallax scale' step=0.001 "},
                DrawableElement{DrawableElement::PT_FLOAT, "HOffset", "label='Parallax offset' step=0.01 "}
            },
            {
                DrawableElement{DrawableElement::PT_BUTTON, "UpdateButton", "label='Update parameters' ", this, updateParameters}
            }
        }
    );

    setName("Decoration");
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

	addProperties();
}

void Decoration::updateShaderMutators()
{
    properties["NStrength"]->ChangeLocation(&model->normalStrength);
    properties["HScale"]->ChangeLocation(&model->parallaxScale);
    properties["HOffset"]->ChangeLocation(&model->parallaxOffset);
}

void Decoration::updateParamsInternal()
{
    glm::vec3 mutValues(model->normalStrength, model->parallaxOffset, model->parallaxScale);
    setupModel(shaderPath,
               modelPath,
               dTexturePath,
               nTexturePath,
               hTexturePath);
    model->normalStrength = mutValues.x;
    model->parallaxOffset = mutValues.y;
    model->parallaxScale = mutValues.z;
    updateShaderMutators();

    drawGuiElements();
}

void Decoration::editorSelect()
{
    Entity::editorSelect();
}

STATE Decoration::main(EntityEvent* ee, Entity* caller)
{
	return;
}
