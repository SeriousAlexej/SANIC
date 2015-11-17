#ifndef MODELLOD_H
#define MODELLOD_H
#include <vector>
#include <rapidjson/document.h>
#include <glm/mat4x4.hpp>

class WorldGraphics;
class ModelInstance;

class ModelLOD
{
    public:
        ModelLOD(rapidjson::Value &lod, WorldGraphics *wGfx);
        virtual ~ModelLOD();

        inline float distance() const { return dist; }
    private:
        void render(glm::mat4 &modelMatrix, std::size_t &shaderHash);
        void renderForShadow(glm::mat4 &modelMatrix);

        std::vector<ModelInstance*> models;
        float dist = 10.0f;
        WorldGraphics *pGfx = nullptr; //to request proper modelinstance creation/deletion

        friend class ModelSet;
};

#endif // MODELLOD_H
