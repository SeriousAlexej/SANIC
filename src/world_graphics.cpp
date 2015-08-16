#include "world_graphics.h"

extern bool g_UseDirectionalLight;

WorldGraphics::WorldGraphics()
{
    directionalLight = nullptr;
    shadowShader = new Shader("./shaders/shadow");
}

WorldGraphics::~WorldGraphics()
{
	for(int i=models.size()-1; i>=0; i--)
		delete models[i];
	for(int i=meshes.size()-1; i>=0; i--)
		delete meshes[i];
	for(int i=shaders.size()-1; i>=0; i--)
		delete shaders[i];
	for(int i=textures.size()-1; i>=0; i--)
		delete textures[i];
	for(int i=lights.size()-1; i>=0; i--)
		delete lights[i];
	models.clear();
	meshes.clear();
	shaders.clear();
	textures.clear();
	lights.clear();
    delete shadowShader;
}

void WorldGraphics::render()
{
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);

	//render visible model instances
	int sz = models.size();

    Light* dirLight = (g_UseDirectionalLight?directionalLight:nullptr);

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
                if(camera.sphereIsVisibleForShadow(models[i]->getRenSphere()))
                {
                    models[i]->renderForShadow(camera, shadowShader);
                }
            }
        }

        //cleanup
        camera.postShadowRender();
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for(int i=0; i<sz; i++)
	{
        if(camera.sphereIsVisible(models[i]->getRenSphere()))
		{
            models[i]->render(camera, pickBestLights(models[i]), dirLight);
		}
	}

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glDisableVertexAttribArray(5);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0); //unbind, just in case
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


	//find shader id
	#ifdef SANIC_DEBUG
	printf("Wanted shader is \"%s\", current shader count is %lu\n", shaderPath.c_str(), shaders.size());
	#endif // SANIC_DEBUG
	int sz = shaders.size();
	for(int i=0; i<sz; i++)
	{
        #ifdef SANIC_DEBUG
        printf("\tFound shader \"%s\" ", shaders[i]->getSrcFnm().c_str());
        #endif // SANIC_DEBUG
		if(shaders[i]->getSrcFnm() == shaderPath)
		{
            #ifdef SANIC_DEBUG
            printf("Shader matches!\n");
            #endif // SANIC_DEBUG
			shaderIndex = i; //if shader was found - use it
			break;
		}
        #ifdef SANIC_DEBUG
        printf("\n");
        #endif // SANIC_DEBUG
	}
	if(shaderIndex == -1) //shader not found - create it
	{
        #ifdef SANIC_DEBUG
        printf("\t\tCreating NEW shader!\n");
        #endif // SANIC_DEBUG
		shaderIndex = shaders.size(); //index of new shader will be the last one
		shaders.push_back(new Shader(shaderPath));
	}
	#ifdef SANIC_DEBUG
	printf("\tNow shader count is %lu\n\n", shaders.size());
	#endif // SANIC_DEBUG
	//find textures id's
	#ifdef SANIC_DEBUG
	printf("Wanted textures are:\n\t\"%s\"\n\t\"%s\"\n\t\"%s\"\ncurrent texture count is %lu\n", diffTexture.c_str(), normTexture.c_str(), heightTexture.c_str(), textures.size());
	#endif // SANIC_DEBUG
	sz = textures.size();
	for(int i=0; i<sz; i++)
	{
	    std::string texName = textures[i]->getSrcFnm();
        #ifdef SANIC_DEBUG
	    printf("\tFound texture \"%s\"", texName.c_str());
        #endif // SANIC_DEBUG
		if(texName == diffTexture)
		{
            #ifdef SANIC_DEBUG
            printf(" Diffuse texture matches!");
            #endif // SANIC_DEBUG
			textureDiffuseIndex = i;
		}
		if(texName == normTexture)
		{
            #ifdef SANIC_DEBUG
            printf(" Normal texture matches!");
            #endif // SANIC_DEBUG
			textureNormalIndex = i;
		}
		if(texName == heightTexture)
        {
            #ifdef SANIC_DEBUG
            printf(" Displacement texture matches!");
            #endif // SANIC_DEBUG
            textureHeightIndex = i;
        }
        #ifdef SANIC_DEBUG
        printf("\n");
        #endif // SANIC_DEBUG
		if(textureDiffuseIndex != -1 && textureNormalIndex != -1 && textureHeightIndex != -1)
        {
            break;
        }
	}
	if(textureDiffuseIndex == -1)
	{
        #ifdef SANIC_DEBUG
        printf("\t\tCreating NEW diffuse texture!\n");
        #endif // SANIC_DEBUG
		textureDiffuseIndex = textures.size();
		textures.push_back(new Texture(diffTexture));
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
        #ifdef SANIC_DEBUG
        printf("\t\tCreating NEW normal texture!\n");
        #endif // SANIC_DEBUG
		textureNormalIndex = textures.size();
		textures.push_back(new Texture(normTexture));
		if(normTexture == heightTexture)
        {
            textureHeightIndex = textureNormalIndex;
        }
	}
	if(textureHeightIndex == -1)
    {
        #ifdef SANIC_DEBUG
        printf("\t\tCreating NEW displacement texture!\n");
        #endif // SANIC_DEBUG
        textureHeightIndex = textures.size();
        textures.push_back(new Texture(heightTexture));
    }
	#ifdef SANIC_DEBUG
	printf("\tNow texture count is %lu\n\n", textures.size());
	#endif // SANIC_DEBUG
	//find mesh

	#ifdef SANIC_DEBUG
	printf("Wanted mesh is \"%s\", current mesh count is %lu\n", modelPath.c_str(), meshes.size());
	#endif // SANIC_DEBUG
	sz = meshes.size();
	for(int i=0; i<sz; i++)
	{
        #ifdef SANIC_DEBUG
	    printf("\tFound mesh \"%s\" ", meshes[i]->getSrcFnm().c_str());
        #endif // SANIC_DEBUG
		if(meshes[i]->getSrcFnm() == modelPath)
		{
            #ifdef SANIC_DEBUG
		    printf("Mesh matches!\n");
            #endif // SANIC_DEBUG
			meshIndex = i;
			break;
		}
        #ifdef SANIC_DEBUG
		printf("\n");
        #endif // SANIC_DEBUG
	}
	if(meshIndex == -1)
	{
        #ifdef SANIC_DEBUG
	    printf("\t\tCreating NEW mesh!\n");
        #endif // SANIC_DEBUG
		meshIndex = meshes.size();
		meshes.push_back(new Mesh(modelPath));
	}
	#ifdef SANIC_DEBUG
	printf("\tNow mesh count is %lu\n\n", meshes.size());
	#endif // SANIC_DEBUG

	//now we're ready to create model instance
	int index = models.size();
	models.push_back(new ModelInstance(meshes[meshIndex],
								   shaders[shaderIndex],
								   textures[textureDiffuseIndex],
								   textures[textureNormalIndex],
								   textures[textureHeightIndex]) );
	return models[index];
}

void WorldGraphics::deleteModel(ModelInstance*& mi)
{
	if(mi == NULL) return;

	int m = -1, s, td, tn, th;
	int sz = models.size();
	for(int i=0; i<sz; i++)
	{
		if(models[i]==mi)
		{
			m = models[i]->pMesh->getMultipass();
			s = models[i]->pShader->getMultipass();
			td = models[i]->pDiffTexture->getMultipass();
			tn = models[i]->pNormTexture->getMultipass();
			th = models[i]->pHeightTexture->getMultipass();
			delete models[i];
			models.erase(models.begin() + i);
			break;
		}
	}
	if(m == -1) return;//dat mesh doesn't exist

	sz = meshes.size();
	for(int i=0; i<sz; i++)
	{
		if(meshes[i]->getMultipass() == m && !meshes[i]->hasSubscribers())
		{
			delete meshes[i];
			meshes.erase(meshes.begin() + i);
			break;
		}
	}
	sz = shaders.size();
	for(int i=0; i<sz; i++)
	{
		if(shaders[i]->getMultipass() == s && !shaders[i]->hasSubscribers())
		{
			delete shaders[i];
			shaders.erase(shaders.begin() + i);
			break;
		}
	}
	sz = textures.size();
	for(int i=0; i<sz; i++)
	{
		if(textures[i]->getMultipass() == td && !textures[i]->hasSubscribers())
		{
			delete textures[i];
			textures.erase(textures.begin() + i);
			break;
		}
	}
	sz = textures.size();
	for(int i=0; i<sz; i++)
	{
		if(textures[i]->getMultipass() == tn && !textures[i]->hasSubscribers())
		{
			delete textures[i];
			textures.erase(textures.begin() + i);
			break;
		}
	}
	sz = textures.size();
	for(int i=0; i<sz; i++)
	{
		if(textures[i]->getMultipass() == th && !textures[i]->hasSubscribers())
		{
			delete textures[i];
			textures.erase(textures.begin() + i);
			break;
		}
	}
	mi = NULL;
}

Light* WorldGraphics::createLight()
{
	Light* newLight = new Light();
	lights.push_back(newLight);
	return newLight;
}

void WorldGraphics::deleteLight(Light*& light)
{
	if(light != NULL)
	{
		int sz = lights.size();
		for(int i=0; i<sz; i++)
		{
			if(lights[i] == light)
			{
				delete light;
				lights.erase(lights.begin() + i);
				light = NULL;
				return;
			}
		}
	}
}

Light* WorldGraphics::createDirLight()
{
    if(directionalLight == nullptr)
    {
        directionalLight = new Light();
    }
    return directionalLight;
}

void WorldGraphics::deleteDirLight()
{
    if(directionalLight != nullptr)
    {
        delete directionalLight;
        directionalLight = nullptr;
    }
}

std::vector<Light*> WorldGraphics::pickBestLights(ModelInstance* mi)
{
    std::vector<Light*> light_chart = {NULL, NULL, NULL, NULL};
	int sz = lights.size();
	if(mi == NULL || sz < 1) return light_chart;

	std::map<float, Light*> arr;
	std::vector<float> values;
	values.resize(sz);

    glm::vec4 modelRenSphere = mi->getRenSphere();
	glm::vec3 bestPosition = glm::vec3(modelRenSphere);

	for(int i=0; i<sz; i++)
	{
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
		float v = distToLight/lfalloff;
		values[i] = v;
		arr[v] = lights[i];
	}
	std::sort(values.begin(), values.end());
	for(int i=std::min(4, sz); i>=0; i--)
    {
        light_chart[i] = arr[values[i]];
    }

	return light_chart;
}
