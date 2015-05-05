#include "modelinstance.h"

ModelInstance::ModelInstance(Mesh* mesh, Shader* shader, Texture* diffuse, Texture* normal)
{
	assert(mesh != NULL && shader != NULL && diffuse != NULL && normal != NULL);

	pMesh = mesh;
	pShader = shader;
	pDiffTexture = diffuse;
	pNormTexture = normal;

	pMesh->doSubscribe();
	pShader->doSubscribe();
	pDiffTexture->doSubscribe();
	pNormTexture->doSubscribe();

	active = true;
	lastRender = 0.0f;
	strCurrAnim = "default";
	uCurrFrame = 0u; //FIX IT
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

void ModelInstance::render(Camera& cam, Light* light)
{
	if(!active && !g_Editor) return;

	float tmNow = g_Clock.getElapsedTime().asSeconds();
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

	glm::mat4 Proj = cam.getProjectionMatrix();
	glm::mat4 View = cam.getViewMatrix();
	glm::mat4 Model = this->getMatrix();
	glm::mat4 MVP = Proj*View*Model;

	glm::mat4 ModelViewMatrix = View * Model;
	glm::mat3 ModelView3x3Matrix = glm::mat3(ModelViewMatrix);

	glUniformMatrix4fv(pShader->getMatrixID(), 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(pShader->getmID(), 1, GL_FALSE, &Model[0][0]);
	glUniformMatrix4fv(pShader->getvID(), 1, GL_FALSE, &View[0][0]);
	glUniformMatrix3fv(pShader->getModelView3x3MatrixID(), 1, GL_FALSE, &ModelView3x3Matrix[0][0]);
	//TODO: add light support

	glActiveTexture(GL_TEXTURE0);
	pDiffTexture->bind();
	glUniform1i(pShader->getDiffuseTextureID(), 0);

	glActiveTexture(GL_TEXTURE1);
	pNormTexture->bind();
	glUniform1i(pShader->getNormalTextureID(), 1);

	//interpolation
	glUniform1f(pShader->getFrameProgressID(), frameProgress);

	glm::vec3 lightPos = glm::vec3(0,0,0);
	glm::vec3 lightCD = lightPos;
	glm::vec3 lightCA = lightPos;
	float lightFA = 1.0f;
	float lightHS = 0.0f;

	if(light != NULL)
	{
		lightPos = light->getPosition();
		lightCD = light->getDiffuseColor();
		lightCA = light->getAmbientColor();
		lightFA = light->getFallOff();
		lightHS = light->getHotSpot();
	}

	glUniform3f(pShader->getLightPositionID(), lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(pShader->getLightDiffuseID(), lightCD.x, lightCD.y, lightCD.z);
	glUniform3f(pShader->getLightAmbientID(), lightCA.x, lightCA.y, lightCA.z);
	glUniform1f(pShader->getLightFallOffID(), lightFA);
	glUniform1f(pShader->getLightHotSpotID(), lightHS);

	pMesh->render(uCurrFrame, uNextFrame);
}

glm::vec4 ModelInstance::getRenSphere()
{
	glm::vec4 renSphere;
	glm::vec3 bSphere = pMesh->getBoundingSphereCenter();
	bSphere += this->getPosition();
	renSphere.x = bSphere.x;
	renSphere.y = bSphere.y;
	renSphere.z = bSphere.z;
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
