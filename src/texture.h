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

	std::string	getSrcFnm() { return srcFnm; }

	void	bind();

    #ifdef SANIC_DEBUG
    static void  printMemoryStatistics()
    {
        printf("\nTEXTURE CLASS WAS:\n\t\tCREATED %d TIMES\n\t\tDELETED %d TIMES\n", numberOfCreations, numberOfDeletions);
    }
    #endif // SANIC_DEBUG

private:
    #ifdef SANIC_DEBUG
    static int numberOfCreations;
    static int numberOfDeletions;
    #endif // SANIC_DEBUG

	GLuint		textureID;
	std::string	srcFnm;

	void	loadTexture();
};

#endif
