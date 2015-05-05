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
	Shader(std::string vertex_file_path, std::string fragment_file_path);
	virtual ~Shader();

	std::string		getSrcVxFnm() { return srcVxShaFnm; }
	std::string		getSrcFgFnm() { return srcFgShaFnm; }

	void	bind();

	GLuint	getMatrixID()				{ return MatrixID; }
	GLuint	getmID()					{ return mID; }
	GLuint	getvID()					{ return vID; }
	GLuint	getNormalTextureID()		{ return NormalTextureID; }
	GLuint	getDiffuseTextureID()		{ return DiffuseTextureID; }
	GLuint	getModelView3x3MatrixID()	{ return ModelView3x3MatrixID; }
	GLuint	getFrameProgressID()		{ return FrameProgressID; }
	GLuint	getLightPositionID()		{ return LightPositionID; }
	GLuint	getLightDiffuseID()			{ return LightDiffuseID; }
	GLuint	getLightAmbientID()			{ return LightAmbientID; }
	GLuint	getLightFallOffID()			{ return LightFallOffID; }
	GLuint	getLightHotSpotID()			{ return LightHotSpotID; }

private:
	void loadShaders(std::string vertex_file_path, std::string fragment_file_path);

	std::string	srcVxShaFnm;
	std::string	srcFgShaFnm;

	GLuint		shaderID;
	GLuint		MatrixID;
	GLuint		mID;
	GLuint		vID;
	GLuint		NormalTextureID;
	GLuint		DiffuseTextureID;
	GLuint		ModelView3x3MatrixID;
	GLuint		FrameProgressID;
	GLuint		LightPositionID;
	GLuint		LightDiffuseID;
	GLuint		LightAmbientID;
	GLuint		LightFallOffID;
	GLuint		LightHotSpotID;
};

#endif
