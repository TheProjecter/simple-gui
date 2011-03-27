#include <iostream>
#include "tinyxml.h"

class TextParser 
{
public:
	TextParser(): m_styleChanged(false) {}

	void Parse(const char* filename)
	{
		doc.LoadFile(filename);
		dump_to_stdout(&doc);
	}


private:
	void dump_to_stdout( TiXmlNode* pParent )
	{
		if ( !pParent ) return;

		TiXmlNode* pChild;
		TiXmlText* pText;
		int t = pParent->Type();

		switch ( t )
		{
		case TiXmlNode::TINYXML_DOCUMENT:
			break;

		case TiXmlNode::TINYXML_ELEMENT:
			dump_attribs_to_stdout(pParent->ToElement());
			break;

		case TiXmlNode::TINYXML_COMMENT:
			break;

		case TiXmlNode::TINYXML_UNKNOWN:
			printf( "Unknown" );
			break;

		case TiXmlNode::TINYXML_TEXT:
			pText = pParent->ToText();
			if(m_styleChanged) {printf("<b> "); m_styleChanged = false; }

			printf( "%s", pText->Value() );
			break;

		case TiXmlNode::TINYXML_DECLARATION:
			break;
		default:
			break;
		}
		for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) 
		{
			dump_to_stdout( pChild);
		}
	}

	// print all attributes of pElement.
	// returns the number of attributes printed
	void dump_attribs_to_stdout(TiXmlElement* pElement)
	{
		if ( !pElement ) return;

		TiXmlAttribute* pAttrib=pElement->FirstAttribute();

		if(strcmp(pElement->Value(),"b") == 0) 
			m_styleChanged = true;
		

	}

	bool m_styleChanged;
	TiXmlDocument doc;

};

int main()
{
	TextParser t;
	t.Parse("demotest.xml");

	return 0;
}

