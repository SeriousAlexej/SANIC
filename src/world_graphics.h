#ifndef _WORLD_GRAPHICS_H_
#define _WORLD_GRAPHICS_H_
#include "modelinstance.h"
#include "camera.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "light.h"
#include "modelset.h"

class Sector;
class Portal;

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
	ModelInstance*	createModel(const std::string& shaderPath,
		                        const std::string& modelPath,
		                        const std::string& diffTexture,
		                        const std::string& normTexture,
		                        const std::string& heightTexture);
    void            deleteModel(ModelInstance*& mi);

    Shader*         createShader(const std::string& shaderPath);
    void            deleteShader(Shader*& s);

	void                        renderBackground() const;
	std::vector<ModelInstance*> getVisibleModels() const;

	std::vector<Light*>		    pickBestLights(const std::vector<Light*>& pool, const glm::vec4& modelRenSphere);
	void                        sortForBackground();

	void                        findVisibleStuff(std::vector<ModelInstance*>& visibleModels, std::vector<Light*>& visibleLights);
	void                        findVisibleStuffForShadow(std::vector<ModelInstance*>& visibleModels);



	Camera						camera;
	std::shared_ptr<Shader>     shadowShader;

    unsigned int                backgroundModels;

	std::vector<std::shared_ptr<ModelInstance>>	models;
	std::vector<std::shared_ptr<Mesh>>			meshes;
	std::vector<std::shared_ptr<Shader>>		shaders;
	std::vector<std::shared_ptr<Texture>>		textures;
	std::vector<std::shared_ptr<Light>>			lights;
	std::vector<std::shared_ptr<ModelSet>>      modelSets;
	std::vector<std::unique_ptr<Sector>>        sectors;
	std::vector<std::unique_ptr<Portal>>        portals;

	Light*                      directionalLight;
	unsigned int                dirLightUsers;

	friend class World;
	friend class ModelLOD;
	friend class ModelSet;
};

#endif
