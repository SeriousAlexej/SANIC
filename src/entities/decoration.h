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

	std::string             modelPath;
	float                   mass;
	glm::vec3               mdlScale;
	bool                    collision;
	std::string             startAnim;
	bool                    background;

	friend void TW_CALL updateParameters(void *decorPtr);
};

#endif
