#pragma once

#include <tinyxml.h>

namespace gui
{
	class XmlParser
	{
	public:
		void ParseString(const char* text);
		void ParseDocument(const char* filename);

	protected:
		virtual void IterateTags(TiXmlNode* node);
		virtual void OnXmlText(TiXmlText* text);
		virtual void OnXmlElement(TiXmlElement* node);
		virtual void OnXmlAttribute(TiXmlElement* element, TiXmlAttribute* attrib);
	};

}
