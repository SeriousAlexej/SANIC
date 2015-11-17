#ifndef _WORLD_GRAPHICS_H_
#define _WORLD_GRAPHICS_H_
#include "modelinstance.h"
#include "camera.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "light.h"
#include "modelset.h"

class WorldGraphics
{
public:
	WorldGraphics();
	~WorldGraphics();

	void		render(); //renders stuff
	Camera*		getCamera() { return &camera; }

	ModelSet*       createModelSet(std::string &modelsetPath);
	void            deleteModelSet(ModelSet*& ms);

	Light*			createLight();
	void			deleteLight(Light*& light);

	Light*          createDirLight();
	void            deleteDirLight();

private:
	ModelInstance*	createModel(std::string shaderPath,
		std::string modelPath, std::string diffTexture, std::string normTexture, std::string heightTexture);

	void			deleteModel(ModelInstance*& mi);

	std::vector<Light*>		    pickBestLights(glm::vec4 modelRenSphere);
	void                        sortForBackground();

	Camera						camera;
	std::shared_ptr<Shader>     shadowShader;

    unsigned int                backgroundModels;
	std::vector<std::shared_ptr<ModelInstance>>	models;
	std::vector<std::shared_ptr<Mesh>>			meshes;
	std::vector<std::shared_ptr<Shader>>		shaders;
	std::vector<std::shared_ptr<Texture>>		textures;
	std::vector<std::shared_ptr<Light>>			lights;
	std::vector<std::shared_ptr<ModelSet>>      modelSets;

	Light*                      directionalLight;
	unsigned int                dirLightUsers;

	friend class World;
	friend class ModelLOD;
	friend class ModelSet;
};

#endif
