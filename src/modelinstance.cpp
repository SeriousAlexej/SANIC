#include "modelinstance.h"
#include "modelset.h"
#include "global.h"

ModelInstance::ModelInstance(Mesh* mesh, Shader* shader, Texture* diffuse, Texture* normal, Texture* height)
{
	assert(mesh != NULL && shader != NULL && diffuse != NULL && normal != NULL && height != NULL);

	pMesh = mesh;
	pShader = shader;
	pDiffTexture = diffuse;
	pNormTexture = normal;
	pHeightTexture = height;

	pMesh->doSubscribe();
	pShader->doSubscribe();
	pDiffTexture->doSubscribe();
	pNormTexture->doSubscribe();
	pHeightTexture->doSubscribe();

	pOverridenShadowShader = nullptr;

    normalStrength = 1.0f;
    parallaxScale = 0.03f;
    parallaxOffset = 0.0f;
	active = true;
	visible = false;
	background = false;
	translucent = false;
	useImageAlpha = false;
	useEditorShader = false;
	castShadow = true;
	lastRender = 0.0f;
	stopAnimation();
}

ModelInstance::~ModelInstance()
{
	pMesh->unSubscribe();
	pShader->unSubscribe();
	pDiffTexture->unSubscribe();
	pNormTexture->unSubscribe();

	if (pOverridenShadowShader) {
        pOverridenShadowShader->unSubscribe();
	}
}

void ModelInstance::playAnimation(const std::string &anim)
{
	if(pMesh->hasAnim(anim))
	{
		strCurrAnim = anim;
		animInfo = pMesh->getAnimInfo(anim);//&pMesh->animations[anim];
		uCurrFrame = animInfo.firstFrame;
		uNextFrame = uCurrFrame;
	}
}

void ModelInstance::stopAnimation()
{
    strCurrAnim = "default";
    uCurrFrame = 0u;
    uNextFrame = 0u;
	animInfo.length = 1;
	animInfo.secondsPerFrame = 1.0f;
	animInfo.firstFrame = 0;
}

void ModelInstance::render(Camera& cam, const std::vector<Light*>& lights, Light* dirLight)
{
    if(!active && !egg::getInstance().g_Editor) return;

    float tmNow = egg::getInstance().g_Clock.getElapsedTime().asSeconds();
	float delta = tmNow - lastRender;
	float frameProgress = delta / animInfo.secondsPerFrame;

	if(frameProgress > 1.0f) //this frame is done
	{ //advance frame
		while(delta > animInfo.secondsPerFrame) { uCurrFrame++; delta -= animInfo.secondsPerFrame; }
		unsigned lastFrame = animInfo.firstFrame + animInfo.length - 1;

		if(uCurrFrame < lastFrame)
		{
			uNextFrame = uCurrFrame + 1;
		} else
		if(uCurrFrame == lastFrame)
		{
			uNextFrame = animInfo.firstFrame;
		} else
		if(uCurrFrame > lastFrame)
		{
			uCurrFrame = animInfo.firstFrame;
			uNextFrame = uCurrFrame + 1;
		}
		if(animInfo.length == 1)
			uNextFrame = uCurrFrame;

		lastRender = tmNow;
		while(frameProgress > 1.0f) frameProgress -= 1.0f;
	}

	//now we have uCurrFrame for drawing
	//uNextFrame and  frameProgress for interpolation

    pShader->bind();

    glUniform1i(pShader->getOverlayBoolID(), useEditorShader);

	const glm::mat4& Model = myModelSet->getMatrix() * offsetMatrix * scaMx;// this->getMatrix();
	if(background && !egg::getInstance().g_Editor)
    {
	    glUniformMatrix4fv(pShader->getViewMatrixID(), 1, GL_FALSE, &cam.getBcgMatrix()[0][0]);
    } else {
	    glUniformMatrix4fv(pShader->getViewMatrixID(), 1, GL_FALSE, &cam.getViewMatrix()[0][0]);
    }
	glm::vec3 CamPos = cam.getPosition();

	glUniformMatrix4fv(pShader->getProjMatrixID(), 1, GL_FALSE, &cam.getProjectionMatrix()[0][0]);
	glUniformMatrix4fv(pShader->getModlMatrixID(), 1, GL_FALSE, &Model[0][0]);
	if(!background)
    {
        glUniform3f(pShader->getCameraPositionID(), CamPos.x, CamPos.y, CamPos.z);
    } else {
        glUniform3f(pShader->getCameraPositionID(), 0.0f, 0.0f, 0.0f);
    }

	glActiveTexture(GL_TEXTURE0);
	pDiffTexture->bind();
	glUniform1i(pShader->getDiffuseTextureID(), 0);

	glActiveTexture(GL_TEXTURE1);
	pNormTexture->bind();
	glUniform1i(pShader->getNormalTextureID(), 1);

	glActiveTexture(GL_TEXTURE2);
	pHeightTexture->bind();
	glUniform1i(pShader->getHeightTextureID(), 2);


	static const glm::mat4 biasMatrix(
			0.5, 0.0, 0.0, 0.0,
			0.0, 0.5, 0.0, 0.0,
			0.0, 0.0, 0.5, 0.0,
			0.5, 0.5, 0.5, 1.0
		);

	//shadows
	if(dirLight != nullptr)
    {
        glUniform1f(pShader->getDirShadowBool(), 1.0f);

        glm::mat4 shadowBiasMVP = biasMatrix * (cam.getShadowPMatrix() * cam.getShadowVMatrix() * Model);
        glUniformMatrix4fv(pShader->getBiasMVP(), 1, GL_FALSE, &shadowBiasMVP[0][0]);

        glm::mat4 shadowBiasMVP_LQ = biasMatrix * (cam.getShadowLQPMatrix() * cam.getShadowLQVMatrix() * Model);
        glUniformMatrix4fv(pShader->getBiasMVP_LQ(), 1, GL_FALSE, &shadowBiasMVP_LQ[0][0]);

        /*glm::vec4*/const glm::mat4& lightInvDir = /*-*/dirLight->getMatrix()/*[2]*/;
        glUniform3f(pShader->getDLightDir(), -lightInvDir[2].x, -lightInvDir[2].y, -lightInvDir[2].z);//lightInvDir.x, lightInvDir.y, lightInvDir.z);

        glm::vec3 dLightA = dirLight->getAmbientColor();
        glm::vec3 dLightD = dirLight->getDiffuseColor();
        glUniform3f(pShader->getDLightAmbient(), dLightA.x, dLightA.y, dLightA.z);
        glUniform3f(pShader->getDLightDiffuse(), dLightD.x, dLightD.y, dLightD.z);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, cam.getShadowTexture());
		glUniform1i(pShader->getShadowMap(), 3);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, cam.getShadowLQTexture());
		glUniform1i(pShader->getShadowMap_LQ(), 4);
    } else {
        glUniform1f(pShader->getDirShadowBool(), 0.0f);
    }
    glUniform1f(pShader->getShadowBorder(), cam.getHQShadowBorder());

	//interpolation
	glUniform1f(pShader->getFrameProgressID(), frameProgress);

	glUniform1f(pShader->getNormalStrengthID(), normalStrength);
	glUniform1f(pShader->getParallaxOffsetID(), parallaxOffset);
	glUniform1f(pShader->getParallaxScaleID(), parallaxScale);
	glUniform2f(pShader->getUVTilingD(), UVTilingD.x, UVTilingD.y);
	glUniform2f(pShader->getUVTilingN(), UVTilingN.x, UVTilingN.y);
	glUniform2f(pShader->getUVTilingH(), UVTilingH.x, UVTilingH.y);
	glUniform1f(pShader->getTimeID(), egg::getInstance().g_LastTime);

	if (!translucent) {
	    glUniform1f(pShader->getTranslucencyAlphaID(), 1.0f);
	    glUniform1f(pShader->getUseImageAlphaID(), 1.0f);
	} else {
	    glUniform1f(pShader->getTranslucencyAlphaID(), translucencyAlpha);
	    glUniform1f(pShader->getUseImageAlphaID(), (useImageAlpha ? 0.0f : 1.0f));
	}

    for(int i=0; i<4; i++)
    {
        Light* light = lights[i];
        glm::vec3 lightPos = glm::vec3(0,0,0);
        glm::vec3 lightCD = lightPos;
        float lightFA = 0.0f;
        float lightHS = 0.0f;
        float lightI = 0.0f;

        if(light != nullptr)
        {
            lightPos = light->getPosition();
            lightCD = light->getDiffuseColor();
            lightFA = light->getFallOff();
            lightHS = light->getHotSpot();
            lightI = light->getIntensity();
        }

        glUniform3f(pShader->getLightPositionID(i), lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(pShader->getLightDiffuseID(i), lightCD.x, lightCD.y, lightCD.z);
        glUniform1f(pShader->getLightFallOffID(i), lightFA);
        glUniform1f(pShader->getLightHotSpotID(i), lightHS);
        glUniform1f(pShader->getLightIntensityID(i), lightI);
    }

	pMesh->render(uCurrFrame, uNextFrame);
	visible = false;
}

void ModelInstance::renderForShadow(Camera& cam, Shader* shader)
{
    if(!active && !egg::getInstance().g_Editor) return;

    float tmNow = egg::getInstance().g_Clock.getElapsedTime().asSeconds();
	float delta = tmNow - lastRender;
	float frameProgress = delta / animInfo.secondsPerFrame;

	if(frameProgress > 1.0f) //this frame is done
	{ //advance frame
		while(delta > animInfo.secondsPerFrame) { uCurrFrame++; delta -= animInfo.secondsPerFrame; }
		unsigned lastFrame = animInfo.firstFrame + animInfo.length - 1;

		if(uCurrFrame < lastFrame)
		{
			uNextFrame = uCurrFrame + 1;
		} else
		if(uCurrFrame == lastFrame)
		{
			uNextFrame = animInfo.firstFrame;
		} else
		if(uCurrFrame > lastFrame)
		{
			uCurrFrame = animInfo.firstFrame;
			uNextFrame = uCurrFrame + 1;
		}
		if(animInfo.length == 1)
			uNextFrame = uCurrFrame;

		lastRender = tmNow;
		while(frameProgress > 1.0f) frameProgress -= 1.0f;
	}

	//now we have uCurrFrame for drawing
	//uNextFrame and  frameProgress for interpolation

	if (pOverridenShadowShader) {
        pOverridenShadowShader->bind();
        shader = pOverridenShadowShader;
	}

	const glm::mat4& Model = myModelSet->getMatrix() * offsetMatrix * scaMx;//this->getMatrix();
	//glm::mat4 MVP = cam.getCurrentShadowPVMatrix()*Model;

	glUniformMatrix4fv(shader->getProjMatrixID(), 1, GL_FALSE, &cam.getCurrentShadowPMatrix()[0][0]);
	glUniformMatrix4fv(shader->getViewMatrixID(), 1, GL_FALSE, &cam.getCurrentShadowVMatrix()[0][0]);
	glUniformMatrix4fv(shader->getModlMatrixID(), 1, GL_FALSE, &Model[0][0]);
	glUniform1f(shader->getTimeID(), egg::getInstance().g_LastTime);

	//interpolation
	glUniform1f(shader->getFrameProgressID(), frameProgress);
	glUniform2f(pShader->getUVTilingD(), UVTilingD.x, UVTilingD.y);

	glActiveTexture(GL_TEXTURE0);
	pDiffTexture->bind();
	glUniform1i(shader->getDiffuseTextureID(), 0);

	pMesh->render(uCurrFrame, uNextFrame);
}

glm::vec3 ModelInstance::getPosition() const
{
    return glm::vec3(myModelSet->getMatrix() * glm::vec4(posMx[3].x - offset.x, posMx[3].y - offset.y, posMx[3].z - offset.z, 1.0f));
}

void ModelInstance::setOverridenShadowShader(Shader* s)
{
    pOverridenShadowShader = s;
    if (s) {
        s->doSubscribe();
    }
}

glm::vec4 ModelInstance::getRenSphere() const
{
    glm::vec4 renSphere = myModelSet->getMatrix() * offsetMatrix * scaMx * glm::vec4(pMesh->getBoundingSphereCenter(), 1.0f);
    renSphere /= renSphere.w;
    const glm::vec3 sc1 = myModelSet->getScale();
    const glm::vec3 sc2 = getScale();
    float scaleModifier1 = sc1[0];
    float scaleModifier2 = sc2[0];
    scaleModifier1 = glm::max(scaleModifier1, sc1[1]);
    scaleModifier1 = glm::max(scaleModifier1, sc1[2]);
    scaleModifier2 = glm::max(scaleModifier2, sc2[1]);
    scaleModifier2 = glm::max(scaleModifier2, sc2[2]);
	renSphere.w = pMesh->getBoundingSphereRadius() * scaleModifier1 * scaleModifier2;
	return renSphere;
}
/*
glm::vec4 ModelInstance::getRenSphere()
{
	glm::vec4 renSphere = glm::vec4(pMesh->getBoundingSphereCenter(), pMesh->getBoundingSphereRadius());
	return renSphere;
}

glm::vec3 ModelInstance::getRenBoxCenter()
{
	return pMesh->getBoundingBoxCenter() + getPosition();
}

glm::vec3 ModelInstance::getRenBoxHalfSizes()
{
	return pMesh->getBoundingBoxHalfSizes();
}
*/
