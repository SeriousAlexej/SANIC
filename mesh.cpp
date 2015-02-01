#define _CRT_SECURE_NO_DEPRECATE
#include <fstream>
#include <assert.h>
#include <string.h>
#include <GL/glew.h>
#include "mesh.h"
#include "dirtools.h"
#include "default_model.h"

unsigned Mesh::defVertexbuffer = 0;
unsigned Mesh::defNormalbuffer = 0;
unsigned Mesh::defUvbuffer = 0;
unsigned Mesh::defTangentbuffer = 0;
unsigned Mesh::defBitangentbuffer = 0;
unsigned Mesh::defElembuffer = 0;
unsigned Mesh::numberOfMeshes = 0;


struct Triangle
{
public:
	glm::uvec3 vertex[3];
};

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


static bool is_near(float v1, float v2){
	return fabs( v1-v2 ) < 0.01f;
}

static bool getSimilarVertexIndex( 
	glm::vec3 & in_vertex, 
	glm::vec2 & in_uv, 
	glm::vec3 & in_normal, 
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals,
	unsigned short & result
){
	// Lame linear search
	for ( unsigned int i=0; i<out_vertices.size(); i++ ){
		if (
			is_near( in_vertex.x , out_vertices[i].x ) &&
			is_near( in_vertex.y , out_vertices[i].y ) &&
			is_near( in_vertex.z , out_vertices[i].z ) &&
			is_near( in_uv.x     , out_uvs     [i].x ) &&
			is_near( in_uv.y     , out_uvs     [i].y ) &&
			is_near( in_normal.x , out_normals [i].x ) &&
			is_near( in_normal.y , out_normals [i].y ) &&
			is_near( in_normal.z , out_normals [i].z )
		){
			result = i;
			return true;
		}
	}
	// No other vertex could be used instead.
	// Looks like we'll have to add it to the VBO.
	return false;
}

static void indexVBO(
	std::vector<glm::vec3> & in_vertices,
	std::vector<glm::vec2> & in_uvs,
	std::vector<glm::vec3> & in_normals,
	std::vector<glm::vec3> & in_tangents,
	std::vector<glm::vec3> & in_bitangents,

	std::vector<unsigned short> & out_indices,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals,
	std::vector<glm::vec3> & out_tangents,
	std::vector<glm::vec3> & out_bitangents
){
	// For each input vertex
	for ( unsigned int i=0; i<in_vertices.size(); i++ ){

		// Try to find a similar vertex in out_XXXX
		unsigned short index;
		bool found = getSimilarVertexIndex(in_vertices[i], in_uvs[i], in_normals[i],     out_vertices, out_uvs, out_normals, index);

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
			out_indices .push_back( (unsigned short)out_vertices.size() - 1 );
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
	animations["default"] = defaultAnim;

	normalbuffer = 0;
	uvbuffer = 0;
	tangentbuffer = 0;
	bitangentbuffer = 0;
	elembuffer = 0;
	boundingSphereRadius = 0.0f;
	boundingSphereCenter = glm::vec3(0,0,0);
	isOk = loadModel(path);
	if(!isOk)
	{
		boundingSphereRadius = 1.0f;
	}
};

Mesh::~Mesh()
{
	frames.clear();
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
	std::ifstream in;
	in.open(path.c_str(), std::ios::in);
	if(!in.is_open())
	{
		printf("ERROR: Model \"%s\" couldn't be loaded!\n", path.c_str());
		return false;
		//exit(1);
	}

	std::vector<glm::vec3>		normals;
	std::vector<glm::vec2>		uvCoords;
	std::vector<glm::vec3>		tangents;
	std::vector<glm::vec3>		bitangents;
	std::vector<unsigned short>	indices;

	std::vector<glm::vec3> tvertices;
	std::vector<glm::vec3> tnormals;
	std::vector<glm::vec2> tuvCoords;
	std::vector<Triangle> ttriangles;

	unsigned vMAX(0u), tMAX(0u), nMAX(0u);

	std::string line = "";
	while(getline(in, line))
	{
		if(line.length()<3)
			continue;
		float f1(0.0f), f2(0.0f), f3(0.0f);
		unsigned u11(0u), u12(0u), u13(0u),
			     u21(0u), u22(0u), u23(0u),
				 u31(0u), u32(0u), u33(0u);
		std::string type = line.substr(0, 2);
		line = line.substr(2);
		while(line[0]==' ')
			line = line.substr(1);
		if(type == "v ")
		{
			assert(sscanf(line.c_str(), "%f %f %f", &f1, &f2, &f3) == 3);
			tvertices.push_back(glm::vec3(f1,f2,f3));
		} else
		if(type == "vn")
		{
			assert(sscanf(line.c_str(), "%f %f %f", &f1, &f2, &f3) == 3);
			tnormals.push_back(glm::vec3(f1, f2, f3));
		} else
		if(type == "vt")
		{
			assert(sscanf(line.c_str(), "%f %f", &f1, &f2) == 2);
			tuvCoords.push_back(glm::vec2(f1,1.0f-f2));
		} else
		if(type == "f ")
		{
			assert(sscanf(line.c_str(), "%u/%u/%u %u/%u/%u %u/%u/%u", &u11, &u12, &u13, &u21, &u22, &u23, &u31, &u32, &u33) == 9);
			Triangle tr;
			if(u11 > vMAX) vMAX = u11;
			if(u21 > vMAX) vMAX = u21;
			if(u31 > vMAX) vMAX = u31;
			if(u12 > tMAX) tMAX = u12;
			if(u22 > tMAX) tMAX = u22;
			if(u32 > tMAX) tMAX = u32;
			if(u13 > nMAX) nMAX = u13;
			if(u23 > nMAX) nMAX = u23;
			if(u33 > nMAX) nMAX = u33;
			tr.vertex[0] = glm::uvec3(u11,u12,u13);
			tr.vertex[1] = glm::uvec3(u21,u22,u23);
			tr.vertex[2] = glm::uvec3(u31,u32,u33);
			ttriangles.push_back(tr);
		}
	}

	//compute bounding sphere
	int sz = tvertices.size();
	for(int i=0; i<sz; i++)
	{
		boundingSphereCenter += tvertices[i];
	}
	boundingSphereCenter /= (float)sz; //center is the average of all vx's positions
	for(int i=0; i<sz; i++)
	{
		glm::vec3 fromCenterToVx = tvertices[i] - boundingSphereCenter;
		float len = fromCenterToVx.length();
		if(len > boundingSphereRadius) boundingSphereRadius = len;
	}

	in.close();

	assert(ttriangles.size() > 0 && vMAX == tvertices.size() && tMAX == tuvCoords.size() && nMAX == tnormals.size());

	std::vector<glm::vec3> t2vertices;
	std::vector<glm::vec3> t2normals;
	std::vector<glm::vec2> t2uvCoords;
	std::vector<glm::vec3> ttangents;
	std::vector<glm::vec3> tbitangents;

	for(int i=0; i<ttriangles.size(); i++)
	{
		glm::vec3 vertex1(tvertices[ttriangles[i].vertex[0][0]-1]);	//    X/o/o  o/o/o   o/o/o
		glm::vec3 vertex2(tvertices[ttriangles[i].vertex[1][0]-1]);	//    o/o/o  X/o/o   o/o/o
		glm::vec3 vertex3(tvertices[ttriangles[i].vertex[2][0]-1]);	//    o/o/o  o/o/o   X/o/o
		
		glm::vec3 normal1(tnormals[ttriangles[i].vertex[0][2]-1]);	//    o/o/X  o/o/o   o/o/o
		glm::vec3 normal2(tnormals[ttriangles[i].vertex[1][2]-1]);	//    o/o/o  o/o/X   o/o/o
		glm::vec3 normal3(tnormals[ttriangles[i].vertex[2][2]-1]);	//    o/o/o  o/o/o   o/o/X
		
		glm::vec2 uvCoord1(tuvCoords[ttriangles[i].vertex[0][1]-1]);//    o/X/o  o/o/o   o/o/o
		glm::vec2 uvCoord2(tuvCoords[ttriangles[i].vertex[1][1]-1]);//    o/o/o  o/X/o   o/o/o
		glm::vec2 uvCoord3(tuvCoords[ttriangles[i].vertex[2][1]-1]);//    o/o/o  o/o/o   o/X/o

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

	computeTangentBasis(t2vertices, t2uvCoords, t2normals, ttangents, tbitangents);

	frames.push_back(Frame());

	indexVBO(t2vertices, t2uvCoords, t2normals, ttangents, tbitangents,
		indices, frames[0].vertices, uvCoords, normals, tangents, bitangents);

	//dump dat shit
	/*
	std::ofstream of;
	of.open((path+".h").c_str());

	of << "// " << frames[0].vertices.size() << "\n";
	of << "const float defVertices[] = { "
		<< frames[0].vertices[0].x << ", "
		<< frames[0].vertices[0].y << ", "
		<< frames[0].vertices[0].z;
	for(int i=1; i<frames[0].vertices.size(); i++)
		of	<< ", "
		<< frames[0].vertices[i].x << ", "
		<< frames[0].vertices[i].y << ", "
		<< frames[0].vertices[i].z;
	of << " };\n";

	of << "// " << normals.size() << "\n";
	of << "const float defNormals[] = { "
		<< normals[0].x << ", "
		<< normals[0].y << ", "
		<< normals[0].z;
	for(int i=1; i<normals.size(); i++)
		of	<< ", "
		<< normals[i].x << ", "
		<< normals[i].y << ", "
		<< normals[i].z;
	of << " };\n";

	of << "// " << uvCoords.size() << "\n";
	of << "const float defUV[] = { "
		<< uvCoords[0].x << ", "
		<< uvCoords[0].y;
	for(int i=1; i<uvCoords.size(); i++)
		of	<< ", "
		<< uvCoords[i].x << ", "
		<< uvCoords[i].y;
	of << " };\n";

	of << "// " << tangents.size() << "\n";
	of << "const float defTangents[] = { "
		<< tangents[0].x << ", "
		<< tangents[0].y << ", "
		<< tangents[0].z;
	for(int i=1; i<tangents.size(); i++)
		of	<< ", "
		<< tangents[i].x << ", "
		<< tangents[i].y << ", "
		<< tangents[i].z;
	of << " };\n";

	of << "// " << bitangents.size() << "\n";
	of << "const float defBiTangents[] = { "
		<< bitangents[0].x << ", "
		<< bitangents[0].y << ", "
		<< bitangents[0].z;
	for(int i=1; i<bitangents.size(); i++)
		of	<< ", "
		<< bitangents[i].x << ", "
		<< bitangents[i].y << ", "
		<< bitangents[i].z;
	of << " };\n";

	of << "// " << indices.size() << "\n";
	of << "const unsigned short defIndices[] = { "
		<< indices[0];
	for(int i=1; i<indices.size(); i++)
		of << ", " << indices[i];
	of << " };\n";

	of.close();
	*/

	std::vector<unsigned> origIndices;
	for(int i=0; i<frames[0].vertices.size(); i++)
	{
		for(int j=0; j<tvertices.size(); j++)
		{
			if(frames[0].vertices[i] == tvertices[j])
			{
				origIndices.push_back((unsigned)j);
				break;
			}
		}
	}

	//now scan for anims
	unsigned found = path.find_last_of("/\\");
	std::string animDir = path.substr(0,found+1) + "anims/";
	found = animDir.find("\\");
	while(found != -1)
	{
		animDir = animDir.substr(0,found) + "/" + animDir.substr(found+1);
		found = animDir.find("\\");
	}
	if(DirectoryExists(animDir))
	{
		int tvertSize = tvertices.size();
		int vertSize = frames[0].vertices.size();
		assert(vertSize == origIndices.size());

		std::vector<std::string> animDirs = GetFilesOfFormat(animDir);
		for(int i=0; i<animDirs.size(); i++)
		{
			std::string animFolder = animDir + animDirs[i] + "/";
			std::vector<std::string> objFrames = GetFilesOfFormat(animFolder, "obj");
			AnimInfo ai;
			ai.firstFrame = frames.size();
			for(int f=0; f<objFrames.size(); f++)
			{
				std::ifstream frm;
				frm.open((animFolder + objFrames[f]).c_str(), std::ios::in);
				if(!frm.is_open())
					continue;
				
				int vert = 0;
				std::string line = "";
				while(vert<tvertSize && getline(frm, line))
				{
					if(line.length()<3)
						continue;
					float f1(0.0f), f2(0.0f), f3(0.0f);
					std::string type = line.substr(0, 2);
					line = line.substr(2);
					while(line[0]==' ')
						line = line.substr(1);
					if(type == "v ")
					{
						assert(sscanf(line.c_str(), "%f %f %f", &f1, &f2, &f3) == 3);
						tvertices[vert] = glm::vec3(f1, f2, f3);
						vert++;
					}
				}
				frm.close();

				Frame newFrame;
				newFrame.vertices.resize(vertSize);
				for(int ind=0; ind<vertSize; ind++)
				{
					newFrame.vertices[ind] = tvertices[origIndices[ind]];
				}
				frames.push_back(newFrame);
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

	//load model onto videocard
	for(int i=0; i<frames.size(); i++)
	{
		glGenBuffers(1, &frames[i].vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, frames[i].vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, frames[i].vertices.size() * sizeof(glm::vec3), &frames[i].vertices[0], GL_STATIC_DRAW);
		frames[i].vertices.clear();
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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

	//unbind buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	indicesSize = indices.size();
	//unload shit from RAM, it's already in GPU memory
	normals.clear();
	uvCoords.clear();
	indices.clear();
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
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5); //for anim interpolation

		// 1rst attribute buffer : vertices of current frame
		glBindBuffer(GL_ARRAY_BUFFER, (isOk?frames[cf].vertexbuffer:defVertexbuffer));
		glVertexAttribPointer(
		   0,                  // attribute 0
		   3,                  // size
		   GL_FLOAT,           // type
		   GL_FALSE,           // normalized?
		   0,                  // stride
		   (void*)0            // array buffer offset
		   );

		// 5th attribute buffer : vertices of next frame for interpolation
		glBindBuffer(GL_ARRAY_BUFFER, (isOk?frames[nf].vertexbuffer:defVertexbuffer));
		glVertexAttribPointer(
		   5,                  // attribute 5
		   3,                  // size
		   GL_FLOAT,           // type
		   GL_FALSE,           // normalized?
		   0,                  // stride
		   (void*)0            // array buffer offset
		   );

		// 2nd attribute buffer : uv coords
		glBindBuffer(GL_ARRAY_BUFFER, (isOk?uvbuffer:defUvbuffer));
		glVertexAttribPointer(
			1,								// attribute 1
			2,								// size
			GL_FLOAT,						// type
			GL_FALSE,						// normalized?
			0,								// stride
			(void*)0						// array buffer offset
			);

		// 3rd attribute buffer : normals
		glBindBuffer(GL_ARRAY_BUFFER, (isOk?normalbuffer:defNormalbuffer));
		glVertexAttribPointer(
			2,                                // attribute 2
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
			);

		// 4th attribute buffer : tangents
		glBindBuffer(GL_ARRAY_BUFFER, (isOk?tangentbuffer:defTangentbuffer));
		glVertexAttribPointer(
			3,                                // attribute 3
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
			);

		// 5th attribute buffer : bitangents
		glBindBuffer(GL_ARRAY_BUFFER, (isOk?bitangentbuffer:defBitangentbuffer));
		glVertexAttribPointer(
			4,                                // attribute 4
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
			);
		 
		// Draw
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (isOk?elembuffer:defElembuffer));

		glDrawElements(
			GL_TRIANGLES,      // mode
			(isOk?indicesSize:defINSize),//indices.size(),    // count
			GL_UNSIGNED_SHORT,   // type
			(void*)0           // element array buffer offset
			);
		 
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);
		glDisableVertexAttribArray(5);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
