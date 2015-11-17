#ifndef TOUCH_FIELD_H
#define TOUCH_FIELD_H
#include <entity.h>

class TouchField : public Entity
{
public:
    FROM_INCUBATOR(TouchField);

	TouchField();
	virtual ~TouchField();
	virtual void initialize() override final;
	void Deserialize(rapidjson::Value& d);

private:
	//states
	DECLARE_STATE(main);
	DECLARE_STATE(WaitingEntry);
	DECLARE_STATE(WaitingExit);
	DECLARE_STATE(Frozen);
	//~states
	virtual void addProperties() override final;
	void         updateParams();
	virtual void fillPointers() override final;
	virtual void renderSelectionIndicator() override final;
	glm::vec3 fieldBox;
	bool active;
	float exitCheckTime;
	EntityPointer enLastIn;

	friend void TW_CALL updateParams(void *tfPtr);
};
#endif // TOUCH_FIELD_H
