#include "pointlight.h"
#include "../dialogs/tinyfiledialogs.h"
#include "global.h"
#include "world_graphics.h"

ADD_TO_INCUBATOR(PointLight);

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

PointLight::PointLight()
{
	lightSource = nullptr;
}

PointLight::~PointLight()
{
    assert(wldGFX);
    wldGFX->deleteLight(lightSource);
}

void PointLight::addProperties()
{
    Entity::addProperties();
    registerProperties(
                "Diffuse Color",    &(lightSource->colorDiffuse),
                "FallOff",          &(lightSource->falloff),
                "HotSpot",          &(lightSource->hotspot),
                "Intensity",        &(lightSource->intensity)
    );

    addDrawableElements(
        {
            {
                DrawableElement{DrawableElement::PT_COLOR, "Diffuse Color", "colormode=rgb "},
                DrawableElement{DrawableElement::PT_BUTTON, "PickDiffuse", "label='Pick' ", &lightSource->colorDiffuse, pickColor}
            },
            {
                DrawableElement{DrawableElement::PT_FLOAT, "FallOff", "label='Fall Off' min=0 step=0.01 "},
                DrawableElement{DrawableElement::PT_FLOAT, "HotSpot", "label='Hot Spot' min=0 step=0.01 "},
                DrawableElement{DrawableElement::PT_FLOAT, "Intensity", "label='Intensity' min=0 step=0.001 "}
            }
        }
    );
    setName("Light");
}

void PointLight::initialize()
{
	setupModel("./models/editor/pointLight.mconf");
    if(egg::getInstance().g_Editor)
    {
	    setupCollision(0.0f);
    }
	switchToEditorModel();
	pushState(main);
	lightSource = wldGFX->createLight();

	addProperties();
}

void PointLight::adjustMoving()
{
    if(lightSource && modelset)
    {
        lightSource->setPosition(modelset->getPosition());
    }
}

void PointLight::renderSelectionIndicator()
{
    Entity::renderSelectionIndicator();
    if(!lightSource || !modelset) return;
    glm::vec3 pos = modelset->getPosition();
    glTranslatef(pos.x, pos.y, pos.z);
	glDisable(GL_CULL_FACE);
	glBlendFunc(GL_ONE, GL_SRC_ALPHA);
	glEnable(GL_BLEND);
    GLUquadricObj* q = gluNewQuadric();
    glColor4f(0,0,0.125f,0.99f);
    gluSphere(q, lightSource->hotspot, 16, 16);
    glColor4f(0,0.125f,0,0.99f);
    gluSphere(q, lightSource->falloff, 16, 16);
    gluDeleteQuadric(q);
    glColor4f(1,1,1,1);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void PointLight::editorUpdate()
{
    if(lightSource)
    {
        lightSource->setHotSpot(lightSource->hotspot);
    }
    Entity::editorUpdate();
}

void PointLight::editorSelect()
{
    Entity::editorSelect();
}

void PointLight::main(EntityEvent *ee, Entity *caller)
{
	return;
}
