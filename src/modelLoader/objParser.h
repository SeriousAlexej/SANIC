// Generated by Bisonc++ V4.05.00 on Sat, 02 May 2015 16:21:17 +0300

#ifndef ObjParser_h_included
#define ObjParser_h_included

#include <glm/glm.hpp>

// $insert baseclass
#include "objParser.base.h"
#include "objScanner.h"


struct Triangle
{
public:
	glm::uvec3 vertex[3];
};


#undef ObjParser
class ObjParser: public ObjParserBase
{

    public:
	ObjParser(std::istream &in, std::vector<glm::vec3>* v,
              std::vector<glm::vec3>* n, std::vector<glm::vec2>* u,
              std::vector<Triangle>* t, std::ostream &out = std::cout)
		: vertices(v), normals(n), uvcoords(u), triangles(t), d_scanner(in, out),
		objectIsCorrect(true),
		maxV(0), maxN(0), maxT(0)
		{
		    if(v)
                vertices->clear();
            if(n)
                normals->clear();
            if(u)
                uvcoords->clear();
            if(t)
                triangles->clear();
		}

		bool load()
		{
		    int result = parse();
		    if(result != 0) { objectIsCorrect = false; }
		    if(vertices != NULL)
            {
                if(!objectIsCorrect || static_cast<int>(vertices->size()) < maxV)
                {
                    vertices->clear();
                    objectIsCorrect = false;
                }
            }
            if(normals != NULL)
            {
                if(!objectIsCorrect || static_cast<int>(normals->size()) < maxN)
                {
                    normals->clear();
                    objectIsCorrect = false;
                }
            }
            if(uvcoords != NULL)
            {
                if(!objectIsCorrect || static_cast<int>(uvcoords->size()) < maxT)
                {
                    uvcoords->clear();
                    objectIsCorrect = false;
                }
            }
            if(triangles != NULL)
            {
                if(!objectIsCorrect)
                {
                    triangles->clear();
                }
            }
            vertexBuffer.clear();
		    return objectIsCorrect;
		}

    private:
        std::vector<glm::vec3>  *vertices;
        std::vector<glm::vec3>  *normals;
        std::vector<glm::vec2>  *uvcoords;
        std::vector<Triangle>   *triangles;
        std::vector<glm::uvec3> vertexBuffer;
        ObjScanner              d_scanner;
        bool                    objectIsCorrect;
        int                     maxV, maxN, maxT;
        void                    validateFace()
        {
            if(objectIsCorrect)
            {
            if(triangles != NULL && vertexBuffer.size() == 3)
            {
                maxV = std::max(maxV, int(vertexBuffer[0][0]));
                maxV = std::max(maxV, int(vertexBuffer[1][0]));
                maxV = std::max(maxV, int(vertexBuffer[2][0]));
                maxT = std::max(maxT, int(vertexBuffer[0][1]));
                maxT = std::max(maxT, int(vertexBuffer[1][1]));
                maxT = std::max(maxT, int(vertexBuffer[2][1]));
                maxN = std::max(maxN, int(vertexBuffer[0][2]));
                maxN = std::max(maxN, int(vertexBuffer[1][2]));
                maxN = std::max(maxN, int(vertexBuffer[2][2]));
                Triangle t;
                t.vertex[0] = vertexBuffer[0];
                t.vertex[1] = vertexBuffer[1];
                t.vertex[2] = vertexBuffer[2];
                triangles->push_back(t);
            } else if(triangles != NULL) {
                objectIsCorrect = false;
            }
            }
            vertexBuffer.clear();
        }

        void error(char const *msg);    // called on (syntax) errors
        int lex();                      // returns the next token from the
                                        // lexical scanner.
        void print();                   // use, e.g., d_token, d_loc

        int parse();

    // support functions for parse():
        void executeAction(int ruleNr);
        void errorRecovery();
        int lookup(bool recovery);
        void nextToken();
        void print__();
        void exceptionHandler__(std::exception const &exc);
};


#endif
