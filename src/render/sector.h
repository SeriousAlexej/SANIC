#ifndef SECTOR_H_INCLUDED
#define SECTOR_H_INCLUDED
#include <vector>

class Portal;
class ModelInstance;
class Light;

struct AABBox
{
public:
    AABBox() : vMIN({0.0f, 0.0f, 0.0f}), vMAX({1.0f, 1.0f, 1.0f}) {}
    AABBox(const glm::vec3& vmin, const glm::vec3& vmax) : vMIN(vmin), vMAX(vmax) { Check(); }

    inline const glm::vec3& getMin() const { return vMIN; }
    inline const glm::vec3& getMax() const { return vMAX; }
    inline glm::vec3 getCenter() const
    {
        return (vMIN + vMAX) * 0.5f;
    }
    inline glm::vec3 getHalfWidth() const
    {
        return (vMAX - vMIN) * 0.5f;
    }

    void setMin(const glm::vec3& vmin)
    {
        vMIN = vmin;
        Check();
    }

    void setMax(const glm::vec3& vmax)
    {
        vMAX = vmax;
        Check();
    }

    static bool collides(const AABBox &a, const AABBox &b)
    {
        const glm::vec3 ac = a.getCenter();
        const glm::vec3 bc = b.getCenter();
        const glm::vec3 ar = a.getHalfWidth();
        const glm::vec3 br = b.getHalfWidth();
        if (fabs(ac[0] - bc[0]) > (ar[0] + br[0])) return false;
        if (fabs(ac[1] - bc[1]) > (ar[1] + br[1])) return false;
        if (fabs(ac[2] - bc[2]) > (ar[2] + br[2])) return false;

        return true;
    }

    static AABBox intersection(const AABBox &a, const AABBox &b)
    {
        AABBox c;

        c.vMIN.x = glm::max(a.vMIN.x, b.vMIN.x);
        c.vMIN.y = glm::max(a.vMIN.y, b.vMIN.y);
        c.vMIN.z = glm::max(a.vMIN.z, b.vMIN.z);

        c.vMAX.x = glm::min(a.vMAX.x, b.vMAX.x);
        c.vMAX.y = glm::min(a.vMAX.y, b.vMAX.y);
        c.vMAX.z = glm::min(a.vMAX.z, b.vMAX.z);

        return c;
    }

private:
    void Check()
    {
        if(vMIN.x > vMAX.x) {
            std::swap(vMIN.x, vMAX.x);
        }
        if(vMIN.y > vMAX.y) {
            std::swap(vMIN.y, vMAX.y);
        }
        if(vMIN.z > vMAX.z) {
            std::swap(vMIN.z, vMAX.z);
        }
    }

    glm::vec3 vMIN;
    glm::vec3 vMAX;
};

class Sector
{
public:
    Sector(const glm::vec3& vmin, const glm::vec3& vmax)
           : bbox(vmin, vmax) {}

    inline const AABBox& getBBox() const { return bbox; }
    inline const std::vector<Portal>& getPortals() const { return portals; }
    inline const std::vector<ModelInstance*>& getModels() const { return models; }
    inline const std::vector<Light*>& getLights() const { return lights; }

    void addPortal(const Portal& p) { portals.push_back(p); }
    void addModel(ModelInstance* m) { models.push_back(m); }
    void addLight(Light* l) { lights.push_back(l); }

private:
    AABBox                      bbox;
    std::vector<Portal>         portals;
    std::vector<ModelInstance*> models;
    std::vector<Light*>         lights;
};

class Portal
{
public:
    Portal(const AABBox& bbx, const Sector* s1, const Sector* s2)
           : sector1(s1), sector2(s2), bbox(bbx) {}

    inline const AABBox& getBBox() const { return bbox; }
    inline const Sector* otherSector(const Sector* s) const { return (s == sector1 ? sector2 : sector1); }

private:
    const Sector*   sector1;
    const Sector*   sector2;
    AABBox          bbox;
};

#endif // SECTOR_H_INCLUDED
