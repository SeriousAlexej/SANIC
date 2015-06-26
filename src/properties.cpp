#include "properties.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

/*
 * float
 * int
 * bool
 * glm::vec3
 * std::string
 */

void Property::Deserialize ( rapidjson::Document& d )
{
    rapidjson::Value val;
    if_type(Serial) {
        Serial* ser = reinterpret_cast<Serial*>(m_data);
        return ser->Deserialize(d);
    }
    if_type(int) {
        int tmp = val.GetInt();
		this->SetValue(tmp);
    }
	if_type(float) {
		float tmp = val.GetDouble();
		this->SetValue(tmp);
	}	
	if_type(double) {
		double tmp = val.GetDouble();
		this->SetValue(tmp);
	}
	if_type(std::string) {
		std::string tmp = val.GetString();
		this->SetValue(tmp);
	}
}

rapidjson::Value Property::Serialize(rapidjson::Document& d)
{
    rapidjson::Value val;
    if_type(Serial) {
        Serial* ser = reinterpret_cast<Serial*>(m_data);
        return ser->Serialize(d);
    }
    else if_type(int) {
        int i = *reinterpret_cast<int*>(m_data);
        val.SetInt(i);
    }
    else if_type(float) {
        float f = *reinterpret_cast<float*>(m_data);
        val.SetDouble((double)f);
    }
    else if_type(bool) {
        bool b = *reinterpret_cast<bool*>(m_data);
    }
    else if_type(glm::vec3) {
        glm::vec3 v = *reinterpret_cast<glm::vec3*>(m_data);
        rapidjson::Value x, y, z;
        x.SetDouble(v.x);
        y.SetDouble(v.y);
        z.SetDouble(v.z);
        val.SetArray();
        val.PushBack(x, d.GetAllocator());
        val.PushBack(y, d.GetAllocator());
        val.PushBack(z, d.GetAllocator());
    }
    else if_type(glm::quat) {
        glm::quat q = *reinterpret_cast<glm::quat*>(m_data);
        rapidjson::Value x, y, z, w;
        x.SetDouble(q.x);
        y.SetDouble(q.y);
        z.SetDouble(q.z);
        w.SetDouble(q.w);
        val.SetArray();
        val.PushBack(x, d.GetAllocator());
        val.PushBack(y, d.GetAllocator());
        val.PushBack(z, d.GetAllocator());
        val.PushBack(w, d.GetAllocator());
    }
    else if_type(std::string) {
        std::string s = *reinterpret_cast<std::string*>(m_data);
        val.SetString(s.c_str(), s.length());
    }
    
    else val.SetNull();

    return val;
}
