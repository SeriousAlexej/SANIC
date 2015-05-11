#include <SFML/Graphics.hpp>
#include "texture.h"

#ifdef SANIC_DEBUG
int Texture::numberOfCreations = 0;
int Texture::numberOfDeletions = 0;
#endif // SANIC_DEBUG

Texture::Texture(std::string path)
{
    #ifdef SANIC_DEBUG
    numberOfCreations++;
    #endif // SANIC_DEBUG
	textureID = 0;
	srcFnm = path;
	loadTexture();
}

Texture::~Texture()
{
    #ifdef SANIC_DEBUG
    numberOfDeletions++;
    #endif // SANIC_DEBUG
	if(textureID)
	{
		glDeleteTextures(1, &textureID);
	}
}

void Texture::bind()
{
	glBindTexture(GL_TEXTURE_2D, textureID);
}
void Texture::loadTexture()
{
	sf::Image image;
	if (image.loadFromFile(srcFnm))
	{
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, image.getSize().x, image.getSize().y, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0); //unbind texture
	}
}
