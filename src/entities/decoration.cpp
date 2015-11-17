#include <algorithm>
#include "decoration.h"
#include "../dialogs/tinyfiledialogs.h"
#include "global.h"

ADD_TO_INCUBATOR(Decoration);

static std::string relativePath(std::string absPath)
{
    std::replace(absPath.begin(), absPath.end(), '\\', '/');
    if(absPath.find(egg::getInstance().g_WorkingDir)==0)
    {
        absPath = "." + absPath.substr(egg::getInstance().g_WorkingDir.length());
    }
    if(absPath[0] != '.')
    {
        absPath = "." + absPath;
    }
    return absPath;
}

static void TW_CALL pickModel(void *stringPtr)
{
    std::string &path = *static_cast<std::string*>(stringPtr);
    const char* filter[] = { "*.mconf" };
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
	startAnim = "";
	collision = true;
	background = false;
	mass = 0.0f;
	mdlScale = glm::vec3(1.0f, 1.0f, 1.0f);
}

Decoration::~Decoration()
{
}

void Decoration::addProperties()
{
    Entity::addProperties();

    registerProperties(
                "ModelPath",     &modelPath,
                "Background",    &background,
                "Mass",          &mass,
                "ScaleX",        &mdlScale[0],
                "ScaleY",        &mdlScale[1],
                "ScaleZ",        &mdlScale[2],
                "Collide",       &collision,
                "StartAnim",     &startAnim
    );

    addDrawableElements(
        {
            {
                DrawableElement{DrawableElement::PT_BOOL, "Background", ""},
                DrawableElement{DrawableElement::PT_STRING, "ModelPath", "label='Model' "},
                DrawableElement{DrawableElement::PT_BUTTON, "PickModel", "label='    Pick' ", &modelPath, pickModel},
                DrawableElement{DrawableElement::PT_STRING, "StartAnim", "label='Start animation'"},
                DrawableElement{DrawableElement::PT_BUTTON, "ScaleLabel", "label='Scale'", NULL, NULL},
                DrawableElement{DrawableElement::PT_FLOAT, "ScaleX", "label='X' step=0.01"},
                DrawableElement{DrawableElement::PT_FLOAT, "ScaleY", "label='Y' step=0.01"},
                DrawableElement{DrawableElement::PT_FLOAT, "ScaleZ", "label='Z' step=0.01"}
            },
            {
                DrawableElement{DrawableElement::PT_BOOL, "Collide", ""},
                DrawableElement{DrawableElement::PT_BUTTON, "CollParams", "label='Collision params'", NULL, NULL},
                DrawableElement{DrawableElement::PT_FLOAT, "Mass", "step=0.2 min=0"},
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
    glm::vec3 lastPos = modelset->getPosition();
    glm::quat lastRot = modelset->getRotationQuat();
    glm::vec3 lastSca = modelset->getScale();
    glm::vec3 lastOff = modelset->getOffset();
    setupModel(modelPath);
    modelset->setScale(lastSca);
    modelset->setOffset(lastOff);
    if(collision || egg::getInstance().g_Editor)
    {
        setupCollision(mass);
    }
    setRotation(lastRot);
    setPosition(lastPos);

    modelset->setBackground(background);
    modelset->setScale(mdlScale);

    if(editor)
    {
        switchToEditorModel();
    }
}

void Decoration::initialize()
{
    modelPath = "./models/default.mconf";
    setupModel(modelPath);
    if(egg::getInstance().g_Editor)
    {
        setupCollision(0.0f);
    }
    switchToModel();
    //touchable = true;
	pushState(main);
	addProperties();
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


IMPLEMENT_STATE(Decoration, main)

	switchEvent {
        case EventCode_Begin: {
            if(caller->modelset != nullptr) { caller->modelset->playAnimation(caller->startAnim); }
            printf("Starting autowait!\n");
            autowait(3.0f, 1);
        }
        case EventCode_AutowaitCallback: {
            switchAutowait {
                case 1: {
                    printf("Lets wait again!\n");
                    nextwait(2.0f);
                    printf("Wait last time\n");
                    nextwait(1.0f);
                    printf("Oki, ok, stop waiting!\n");
                    return;
                }
            }
        }
        default : return; //this state ignores all other events
	}
END_STATE();
