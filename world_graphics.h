#ifndef _WORLD_GRAPHICS_H_
#define _WORLD_GRAPHICS_H_
#include "modelinstance.h"
#include "camera.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "light.h"

class WorldGraphics
{
public:
	WorldGraphics();
	~WorldGraphics();

	void		render(); //renders stuff
	Camera*		getCamera() { return &camera; }

	ModelInstance*	createModel(std::string vertShader, std::string fragShader,
		std::string modelPath, std::string diffTexture, std::string normTexture);

	void			deleteModel(ModelInstance*& mi);

	Light*			createLight();
	void			deleteLight(Light*& light);

private:
	Light*		pickBestLight(ModelInstance* mi);

	Camera						camera;

	std::vector<ModelInstance*>	models;

	std::vector<Mesh*>			meshes;
	std::vector<Shader*>		shaders;
	std::vector<Texture*>		textures;

	std::vector<Light*>			lights;

	friend class World;
};

#endif