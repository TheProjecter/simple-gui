#include "../include/gui/Settings.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

namespace gui {
	Settings::Settings(void)
	{
	}


	Settings::Settings( const Settings& settings )
	{
		m_stringValues.clear();
		m_int32Values.clear();

		m_stringValues = settings.m_stringValues;
		m_int32Values = settings.m_int32Values;
	}
	Settings::~Settings(void)
	{
	}

	std::string Settings::GetStringValue( const std::string& index ) const
	{
		std::map<std::string,std::string>::const_iterator i = m_stringValues.find(index);
		if(i == m_stringValues.end()) {
			return "";
		} else return i->second;
	}

	void Settings::SetStringValue( const std::string& id, const std::string& value )
	{
		m_stringValues[id] = value;
	}


	bool Settings::HasStringValue( const std::string& id )
	{
		std::map<std::string,std::string>::const_iterator i = m_stringValues.find(id);
		if(i == m_stringValues.end()) return false;

		return true;
	}

	void Settings::Dump( TiXmlNode* propertyElement ) const
	{
		if(!propertyElement) return;

		//add int32 values
		for(std::map<std::string,int32>::const_iterator it = m_int32Values.begin(); it != m_int32Values.end(); it++) 
		{
			TiXmlElement* e = new TiXmlElement("int");
			e->SetAttribute("id",it->first.c_str());
			e->SetAttribute("value", (int)it->second);
			propertyElement->LinkEndChild(e);
		}

		//add uint32 values
		for(std::map<std::string,uint32>::const_iterator it = m_uint32Values.begin(); it != m_uint32Values.end(); it++) 
		{
			TiXmlElement* e = new TiXmlElement("uint");
			e->SetAttribute("id",it->first.c_str());
			e->SetAttribute("value", (int)it->second);
			propertyElement->LinkEndChild(e);
		}

		//add string values
		for(std::map<std::string,std::string>::const_iterator it = m_stringValues.begin(); it != m_stringValues.end(); it++) 
		{
			TiXmlElement* e = new TiXmlElement("string");
			e->SetAttribute("id",it->first.c_str());
			e->SetAttribute("value", it->second.c_str());
			propertyElement->LinkEndChild(e);
		}
	}

	bool Settings::HasInt32Value( const std::string& id )
	{
		std::map<std::string,int32>::const_iterator i = m_int32Values.find(id);
		if(i == m_int32Values.end()) return false;

		return true;
	}

	gui::int32 Settings::GetInt32Value( const std::string& id ) const
	{
		std::map<std::string,int32>::const_iterator i = m_int32Values.find(id);
		if(i == m_int32Values.end()) return -1;

		return i->second;
	}

	void Settings::SetInt32Value( const std::string& id, int32 value )
	{
		m_int32Values[id] = value;
	}

	void Settings::Load( TiXmlNode* pParent )
	{
		if(!pParent) return;

		//clear previous settings;
		m_int32Values.clear();
		m_uint32Values.clear();
		m_stringValues.clear();

		_Load(pParent);
	}

	gui::uint32 Settings::size() const
	{
		return m_int32Values.size() + m_uint32Values.size() + m_stringValues.size();
	}

	void Settings::_Load( TiXmlNode* pParent )
	{
		if ( !pParent ) return;

		TiXmlNode* pChild;
		int t = pParent->Type();

		switch ( t )
		{
		case TiXmlNode::TINYXML_ELEMENT:
			{
				TiXmlAttribute* pAttrib = pParent->ToElement()->FirstAttribute();
				std::string propertyType = pParent->Value();


				//check all attributes
				if(propertyType == "int") {
					std::string valueId; int value = 0;
					while (pAttrib) 
					{
						if(strcmp(pAttrib->Name(),"id") == 0)
							valueId = pAttrib->Value();
						else if(strcmp(pAttrib->Name(),"value") == 0) {
							pAttrib->QueryIntValue(&value);
						} else {
							debug_log("Unhandled attribute(\"%s\") when loading property!", pAttrib->Name());
						}
						pAttrib = pAttrib->Next();
					}
					m_int32Values[valueId] = value;
				} else if(propertyType == "uint") {
					std::string valueId; int value = 0;
					while (pAttrib) 
					{
						if(strcmp(pAttrib->Name(),"id") == 0)
							valueId = pAttrib->Value();
						else if(strcmp(pAttrib->Name(),"value") == 0) {
							pAttrib->QueryIntValue(&value);
						} else {
							debug_log("Unhandled attribute(\"%s\") when loading property!", pAttrib->Name());
						}
						pAttrib = pAttrib->Next();
					}
					m_uint32Values[valueId] = (uint32)value;
				} else if(propertyType == "string") {
					std::string valueId; std::string value = "";
					while (pAttrib) 
					{
						if(strcmp(pAttrib->Name(),"id") == 0)
							valueId = pAttrib->Value();
						else if(strcmp(pAttrib->Name(),"value") == 0) {
							value = pAttrib->Value();
						} else {
							debug_log("Unhandled attribute(\"%s\") when loading property!", pAttrib->Name());
						}
						pAttrib = pAttrib->Next();
					}
					m_stringValues[valueId] = value;
				} else {
					//debug_log("Unhandled element received when parsing %s",pParent->Value());
				}
			} break;

		default:	break;
		}

		for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSiblingElement()) 
		{
			_Load( pChild);
		}

	}

	bool Settings::HasUint32Value( const std::string& id )
	{
		return (m_uint32Values.find(id) != m_uint32Values.end());
	}

	gui::uint32 Settings::GetUint32Value( const std::string& id ) const
	{
		std::map<std::string, uint32>::const_iterator it;
		it = m_uint32Values.find(id);
		if(it == m_uint32Values.end()) {
			debug_log("Couldn't retrieve uint32 value with id = %s", id.c_str());
			return 0;
		} 
		
		return it->second;
		
	}

	void Settings::SetUint32Value( const std::string& id, uint32 value )
	{
		m_uint32Values[id] = value;
	}
}
