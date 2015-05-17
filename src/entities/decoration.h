#ifndef _DECORATION_H_
#define _DECORATION_H_
#include "../entity.h"

class Decoration : public Entity
{
public:
	Decoration();
	virtual ~Decoration();
	virtual void initialize();

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
