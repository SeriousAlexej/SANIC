#ifndef _DECORATION_H_
#define _DECORATION_H_
#include "../entity.h"

class Decoration : public Entity
{
public:
    FROM_INCUBATOR(Decoration)

	Decoration();
	virtual ~Decoration();
	virtual void initialize();
    void Deserialize(rapidjson::Value& d);

private:
	//states
	DECLARE_STATE(main);
	//~states
	virtual void            editorSelect();
	virtual void            addProperties();
	void                    updateParamsInternal();
	void                    reloadRig();

	std::string             shaderPath;
	std::string             modelPath;
	std::string             dTexturePath;
	std::string             nTexturePath;
	std::string             hTexturePath;
	float                   mass;
	int                     bodyType;
	float                   radius;
	glm::vec3               collBoxSize;
	glm::vec3               collOffset;
	glm::vec3               mdlScale;
	bool                    collision;
	std::string             startAnim;

    void    updateShaderMutators();
	friend void TW_CALL updateParameters(void *decorPtr);
};

#endif
