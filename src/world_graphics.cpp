#include <algorithm>
#include <functional>
#include <glm/gtx/norm.hpp>
#include "world_graphics.h"
#include "render/sector.h"
#include "global.h"

WorldGraphics::WorldGraphics()
{
    backgroundModels = 0u;
    dirLightUsers = 0u;
    directionalLight = nullptr;
    camera.pGfx = static_cast<void*>(this);
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
    std::sort(models.begin(), models.end(), [this](std::shared_ptr<ModelInstance> &m1, std::shared_ptr<ModelInstance> &m2)
            {
                if(!m2->background && m1->background) {
                    return true;
                }
                if(!m1->background && m2->background) {
                    return false;
                }
                if(m1->background && m2->background) {
                    if(!m1->isTranslucent() && m2->isTranslucent()) {
                        return true;
                    }
                    if(!m2->isTranslucent() && m1->isTranslucent()) {
                        return false;
                    }
                    if(m1->isTranslucent() && m2->isTranslucent()) {
                        return glm::distance2(m1->getPosition(), camera.getPosition())
                               >
                               glm::distance2(m2->getPosition(), camera.getPosition());
                    }
                    return m1->pShader->getHash() > m2->pShader->getHash();
                }
                return false;
            });
    for(unsigned i=0; i<models.size(); ++i)
    {
        if(!models[i]->background)
        {
            break;
        } else {
            ++backgroundModels;
        }
    }
}

void WorldGraphics::findVisibleStuff(std::vector<ModelInstance*>& visibleModels, std::vector<Light*>& visibleLights)
{
    Sector* s = camera.getSector();

    if(s == nullptr) {
        for(auto &mi : models) {
            visibleModels.push_back(mi.get());
        }
        for(auto &li : lights) {
            visibleLights.push_back(li.get());
        }
        return;
    }

    std::vector<const Sector*> visibleSectors = { s };
    std::vector<const Sector*> firstSectors = { s };
    std::vector<const Sector*> nextSectors = {};

    while(!firstSectors.empty()) {
        for(auto& sector : firstSectors) {
            for(auto& portal : sector->getPortals()) {
                if(camera.aabboxIsVisible(portal.getBBox())
                   && std::find(visibleSectors.begin(), visibleSectors.end(), portal.otherSector(sector)) == visibleSectors.end()) {
                    nextSectors.push_back(portal.otherSector(sector));
                }
            }
        }
        visibleSectors.insert(visibleSectors.end(), nextSectors.begin(), nextSectors.end());
        firstSectors = std::move(nextSectors);
        nextSectors.clear();
    }

    for(auto& sector : visibleSectors) {
        visibleModels.insert(visibleModels.end(), sector->getModels().begin(), sector->getModels().end());
        visibleLights.insert(visibleLights.end(), sector->getLights().begin(), sector->getLights().end());
    }
}

void WorldGraphics::findVisibleStuffForShadow(std::vector<ModelInstance*>& visibleModels)
{
    Sector* s = camera.getSector();

    if(s == nullptr) {
        for(auto &mi : models) {
            visibleModels.push_back(mi.get());
        }
        return;
    }

    std::vector<const Sector*> visibleSectors = { s };
    std::vector<const Sector*> firstSectors = { s };
    std::vector<const Sector*> nextSectors = {};

    while(!firstSectors.empty()) {
        for(auto& sector : firstSectors) {
            for(auto& portal : sector->getPortals()) {
                if(camera.aabboxIsVisibleForShadow(portal.getBBox())
                   && std::find(visibleSectors.begin(), visibleSectors.end(), portal.otherSector(sector)) == visibleSectors.end()) {
                    nextSectors.push_back(portal.otherSector(sector));
                }
            }
        }
        visibleSectors.insert(visibleSectors.end(), nextSectors.begin(), nextSectors.end());
        firstSectors = std::move(nextSectors);
        nextSectors.clear();
    }

    for(auto& sector : visibleSectors) {
        visibleModels.insert(visibleModels.end(), sector->getModels().begin(), sector->getModels().end());
    }
}

void WorldGraphics::render()
{
    for(auto& ms : modelSets) {
        ms->findVisibleLOD();
    }

    Light* dirLight = (egg::getInstance().g_UseDirectionalLight?directionalLight:nullptr);

    /************************ RENDER SHADOW ***************************/
    if(dirLight) {
        std::vector<ModelInstance*> modelsForShadow = {};
        findVisibleStuffForShadow(modelsForShadow);

        shadowShader->bind();
        const glm::mat4& dlMatrix = dirLight->getMatrix();

        bool shaderChanged = false;
        //first pass - low quality shadow
        //second pass - high quality shadow
        for(int p=0; p<2; p++)
        {
            camera.setShadowRenderMode((p==0?Camera::LQ:Camera::HQ));

            camera.setShadowViewMatrix(dlMatrix);
            camera.preShadowRender();
            glClear(GL_DEPTH_BUFFER_BIT);

            for(auto& mi : modelsForShadow) {
                if(!mi->background
                 && mi->visible
                 && mi->castShadow//!mi->isTranslucent()
                 && camera.sphereIsVisibleForShadow(mi->getRenSphere()) )
                {
                    if (shaderChanged && !mi->overridesShadowShader()) {
                        shadowShader->bind();
                    }
                    mi->renderForShadow(camera, shadowShader.get());
                    shaderChanged = mi->overridesShadowShader();
                }
            }
        }

        camera.postShadowRender();
    }

    if(egg::getInstance().g_Editor)
    {
        glViewport(egg::getInstance().g_DrawOrigin.x, egg::getInstance().g_DrawOrigin.y, egg::getInstance().g_Resolution.x, egg::getInstance().g_Resolution.y);
    }
    glClear( (egg::getInstance().g_Editor?GL_COLOR_BUFFER_BIT:0) | GL_DEPTH_BUFFER_BIT);

    /************************ RENDER BACKGROUND ***************************/
    static const std::vector<Light*> dummy(4, nullptr);
    for(int i=0; i<backgroundModels; i++)
    {
        if(!egg::getInstance().g_Editor) {
            models[i]->render(camera, dummy, dirLight);
        }
    }
	if(!egg::getInstance().g_Editor)
    {
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    /************************ RENDER VISIBLE MODELS ***************************/
    std::vector<ModelInstance*> visibleModels = {};
    std::vector<Light*> visibleLights = {};
    findVisibleStuff(visibleModels, visibleLights);

    std::sort(visibleModels.begin(), visibleModels.end(), [this](const ModelInstance* m1, const ModelInstance* m2)
        {
            if(!m1->isTranslucent() && m2->isTranslucent()) {
                return true;
            }
            if(!m2->isTranslucent() && m1->isTranslucent()) {
                return false;
            }
            if(m1->isTranslucent() && m2->isTranslucent()) {
                return glm::distance2(m1->getPosition(), camera.getPosition())
                       >
                       glm::distance2(m2->getPosition(), camera.getPosition());
            }
            return m1->pShader->getHash() > m2->pShader->getHash();
        });

    for(auto& mi : visibleModels) {
        const glm::vec4 renSphere = mi->getRenSphere();
        if((!mi->background || egg::getInstance().g_Editor) && mi->visible && camera.sphereIsVisible(renSphere)) {
            mi->render(camera, pickBestLights(visibleLights, renSphere), dirLight);
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

Shader* WorldGraphics::createShader(const std::string& shaderPath)
{
	int shaderIndex = -1;
	std::size_t sHash(std::hash<std::string>()(shaderPath));

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

	return shaders[shaderIndex].get();
}

void WorldGraphics::deleteShader(Shader*& s)
{
	if(s == nullptr) return;

	int sID = s->getMultipass();
	int sz = shaders.size();
    for(int i=0; i<sz; i++)
    {
        if(shaders[i]->getMultipass() == sID && !shaders[i]->hasSubscribers())
        {
            shaders.erase(shaders.begin() + i);
            break;
        }
    }
	s = nullptr;
}

ModelInstance* WorldGraphics::createModel(
	const std::string& shaderPath,
	const std::string& modelPath,
	const std::string& diffTexture,
	const std::string& normTexture,
	const std::string& heightTexture)
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

	Shader* pShaderOverr = mi->pOverridenShadowShader;
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

	deleteShader(pShaderOverr);

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

std::vector<Light*> WorldGraphics::pickBestLights(const std::vector<Light*>& pool, const glm::vec4& modelRenSphere)
{
    std::vector<Light*> light_chart(4, nullptr);
	int sz = pool.size();
	if(sz < 1) return light_chart;

	std::map<float, Light*> arr;
	std::vector<float> values;
	values.resize(sz);


	glm::vec3 bestPosition = glm::vec3(modelRenSphere);

	for(int i=0; i<sz; i++)
	{
	    values[i] = -1.0f;
        glm::vec3 lpos = pool[i]->getPosition();
        float lfalloff = pool[i]->getFallOff();
		float distToLight = glm::distance2(bestPosition, lpos);
		const float maxPossibleDistance = lfalloff + modelRenSphere.w;
		if(distToLight > maxPossibleDistance * maxPossibleDistance)
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
		arr[v] = pool[i];
	}
	std::sort(values.begin(), values.end());
	for(int i=std::min(3, sz-1); i>=0; i--)
    {
        light_chart[i] = arr[values[sz-i-1]];
    }

	return light_chart;
}
