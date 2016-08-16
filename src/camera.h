#ifndef _CAMERA_H_
#define _CAMERA_H_
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <GL/glew.h>

class Sector;
struct AABBox;

class Camera
{
public:
    Camera();
    ~Camera();

    enum ShadowRenderMode
    {
        LQ = 0,
        HQ = 1
    };

    void                        setPerspective(float fovxdeg, float ratio, float near, float _far);

    inline const glm::mat4&     getProjectionMatrix() const { return prjMx; }
    inline const glm::mat4&     getViewMatrix() const { return viewMx; }
    inline const glm::mat4&     getBcgMatrix() const { return bcgMx; }

    inline Sector*              getSector() const { return mySector; }
    const glm::mat4&            getCurrentShadowVMatrix() const;
    const glm::mat4&            getCurrentShadowPMatrix() const;
    inline const glm::mat4&     getShadowVMatrix() const { return shadowVMatrix; }
    inline const glm::mat4&     getShadowLQVMatrix() const { return shadowLQVMatrix; }
    inline const glm::mat4&     getShadowPMatrix() const { return shadowProjMatrix; }
    inline const glm::mat4&     getShadowLQPMatrix() const { return shadowLQProjMatrix; }

    inline GLuint               getShadowTexture() const { return shadowTexture; }
    inline GLuint               getShadowLQTexture() const { return shadowTextureLQ; }
    inline float                getHQShadowBorder() const { return shadowCubeHSide*0.5f; }

    inline void                 setShadowRenderMode(ShadowRenderMode rm) { renMode = rm; }

    bool                        pointIsVisible              (const glm::vec3& point) const;
    bool                        sphereIsVisible             (const glm::vec4& sphere) const;//(glm::vec3 pos, float radius);
    bool                        sphereIsVisibleForShadow    (const glm::vec4& sphere) const;
    bool                        aabboxIsVisible             (const AABBox& box) const;//(const glm::vec3& pointMin, const glm::vec3& pointMax) const;
    bool                        aabboxIsVisibleForShadow    (const AABBox& box) const;//(const glm::vec3& pointMin, const glm::vec3& pointMax) const;

    void                        rotate(float h, float v);
    void                        setRotation(float h, float v);
    glm::vec2                   getRotation();

    void                        moveFront(float dist);
    void                        moveRight(float dist);
    void                        moveUp(float dist);
    void                        moveFree(glm::vec3 dist);
    void                        translate(glm::vec3 addpos);
    void                        setPosition(glm::vec3 pos);
    glm::vec3                   getPosition();
    inline glm::vec3            getFront() const { return front; }
    inline glm::vec3            getRight() const { return right; }
    inline glm::vec3            getUp()    const { return up;    }

    void                        setOffset(glm::vec3 _offset);
    glm::vec3                   getOffset();

    void                        preShadowRender();
    void                        postShadowRender() const;
    void                        setShadowViewMatrix(const glm::mat4 &dirLight);

private:
    void                        findSector();
    Sector*                     mySector;
    void*                       pGfx;

    void                        extractFrustum();
    void                        extractShadowFrustum();
    void                        updateViewAngles();
    void                        updateViewMatrix();

    void                        createFrameBuffersAndTextures();
    void                        getFbTexRes(GLuint*& fb, GLuint*& tex, GLsizei& res);

    float                       frustum[6][4];
    float                       shadowFrustum[6][4];
    float                       horizontalAngle;
    float                       verticalAngle;
    float                       far;
    glm::vec3                   position;
    glm::vec3                   lastPosition;
    glm::vec3                   front;
    glm::vec3                   right;
    glm::vec3                   up;
    glm::mat4                   prjMx;
    glm::mat4                   viewMx;
    glm::mat4                   bcgMx;
    glm::mat4                   offset;

    float                       shadowCubeHSide;
    float                       shadowCubeHSideHQ;

    glm::mat4                   shadowProjMatrix;
    glm::mat4                   shadowLQProjMatrix;

    glm::mat4                   shadowVMatrix;
    glm::mat4                   shadowLQVMatrix;

    GLuint                      shadowFramebuffer;
    GLuint                      shadowFramebufferLQ;

    GLuint                      shadowTexture;
    GLuint                      shadowTextureLQ;

    ShadowRenderMode            renMode;

    friend class WorldGraphics;
};

#endif
