#ifndef _TEXTURE_H_
#define _TEXTURE_H_
#include <string>
#include <GL/glew.h>
#include "basic.h"

class Texture : public Unique, public Subscribable
{
public:
	Texture(std::string path);
	virtual ~Texture();

	inline std::size_t  getHash() const { return srcFnmHash; }

	void	bind();
	static void unbind();

private:

	GLuint		textureID;
	std::size_t	srcFnmHash;
	static GLuint currentTexture;

	void	loadTexture(std::string &srcFnm);
};

#endif
