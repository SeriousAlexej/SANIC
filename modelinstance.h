#ifndef _MODELINSTANCE_H_
#define _MODELINSTANCE_H_
#include <SFML/System.hpp>
#include "basic.h"
#include "camera.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "light.h"

extern sf::Clock g_Clock;

class ModelInstance : public Movable
{
public:
	ModelInstance(Mesh* mesh, Shader* shader, Texture* diffuse, Texture* normal, Texture* height);
	virtual ~ModelInstance();

	void		playAnimation(std::string anim);
	glm::vec4	getRenSphere();
	glm::vec3	getRenBoxCenter();
	glm::vec3	getRenBoxHalfSizes();

	void		activate() { active = true; }
	void		deactivate() { active = false; }

    float           normalStrength;
    float           parallaxScale;
    float           parallaxOffset;

private:
	void		render(Camera& cam, std::vector<Light*> lights);

	Mesh*			pMesh;
	Shader*			pShader;
	Texture*		pDiffTexture;
	Texture*		pNormTexture;
	Texture*        pHeightTexture;

	bool			active;
	float			lastRender;
	std::string		strCurrAnim;
	unsigned		uCurrFrame;
	unsigned		uNextFrame;
	AnimInfo		animInfo; //for convenience

	friend class WorldGraphics;
	friend class SolidBody;
};

#endif
