#include "../include/gui/XmlParser.hpp"

namespace gui
{

	void XmlParser::IterateTags( TiXmlNode* pParent )
	{
		if ( !pParent ) return;

		TiXmlNode* pChild;
		int t = pParent->Type();

		switch ( t )
		{
		case TiXmlNode::TINYXML_ELEMENT:
			OnXmlElement(pParent->ToElement());
			break;
		case TiXmlNode::TINYXML_TEXT:
			OnXmlText(pParent->ToText());
			break;

		default:
			break;
		}
		for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) 
		{
			IterateTags( pChild);
		}
	}

	void XmlParser::OnXmlElement( TiXmlElement* pElement )
	{
		if ( !pElement ) return;

		TiXmlAttribute* pAttrib = pElement->FirstAttribute();

		while (pAttrib)
		{
			OnXmlAttribute(pElement, pAttrib);
			pAttrib=pAttrib->Next();
		}
	}

	void XmlParser::OnXmlAttribute( TiXmlElement* element, TiXmlAttribute* attrib )
	{

	}

	void gui::XmlParser::ParseDocument( const char* filename )
	{
		TiXmlDocument doc;
		doc.LoadFile(filename);
		IterateTags(&doc);
	}

	void gui::XmlParser::OnXmlText( TiXmlText* text )
	{

	}

}
