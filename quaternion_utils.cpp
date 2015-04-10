#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
#include "quaternion_utils.h"

glm::quat rotateTowards(glm::quat q1, glm::quat q2, float maxAngle){
	
	if( maxAngle < 0.001f ){
		return q1;
	}
	
	float cosTheta = glm::dot(q1, q2);
	
	if(cosTheta > 0.9999f){
		return q2;
	}
	
	if (cosTheta < 0){
		q1 = q1*-1.0f;
		cosTheta *= -1.0f;
	}
	
	float angle = acos(cosTheta);
	
	if (angle < maxAngle){
		return q2;
	}

	float t = maxAngle / angle;
	angle = maxAngle;
	
    glm::quat res = (float(sin((1.0f - t) * angle)) * q1 + float(sin(t * angle)) * q2) / float(sin(angle));
	res = glm::normalize(res);
	return res;
}
