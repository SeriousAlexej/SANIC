#ifndef _MODELINSTANCE_H_
#define _MODELINSTANCE_H_
#include <SFML/System.hpp>
#include "basic.h"
#include "camera.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "light.h"

class ModelSet;

class ModelInstance : public Movable
{
public:

    ModelInstance(Mesh* mesh, Shader* shader, Texture* diffuse, Texture* normal, Texture* height);
    virtual ~ModelInstance();

    glm::vec4       getRenSphere() const;

    virtual glm::vec3 getPosition() const override;

    void            activate() { active = true; }
    void            deactivate() { active = false; }

    bool            castShadow;
    bool            background;
    float           normalStrength;
    float           parallaxScale;
    float           parallaxOffset;
    bool            useEditorShader;

    inline bool     overridesShadowShader() const { return pOverridenShadowShader != nullptr; }
    inline bool     isTranslucent() const { return translucent; }
    void            setAlpha(float a) { translucencyAlpha = glm::clamp(a, 0.0f, 1.0f); }
    inline float    getAlpha() const { return translucencyAlpha; }

private:
    void            render(Camera& cam, const std::vector<Light*>& lights, Light* dirLight);
    void            renderForShadow(Camera& cam, Shader* shader);

    void            setOverridenShadowShader(Shader* s);
    void            playAnimation(const std::string &anim);
    void            stopAnimation();

    Mesh*           pMesh;
    Shader*         pShader;
    Shader*         pOverridenShadowShader;
    Texture*        pDiffTexture;
    Texture*        pNormTexture;
    Texture*        pHeightTexture;

    glm::vec2       UVTilingD = glm::vec2(1.0f,1.0f);
    glm::vec2       UVTilingN = glm::vec2(1.0f,1.0f);
    glm::vec2       UVTilingH = glm::vec2(1.0f,1.0f);
    float           translucencyAlpha = 1.0f;
    bool            translucent;
    bool            useImageAlpha;
    bool            visible;
    bool            active;
    float           lastRender;
    std::string     strCurrAnim;
    unsigned        uCurrFrame;
    unsigned        uNextFrame;
    AnimInfo        animInfo; //for convenience
    ModelSet*       myModelSet = nullptr;

    glm::mat4       offsetMatrix = glm::mat4(1);

    friend class WorldGraphics;
    friend class ModelSet;
    friend class ModelLOD;
    friend class SolidBody;
};

#endif
