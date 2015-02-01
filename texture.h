#ifndef _TEXTURE_H_
#define _TEXTURE_H_
#include <string>
#include <GL/glew.h>
#include "basic.h"

class Texture : public Unique, public Subscribable
{
public:
	Texture(std::string path);
	~Texture();

	std::string	getSrcFnm() { return srcFnm; }

	void	bind();

private:
	GLuint		textureID;
	std::string	srcFnm;

	void	loadTexture();
};

#endif