#include <cassert>
#include "modellod.h"
#include "world_graphics.h"
#include "modelinstance.h"
#include "global.h"

ModelLOD::ModelLOD(rapidjson::Value &lod, WorldGraphics *wGfx) : pGfx(wGfx)
{
    assert(pGfx != nullptr);
    dist = static_cast<float>(lod["Distance"].GetDouble());
    rapidjson::Value &marr = lod["Models"];
    assert(marr.IsArray());
    for(auto it=marr.Begin(); it!=marr.End(); ++it)
    {
        rapidjson::Value &model = *it;
        std::string sMdl(""), sSha(""), sDif(""), sNor(""), sHei("");
        float normStr(1.0f), parrSca(0.03f), parrOff(0.0f);
        sMdl = model["Model"].GetString();
        sSha = model["Shader"].GetString();
        sDif = model["Diffuse"].GetString();
        sNor = model["Normal"].GetString();
        sHei = model["Height"].GetString();
        normStr = static_cast<float>(model["NormalStrength"].GetDouble());
        parrSca = static_cast<float>(model["ParallaxScale"].GetDouble());
        parrOff = static_cast<float>(model["ParallaxOffset"].GetDouble());

        rapidjson::Value &offset = model["Offset"];
        rapidjson::Value &rotation = model["Rotation"];
        rapidjson::Value &scale = model["Scale"];
        rapidjson::Value &uvTilingD = model["TilingDiffuse"];
        rapidjson::Value &uvTilingN = model["TilingNormal"];
        rapidjson::Value &uvTilingH = model["TilingHeight"];
        glm::vec3 offs = glm::vec3(offset[0].GetDouble(), offset[1].GetDouble(), offset[2].GetDouble());
        glm::vec3 rota = glm::vec3(rotation[0].GetDouble(), rotation[1].GetDouble(), rotation[2].GetDouble());
        glm::vec3 scal = glm::vec3(scale[0].GetDouble(), scale[1].GetDouble(), scale[2].GetDouble());

        glm::mat4 tmpMx1, tmpMx2, tmpMx3, relMx;
        tmpMx1 = glm::rotate(glm::radians(rota.x), glm::vec3(1,0,0));
        tmpMx2 = glm::rotate(glm::radians(rota.y), glm::vec3(0,1,0));
        tmpMx3 = glm::rotate(glm::radians(rota.z), glm::vec3(0,0,1));
        relMx = glm::translate(offs) * tmpMx3 * tmpMx2 * tmpMx1;

        auto md = pGfx->createModel(sSha, sMdl, sDif, sNor, sHei);
        md->offsetMatrix = relMx;
        md->setScale(scal);
        md->normalStrength = normStr;
        md->parallaxScale = parrSca;
        md->parallaxOffset = parrOff;
        md->UVTilingD = glm::vec2(uvTilingD[0].GetDouble(), uvTilingD[1].GetDouble());
        md->UVTilingN = glm::vec2(uvTilingN[0].GetDouble(), uvTilingN[1].GetDouble());
        md->UVTilingH = glm::vec2(uvTilingH[0].GetDouble(), uvTilingH[1].GetDouble());
        models.push_back(md);
    }
}

void ModelLOD::renderForShadow(glm::mat4 &modelMatrix)
{
    assert(pGfx!=nullptr);
    for(ModelInstance* &mi : models)
    {
        if( !mi->background && pGfx->camera.sphereIsVisibleForShadow(mi->getRenSphereAt(modelMatrix)))
        {
            mi->renderForShadow(modelMatrix, pGfx->camera, pGfx->shadowShader.get());
        }
    }
}

void ModelLOD::render(glm::mat4 &modelMatrix, std::size_t &shaderHash)
{
    assert(pGfx!=nullptr);
    for(ModelInstance* &mi : models)
    {
        if(mi->pShader->getHash() != shaderHash)
        {
            continue;
        }
        if((mi->background && !egg::getInstance().g_Editor) || pGfx->camera.sphereIsVisible(mi->getRenSphereAt(modelMatrix)))
        {
            mi->render(modelMatrix, pGfx->camera, pGfx->pickBestLights(mi->getRenSphereAt(modelMatrix)), pGfx->directionalLight);
        }
    }
}

ModelLOD::~ModelLOD()
{
    if(pGfx != nullptr)
    for(ModelInstance* &mi : models)
    {
        pGfx->deleteModel(mi);
    }
    models.clear();
}
