#ifndef MODELSET_H
#define MODELSET_H
#include <basic.h>
#include <modellod.h>
#include <memory>
#include <vector>
#include <string>

class WorldGraphics;
class Mesh;

class ModelSet : public Movable
{
    public:
        ModelSet(std::string path, WorldGraphics *wGfx);
        virtual ~ModelSet();

        void activate();
        void deactivate();
        void playAnimation(std::string anim);
        void stopAnimations();
        void setBackground(bool bcg);
        bool isBackground() const { return background; }
        void setUseEditorShader(bool use);

    private:
//        void render(std::size_t shaderHash);
//        void renderForShadow();
        void findVisibleLOD();

        std::vector<std::unique_ptr<ModelLOD>> lods;
        WorldGraphics *pGfx = nullptr;
        unsigned currLOD = 0u;

        enum CollType
        {
            CT_MESH,
            CT_BOX,
            CT_SPHERE
        };
        CollType    collision = CT_SPHERE;
        Mesh*       collisionMesh = nullptr;
        glm::vec3   collisionHalfBox = glm::vec3(0.5f,0.5f,0.5f);
        float       collisionRadius = 0.5f;
        glm::vec3   collisionOffset = glm::vec3(0.0f,0.0f,0.0f);
        bool background = false;

        friend class Entity;
        friend class WorldGraphics;
};

#endif // MODELSET_H
