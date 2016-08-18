#ifndef SERIAL_H_INCLUDED
#define SERIAL_H_INCLUDED

/**
 * @brief Objects of this class can be (de)serialized into JSON
 *
 */
class Serial {
public:
	/**
	 * @brief Deserializes object from JSON value
	 *
	 * @param value JSON value
	 * @return void
	 */
	virtual void Deserialize(rapidjson::Value& value) = 0;
	/**
	 * @brief Serializes class into JSON objects
	 *
	 * @param document Document object, need for encoding and stuff
	 * @return rapidjson::Value
	 */
	virtual rapidjson::Value Serialize(rapidjson::Document& document) = 0;

    virtual ~Serial() {}
};

#endif // SERIAL_H_INCLUDED
