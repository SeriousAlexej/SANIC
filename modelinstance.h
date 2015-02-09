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
	ModelInstance(Mesh* mesh, Shader* shader, Texture* diffuse, Texture* normal);
	virtual ~ModelInstance();

	void		playAnimation(std::string anim);
	glm::vec4	getRenSphere();

	void		activate() { active = true; }
	void		deactivate() { active = false; }

private:
	void		render(Camera& cam, Light* light);

	Mesh*			pMesh;
	Shader*			pShader;
	Texture*		pDiffTexture;
	Texture*		pNormTexture;

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
