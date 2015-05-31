#include <SFML/Graphics.hpp>
#include "texture.h"

extern std::string g_WorkingDir;

Texture::Texture(std::string path)
{
	textureID = 0;
	srcFnm = path;
	loadTexture();
}

Texture::~Texture()
{
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
    std::string toUse = srcFnm;
    if(toUse!="")
    {
        toUse = g_WorkingDir + toUse.substr(1);
    }
	sf::Image image;
	if (image.loadFromFile(toUse))
	{
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, image.getSize().x, image.getSize().y, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0); //unbind texture
	}
}
