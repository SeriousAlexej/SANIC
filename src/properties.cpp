#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include "basic.h"
#include "world.h"
#include "properties.h"
#include "entitypointer.h"
#include "global.h"

#define if_type(X) if(typeid(X).hash_code() ==  m_tid)

/*
 * float
 * int
 * bool
 * glm::vec3
 * std::string
 * EntityPointer
 * FileName
 * Color
 */

void Property::Deserialize ( rapidjson::Value& d )
{
    rapidjson::Value& val = d;
    if_type(int) {
        int tmp = val.GetInt();
		this->SetValue(tmp);
    }
    else if_type(FileName) {
        std::string tmp = val.GetString();
        this->SetValue(FileName(tmp));
    }
    else if_type(EntityPointer) {
        int id = val.GetInt();
        this->SetValue(EntityPointer(egg::getInstance().g_World->GetEntityWithID(id)));
    }
    else if_type(bool) {
        bool tmp = val.GetBool();
        this->SetValue(tmp);
    }
    else if_type(float) {
		float tmp = val.GetDouble();
		this->SetValue(tmp);
	}
    else if_type(double) {
		double tmp = val.GetDouble();
		this->SetValue(tmp);
	}
    else if_type(std::string) {
		std::string tmp = val.GetString();
		this->SetValue(tmp);
	}
	else if_type(Color) {
        glm::vec3 tmp;
        tmp.x = val[0].GetDouble();
        tmp.y = val[1].GetDouble();
        tmp.z = val[2].GetDouble();
        this->SetValue(Color(tmp));
	}
    else if_type(glm::vec3) {
        glm::vec3 tmp;
        tmp.x = val[0].GetDouble();
        tmp.y = val[1].GetDouble();
        tmp.z = val[2].GetDouble();
        this->SetValue(tmp);
	}
    else {
        Serial* ser = static_cast<Serial*>(m_data);
        return ser->Deserialize(d);
    }
}

rapidjson::Value Property::Serialize(rapidjson::Document& d)
{
    rapidjson::Value val;
    if_type(int) {
        int i = *static_cast<int*>(m_data);
        val.SetInt(i);
    }
    else if_type(FileName) {
        std::string s = (static_cast<FileName*>(m_data))->path;
        val.SetString(s.c_str(), s.length(), d.GetAllocator());
    }
    else if_type(EntityPointer) {
        int id = (static_cast<EntityPointer*>(m_data))->GetCurrentID();
        val.SetInt(id);
    }
    else if_type(float) {
        float f = *static_cast<float*>(m_data);
        val.SetDouble((double)f);
    }
    else if_type(bool) {
        bool b = *static_cast<bool*>(m_data);
        val.SetBool(b);
    }
    else if_type(Color) {
        glm::vec3 v = static_cast<Color*>(m_data)->value;
        rapidjson::Value x, y, z;
        x.SetDouble(v.x);
        y.SetDouble(v.y);
        z.SetDouble(v.z);
        val.SetArray();
        val.PushBack(x, d.GetAllocator());
        val.PushBack(y, d.GetAllocator());
        val.PushBack(z, d.GetAllocator());
    }
    else if_type(glm::vec3) {
        glm::vec3 v = *static_cast<glm::vec3*>(m_data);
        rapidjson::Value x, y, z;
        x.SetDouble(v.x);
        y.SetDouble(v.y);
        z.SetDouble(v.z);
        val.SetArray();
        val.PushBack(x, d.GetAllocator());
        val.PushBack(y, d.GetAllocator());
        val.PushBack(z, d.GetAllocator());
    }
    else if_type(std::string) {
        std::string s = *static_cast<std::string*>(m_data);
        val.SetString(s.c_str(), s.length(), d.GetAllocator());
    }

    else {
    	Serial* ser = static_cast<Serial*>(m_data);
    	return ser->Serialize(d);
    }

    return val;
}
