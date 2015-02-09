#ifndef _BASIC_H_
#define _BASIC_H_
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>
#include <queue>
#include <string>

class Movable
{
public:
	Movable();
	~Movable();

	glm::vec3	getOffset();
	glm::vec3	getPosition();
	glm::mat4	getRotation();
	glm::quat	getRotationQuat();
	glm::vec3	getScale();
	const glm::mat4&	getMatrix();

	void		setPosition(glm::vec3 pos);
	void		setRotation(glm::vec3 rot);
	void		setRotation(float angle, glm::vec3 dir);
	void		setScale(glm::vec3 sca);
	void		setOffset(glm::vec3 off);

	void		translate(glm::vec3 addpos);
	void		rotate(glm::vec3 addrot);
	void		rotate(float angle, glm::vec3 dir);
	void		scale(glm::vec3 addsca);
protected:
	glm::vec3		offset;
	glm::mat4		posMx;
	glm::mat4		rotMx;
	glm::mat4		scaMx;
	glm::mat4		mdlMx;

	virtual void		updateModelMatrix();
};

class Subscribable
{
public:
	Subscribable()
	{
		subscribers = 0;
	}
	~Subscribable()
	{
	}

	void	doSubscribe() { subscribers++; }
	void	unSubscribe()
	{
		subscribers--;
		if(subscribers <= 0)
		{
			allSubscribersLeft();
		}
	}
	bool	hasSubscribers() { return subscribers > 0; }
private:
	virtual void allSubscribersLeft() {}
	int		subscribers;
};

class Unique
{
public:
	Unique()
	{
		multipass = 0;
		int sz = registeredMultipasses.size();
		for(int i=0; i<sz; i++)
		{
			if(registeredMultipasses[i] == multipass)
			{
				multipass++;
				i=0;
			}
		}
		registeredMultipasses.push_back(multipass);
	}

	~Unique()
	{
		int sz = registeredMultipasses.size();
		for(int i=0; i<sz; i++)
		{
			if(registeredMultipasses[i] == multipass)
			{
				registeredMultipasses.erase(registeredMultipasses.begin() + i);
				break;
			}
		}
	}

	int	getMultipass() { return multipass; }
private:
	static std::vector<int>	registeredMultipasses;
	int		multipass; //LILUKORBENDALAS!
};

//this class is used for storing results of 'custom callback' from Bullet
class Touchable
{
public:
	Touchable() {}
	~Touchable() { while(!whoTouchedMe.empty()) whoTouchedMe.pop(); }
	void	touch(void* t) { if(t) whoTouchedMe.push(t); }
	void*	popToucher() { void* r = whoTouchedMe.front(); whoTouchedMe.pop(); return r; }
	bool	lonely() { return whoTouchedMe.empty(); }
private:
	std::queue<void*>	whoTouchedMe;
};

//this class should help with defining entity class
class FamilyTree
{
public:
	FamilyTree() : ancestorClasses(""), currentClass("") {}
	~FamilyTree() {}

	void	setClass(std::string myClass)
	{
		//can't have empty classname
		//can't have classname with '&'
		//can't be of the same class as one of the ancestors
		assert(myClass != "" &&
			myClass.find("&") == std::string::npos &&
			ancestorClasses.find(myClass) == std::string::npos);

		if(currentClass != "")
		{
			ancestorClasses += "&" + currentClass;
		}
		currentClass = myClass;
	}

	bool	isDerivedFrom(std::string ancestor)
	{
		assert(ancestor != "");
		return ancestorClasses.find(ancestor) != std::string::npos ||
			currentClass == ancestor;
	}

	bool	isOfClass(std::string classname)
	{
		assert(classname != "");
		return classname == currentClass;
	}

	std::string		getClass() const { return currentClass; }
private:
	std::string		ancestorClasses;
	std::string		currentClass;
};

#endif