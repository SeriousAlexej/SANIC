#include "GLDebugDrawer.h"

#include <stdio.h>

GLDebugDrawer::GLDebugDrawer()
:m_debugMode(0)
{

}

void    GLDebugDrawer::drawLine(const btVector3& from,const btVector3& to,const btVector3& color)
{
	glBegin(GL_LINES);
		glColor3f(color.getX(), color.getY(), color.getZ());
		glVertex3d(from.getX(), from.getY(), from.getZ());
		//glColor3f(color.getX(), color.getY(), color.getZ());
		glVertex3d(to.getX(), to.getY(), to.getZ());
	glEnd();
}

void    GLDebugDrawer::setDebugMode(int debugMode)
{
   m_debugMode = debugMode;
}

void    GLDebugDrawer::draw3dText(const btVector3& location,const char* textString)
{
}

void    GLDebugDrawer::reportErrorWarning(const char* warningString)
{
   printf("%s\n", warningString);
}

void    GLDebugDrawer::drawContactPoint(const btVector3& pointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color)
{
}
