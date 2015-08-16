#ifndef _WORLD_GRAPHICS_H_
#define _WORLD_GRAPHICS_H_
#include "modelinstance.h"
#include "camera.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "light.h"
//#include "world.h"

class WorldGraphics
{
public:
	WorldGraphics();
	~WorldGraphics();

	void		render(); //renders stuff
    Camera*		getCamera() { return &camera; }

	ModelInstance*	createModel(std::string shaderPath,
		std::string modelPath, std::string diffTexture, std::string normTexture, std::string heightTexture);

	void			deleteModel(ModelInstance*& mi);

	Light*			createLight();
	void			deleteLight(Light*& light);

	Light*          createDirLight();
	void            deleteDirLight();

private:
	std::vector<Light*>		pickBestLights(ModelInstance* mi);

    Camera						camera;
	Shader*                     shadowShader;
    //World*                      pWorld;

	std::vector<ModelInstance*>	models;

	std::vector<Mesh*>			meshes;
	std::vector<Shader*>		shaders;
	std::vector<Texture*>		textures;

	std::vector<Light*>			lights;

	Light*                      directionalLight;

    friend class World;
};

#endif
