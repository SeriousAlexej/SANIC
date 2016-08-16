#include <cassert>
#include <SFML/Graphics.hpp>
#include <cstdio>
#include "camera.h"
#include "global.h"
#include "world_graphics.h"
#include "render/sector.h"

Camera::Camera()
{
    this->setPerspective(75.0f, 4.0f/3.0f, 0.1f, 100.0f);
    horizontalAngle = 0.0f;
    verticalAngle = -0.3f;
    position = glm::vec3(0,0,0);
    lastPosition = position;
    offset = glm::translate(glm::vec3(0,0,0));
    updateViewAngles(); //this calls updateMatrix && extractFrustum
    renMode = LQ;
    createFrameBuffersAndTextures();
    renMode = HQ;
    createFrameBuffersAndTextures();
    mySector = nullptr;
    pGfx = nullptr;
}

void Camera::getFbTexRes(GLuint*& fb, GLuint*& tex, GLsizei& res)
{
    switch(renMode)
    {
        case HQ:
            {
                fb = &shadowFramebuffer;
                tex = &shadowTexture;
                res = 2048;
                break;
            }
        case LQ:
            {
                fb = &shadowFramebufferLQ;
                tex = &shadowTextureLQ;
                res = 1024;
                break;
            }
        default:
            {
                assert(false);
            }
    }
}

void Camera::createFrameBuffersAndTextures()
{
    GLuint *buffer;
    GLuint *texture;
    GLsizei resolution;
    getFbTexRes(buffer, texture, resolution);


    glGenFramebuffers(1, buffer);
    assert(*buffer != 0);
    glBindFramebuffer(GL_FRAMEBUFFER, *buffer);

    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT16, resolution, resolution, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *texture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    auto reslt = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(reslt != GL_FRAMEBUFFER_COMPLETE)
    {
        egg::getInstance().g_UseDirectionalLight = false;
        #ifdef SANIC_DEBUG
        switch(reslt)
        {
            case GL_FRAMEBUFFER_UNDEFINED : printf("GL_FRAMEBUFFER_UNDEFINED\n"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT : printf("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT : printf("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER : printf("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER\n"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER : printf("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER\n"); break;
            case GL_FRAMEBUFFER_UNSUPPORTED : printf("GL_FRAMEBUFFER_UNSUPPORTED\n"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE : printf("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE\n"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS : printf("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS\n"); break;
            default: printf("UNKNOWN ERROR %d !\n", reslt);
        }
        #endif // SANIC_DEBUG
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Camera::~Camera()
{
    glDeleteFramebuffers(1, &shadowFramebuffer);
    glDeleteTextures(1, &shadowTexture);
    glDeleteFramebuffers(1, &shadowFramebufferLQ);
    glDeleteTextures(1, &shadowTextureLQ);
}

void Camera::preShadowRender()
{
    GLuint *buffer;
    GLuint *texture;
    GLsizei resolution;
    getFbTexRes(buffer, texture, resolution);

    glBindFramebuffer(GL_FRAMEBUFFER, *buffer);
    glViewport(0,0,resolution,resolution);
}

void Camera::postShadowRender() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if(!egg::getInstance().g_Editor)
    {
        glViewport(egg::getInstance().g_DrawOrigin.x, egg::getInstance().g_DrawOrigin.y, egg::getInstance().g_Resolution.x, egg::getInstance().g_Resolution.y);
    }
}

void Camera::setPerspective(float fovxdeg, float ratio, float near, float _far)
{
    far = _far;
    float fovRad = glm::radians(fovxdeg);
    shadowCubeHSide = far/glm::cos(fovRad*0.5f);
    shadowCubeHSideHQ = shadowCubeHSide*0.5f;
    shadowProjMatrix = glm::ortho(-shadowCubeHSideHQ, shadowCubeHSideHQ, -shadowCubeHSideHQ, shadowCubeHSideHQ, -shadowCubeHSideHQ, shadowCubeHSideHQ*2.0f);
    shadowLQProjMatrix = glm::ortho(-shadowCubeHSide, shadowCubeHSide, -shadowCubeHSide, shadowCubeHSide, -shadowCubeHSide, shadowCubeHSide*2.0f);
    prjMx = glm::perspective(fovRad/ratio, ratio, near, far);
    extractFrustum();
}

void Camera::setShadowViewMatrix(const glm::mat4 &dirLight)
{
    glm::mat4 *vMx;
    float *fSide;
    switch(renMode)
    {
    case LQ:
        {
            vMx = &shadowLQVMatrix;
            fSide = &shadowCubeHSide;
            break;
        }
    case HQ:
        {
            vMx = &shadowVMatrix;
            fSide = &shadowCubeHSideHQ;
            break;
        }
        default: { assert(false); }
    }

    if(glm::distance(lastPosition, position) > 5.0f)
    {
        lastPosition = position;
    }

    glm::vec3 dirLightPos = lastPosition - glm::vec3(dirLight[2].x, dirLight[2].y, dirLight[2].z)*(*fSide);
    glm::vec3 dirLightFront = dirLightPos + glm::vec3(dirLight[2].x, dirLight[2].y, dirLight[2].z);
    *vMx = glm::lookAt(dirLightPos, dirLightFront, glm::vec3(0.0f,1.0f,0.0f));

    extractShadowFrustum();
}

const glm::mat4& Camera::getCurrentShadowVMatrix() const
{
    if(renMode == LQ)
    {
        return getShadowLQVMatrix();
    }
    return getShadowVMatrix();
}

const glm::mat4& Camera::getCurrentShadowPMatrix() const
{
    if(renMode == LQ)
    {
        return getShadowLQPMatrix();
    }
    return getShadowPMatrix();
}

bool Camera::sphereIsVisibleForShadow(const glm::vec4& sphere) const
{
    int p;
    for(p=0; p<6; p++)
        if( shadowFrustum[p][0] * sphere.x + shadowFrustum[p][1] * sphere.y + shadowFrustum[p][2] * sphere.z + shadowFrustum[p][3] <= -sphere.w)
            return false;
    return true;
}

bool Camera::sphereIsVisible(const glm::vec4& sphere) const//(glm::vec3 pos, float radius)
{
    int p;
    for(p=0; p<6; p++)
        if( frustum[p][0] * sphere.x + frustum[p][1] * sphere.y + frustum[p][2] * sphere.z + frustum[p][3] <= -sphere.w)
            return false;
    return true;
}

bool Camera::pointIsVisible(const glm::vec3& point) const
{
    int p;
    for(p=0; p<6; p++)
        if( frustum[p][0] * point.x + frustum[p][1] * point.y + frustum[p][2] * point.z + frustum[p][3] <= 0.0f )
            return false;
    return true;
}

bool Camera::aabboxIsVisible(const AABBox& box) const
{
    const glm::vec3& pointMin = box.getMin();
    const glm::vec3& pointMax = box.getMax();
    for (int i = 0; i < 6; ++i)
    {
      const float dp =
        (frustum[i][0]*(frustum[i][0] > 0.0f ? pointMax.x : pointMin.x)) +
        (frustum[i][1]*(frustum[i][1] > 0.0f ? pointMax.y : pointMin.y)) +
        (frustum[i][2]*(frustum[i][2] > 0.0f ? pointMax.z : pointMin.z));

      if (dp < -frustum[i][3]) { return false; }
    }
  return true;
}

bool Camera::aabboxIsVisibleForShadow(const AABBox& box) const
{
    const glm::vec3& pointMin = box.getMin();
    const glm::vec3& pointMax = box.getMax();
    for (int i = 0; i < 6; ++i)
    {
      const float dp =
        (shadowFrustum[i][0]*(shadowFrustum[i][0] > 0.0f ? pointMax.x : pointMin.x)) +
        (shadowFrustum[i][1]*(shadowFrustum[i][1] > 0.0f ? pointMax.y : pointMin.y)) +
        (shadowFrustum[i][2]*(shadowFrustum[i][2] > 0.0f ? pointMax.z : pointMin.z));

      if (dp < -shadowFrustum[i][3]) { return false; }
    }
  return true;
}

void Camera::extractShadowFrustum()
{
    float    t;

    glm::mat4 clip;
    switch(renMode)
    {
    case LQ:
        {
            clip = shadowLQProjMatrix * shadowLQVMatrix;
            break;
        }
    case HQ:
        {
            clip = shadowProjMatrix * shadowVMatrix;
            break;
        }
        default: { assert(false); }
    }

    // Extract the RIGHT clipping plane
    shadowFrustum[0][0] = clip[0][3] - clip[0][0];
    shadowFrustum[0][1] = clip[1][3] - clip[1][0];
    shadowFrustum[0][2] = clip[2][3] - clip[2][0];
    shadowFrustum[0][3] = clip[3][3] - clip[3][0];

    // Extract the LEFT clipping plane
    shadowFrustum[1][0] = clip[0][3] + clip[0][0];
    shadowFrustum[1][1] = clip[1][3] + clip[1][0];
    shadowFrustum[1][2] = clip[2][3] + clip[2][0];
    shadowFrustum[1][3] = clip[3][3] + clip[3][0];

    // Extract the BOTTOM clipping plane
    shadowFrustum[2][0] = clip[0][3] + clip[0][1];
    shadowFrustum[2][1] = clip[1][3] + clip[1][1];
    shadowFrustum[2][2] = clip[2][3] + clip[2][1];
    shadowFrustum[2][3] = clip[3][3] + clip[3][1];

    // Extract the TOP clipping plane
    shadowFrustum[3][0] = clip[0][3] - clip[0][1];
    shadowFrustum[3][1] = clip[1][3] - clip[1][1];
    shadowFrustum[3][2] = clip[2][3] - clip[2][1];
    shadowFrustum[3][3] = clip[3][3] - clip[3][1];

    // Extract the FAR clipping plane
    shadowFrustum[4][0] = clip[0][3] - clip[0][2];
    shadowFrustum[4][1] = clip[1][3] - clip[1][2];
    shadowFrustum[4][2] = clip[2][3] - clip[2][2];
    shadowFrustum[4][3] = clip[3][3] - clip[3][2];

    // Extract the NEAR clipping plane
    shadowFrustum[5][0] = clip[0][3] + clip[0][2];
    shadowFrustum[5][1] = clip[1][3] + clip[1][2];
    shadowFrustum[5][2] = clip[2][3] + clip[2][2];
    shadowFrustum[5][3] = clip[3][3] + clip[3][2];

    // Normalize it
    for(int i=0; i<6; ++i)
    {
        t = sqrtf( shadowFrustum[i][0] * shadowFrustum[i][0] + shadowFrustum[i][1] * shadowFrustum[i][1] + shadowFrustum[i][2] * shadowFrustum[i][2] );
        shadowFrustum[i][0] /= t;
        shadowFrustum[i][1] /= t;
        shadowFrustum[i][2] /= t;
        shadowFrustum[i][3] /= t;
    }
}

void Camera::extractFrustum()
{
    float    t;

    glm::mat4 clip = prjMx * viewMx;

    // Extract the RIGHT clipping plane
    frustum[0][0] = clip[0][3] - clip[0][0];
    frustum[0][1] = clip[1][3] - clip[1][0];
    frustum[0][2] = clip[2][3] - clip[2][0];
    frustum[0][3] = clip[3][3] - clip[3][0];

    // Extract the LEFT clipping plane
    frustum[1][0] = clip[0][3] + clip[0][0];
    frustum[1][1] = clip[1][3] + clip[1][0];
    frustum[1][2] = clip[2][3] + clip[2][0];
    frustum[1][3] = clip[3][3] + clip[3][0];

    // Extract the BOTTOM clipping plane
    frustum[2][0] = clip[0][3] + clip[0][1];
    frustum[2][1] = clip[1][3] + clip[1][1];
    frustum[2][2] = clip[2][3] + clip[2][1];
    frustum[2][3] = clip[3][3] + clip[3][1];

    // Extract the TOP clipping plane
    frustum[3][0] = clip[0][3] - clip[0][1];
    frustum[3][1] = clip[1][3] - clip[1][1];
    frustum[3][2] = clip[2][3] - clip[2][1];
    frustum[3][3] = clip[3][3] - clip[3][1];

    // Extract the FAR clipping plane
    frustum[4][0] = clip[0][3] - clip[0][2];
    frustum[4][1] = clip[1][3] - clip[1][2];
    frustum[4][2] = clip[2][3] - clip[2][2];
    frustum[4][3] = clip[3][3] - clip[3][2];

    // Extract the NEAR clipping plane
    frustum[5][0] = clip[0][3] + clip[0][2];
    frustum[5][1] = clip[1][3] + clip[1][2];
    frustum[5][2] = clip[2][3] + clip[2][2];
    frustum[5][3] = clip[3][3] + clip[3][2];

    // Normalize it
    for(int i=0; i<6; ++i)
    {
        t = sqrtf( frustum[i][0] * frustum[i][0] + frustum[i][1] * frustum[i][1] + frustum[i][2] * frustum[i][2] );
        frustum[i][0] /= t;
        frustum[i][1] /= t;
        frustum[i][2] /= t;
        frustum[i][3] /= t;
    }
}

void Camera::rotate(float h, float v)
{
    horizontalAngle += h;
    verticalAngle += v;
    updateViewAngles();
}

void Camera::setRotation(float h, float v)
{
    horizontalAngle = h;
    verticalAngle = v;
    updateViewAngles();
}

glm::vec2 Camera::getRotation()
{
    return glm::vec2(horizontalAngle, verticalAngle);
}

void Camera::moveFront(float dist)
{
    position += front * dist;
    updateViewMatrix();
}

void Camera::moveRight(float dist)
{
    position += right * dist;
    updateViewMatrix();
}

void Camera::moveUp(float dist)
{
    position += up * dist;
    updateViewMatrix();
}

void Camera::moveFree(glm::vec3 dist)
{
    position += front * dist.z;
    position += right * dist.x;
    position += up    * dist.y;
    updateViewMatrix();
}

void Camera::translate(glm::vec3 addpos)
{
    position += addpos;
    updateViewMatrix();
}

void Camera::setPosition(glm::vec3 pos)
{
    position = pos;
    updateViewMatrix();
}

glm::vec3 Camera::getPosition()
{
    return position;
}

void Camera::setOffset(glm::vec3 _offset)
{
    offset = glm::translate(-_offset);
    updateViewMatrix();
}

glm::vec3 Camera::getOffset()
{
    return glm::vec3(offset[3].x, offset[3].y, offset[3].z);
}

void Camera::updateViewAngles()
{
    if(verticalAngle > 3.14f*0.5f) verticalAngle = 3.14f*0.5f;
    else
    if(verticalAngle < -3.14f*0.5f) verticalAngle = -3.14f*0.5f;

    while(horizontalAngle > 3.14f*2.0f) horizontalAngle -= 3.14f*2.0f;
    while(horizontalAngle < 0.0f) horizontalAngle += 3.14f*2.0f;

    front = glm::vec3(
        cos(verticalAngle) * sin(horizontalAngle),
        sin(verticalAngle),
        cos(verticalAngle) * cos(horizontalAngle)
        );
    right = glm::vec3(
        sin(horizontalAngle - 3.14f/2.0f),
        0,
        cos(horizontalAngle - 3.14f/2.0f)
        );
    up = glm::cross( right, front );
    updateViewMatrix();
}

void Camera::updateViewMatrix()
{
    bcgMx = glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), front, up);
    viewMx = offset * glm::lookAt(position, position+front, up);
    extractFrustum();
    findSector();
}

void Camera::findSector()
{
    //assert(false);
    if(!pGfx) return;
}
