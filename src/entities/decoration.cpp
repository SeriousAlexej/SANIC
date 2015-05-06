#include "decoration.h"
#include "../dialogs/tinyfiledialogs.h"

Decoration::Decoration()
{
	setClass("Decoration");
}

Decoration::~Decoration()
{
}

void Decoration::initialize()
{
    vShaderPath     = "./shaders/fullbright.vsh";
	fShaderPath     = "./shaders/fullbright.fsh";
    modelPath       = "./models/axis/axis.obj";
    dTexturePath    = "./models/uv_checker.jpg";
    nTexturePath    = "";
    setupModel(vShaderPath,
		       fShaderPath,
               modelPath,
               dTexturePath,
               nTexturePath);
    setupCollision(0.0f, glm::vec3(0.5f, 0.5f, 0.5f));
    switchToModel();
	pushState(main);
}

static void TW_CALL pickFShader(void *stringPtr)
{
    std::string &path = *(std::string*)stringPtr;
    const char* filter[] = { "*.fsh" };
    const char * result = tinyfd_openFileDialog("Select Fragment shader","./",1,filter,0);
    if(result) {
        path = result;
    }
}
static void TW_CALL pickVShader(void *stringPtr)
{
    std::string &path = *(std::string*)stringPtr;
    const char* filter[] = { "*.vsh" };
    const char * result = tinyfd_openFileDialog("Select Vertex shader","./",1,filter,0);
    if(result) {
        path = result;
    }
}
static void TW_CALL pickTexture(void *stringPtr)
{
    std::string &path = *(std::string*)stringPtr;
    const char* filter[] = { "*.jpg", "*.png", "*.tga" };
    const char * result = tinyfd_openFileDialog("Select Texture","./",3,filter,0);
    if(result) {
        path = result;
    }
}
static void TW_CALL pickModel(void *stringPtr)
{
    std::string &path = *(std::string*)stringPtr;
    const char* filter[] = { "*.obj" };
    const char * result = tinyfd_openFileDialog("Select Model","./",1,filter,0);
    if(result) {
        path = result;
    }
}

void TW_CALL updateParameters(void *decorPtr)
{
    Decoration &decor = *(Decoration*)decorPtr;
    decor.updateParamsInternal();
}

void Decoration::updateParamsInternal()
{
    setupModel(vShaderPath,
		       fShaderPath,
               modelPath,
               dTexturePath,
               nTexturePath);
}

void Decoration::editorSelect()
{
    Entity::editorSelect();
    TwAddVarRW(entityBar, "ModelPath", TW_TYPE_STDSTRING, &modelPath, "label='Model' group='Filenames'");
    TwAddButton(entityBar, "PickModel", pickModel, &modelPath, "label='    Pick' group='Filenames'");

    TwAddVarRW(entityBar, "DTextPath", TW_TYPE_STDSTRING, &dTexturePath, "label='Diffuse map' group='Filenames'");
    TwAddButton(entityBar, "PickDText", pickTexture, &dTexturePath, "label='    Pick' group='Filenames'");

    TwAddVarRW(entityBar, "NTextPath", TW_TYPE_STDSTRING, &nTexturePath, "label='Normal map' group='Filenames'");
    TwAddButton(entityBar, "PickNText", pickTexture, &nTexturePath, "label='    Pick' group='Filenames'");

    TwAddVarRW(entityBar, "VShaPath", TW_TYPE_STDSTRING, &vShaderPath, "label='Vertex shader' group='Filenames'");
    TwAddButton(entityBar, "PickVSha", pickVShader, &vShaderPath, "label='    Pick' group='Filenames'");

    TwAddVarRW(entityBar, "FShaPath", TW_TYPE_STDSTRING, &fShaderPath, "label='Fragment shader' group='Filenames'");
    TwAddButton(entityBar, "PickFSha", pickFShader, &fShaderPath, "label='    Pick' group='Filenames'");

    TwAddSeparator(entityBar, "sep04", "");

    TwAddButton(entityBar, "UpdateButton", updateParameters, this, "label='Update parameters'");
}

STATE Decoration::main(EntityEvent* ee, Entity* caller)
{
	return;
}
