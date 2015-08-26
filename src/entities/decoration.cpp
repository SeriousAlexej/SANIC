#include "decoration.h"
#include "../dialogs/tinyfiledialogs.h"

ADD_TO_INCUBATOR(Decoration);

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
    std::string &path = *static_cast<std::string*>(stringPtr);
    const char* filter[] = { "*.shader" };
    const char * result = tinyfd_openFileDialog("Select shader","./",1,filter,0);
    if(result) {
        path = relativePath(result);
    }
}
static void TW_CALL pickTexture(void *stringPtr)
{
    std::string &path = *static_cast<std::string*>(stringPtr);
    const char* filter[] = { "*.jpg", "*.png", "*.tga" };
    const char * result = tinyfd_openFileDialog("Select Texture","./",3,filter,0);
    if(result) {
        path = relativePath(result);
    }
}
static void TW_CALL pickModel(void *stringPtr)
{
    std::string &path = *static_cast<std::string*>(stringPtr);
    const char* filter[] = { "*.obj" };
    const char * result = tinyfd_openFileDialog("Select Model","./",1,filter,0);
    if(result) {
        path = relativePath(result);
    }
}

void TW_CALL updateParameters(void *decorPtr)
{
    Decoration &decor = *static_cast<Decoration*>(decorPtr);
    decor.updateParamsInternal();
}

Decoration::Decoration()
{
	setClass("Decoration");
	collision = true;
	mass = 0.0f;
	radius = 0.5f;
	collBoxSize = glm::vec3(1.0f,1.0f,1.0f);
	mdlScale = glm::vec3(1.0f, 1.0f, 1.0f);
	bodyType = 0;
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
                "HOffset",       &model->parallaxOffset,
                "Mass",            &mass,
                "BodyType",   &bodyType,
                "Radius",         &radius,
                "CBoxX",          &collBoxSize[0],
                "CBoxY",          &collBoxSize[1],
                "CBoxZ",          &collBoxSize[2],
                "COffX",           &collOffset[0],
                "COffY",           &collOffset[1],
                "COffZ",           &collOffset[2],
                "ScaleX",          &mdlScale[0],
                "ScaleY",          &mdlScale[1],
                "ScaleZ",          &mdlScale[2],
                "Collide",         &collision
    );

    addDrawableElements(
        {
            {
                DrawableElement{DrawableElement::PT_STRING, "ModelPath", "label='Model' "},
                DrawableElement{DrawableElement::PT_BUTTON, "PickModel", "label='    Pick' ", &modelPath, pickModel},
                DrawableElement{DrawableElement::PT_BUTTON, "ScaleLabel", "label='Scale'", NULL, NULL},
                DrawableElement{DrawableElement::PT_FLOAT, "ScaleX", "label='X' min=0 step=0.01"},
                DrawableElement{DrawableElement::PT_FLOAT, "ScaleY", "label='Y' min=0 step=0.01"},
                DrawableElement{DrawableElement::PT_FLOAT, "ScaleZ", "label='Z' min=0 step=0.01"}
            },
            {
                DrawableElement{DrawableElement::PT_STRING, "DTextPath", "label='Diffuse map' "},
                DrawableElement{DrawableElement::PT_BUTTON, "PickDText", "label='    Pick' ", &dTexturePath, pickTexture},
                DrawableElement{DrawableElement::PT_STRING, "NTextPath", "label='Normal map' "},
                DrawableElement{DrawableElement::PT_BUTTON, "PickNText", "label='    Pick' ", &nTexturePath, pickTexture},
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
                DrawableElement{DrawableElement::PT_BOOL, "Collide", ""},
                DrawableElement{DrawableElement::PT_ENUM, "BdType", "label='Collision'", &bodyType, NULL, "Box,Sphere,Model"},
                DrawableElement{DrawableElement::PT_BUTTON, "CollParams", "label='Collision params'", NULL, NULL},
                DrawableElement{DrawableElement::PT_FLOAT, "Mass", "step=0.2 min=0"},
                DrawableElement{DrawableElement::PT_FLOAT, "Radius", "min=0 step=0.2"},
                DrawableElement{DrawableElement::PT_FLOAT, "CBoxX", "label='Width' min=0 step=0.2"},
                DrawableElement{DrawableElement::PT_FLOAT, "CBoxY", "label='Height' min=0 step=0.2"},
                DrawableElement{DrawableElement::PT_FLOAT, "CBoxZ", "label='Length' min=0 step=0.2"},
                DrawableElement{DrawableElement::PT_BUTTON, "CollOffset", "label='Collision offset'", NULL, NULL},
                DrawableElement{DrawableElement::PT_FLOAT, "COffX", "label='X' min=0 step=0.2"},
                DrawableElement{DrawableElement::PT_FLOAT, "COffY", "label='Y' min=0 step=0.2"},
                DrawableElement{DrawableElement::PT_FLOAT, "COffZ", "label='Z' min=0 step=0.2"}
            },
            {
                DrawableElement{DrawableElement::PT_BUTTON, "UpdateButton", "label='Update parameters' ", this, updateParameters}
            }
        }
    );

    setName("Decoration");
}

void Decoration::Deserialize(rapidjson::Value& d)
{
    Entity::Deserialize(d);
    reloadRig();
}

void Decoration::reloadRig()
{
    glm::vec3 mutValues(model->normalStrength, model->parallaxOffset, model->parallaxScale);
    glm::vec3 lastPos = model->getPosition();
    glm::quat lastRot = model->getRotationQuat();
    glm::vec3 lastSca = model->getScale();
    glm::vec3 lastOff = model->getOffset();
    setupModel(shaderPath,
               modelPath,
               dTexturePath,
               nTexturePath,
               hTexturePath);
    model->setScale(lastSca);
    model->setOffset(lastOff);
    if(collision || g_Editor)
    {
        switch(bodyType)
        {
            case 2:
            {
                setupCollision(0.0f);
                mass = 0.0f; //somewhy mesh objects fail to collide when mass > 0... F*king bullet
                break;
            }
            case 1:
            {
                setupCollision(mass, radius);
                break;
            }
            case 0:
            default:
            {
                setupCollision(mass, collBoxSize*0.5f);
                break;
            }
        }
        body->setOffset(collOffset);
    }
    setRotation(lastRot);
    setPosition(lastPos);

    model->normalStrength = mutValues.x;
    model->parallaxOffset = mutValues.y;
    model->parallaxScale = mutValues.z;
    model->setScale(mdlScale);
    updateShaderMutators();

    if(editor)
    {
        switchToEditorModel();
    }
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
    if(g_Editor)
    {
        setupCollision(0.0f, glm::vec3(0.5f, 0.5f, 0.5f));
    }
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
    reloadRig();
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
