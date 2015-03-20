#include "pointlight.h"
#include "../dialogs/tinyfiledialogs.h"

PointLight::PointLight()
{
    setClass("PointLight");
    setName("Light");
	setTranslatedByBody(false);
	orientationType = NONE;
	lightSource = NULL;
}

PointLight::~PointLight()
{
    assert(wldGFX);
    wldGFX->deleteLight(lightSource);
}

void PointLight::initialize()
{
	setupModel("./shaders/fullbright.vsh",
		       "./shaders/fullbright.fsh",
			   "./models/editor/light.obj",
			   "./models/editor/yellow.png",
			   "");
	setupCollision(0.0f, glm::vec3(0.25f, 0.25f, 0.25f));
	switchToEditorModel();
	pushState(main);
	lightSource = wldGFX->createLight();
}

void PointLight::adjustMoving()
{
    if(lightSource && body)
    {
        lightSource->setPosition(body->getPosition());
    }
}

void TW_CALL pickColor(void *colorPtr)
{
    glm::vec3 &col = *(glm::vec3*)colorPtr;
    unsigned char startColor[3];
    startColor[0] = col.x*255;
    startColor[1] = col.y*255;
    startColor[2] = col.z*255;
    tinyfd_colorChooser("Pick color", NULL, startColor, startColor);
    col.x = startColor[0]/255.0f;
    col.y = startColor[1]/255.0f;
    col.z = startColor[2]/255.0f;
}

void PointLight::renderSelectionIndicator()
{
    Entity::renderSelectionIndicator();
    if(!lightSource || !model) return;
    glm::vec3 pos = model->getPosition();
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
    TwAddVarRW(entityBar, "Ambient Color", TW_TYPE_COLOR3F, &(lightSource->colorAmbient[0]), "group='Color' colormode=rgb");
    TwAddButton(entityBar, "PickAmbient", pickColor, &lightSource->colorAmbient, "group='Color' label='Pick'");
    TwAddSeparator(entityBar, "sepCol", "group='Color'");
    TwAddVarRW(entityBar, "Diffuse Color", TW_TYPE_COLOR3F, &(lightSource->colorDiffuse[0]), "group='Color' colormode=rgb");
    TwAddButton(entityBar, "PickDiffuse", pickColor, &lightSource->colorDiffuse, "group='Color' label='Pick'");

    TwAddSeparator(entityBar, "sep04", "");

    TwAddVarRW(entityBar, "FallOff", TW_TYPE_FLOAT, &(lightSource->falloff), "label='Fall Off' min=0 step=0.01");
    TwAddVarRW(entityBar, "HotSpot", TW_TYPE_FLOAT, &(lightSource->hotspot), "label='Hot Spot' min=0 step=0.01");
}

STATE PointLight::main(EntityEvent *ee, Entity *caller)
{
	return;
}
