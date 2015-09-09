#include "modelinstance.h"
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

    normalStrength = 1.0f;
    parallaxScale = 0.03f;
    parallaxOffset = 0.0f;
	active = true;
	background = false;
	lastRender = 0.0f;
	strCurrAnim = "default";
	uCurrFrame = 0u;
	uNextFrame = 0u;
	animInfo.length = 1;
	animInfo.secondsPerFrame = 1.0f;
	animInfo.firstFrame = 0;
}

ModelInstance::~ModelInstance()
{
	pMesh->unSubscribe();
	pShader->unSubscribe();
	pDiffTexture->unSubscribe();
	pNormTexture->unSubscribe();
}

void ModelInstance::playAnimation(std::string anim)
{
	if(pMesh->hasAnim(anim))
	{
		strCurrAnim = anim;
		animInfo = pMesh->getAnimInfo(anim);//&pMesh->animations[anim];
		uCurrFrame = animInfo.firstFrame;
		uNextFrame = uCurrFrame;
	} else {
		printf("WARNING: Model \"%s\" does not contain animation \"%s\"!\n",
			pMesh->getSrcFnm().c_str(), anim.c_str());
	}
}

void ModelInstance::render(Camera& cam, std::vector<Light*> lights, Light* dirLight)
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

	glm::mat4 Model = this->getMatrix();
	glm::mat4 MVP;
	if(background && !egg::getInstance().g_Editor)
    {
        MVP = cam.getProjectionMatrix()*cam.getBcgMatrix()*Model;
    } else {
        MVP = cam.getProjectionMatrix()*cam.getViewMatrix()*Model;
    }
	glm::vec3 CamPos = cam.getPosition();

	glUniformMatrix4fv(pShader->getMatrixID(), 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(pShader->getmID(), 1, GL_FALSE, &Model[0][0]);
	glUniform3f(pShader->getCameraPositionID(), CamPos.x, CamPos.y, CamPos.z);

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

        glm::mat4 shadowBiasMVP = biasMatrix * (cam.getShadowPVMatrix() * Model);
        glUniformMatrix4fv(pShader->getBiasMVP(), 1, GL_FALSE, &shadowBiasMVP[0][0]);

        glm::mat4 shadowBiasMVP_LQ = biasMatrix * (cam.getShadowLQPVMatrix() * Model);
        glUniformMatrix4fv(pShader->getBiasMVP_LQ(), 1, GL_FALSE, &shadowBiasMVP_LQ[0][0]);

        glm::vec4 lightInvDir = -dirLight->getMatrix()[2];
        glUniform3f(pShader->getDLightDir(), lightInvDir.x, lightInvDir.y, lightInvDir.z);

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

    for(int i=0; i<4; i++)
    {
        Light*& light = lights[i];
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


	glm::mat4 Model = this->getMatrix();
	glm::mat4 MVP = cam.getCurrentShadowPVMatrix()*Model; ///projection matrix from camera, viewmatrix from camera AND light

	glUniformMatrix4fv(shader->getMatrixID(), 1, GL_FALSE, &MVP[0][0]); /// shadow mvp here

	//interpolation
	glUniform1f(shader->getFrameProgressID(), frameProgress);

	glActiveTexture(GL_TEXTURE0);
	pDiffTexture->bind();
	glUniform1i(shader->getDiffuseTextureID(), 0);

	pMesh->render(uCurrFrame, uNextFrame);
}

glm::vec4 ModelInstance::getRenSphere()
{
	glm::vec4 renSphere;
	glm::vec3 bSphere = pMesh->getBoundingSphereCenter();
	glm::vec4 rotated = getRotation()*glm::vec4(bSphere.x, bSphere.y, bSphere.z, 1.0f);
	bSphere = getPosition();
	renSphere.x = bSphere.x + rotated.x;
	renSphere.y = bSphere.y + rotated.y;
	renSphere.z = bSphere.z + rotated.z;
	renSphere.w = pMesh->getBoundingSphereRadius();
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
