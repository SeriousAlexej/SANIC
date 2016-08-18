#include <SFML/Graphics.hpp>
#include <functional>
#include "texture.h"
#include "global.h"

GLuint Texture::currentTexture = 0u;

Texture::Texture(std::string path)
{
	textureID = 0;
	srcFnmHash = std::hash<std::string>()(path);
	loadTexture(path);
}

Texture::~Texture()
{
	if(textureID)
	{
		unbind();
        glBindTexture(GL_TEXTURE_2D, 0u);
		glDeleteTextures(1, &textureID);
	}
}

void Texture::bind()
{
    if(textureID != currentTexture)
	{
	    currentTexture = textureID;
	    glBindTexture(GL_TEXTURE_2D, textureID);
	}
}

void Texture::unbind()
{
    currentTexture = 0u;
}

void Texture::loadTexture(std::string &srcFnm)
{
    std::string toUse = srcFnm;
    if(toUse!="")
    {
        toUse = egg::getInstance().g_WorkingDir + toUse.substr(1);
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
