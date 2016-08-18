#include <algorithm>
#include "decoration.h"
#include "entities/events.h"
#include "modelset.h"
#include "global.h"

ADD_TO_INCUBATOR(Decoration);

Decoration::Decoration()
{
    modelPath = "./models/default.mconf";
	hasCollision = true;
	name = "Decoration";

	mdlScale = glm::vec3(1.0f, 1.0f, 1.0f);
	startAnim = "";

	registerProperties("Scale",             &mdlScale,      [this]() { if(modelset) modelset->setScale(mdlScale); },
                       "Start Animation",   &startAnim,     nullptr,
                       "Background",        &background,    [this]() { if(modelset) modelset->setBackground(background); },
                       "Model",             &modelPath,     [this]() { setupModel(); },
                       "EditorOnly",        &editorOnly,    [this]() { editorOnly ? switchToEditorModel() : switchToModel(); },
                       "Collide",           &hasCollision,  [this]() { if(hasCollision) { setupCollision(mass); setPosition(position); setRotation(rotationEuler); }},
                       "Mass",              &mass,          [this]() { if(hasCollision) { setupCollision(mass); setPosition(position); setRotation(rotationEuler); }});
}

Decoration::~Decoration()
{
}

void Decoration::initialize()
{
    Entity::initialize();
    setProperty("EditorOnly", false);
}

void Decoration::setupAI()
{
    Entity::setupAI();

    ON("Main", EBegin)
        modelset->playAnimation(startAnim);
    ENDON

    ON("Main", EActivate)
        setProperty("EditorOnly", false);
    ENDON

    ON("Main", EDeactivate)
        setProperty("EditorOnly", true);
    ENDON
}
