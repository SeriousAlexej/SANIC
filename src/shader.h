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

	inline std::size_t  getHash() const { return srcShaFnmHash; }

	void	bind();
	static void unbind();

	inline GLuint  getMatrixID() const             { return MatrixID; }
	inline GLuint  getmID() const                  { return mID; }
	inline GLuint  getNormalTextureID() const      { return NormalTextureID; }
	inline GLuint  getDiffuseTextureID() const     { return DiffuseTextureID; }
	inline GLuint  getHeightTextureID() const      { return HeightTextureID; }
	inline GLuint  getFrameProgressID() const      { return FrameProgressID; }
	inline GLuint  getCameraPositionID() const     { return CamPosID; }
	inline GLuint  getLightPositionID(int i) const { return LightPositionID[i]; }
	inline GLuint  getLightDiffuseID(int i) const  { return LightDiffuseID[i]; }
	inline GLuint  getLightFallOffID(int i) const  { return LightFallOffID[i]; }
	inline GLuint  getLightHotSpotID(int i) const  { return LightHotSpotID[i]; }
	inline GLuint  getLightIntensityID(int i) const{ return LightIntensityID[i]; }
	inline GLuint  getNormalStrengthID() const     { return normStrengthID; }
	inline GLuint  getParallaxScaleID() const      { return parallaxScaleID; }
	inline GLuint  getParallaxOffsetID() const     { return parallaxOffsetID; }
    inline GLuint  getBiasMVP() const              { return BiasMVP; }
    inline GLuint  getDLightAmbient() const        { return DLightAmbient; }
    inline GLuint  getDLightDiffuse() const        { return DLightDiffuse; }
    inline GLuint  getDirShadowBool() const        { return DirShadowBool; }
    inline GLuint  getShadowMap() const            { return ShadowMap; }
    inline GLuint  getDLightDir() const            { return DLightDir; }
    inline GLuint  getBiasMVP_LQ() const           { return BiasMVP_LQ; }
    inline GLuint  getShadowMap_LQ() const         { return ShadowMap_LQ; }
    inline GLuint  getShadowBorder() const         { return ShadowBorder; }
    inline GLuint  getUVTilingD() const            { return UVTilingD; }
    inline GLuint  getUVTilingN() const            { return UVTilingN; }
    inline GLuint  getUVTilingH() const            { return UVTilingH; }

private:

	void loadShaders(std::string &shaderPath);

	std::size_t	srcShaFnmHash;

	GLuint		shaderID;
	static GLuint currentShader;

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
	GLuint      BiasMVP;
	GLuint      DLightAmbient;
	GLuint      DLightDiffuse;
	GLuint      DirShadowBool;
	GLuint      ShadowMap;
	GLuint      DLightDir;
	GLuint      ShadowMap_LQ;
	GLuint      BiasMVP_LQ;
	GLuint      ShadowBorder;
	GLuint      UVTilingD;
	GLuint      UVTilingN;
	GLuint      UVTilingH;
};

#endif
