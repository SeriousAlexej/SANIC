#ifndef _MESH_H_
#define _MESH_H_
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <string>
#include <GL/glew.h>
#include "basic.h"

struct AnimInfo
{
public:
	float			secondsPerFrame;
	unsigned		length;
	unsigned		firstFrame;
};

struct Frame
{
public:
	unsigned vertexbuffer;
	std::vector<glm::vec3>	vertices;
	Frame()
	{
		vertexbuffer = 0;
	}
	~Frame()
	{
		vertices.clear();
		if(vertexbuffer)
			glDeleteBuffers(1, &vertexbuffer);
	}
};

class Mesh : public Unique, public Subscribable
{
public:
	//every mesh must have it's model
	Mesh(std::string path);
	virtual ~Mesh();
	inline std::size_t  getHash() const { return srcFileHash; }
	//cf - current frame, nf - next frame. For interpolation, received from ModelInstance
	void		render(unsigned cf, unsigned nf);
	float		getBoundingSphereRadius() const { return boundingSphereRadius; }
	glm::vec3	getBoundingSphereCenter() const { return boundingSphereCenter; }
	glm::vec3	getBoundingBoxCenter() const { return boundingBoxCenter; }
	glm::vec3	getBoundingBoxHalfSizes() const { return boundingBoxHalfSizes; }
	AnimInfo	getAnimInfo(std::string animName) { return animations[animName]; }
	bool		hasAnim(std::string animName) { return animations.find(animName) != animations.end(); }

private:

	std::map<std::string, AnimInfo>	animations;
	//dat shit stores buffer indices for frames
	std::vector<Frame>	frames;
	std::size_t		srcFileHash;
	//for culling purposes
	float				boundingSphereRadius;
	glm::vec3			boundingSphereCenter;
	glm::vec3			boundingBoxCenter;
	glm::vec3			boundingBoxHalfSizes;
	//avoid calling elembuffer.size() too often
	unsigned			indicesSize;
	//FALSE - use AXIS model, TRUE - ??? PROFIT!
	bool				isOk;

	unsigned normalbuffer;
	unsigned uvbuffer;
	unsigned tangentbuffer;
	unsigned bitangentbuffer;
	unsigned elembuffer;

	//keep theese for collision generation, if needed
	std::vector<int>		indices;

//AXIS model - default mesh, used when failed to load given model
//AXIS model has only one instance
static	unsigned defVertexbuffer;
static	unsigned defNormalbuffer;
static	unsigned defUvbuffer;
static	unsigned defTangentbuffer;
static	unsigned defBitangentbuffer;
static	unsigned defElembuffer;

//AXIS model is created when first instance of Mesh is constructed
//and deleted when the last instance of Mesh is destroyed
static	unsigned numberOfMeshes;

	bool loadModel(std::string path);
	void buildAxisModel();
	void deleteAxisModel();

	friend class SolidBody; // for collision generation out of mesh
};

#endif
