#include "directional_light.h"
#include "../dialogs/tinyfiledialogs.h"
#include "global.h"

ADD_TO_INCUBATOR(DirectionalLight);

static void TW_CALL pickColor(void *colorPtr)
{
    glm::vec3 &col = *static_cast<glm::vec3*>(colorPtr);
    unsigned char startColor[3];
    startColor[0] = col.x*255;
    startColor[1] = col.y*255;
    startColor[2] = col.z*255;
    tinyfd_colorChooser("Pick color", NULL, startColor, startColor);
    col.x = startColor[0]/255.0f;
    col.y = startColor[1]/255.0f;
    col.z = startColor[2]/255.0f;
}

DirectionalLight::DirectionalLight()
{
    setClass("DirectionalLight");
	lightSource = nullptr;
}

DirectionalLight::~DirectionalLight()
{
    assert(wldGFX);
    wldGFX->deleteDirLight();
}

void DirectionalLight::addProperties()
{
    Entity::addProperties();
    registerProperties(
                "Ambient Color",    &(lightSource->colorAmbient),
                "Diffuse Color",    &(lightSource->colorDiffuse)
    );

    addDrawableElements(
        {
            {
                DrawableElement{DrawableElement::PT_COLOR, "Ambient Color", "colormode=rgb "},
                DrawableElement{DrawableElement::PT_BUTTON, "PickAmbient", "label='Pick' ", &lightSource->colorAmbient, pickColor}
            },
            {
                DrawableElement{DrawableElement::PT_COLOR, "Diffuse Color", "colormode=rgb "},
                DrawableElement{DrawableElement::PT_BUTTON, "PickDiffuse", "label='Pick' ", &lightSource->colorDiffuse, pickColor}
            }
        }
    );
    setName("DirectionalLight");
}

void DirectionalLight::initialize()
{
	setupModel("./shaders/sprite.shader",
			   "./models/editor/sprite.obj",
			   "./models/editor/dlight.png",
			   "", "");
    if(egg::getInstance().g_Editor)
    {
	    setupCollision(0.0f, glm::vec3(0.25f, 0.25f, 0.25f));
    }
	switchToEditorModel();
	pushState(main);
	lightSource = wldGFX->createDirLight();

	addProperties();
}


void DirectionalLight::adjustMoving()
{
    if(lightSource && model)
    {
        lightSource->setRotation(model->getRotation());
    }
}


void DirectionalLight::renderSelectionIndicator()
{
    Entity::renderSelectionIndicator();
    if(!lightSource || !model) return;
    glm::vec3 pos = model->getPosition();
    glm::vec4 zAxis = model->getMatrix()[2];
    glPushMatrix();
    glLineWidth(10.0f);
    glTranslatef(pos.x, pos.y, pos.z);
	glBegin(GL_LINES);
		glColor3fv(&lightSource->colorDiffuse[0]);
		glVertex3d(0, 0, 0);
		glColor3fv(&lightSource->colorAmbient[0]);
		glVertex3d(zAxis.x, zAxis.y, zAxis.z);
	glEnd();
    glLineWidth(1.0f);
	glPopMatrix();
}

void DirectionalLight::editorUpdate()
{
    Entity::editorUpdate();
}

void DirectionalLight::editorSelect()
{
    Entity::editorSelect();
}

STATE DirectionalLight::main(EntityEvent *ee, Entity *caller)
{
	return;
}
