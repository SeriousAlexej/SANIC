#include "world_graphics.h"
#include "global.h"
#include <algorithm>
#include <functional>

WorldGraphics::WorldGraphics()
{
    backgroundModels = 0u;
    dirLightUsers = 0u;
    directionalLight = nullptr;
    shadowShader = std::make_shared<Shader>("./shaders/shadow");
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
}

WorldGraphics::~WorldGraphics()
{
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glDisableVertexAttribArray(5);
	models.clear();
	meshes.clear();
	shaders.clear();
	textures.clear();
	lights.clear();
	modelSets.clear();
    if(directionalLight != nullptr)
    {
        delete directionalLight;
        directionalLight = nullptr;
    }
}

void WorldGraphics::sortForBackground()
{
    std::sort(modelSets.begin(), modelSets.end(), [](std::shared_ptr<ModelSet> &i, std::shared_ptr<ModelSet> &j) { return !j->background && i->background; });
    for(unsigned i=0; i<modelSets.size(); ++i)
    {
        if(!modelSets[i]->background)
        {
            break;
        } else {
            ++backgroundModels;
        }
    }
}

void WorldGraphics::render()
{
	//render visible model instances
	int sz = modelSets.size();

    Light* dirLight = (egg::getInstance().g_UseDirectionalLight?directionalLight:nullptr);

    //render shadows
    if(dirLight != nullptr)
    {
        shadowShader->bind();
        glm::mat4 dlMatrix = dirLight->getMatrix();

        //first pass - low quality shadow
        //second pass - high quality shadow
        for(int p=0; p<2; p++)
        {
            camera.setShadowRenderMode((p==0?Camera::LQ:Camera::HQ));

            camera.setShadowViewMatrix(dlMatrix);
            camera.preShadowRender();
            glClear(GL_DEPTH_BUFFER_BIT);
            for(int i=0; i<sz; i++)
            {
                modelSets[i]->findVisibleLOD();
                modelSets[i]->renderForShadow();
            }
        }

        //cleanup
        camera.postShadowRender();
    }
    if(egg::getInstance().g_Editor)
    {
        glViewport(egg::getInstance().g_DrawOrigin.x, egg::getInstance().g_DrawOrigin.y, egg::getInstance().g_Resolution.x, egg::getInstance().g_Resolution.y);
    }

    glClear( (egg::getInstance().g_Editor?GL_COLOR_BUFFER_BIT:0) | GL_DEPTH_BUFFER_BIT);

    int shaSz = shaders.size();
    for(int shad=0; shad<shaSz; shad++)
    {
        for(int i=0; i<backgroundModels; i++)
        {
            if(shad == 0)
            {
                modelSets[i]->findVisibleLOD();
            }
            modelSets[i]->render(shaders[shad]->getHash());
        }
	}
	if(!egg::getInstance().g_Editor)
    {
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    for(int shad=0; shad<shaSz; shad++)
    {
        for(int i=backgroundModels; i<sz; i++)
        {
            if(shad == 0)
            {
                modelSets[i]->findVisibleLOD();
            }
            modelSets[i]->render(shaders[shad]->getHash());
        }
	}

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0); //unbind, just in case
	Shader::unbind();
	Texture::unbind();
}

ModelSet* WorldGraphics::createModelSet(std::string &modelsetPath)
{
    std::shared_ptr<ModelSet> ms = std::make_shared<ModelSet>(modelsetPath, this);
    modelSets.push_back(ms);
    return ms.get();
}

void WorldGraphics::deleteModelSet(ModelSet*& ms)
{
    if(ms == nullptr) return;
    for(int i=modelSets.size()-1; i>=0; i--)
    {
        if(modelSets[i].get() == ms)
        {
            modelSets.erase(modelSets.begin() + i);
            ms = nullptr;
            return;
        }
    }
}

ModelInstance* WorldGraphics::createModel(
	std::string shaderPath,
	std::string modelPath,
	std::string diffTexture, std::string normTexture, std::string heightTexture)
{
	int meshIndex = -1;
	int shaderIndex = -1;
	int textureDiffuseIndex = -1;
	int textureNormalIndex = -1;
	int textureHeightIndex = -1;

	std::size_t dHash(std::hash<std::string>()(diffTexture)),
                nHash(std::hash<std::string>()(normTexture)),
                hHash(std::hash<std::string>()(heightTexture)),
                mHash(std::hash<std::string>()(modelPath)),
                sHash(std::hash<std::string>()(shaderPath));


	//find shader id
	int sz = shaders.size();
	for(int i=0; i<sz; i++)
	{
		if(shaders[i]->getHash() == sHash)
		{
			shaderIndex = i; //if shader was found - use it
			break;
		}
	}
	if(shaderIndex == -1) //shader not found - create it
	{
		shaderIndex = shaders.size(); //index of new shader will be the last one
		shaders.push_back(std::make_shared<Shader>(shaderPath));
	}
	//find textures id's
	sz = textures.size();
	for(int i=0; i<sz; i++)
	{
	    std::size_t texName = textures[i]->getHash();
		if(texName == dHash)
		{
			textureDiffuseIndex = i;
		}
		if(texName == nHash)
		{
			textureNormalIndex = i;
		}
		if(texName == hHash)
        {
            textureHeightIndex = i;
        }
		if(textureDiffuseIndex != -1 && textureNormalIndex != -1 && textureHeightIndex != -1)
        {
            break;
        }
	}
	if(textureDiffuseIndex == -1)
	{
		textureDiffuseIndex = textures.size();
		textures.push_back(std::make_shared<Texture>(diffTexture));
		if(normTexture == diffTexture)
        {
            textureNormalIndex = textureDiffuseIndex;
        }
        if(heightTexture == diffTexture)
        {
            textureHeightIndex = textureDiffuseIndex;
        }
	}
	if(textureNormalIndex == -1)
	{
		textureNormalIndex = textures.size();
		textures.push_back(std::make_shared<Texture>(normTexture));
		if(normTexture == heightTexture)
        {
            textureHeightIndex = textureNormalIndex;
        }
	}
	if(textureHeightIndex == -1)
    {
        textureHeightIndex = textures.size();
        textures.push_back(std::make_shared<Texture>(heightTexture));
    }
	//find mesh

	sz = meshes.size();
	for(int i=0; i<sz; i++)
	{
		if(meshes[i]->getHash() == mHash)
		{
			meshIndex = i;
			break;
		}
	}
	if(meshIndex == -1)
	{
		meshIndex = meshes.size();
		meshes.push_back(std::make_shared<Mesh>(modelPath));
	}

	//now we're ready to create model instance
	int index = models.size();
	models.push_back(std::make_shared<ModelInstance>
                                  (meshes[meshIndex].get(),
								   shaders[shaderIndex].get(),
								   textures[textureDiffuseIndex].get(),
								   textures[textureNormalIndex].get(),
								   textures[textureHeightIndex].get())
                    );
	return models[index].get();
}

void WorldGraphics::deleteModel(ModelInstance*& mi)
{
	if(mi == nullptr) return;

	int m(-1), s(-1), td(-1), tn(-1), th(-1);
	int sz = models.size();
	for(int i=0; i<sz; i++)
	{
		if(models[i].get()==mi)
		{
			m = models[i]->pMesh->getMultipass();
			s = models[i]->pShader->getMultipass();
			td = models[i]->pDiffTexture->getMultipass();
			tn = models[i]->pNormTexture->getMultipass();
			th = models[i]->pHeightTexture->getMultipass();
			models.erase(models.begin() + i);
			break;
		}
	}

    if(m != -1)
    {
        sz = meshes.size();
        for(int i=0; i<sz; i++)
        {
            if(meshes[i]->getMultipass() == m && !meshes[i]->hasSubscribers())
            {
                meshes.erase(meshes.begin() + i);
                break;
            }
        }
    }
    if(s != -1)
    {
        sz = shaders.size();
        for(int i=0; i<sz; i++)
        {
            if(shaders[i]->getMultipass() == s && !shaders[i]->hasSubscribers())
            {
                shaders.erase(shaders.begin() + i);
                break;
            }
        }
    }
    if(td != -1)
    {
        sz = textures.size();
        for(int i=0; i<sz; i++)
        {
            if(textures[i]->getMultipass() == td && !textures[i]->hasSubscribers())
            {
                textures.erase(textures.begin() + i);
                break;
            }
        }
    }
    if(tn != -1)
    {
        sz = textures.size();
        for(int i=0; i<sz; i++)
        {
            if(textures[i]->getMultipass() == tn && !textures[i]->hasSubscribers())
            {
                textures.erase(textures.begin() + i);
                break;
            }
        }
    }
    if(th != -1)
    {
        sz = textures.size();
        for(int i=0; i<sz; i++)
        {
            if(textures[i]->getMultipass() == th && !textures[i]->hasSubscribers())
            {
                textures.erase(textures.begin() + i);
                break;
            }
        }
    }
	mi = nullptr;
}

Light* WorldGraphics::createLight()
{
	std::shared_ptr<Light> newLight = std::make_shared<Light>();
	lights.push_back(newLight);
	return newLight.get();
}

void WorldGraphics::deleteLight(Light*& light)
{
	if(light != nullptr)
	{
		int sz = lights.size();
		for(int i=0; i<sz; i++)
		{
			if(lights[i].get() == light)
			{
				lights.erase(lights.begin() + i);
				light = nullptr;
				return;
			}
		}
	}
}

Light* WorldGraphics::createDirLight()
{
    ++dirLightUsers;
    if(directionalLight == nullptr)
    {
        directionalLight = new Light();
    }
    return directionalLight;
}

void WorldGraphics::deleteDirLight()
{
    --dirLightUsers;
    if(directionalLight != nullptr && dirLightUsers == 0u)
    {
        delete directionalLight;
        directionalLight = nullptr;
    }
}

std::vector<Light*> WorldGraphics::pickBestLights(glm::vec4 modelRenSphere)
{
    std::vector<Light*> light_chart(4, nullptr);
	int sz = lights.size();
	if(sz < 1) return light_chart;

	std::map<float, Light*> arr;
	std::vector<float> values;
	values.resize(sz);


	glm::vec3 bestPosition = glm::vec3(modelRenSphere);

	for(int i=0; i<sz; i++)
	{
	    values[i] = -1.0f;
        glm::vec3 lpos = lights[i]->getPosition();
        float lfalloff = lights[i]->getFallOff();
		float distToLight = glm::distance(bestPosition, lpos);
		if(distToLight > lfalloff + modelRenSphere.w)
        {// this light can't even reach model's bounding sphere, discard it
            continue;
        }
        if(!camera.sphereIsVisible(glm::vec4(lpos.x, lpos.y, lpos.z, lfalloff)))
        {// this light might reach our model, but it's not visible anyway
            continue;
        }
		if(distToLight == 0.0f) distToLight = 0.001f;
		float v = lfalloff/distToLight;
		values[i] = v;
		arr[v] = lights[i].get();
	}
	std::sort(values.begin(), values.end());
	for(int i=std::min(3, sz-1); i>=0; i--)
    {
        light_chart[i] = arr[values[sz-i-1]];
    }

	return light_chart;
}
