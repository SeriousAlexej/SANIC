/*
build with the following command!
bisonc++ -p objParser.cpp -i objParser.impl.h -c objParser.h -b objParser.base.h objParser.y
*/
%class-name ObjParser

%polymorphic _FLOAT: float; _STRING: std::string;

%token GROUP
%token OBJECT
%token MTLLIB
%token USEMTL
%token VERTEX
%token TEXCOORD
%token NORMAL
%token PARAMETER
%token POLYGON
%token SMOOTH
%token OFF
%token FLOAT
%type <_STRING> mtllib_line
%type <_STRING> usemtl_line
%type <_STRING> object_line
%type <_STRING> group_line
%type <_FLOAT> float
%type <_FLOAT> smth_value

%%

object:  |
	object preface_lines face_lines;

preface_lines: '\n' |
	mtllib_line |
	object_line |
	vtx_line |
	vnm_line |
	vtc_line |
	vpm_line |
	preface_lines mtllib_line |
	preface_lines object_line |
	preface_lines vtx_line |
	preface_lines vnm_line |
	preface_lines vtc_line |
	preface_lines vpm_line;

face_lines: '\n' |
    	EOF |
	usemtl_line |
	group_line |
	smth_line |
	face_line |
	face_lines usemtl_line |
	face_lines group_line |
	face_lines smth_line |
	face_lines face_line;

mtllib_line: MTLLIB ;

usemtl_line: USEMTL ;

object_line: OBJECT ;

group_line: GROUP ;

vtx_line: VERTEX float float float { if(!objectIsCorrect){return;} if(vertices!=NULL){ vertices->push_back(glm::vec3($2, $3, $4)); } } |
	  VERTEX float float float float { if(!objectIsCorrect){return;} if(vertices!=NULL){ vertices->push_back(glm::vec3($2, $3, $4)); } } ;

vnm_line: NORMAL float float float { if(!objectIsCorrect){return;} if(normals!=NULL){ normals->push_back(glm::vec3($2, $3, $4)); } } ;

vtc_line: TEXCOORD float float { if(!objectIsCorrect){return;} if(uvcoords!=NULL){ uvcoords->push_back(glm::vec2($2, 1.0f-$3)); } } |
	  TEXCOORD float float float { if(!objectIsCorrect){return;} if(uvcoords!=NULL){ uvcoords->push_back(glm::vec2($2, 1.0f-$3)); } } ;

vpm_line: PARAMETER float  |
	  PARAMETER float float |
	  PARAMETER float float float ;

face_line: POLYGON face_elements { validateFace(); } ;

face_elements: face_element | face_elements face_element ;

face_element: float { objectIsCorrect = false; } |
	      float '/' '/' float { objectIsCorrect = false; } |
	      float '/' float '/' float { if(!objectIsCorrect){return;} vertexBuffer.push_back(glm::uvec3((unsigned int)$1,(unsigned int)$3,(unsigned int)$5)); } ;

smth_line: SMOOTH smth_value ;

smth_value: OFF { $$ = 0.5f; } |
	    float { $$ = $1; } ;

float: FLOAT { $$ = atof(d_scanner.matched().c_str()); } ;
