#ifndef _DECORATION_H_
#define _DECORATION_H_
#include "../entity.h"

class Decoration : public Entity
{
public:
    FROM_INCUBATOR(Decoration);

	Decoration();
	virtual ~Decoration();
	virtual void initialize();
    void Deserialize(rapidjson::Value& d);

private:
	//states
	static STATE main(EntityEvent* ee, Entity* caller);
	//~states
	virtual void            editorSelect();
	virtual void            addProperties();
	void                    updateParamsInternal();

	std::string             shaderPath;
	std::string             modelPath;
	std::string             dTexturePath;
	std::string             nTexturePath;
	std::string             hTexturePath;

    void    updateShaderMutators();
	friend void TW_CALL updateParameters(void *decorPtr);
};

#endif
