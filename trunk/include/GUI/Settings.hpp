#pragma once

#include "Defines.hpp"
#include "Debug.hpp"
#include <map>
#include <tinyxml.h>

namespace gui {
	class Settings
	{
	public:
		Settings(void);
		Settings(const Settings& settings);
		~Settings(void);

		std::string GetStringValue(const std::string& id) const;
		int32 GetInt32Value(const std::string& id) const;
		uint32 GetUint32Value(const std::string& id) const;

		void SetUint32Value(const std::string& id, uint32 value);
		void SetInt32Value(const std::string& id, int32 value);
		void SetStringValue(const std::string& id, const std::string& value);

		bool HasUint32Value(const std::string& id);
		bool HasInt32Value(const std::string& id);
		bool HasStringValue(const std::string& id);

		void Dump(TiXmlNode* propertyElement) const;
		void Load(TiXmlNode* propertyElement);

		uint32 size() const;
	private:
		void _Load(TiXmlNode* propertyElement);
		std::map<std::string, uint32> m_uint32Values;
		std::map<std::string, int32> m_int32Values;
		std::map<std::string, std::string> m_stringValues;

	};

}