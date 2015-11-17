#include "touch_field.h"
#include "world_graphics.h"

ADD_TO_INCUBATOR(TouchField);

void TW_CALL updateParams(void *tfPtr)
{
    TouchField &tf = *static_cast<TouchField*>(tfPtr);
    tf.updateParams();
}

void TouchField::fillPointers()
{
    pointers.push_back(EntityPointer("Enter Target"));
    pointers.push_back(EntityPointer("Exit Target"));
}

TouchField::TouchField()
{
    fillPointers();
    exitCheckTime = 0.0f;
    active = true;
    enLastIn = nullptr;
}

TouchField::~TouchField()
{
}

void TouchField::renderSelectionIndicator()
{
	glUseProgram(0);
	glm::mat4 proj = wldGFX->getCamera()->getProjectionMatrix();
	glm::mat4 view = wldGFX->getCamera()->getViewMatrix();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMultMatrixf(&proj[0][0]);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultMatrixf(&view[0][0]);
	glPushMatrix();
	glm::vec3 spherePos = body->getPosition();
	spherePos.y += fieldBox[1]*0.5f + 0.25f;
	glTranslatef(spherePos.x, spherePos.y, spherePos.z);
	glRotatef(-90.0f, 1,0,0);
	glColor3f(1.0f, 0.0f, 0.0f);
	glDisable(GL_CULL_FACE);
	GLUquadricObj* quadric = gluNewQuadric();
	gluCylinder(quadric, 0, std::max(fieldBox[0],fieldBox[2])*0.125f, std::max(fieldBox[0],fieldBox[2])*0.25f, 8, 1);
	glTranslatef(0,0,std::max(fieldBox[0],fieldBox[2])*0.25f);
	gluCylinder(quadric, std::max(fieldBox[0],fieldBox[2])*0.04f, std::max(fieldBox[0],fieldBox[2])*0.04f, std::max(fieldBox[0],fieldBox[2])*0.25f, 8, 1);
	gluDeleteQuadric(quadric);
	glEnable(GL_CULL_FACE);
	glPopMatrix();
}

void TouchField::Deserialize(rapidjson::Value& d)
{
    Entity::Deserialize(d);
    updateParams();
}

void TouchField::updateParams()
{
    glm::vec3 lastPos = body->getPosition();
    glm::quat lastRot = body->getRotationQuat();
    glm::vec3 fb = fieldBox*0.5f;
    setupCollision(0.0f, fb);
    body->setPassable(true);
    setRotation(lastRot);
    setPosition(lastPos);
}

void TouchField::initialize()
{
    addProperties();
    fieldBox = glm::vec3(1.0f, 2.0f, 1.0f);
    glm::vec3 fb = fieldBox*0.5f;
    setupCollision(0.0f, fb);
    body->setPassable(true);
    touchable = true;
    pushState(main);
}

void TouchField::addProperties()
{
    Entity::addProperties();

    registerProperties("FieldW", &fieldBox[0],
                       "FieldH", &fieldBox[1],
                       "FieldL", &fieldBox[2],
                       "Active", &active,
                       "ExitCheckTime", &exitCheckTime);

    addDrawableElements(
        {
            {
                DrawableElement{DrawableElement::PT_FLOAT, "FieldW", "label='Width' min=0 step=0.01"},
                DrawableElement{DrawableElement::PT_FLOAT, "FieldL", "label='Length' min=0 step=0.01"},
                DrawableElement{DrawableElement::PT_FLOAT, "FieldH", "label='Height' min=0 step=0.01"}
            },
            {
                DrawableElement{DrawableElement::PT_BOOL, "Active", ""},
                DrawableElement{DrawableElement::PT_FLOAT, "ExitCheckTime", "label='Exit check time' min=0"}
            },
            {
                DrawableElement{DrawableElement::PT_BUTTON, "UpdateBtn", "label='Update'", this, ::updateParams}
            }
        }
    );

    setName("TouchField");
}
