#include "world_graphics.h"

WorldGraphics::WorldGraphics()
{
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
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
}

void WorldGraphics::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//render visible model instances
	int sz = models.size();
	for(int i=0; i<sz; i++)
	{
		if(camera.sphereIsVisible(models[i]->getRenSphere()))
		//if(camera.boxIsVisible(models[i]->getRenBoxCenter(), models[i]->getRenBoxHalfSizes()))
		{
			models[i]->render(camera, pickBestLight(models[i]));
		}
	}
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0); //unbind, just in case
}

ModelInstance* WorldGraphics::createModel(
	std::string vertShader, std::string fragShader,
	std::string modelPath,
	std::string diffTexture, std::string normTexture)
{
	int meshIndex = -1;
	int shaderIndex = -1;
	int textureDiffuseIndex = -1;
	int textureNormalIndex = -1;



	//find shader id
	int sz = shaders.size();
	for(int i=0; i<sz; i++)
	{
		if(shaders[i]->getSrcVxFnm() == vertShader &&
		   shaders[i]->getSrcFgFnm() == fragShader)
		{
			shaderIndex = i; //if shader was found - use it
			break;
		}
	}
	if(shaderIndex == -1) //shader not found - create it
	{
		shaderIndex = shaders.size(); //index of new shader will be the last one
		shaders.push_back(new Shader(vertShader, fragShader));
	}
	//find textures id's
	sz = textures.size();
	for(int i=0; i<sz; i++)
	{
		if(textures[i]->getSrcFnm() == diffTexture)
		{
			textureDiffuseIndex = i;
		}
		if(textures[i]->getSrcFnm() == normTexture)
		{
			textureNormalIndex = i;
		}
		if(textureDiffuseIndex != -1 && textureNormalIndex != -1) break;
	}
	if(textureDiffuseIndex == -1)
	{
		textureDiffuseIndex = textures.size();
		textures.push_back(new Texture(diffTexture));
	}
	if(textureNormalIndex == -1)
	{
		textureNormalIndex = textures.size();
		textures.push_back(new Texture(normTexture));
	}
	//find mesh
	sz = meshes.size();
	for(int i=0; i<sz; i++)
	{
		if(meshes[i]->getSrcFnm() == modelPath)
		{
			meshIndex = i;
			break;
		}
	}
	if(meshIndex == -1)
	{
		meshIndex = meshes.size();
		meshes.push_back(new Mesh(modelPath));
	}

	//now we're ready to create model instance
	int index = models.size();
	models.push_back(new ModelInstance(meshes[meshIndex],
								   shaders[shaderIndex],
								   textures[textureDiffuseIndex],
								   textures[textureNormalIndex]) );
	return models[index];
}

void WorldGraphics::deleteModel(ModelInstance*& mi)
{
	if(mi == NULL) return;

	int m = -1, s, td, tn;
	int sz = models.size();
	for(int i=0; i<sz; i++)
	{
		if(models[i]==mi)
		{
			m = models[i]->pMesh->getMultipass();
			s = models[i]->pShader->getMultipass();
			td = models[i]->pDiffTexture->getMultipass();
			tn = models[i]->pNormTexture->getMultipass();
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

Light* WorldGraphics::pickBestLight(ModelInstance* mi)
{
	if(mi == NULL) return NULL;

	glm::vec3 bestPosition = glm::vec3(mi->getRenSphere());

	Light* candidate = NULL;
	float candidateValue = 0.0f;

	int sz = lights.size();
	for(int i=0; i<sz; i++)
	{
		float distToLight = glm::distance( bestPosition, lights[i]->getPosition() );
		if(distToLight == 0.0f) distToLight = 0.001f;
		float value = lights[i]->getFallOff()/distToLight;
		if(value > candidateValue)
		{
			candidateValue = value;
			candidate = lights[i];
		}
	}

	return candidate;
}
