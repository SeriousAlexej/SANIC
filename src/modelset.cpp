#include "modelset.h"
#include "mesh.h"
#include "global.h"
#include "world_graphics.h"
#include "dirtools.h"
#include <rapidjson/document.h>
#include <fstream>
#include <algorithm>

ModelSet::ModelSet(std::string path, WorldGraphics *wGfx) : pGfx(wGfx)
{
    path = egg::getInstance().g_WorkingDir + path.substr(1);
	std::replace(path.begin(), path.end(), '\\', '/');
	if(!DirectoryExists(path)) //it's a file, but who cares
    {
        path = egg::getInstance().g_WorkingDir + "/models/default.mconf";
        std::replace(path.begin(), path.end(), '\\', '/');
    }
	std::ifstream file(path);
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    rapidjson::Document doc;
    doc.Parse(content.c_str());

    std::string ctp = doc["CollisionType"].GetString();
    if(ctp == "mesh")
    {
        collision = CT_MESH;
        collisionMesh = new Mesh(doc["CollisionMesh"].GetString());
    } else
    if(ctp == "sphere")
    {
        collision = CT_SPHERE;
        collisionRadius = static_cast<float>(doc["CollisionRadius"].GetDouble());
    } else
    if(ctp == "box")
    {
        collision = CT_BOX;
        rapidjson::Value &hbx = doc["CollisionHalfBox"];
        collisionHalfBox = glm::vec3(hbx[0].GetDouble(), hbx[1].GetDouble(), hbx[2].GetDouble());
    }
    rapidjson::Value &cboff = doc["CollisionOffset"];
    collisionOffset = glm::vec3(cboff[0].GetDouble(), cboff[1].GetDouble(), cboff[2].GetDouble());

    rapidjson::Value &lds = doc["LODS"];
    assert(lds.IsArray());
    for(auto it=lds.Begin(); it!=lds.End(); ++it)
    {
        lods.push_back(std::make_unique<ModelLOD>(*it, pGfx));
    }
    std::sort(lods.begin(), lods.end(),
              [](std::unique_ptr<ModelLOD> &i, std::unique_ptr<ModelLOD> &j)
              { return j->distance() > i->distance(); }
              );
}

void ModelSet::setBackground(bool bcg)
{
    background = bcg;
    for(auto &lod : lods)
    {
        for(auto &mi : lod->models)
        {
            mi->background = bcg;
        }
    }
}

void ModelSet::renderForShadow()
{
    lods[currLOD]->renderForShadow(mdlMx);
}

void ModelSet::render(std::size_t shaderHash)
{
    lods[currLOD]->render(mdlMx, shaderHash);
}

void ModelSet::findVisibleLOD()
{
    assert(pGfx!=nullptr);
    float distance = glm::distance(pGfx->camera.getPosition(), getPosition());
    for(int i=lods.size()-1; i>=0; i--)
    {
        if(lods[i]->distance() < distance)
        {
            if(i+1 < lods.size())
            {
                currLOD = static_cast<unsigned>(i+1);
            }
            break;
        }
        else
        if(i==0 && lods[i]->distance() >= distance)
        {
            currLOD = 0u;
        }
    }
}

ModelSet::~ModelSet()
{
    lods.clear();
    if(collisionMesh)
    {
        delete collisionMesh;
    }
}

void ModelSet::playAnimation(std::string anim)
{
    for(auto &lod : lods)
    {
        for(auto &mi : lod->models)
        {
            mi->playAnimation(anim);
        }
    }
}

void ModelSet::stopAnimations()
{
    for(auto &lod : lods)
    {
        for(auto &mi : lod->models)
        {
            mi->stopAnimation();
        }
    }
}

void ModelSet::activate()
{
    for(auto &lod : lods)
    {
        for(auto &mi : lod->models)
        {
            mi->activate();
        }
    }
}

void ModelSet::deactivate()
{
    for(auto &lod : lods)
    {
        for(auto &mi : lod->models)
        {
            mi->deactivate();
        }
    }
}
