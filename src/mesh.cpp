#define _CRT_SECURE_NO_DEPRECATE
#include <fstream>
#include <algorithm>
#include <assert.h>
#include <string.h>
#include <GL/glew.h>
#include "mesh.h"
#include "dirtools.h"
#include "default_model.h"
#include "modelLoader/objParser.h"
#include "global.h"

unsigned Mesh::defVertexbuffer = 0;
unsigned Mesh::defNormalbuffer = 0;
unsigned Mesh::defUvbuffer = 0;
unsigned Mesh::defTangentbuffer = 0;
unsigned Mesh::defBitangentbuffer = 0;
unsigned Mesh::defElembuffer = 0;
unsigned Mesh::numberOfMeshes = 0;

static void computeTangentBasis(
	// inputs
	std::vector<glm::vec3> & vertices,
	std::vector<glm::vec2> & uvs,
	std::vector<glm::vec3> & normals,
	// outputs
	std::vector<glm::vec3> & tangents,
	std::vector<glm::vec3> & bitangents
){

	for (unsigned int i=0; i<vertices.size(); i+=3 ){

		// Shortcuts for vertices
		glm::vec3 & v0 = vertices[i+0];
		glm::vec3 & v1 = vertices[i+1];
		glm::vec3 & v2 = vertices[i+2];

		// Shortcuts for UVs
		glm::vec2 & uv0 = uvs[i+0];
		glm::vec2 & uv1 = uvs[i+1];
		glm::vec2 & uv2 = uvs[i+2];

		// Edges of the triangle : postion delta
		glm::vec3 deltaPos1 = v1-v0;
		glm::vec3 deltaPos2 = v2-v0;

		// UV delta
		glm::vec2 deltaUV1 = uv1-uv0;
		glm::vec2 deltaUV2 = uv2-uv0;

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		glm::vec3 tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r;
		glm::vec3 bitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r;

		// Set the same tangent for all three vertices of the triangle.
		// They will be merged later, in vboindexer.cpp
		tangents.push_back(tangent);
		tangents.push_back(tangent);
		tangents.push_back(tangent);

		// Same thing for binormals
		bitangents.push_back(bitangent);
		bitangents.push_back(bitangent);
		bitangents.push_back(bitangent);

	}

	// See "Going Further"
	for (unsigned int i=0; i<vertices.size(); i+=1 )
	{
		glm::vec3 & n = normals[i];
		glm::vec3 & t = tangents[i];
		glm::vec3 & b = bitangents[i];

		// Gram-Schmidt orthogonalize
		t = glm::normalize(t - n * glm::dot(n, t));

		// Calculate handedness
		if (glm::dot(glm::cross(n, t), b) < 0.0f){
			t = t * -1.0f;
		}

	}


}

struct PackedVertex{
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
	bool operator<(const PackedVertex that) const{
		return memcmp((this), (&that), sizeof(PackedVertex))>0;
	};
};

bool getSimilarVertexIndex_fast(
	PackedVertex & packed,
	std::map<PackedVertex,int> & VertexToOutIndex,
	int & result
){
	std::map<PackedVertex,int>::iterator it = VertexToOutIndex.find(packed);
	if ( it == VertexToOutIndex.end() ){
		return false;
	}else{
		result = it->second;
		return true;
	}
}

static void indexVBO(
	std::vector<glm::vec3> & in_vertices,
	std::vector<glm::vec2> & in_uvs,
	std::vector<glm::vec3> & in_normals,
	std::vector<glm::vec3> & in_tangents,
	std::vector<glm::vec3> & in_bitangents,

	std::vector<int> & out_indices,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals,
	std::vector<glm::vec3> & out_tangents,
	std::vector<glm::vec3> & out_bitangents
){
	std::map<PackedVertex,int> VertexToOutIndex;
	// For each input vertex
	for ( unsigned int i=0; i<in_vertices.size(); i++ ){

		PackedVertex packed = {in_vertices[i], in_uvs[i], in_normals[i]};

		// Try to find a similar vertex in out_XXXX
		int index;
		bool found = getSimilarVertexIndex_fast( packed, VertexToOutIndex, index);

		if ( found ){ // A similar vertex is already in the VBO, use it instead !
			out_indices.push_back( index );

			// Average the tangents and the bitangents
			out_tangents[index] += in_tangents[i];
			out_bitangents[index] += in_bitangents[i];
		}else{ // If not, it needs to be added in the output data.
			out_vertices.push_back( in_vertices[i]);
			out_uvs     .push_back( in_uvs[i]);
			out_normals .push_back( in_normals[i]);
			out_tangents .push_back( in_tangents[i]);
			out_bitangents .push_back( in_bitangents[i]);
			int newindex = int(out_vertices.size()) - 1;
			out_indices .push_back( newindex );
			VertexToOutIndex[ packed ] = newindex;
		}
	}
}


Mesh::Mesh(std::string path)
{
	srcFile = path;
	if(numberOfMeshes == 0)
	{
		buildAxisModel();
	}
	numberOfMeshes++;

	//default animation is the first frame
	AnimInfo defaultAnim;
	defaultAnim.length = 1;
	defaultAnim.secondsPerFrame = 1.0f;
	defaultAnim.firstFrame = 0;
	animations[""] = defaultAnim;

	normalbuffer = 0;
	uvbuffer = 0;
	tangentbuffer = 0;
	bitangentbuffer = 0;
	elembuffer = 0;
	boundingSphereRadius = 0.0f;
	boundingSphereCenter = glm::vec3(0,0,0);
	boundingBoxCenter = glm::vec3(0,0,0);
	boundingBoxHalfSizes = glm::vec3(0,0,0);
	isOk = loadModel(path);
	if(!isOk)
	{
		boundingSphereRadius = 1.0f;
		boundingBoxHalfSizes = glm::vec3(0.5,0.5,0.5);
	}
};

Mesh::~Mesh()
{
	animations.clear();

	if(normalbuffer)
		glDeleteBuffers(1, &normalbuffer);
	if(uvbuffer)
		glDeleteBuffers(1, &uvbuffer);
	if(tangentbuffer)
		glDeleteBuffers(1, &tangentbuffer);
	if(bitangentbuffer)
		glDeleteBuffers(1, &bitangentbuffer);
	if(elembuffer)
		glDeleteBuffers(1, &elembuffer);

	if(isOk) //if not, then theese vectors are empty anyway
	{
		indices.clear();
		frames[0].vertices.clear();
	}
	frames.clear();

	if(numberOfMeshes == 1)
	{
		deleteAxisModel();
	}
	numberOfMeshes--;
};

void Mesh::deleteAxisModel()
{
	if(defVertexbuffer)
		glDeleteBuffers(1, &defVertexbuffer);
	if(defNormalbuffer)
		glDeleteBuffers(1, &defNormalbuffer);
	if(defUvbuffer)
		glDeleteBuffers(1, &defUvbuffer);
	if(defTangentbuffer)
		glDeleteBuffers(1, &defTangentbuffer);
	if(defBitangentbuffer)
		glDeleteBuffers(1, &defBitangentbuffer);
	if(defElembuffer)
		glDeleteBuffers(1, &defElembuffer);
}

void Mesh::buildAxisModel()
{
	//load model onto videocard
	glGenBuffers(1, &defVertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, defVertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(defVertices), &defVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &defNormalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, defNormalbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(defNormals), &defNormals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &defUvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, defUvbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(defUV), &defUV[0], GL_STATIC_DRAW);

	glGenBuffers(1, &defTangentbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, defTangentbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(defTangents), &defTangents[0], GL_STATIC_DRAW);

	glGenBuffers(1, &defBitangentbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, defBitangentbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(defBiTangents), &defBiTangents[0], GL_STATIC_DRAW);

	glGenBuffers(1, &defElembuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, defElembuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(defIndices), &defIndices[0], GL_STATIC_DRAW);

	//unbind buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

bool Mesh::loadModel(std::string path)
{
    if(path!="")
    {
        path = egg::getInstance().g_WorkingDir + path.substr(1);
    }
	std::replace(path.begin(), path.end(), '\\', '/');

	std::vector<glm::vec3> tvertices;
	std::vector<glm::vec3> tnormals;
	std::vector<glm::vec2> tuvCoords;
	std::vector<Triangle> ttriangles;

    {
	std::ifstream in;
	in.open(path.c_str(), std::ios::in);
	if(!in.is_open())
	{
		printf("ERROR: Model \"%s\" failed to open!\n", path.c_str());
		return false;
	}
    ObjParser p(in, &tvertices, &tnormals, &tuvCoords, &ttriangles);
	if(!p.load())
    {
        printf("ERROR: Model \"%s\" failed to load!\n", path.c_str());
        return false;
    }
	in.close();
	}

	//compute bounding sphere
	{
	int sz = tvertices.size();
	float mostRight(tvertices[0].x), mostLeft(tvertices[0].x),
		  mostUp(tvertices[0].y), mostDown(tvertices[0].y),
		  mostFront(tvertices[0].z), mostBack(tvertices[0].z);//mostWanted
	for(int i=0; i<sz; i++)
	{
		boundingSphereCenter += tvertices[i];
		mostRight       = std::max(tvertices[i].x, mostRight);
		mostLeft        = std::min(tvertices[i].x, mostLeft);
		mostUp          = std::max(tvertices[i].y, mostUp);
		mostDown        = std::min(tvertices[i].y, mostDown);
		mostFront       = std::max(tvertices[i].z, mostFront);
		mostBack        = std::min(tvertices[i].z, mostBack);
	}
	boundingSphereCenter /= float(sz); //center is the average of all vx's positions
	boundingBoxHalfSizes = glm::vec3(mostRight - mostLeft, mostUp - mostDown, mostFront - mostBack) * 0.5f;
	boundingBoxCenter = glm::vec3(mostRight + mostLeft, mostUp + mostDown, mostFront + mostBack) * 0.5f;
	for(int i=0; i<sz; i++)
	{
		glm::vec3 fromCenterToVx = tvertices[i] - boundingSphereCenter;
		float len = glm::length(fromCenterToVx);
		boundingSphereRadius = std::max(len, boundingSphereRadius);
	}
	}


	std::vector<glm::vec3> t2vertices;
	std::vector<glm::vec3> t2normals;
	std::vector<glm::vec2> t2uvCoords;
	std::vector<glm::vec3> ttangents;
	std::vector<glm::vec3> tbitangents;

    {
    int ttsz = ttriangles.size();
	for(int i=0; i<ttsz; i++)
	{
		glm::vec3 vertex1(tvertices[ttriangles[i].vertex[0][0]-1]);	//    X/o/o  o/o/o   o/o/o
		glm::vec3 vertex2(tvertices[ttriangles[i].vertex[1][0]-1]);	//    o/o/o  X/o/o   o/o/o
		glm::vec3 vertex3(tvertices[ttriangles[i].vertex[2][0]-1]);	//    o/o/o  o/o/o   X/o/o

		glm::vec2 uvCoord1(tuvCoords[ttriangles[i].vertex[0][1]-1]);//    o/X/o  o/o/o   o/o/o
		glm::vec2 uvCoord2(tuvCoords[ttriangles[i].vertex[1][1]-1]);//    o/o/o  o/X/o   o/o/o
		glm::vec2 uvCoord3(tuvCoords[ttriangles[i].vertex[2][1]-1]);//    o/o/o  o/o/o   o/X/o

		glm::vec3 normal1(tnormals[ttriangles[i].vertex[0][2]-1]);	//    o/o/X  o/o/o   o/o/o
		glm::vec3 normal2(tnormals[ttriangles[i].vertex[1][2]-1]);	//    o/o/o  o/o/X   o/o/o
		glm::vec3 normal3(tnormals[ttriangles[i].vertex[2][2]-1]);	//    o/o/o  o/o/o   o/o/X

		t2vertices.push_back(vertex1);
		t2vertices.push_back(vertex2);
		t2vertices.push_back(vertex3);

		t2normals.push_back(normal1);
		t2normals.push_back(normal2);
		t2normals.push_back(normal3);

		t2uvCoords.push_back(uvCoord1);
		t2uvCoords.push_back(uvCoord2);
		t2uvCoords.push_back(uvCoord3);
	}
	}

	std::vector<glm::vec3>		normals;
	std::vector<glm::vec2>		uvCoords;
	std::vector<glm::vec3>		tangents;
	std::vector<glm::vec3>		bitangents;

	frames.push_back(Frame());

	computeTangentBasis(t2vertices, t2uvCoords, t2normals, ttangents, tbitangents);
	indexVBO(t2vertices, t2uvCoords, t2normals, ttangents, tbitangents,
		indices, frames[0].vertices, uvCoords, normals, tangents, bitangents);

	std::vector<unsigned> origIndices;
	for(int i=frames[0].vertices.size()-1; i>=0; i--)
	{
		for(int j=tvertices.size()-1; j>=0; j--)
		{
			if(frames[0].vertices[i] == tvertices[j])
			{
				origIndices.push_back(unsigned(j));
				break;
			}
		}
	}
	//xD silly me, the loops are reversed, but the logic isn't! fix this...
	std::reverse(origIndices.begin(), origIndices.end());

	//now scan for anims
	int found = path.find_last_of("/");
	std::string animDir = path.substr(0,found+1) + "anims/";
	if(DirectoryExists(animDir))
	{
		int vertSize = frames[0].vertices.size();
		assert(vertSize == int(origIndices.size()));

		std::vector<std::string> animDirs = GetFilesOfFormat(animDir);
		{
		int adsz = animDirs.size();
		for(int i=0; i<adsz; i++)
		{
			std::string animFolder = animDir + animDirs[i] + "/";
			std::vector<std::string> objFrames = GetFilesOfFormat(animFolder, "obj");
			AnimInfo ai;
			ai.firstFrame = frames.size();
			{
			int ofsz = objFrames.size();
			for(int f=0; f<ofsz; f++)
			{
			    {
				std::ifstream frm;
				frm.open((animFolder + objFrames[f]).c_str(), std::ios::in);
				if(!frm.is_open())
					continue;
                ObjParser p(frm, &tvertices, NULL, NULL, NULL);
                if(!p.load())
                    continue;
				frm.close();
			    }

				Frame newFrame;
				newFrame.vertices.resize(vertSize);
				for(int ind=0; ind<vertSize; ind++)
				{
					newFrame.vertices[ind] = tvertices[origIndices[ind]];
				}
				frames.push_back(newFrame);
			}
			}
			ai.length = frames.size() - ai.firstFrame;

			//load spf
			float secPerFrame = 1.0f;
			std::ifstream spf;
			spf.open((animFolder + animDirs[i]).c_str(), std::ios::in);
			if(spf.is_open())
			{
				spf >> secPerFrame;
				spf.close();
			}
			ai.secondsPerFrame = secPerFrame;

			animations[animDirs[i]] = ai;
		}
		}
	}

	//load model onto videocard
	{
	int fsz = frames.size();
	for(int i=0; i<fsz; i++)
	{
		glGenBuffers(1, &frames[i].vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, frames[i].vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, frames[i].vertices.size() * sizeof(glm::vec3), &frames[i].vertices[0], GL_STATIC_DRAW);
		//do not delete vertices of first frame
		//they're used for collision generation, if asked
		if(i != 0)
		{
			frames[i].vertices.clear();
		}
	}
	}

	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvCoords.size() * sizeof(glm::vec2), &uvCoords[0], GL_STATIC_DRAW);

	glGenBuffers(1, &tangentbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, tangentbuffer);
	glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(glm::vec3), &tangents[0], GL_STATIC_DRAW);

	glGenBuffers(1, &bitangentbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, bitangentbuffer);
	glBufferData(GL_ARRAY_BUFFER, bitangents.size() * sizeof(glm::vec3), &bitangents[0], GL_STATIC_DRAW);

	glGenBuffers(1, &elembuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elembuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);

	//unbind buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	indicesSize = indices.size();
	//unload shit from RAM, it's already in GPU memory
	normals.clear();
	uvCoords.clear();
	tangents.clear();
	bitangents.clear();

	tvertices.clear();
	tnormals.clear();
	tuvCoords.clear();
	t2vertices.clear();
	t2normals.clear();
	t2uvCoords.clear();
	ttriangles.clear();
	ttangents.clear();
	tbitangents.clear();
	origIndices.clear();
	return true;
};

void Mesh::render(unsigned cf, unsigned nf)
{
    /*
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		*/
		// 1rst attribute buffer : vertices of current frame
		glBindBuffer(GL_ARRAY_BUFFER, (isOk?frames[cf].vertexbuffer:defVertexbuffer));
		glVertexAttribPointer(
		   0,                  // attribute 0
		   3,                  // size
		   GL_FLOAT,           // type
		   GL_FALSE,           // normalized?
		   0,                  // stride
		   nullptr            // array buffer offset
		   );

		// 5th attribute buffer : vertices of next frame for interpolation
		glBindBuffer(GL_ARRAY_BUFFER, (isOk?frames[nf].vertexbuffer:defVertexbuffer));
		glVertexAttribPointer(
		   5,                  // attribute 5
		   3,                  // size
		   GL_FLOAT,           // type
		   GL_FALSE,           // normalized?
		   0,                  // stride
		   nullptr            // array buffer offset
		   );

		// 2nd attribute buffer : uv coords
		glBindBuffer(GL_ARRAY_BUFFER, (isOk?uvbuffer:defUvbuffer));
		glVertexAttribPointer(
			1,								// attribute 1
			2,								// size
			GL_FLOAT,						// type
			GL_FALSE,						// normalized?
			0,								// stride
			nullptr						// array buffer offset
			);

		// 3rd attribute buffer : normals
		glBindBuffer(GL_ARRAY_BUFFER, (isOk?normalbuffer:defNormalbuffer));
		glVertexAttribPointer(
			2,                                // attribute 2
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			nullptr                          // array buffer offset
			);

		// 4th attribute buffer : tangents
		glBindBuffer(GL_ARRAY_BUFFER, (isOk?tangentbuffer:defTangentbuffer));
		glVertexAttribPointer(
			3,                                // attribute 3
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			nullptr                          // array buffer offset
			);

		// 5th attribute buffer : bitangents
		glBindBuffer(GL_ARRAY_BUFFER, (isOk?bitangentbuffer:defBitangentbuffer));
		glVertexAttribPointer(
			4,                                // attribute 4
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			nullptr                          // array buffer offset
			);

		// Draw
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (isOk?elembuffer:defElembuffer));

		glDrawElements(
			GL_TRIANGLES,      // mode
			(isOk?indicesSize:defINSize),//indices.size(),    // count
			GL_UNSIGNED_INT,   // type
			nullptr           // element array buffer offset
			);
/*
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);
		glDisableVertexAttribArray(5);

		*/
}
