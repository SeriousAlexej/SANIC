#ifndef _BASIC_H_
#define _BASIC_H_
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>

class Movable
{
public:
	Movable();
	~Movable();

	glm::vec3	getPosition();
	glm::mat4	getRotation();
	glm::vec3	getScale();
	const glm::mat4&	getMatrix();

	void		setPosition(glm::vec3 pos);
	void		setRotation(glm::vec3 rot);
	void		setRotation(float angle, glm::vec3 dir);
	void		setScale(glm::vec3 sca);

	void		translate(glm::vec3 addpos);
	void		rotate(glm::vec3 addrot);
	void		rotate(float angle, glm::vec3 dir);
	void		scale(glm::vec3 addsca);
protected:
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

#endif