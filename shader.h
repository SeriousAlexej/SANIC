#ifndef _SHADER_H_
#define _SHADER_H_
#include <string>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include "basic.h"

class Shader : public Unique, public Subscribable
{
public:
	Shader(std::string shaderPath);
	virtual ~Shader();

	std::string		getSrcFnm() { return srcShaFnm; }

	void	bind();

	GLuint	getMatrixID() const				{ return MatrixID; }
	GLuint	getmID() const					{ return mID; }
	GLuint	getNormalTextureID() const		{ return NormalTextureID; }
	GLuint	getDiffuseTextureID() const		{ return DiffuseTextureID; }
	GLuint  getHeightTextureID() const      { return HeightTextureID; }
	GLuint	getFrameProgressID() const		{ return FrameProgressID; }
	GLuint  getCameraPositionID() const     { return CamPosID; }
	GLuint	getLightPositionID(int i) const { return LightPositionID[i]; }
	GLuint	getLightDiffuseID(int i) const	{ return LightDiffuseID[i]; }
	GLuint	getLightFallOffID(int i) const	{ return LightFallOffID[i]; }
	GLuint	getLightHotSpotID(int i) const  { return LightHotSpotID[i]; }
	GLuint	getLightIntensityID(int i) const{ return LightIntensityID[i]; }
	GLuint  getNormalStrengthID() const     { return normStrengthID; }
	GLuint  getParallaxScaleID() const      { return parallaxScaleID; }
	GLuint  getParallaxOffsetID() const     { return parallaxOffsetID; }

    #ifdef SANIC_DEBUG
    static void  printMemoryStatistics()
    {
        printf("\nSHADER CLASS WAS:\n\t\tCREATED %d TIMES\n\t\tDELETED %d TIMES\n", numberOfCreations, numberOfDeletions);
    }
    #endif // SANIC_DEBUG

private:
    #ifdef SANIC_DEBUG
    static int numberOfCreations;
    static int numberOfDeletions;
    #endif // SANIC_DEBUG

	void loadShaders(std::string shaderPath);

	std::string	srcShaFnm;

	GLuint		shaderID;
	GLuint		MatrixID;
	GLuint		mID;
	GLuint		NormalTextureID;
	GLuint		DiffuseTextureID;
	GLuint      HeightTextureID;
	GLuint		FrameProgressID;
	GLuint      normStrengthID;
	GLuint      parallaxScaleID;
	GLuint      parallaxOffsetID;
	GLuint      CamPosID;
	GLuint		LightPositionID[4];
	GLuint		LightDiffuseID[4];
	GLuint		LightFallOffID[4];
	GLuint		LightHotSpotID[4];
	GLuint      LightIntensityID[4];
};

#endif
