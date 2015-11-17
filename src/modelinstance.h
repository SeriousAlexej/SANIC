#ifndef _MODELINSTANCE_H_
#define _MODELINSTANCE_H_
#include <SFML/System.hpp>
#include "basic.h"
#include "camera.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "light.h"

class ModelInstance : public Movable
{
public:

	ModelInstance(Mesh* mesh, Shader* shader, Texture* diffuse, Texture* normal, Texture* height);
	virtual ~ModelInstance();

	glm::vec4	getRenSphereAt(glm::mat4 modelMatrix);
	glm::vec4	getRenSphere();
	glm::vec3	getRenBoxCenter();
	glm::vec3	getRenBoxHalfSizes();

	void		activate() { active = true; }
	void		deactivate() { active = false; }

	bool            background;
    float           normalStrength;
    float           parallaxScale;
    float           parallaxOffset;

private:
	void		render(glm::mat4 &modelMatrix, Camera& cam, std::vector<Light*> lights, Light* dirLight);
	void        renderForShadow(glm::mat4 &modelMatrix, Camera& cam, Shader* shader);

	void		playAnimation(std::string &anim);
	void        stopAnimation();

	Mesh*			pMesh;
	Shader*			pShader;
	Texture*		pDiffTexture;
	Texture*		pNormTexture;
	Texture*        pHeightTexture;

    glm::vec2       UVTilingD = glm::vec2(1.0f,1.0f);
    glm::vec2       UVTilingN = glm::vec2(1.0f,1.0f);
    glm::vec2       UVTilingH = glm::vec2(1.0f,1.0f);
	bool			active;
	float			lastRender;
	std::string		strCurrAnim;
	unsigned		uCurrFrame;
	unsigned		uNextFrame;
	AnimInfo		animInfo; //for convenience

	glm::mat4       offsetMatrix = glm::mat4(1);

	friend class WorldGraphics;
	friend class ModelSet;
	friend class ModelLOD;
	friend class SolidBody;
};

#endif
