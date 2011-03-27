#include "../include/gui/Theme.hpp"
#include "../include/gui/Debug.hpp"

#include <tinyxml.h>

#include <fstream>

namespace gui {

	Theme::Theme()
	{

	}

	Theme::Theme(const std::string& filename)
	{
		LoadFromFile(filename);
	}

	void Theme::LoadFromFile( const std::string& filename )
	{
		std::ifstream is;
		is.open(filename.c_str(),std::ios::in | std::ios::binary);
		if(!is.is_open()) {
			debug_log("Couldn't open theme.sgt!");
			return;
		}
		char buffer[128];
		uint32 temp;
		if(!is.read(buffer,128)) {
			debug_log("Invalid file format?");
			return;
		} else {
			if(strcmp(buffer, "Simple Gui Theme") != 0) {
				debug_log("File theme.sgt is of invalid format!");
				return;
			} else {
				//file is ok to read and has valid format!
				if(!is.read((char*)&temp, sizeof(uint32))) {
					debug_log("Weird format error in theme.sgt?");
					return;
				} else {
					unsigned color = 0;
					std::string name;
					for(unsigned i=0; i<temp; i++) {
						//get the name
						is.read(buffer,128);
						name = buffer;

						//get the color
						is.read((char*)&color, sizeof(uint32));
						m_colors[name] = color;
					}
				}
			}
		}
		m_filepath = filename;
	}

	void Theme::SaveToFile( const std::string& filename )
	{
		std::ofstream os;
		os.open(filename.c_str(), std::ios::out | std::ios::binary);
		if(!os.is_open()) {
			debug_log("Couldn't open the file theme.sgt!");
			return;
		}
		//to make sure it's a .sgt file
		os.write("Simple Gui Theme",128);

		//get the colors count
		uint32 color_count = m_colors.size();

		//save the colors count
		os.write((char*)&color_count,sizeof(uint32));

		char buffer[128];
		uint32 color = 0;

		//save the colors and their names
		for(ColorMap::iterator it = m_colors.begin();it != m_colors.end(); it++)
		{
			//get the color
			color = it->second;

			//use 128 bytes max to store the name
			strcpy_s(buffer, it->first.c_str());

			//save the name
			os.write(buffer,128);

			//save the color
			os.write((char*)&color,sizeof(uint32));
		}
		//save the images and their names
	}

	bool Theme::HasProperty( const std::string& property )
	{
		if(m_colors.find(property) != m_colors.end()) return true;
		if(m_images.find(property) != m_images.end()) return true;

		return false;
	}

	sf::Color Theme::GetColor( const std::string& id ) const
	{
		ColorMap::const_iterator it = m_colors.find(id);
		if(it == m_colors.end()) {
			debug_log("Couldn't find property with id \"%s\"",id.c_str());
			return sf::Color(255,255,255);
		}
		return UnsignedToColor(it->second);
	}

	//If a user image has the same name as a theme image, the user has priority
	sf::Image* Theme::GetImage( const std::string& id ) const
	{
		UserImageMap::const_iterator it = m_userImages.find(id);
		if(it == m_userImages.end()) {
			ImageMap::const_iterator i = m_images.find(id);

			if(i == m_images.end()) {
				debug_log("Couldn't find property with id \"%s\"",id.c_str());
				return NULL;
			} 
			return i->second;
		}
		return it->second.second;	
	}

	void Theme::Save()
	{
		if(m_filepath.size())
			SaveToFile(m_filepath);
	}

	const std::string& Theme::GetFilePath() const
	{
		return m_filepath;
	}

	void Theme::LoadFromXml( TiXmlNode* node )
	{
		//free memory allocated. MIGHT CAUSE CRASHING if you still use the old pointers
		for(UserImageMap::iterator it = m_userImages.begin(); it != m_userImages.end(); it++) {
			delete it->second.second;
		}

		m_userImages.clear();
		m_userUint32Data.clear();

		_Load(node);

	}

	void Theme::SaveToXml( TiXmlNode* node ) const
	{
		if(!node) return;

		//only save user data
		TiXmlElement* e = NULL;
		TiXmlComment* c = new TiXmlComment("user-defined data");
		for(UserImageMap::const_iterator it = m_userImages.begin();
			it != m_userImages.end(); it++) 
		{
			e = new TiXmlElement("image");

			e->SetAttribute("id",it->first.c_str());
			e->SetAttribute("path",it->second.first.c_str());

			node->LinkEndChild(e);
		}

		for(ColorMap::const_iterator it = m_userUint32Data.begin();
			it != m_userUint32Data.end(); it ++) 
		{
			e = new TiXmlElement("uint");

			e->SetAttribute("id",it->first.c_str());
			e->SetAttribute("value", it->second);

			node->LinkEndChild(e);
		}
		
	}

	void Theme::_Load( TiXmlNode* pParent )
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
				if(propertyType == "image") {
					std::string valueId; std::string value;
					while (pAttrib) 
					{
						if(strcmp(pAttrib->Name(),"id") == 0)
							valueId = pAttrib->Value();
						else if(strcmp(pAttrib->Name(),"path") == 0) {
							value = pAttrib->Value();
						} else {
							debug_log("Unhandled attribute(\"%s\") when loading property!", pAttrib->Name());
						}
						pAttrib = pAttrib->Next();
					}
					sf::Image* img = new sf::Image();
					if(!img->LoadFromFile(value)) {
						error_log("Unable to load image with id=%s from path=\"%s\"",valueId.c_str(),value.c_str());
						delete img;
						return;
					}

					m_userImages[valueId] = std::make_pair<std::string,sf::Image*>(value,img);
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
					m_userUint32Data[valueId] = (uint32)value;
				}
			} break;

		default:	break;
		}

		for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSiblingElement()) 
		{
			_Load( pChild);
		}
	}

	gui::uint32 Theme::GetUserData( const std::string& id ) const
	{
		ColorMap::const_iterator it = m_userUint32Data.find(id);
		if(it == m_userUint32Data.end()) {
			debug_log("Couldn't find user data with id \"%s\"!",id.c_str());
			return 0;
		} 

		return it->second;
	}

	Theme::~Theme()
	{
		for(UserImageMap::iterator it = m_userImages.begin(); 
			it != m_userImages.end(); it++) 
		{
			delete it->second.second;
		}
	}

	bool Theme::AddImage( const std::string& id, const std::string& path )
	{
		if(m_userImages.find(id) != m_userImages.end()) 
		{
			error_log("There's already a image registered with that id! \"%s\"", id.c_str());
			return false;
		}
		if(m_images.find(id) != m_images.end()) {
			debug_log("User image with id (\"%s\") replaces the one in the theme!");
		}
		

		sf::Image* img = new sf::Image();
		if(!img->LoadFromFile(path)) {
			error_log("Failed to load image with id=%s from path=\"%s\"",id.c_str(),path.c_str());
			delete img;
			return false;
		}
		m_userImages[id] = std::make_pair<std::string,sf::Image*>(path,img);

		return true;
	}

	bool Theme::AddUserData( const std::string& id, uint32 value )
	{
		if(m_userUint32Data.find(id) == m_userUint32Data.end()) {
			error_log("UserData with id=\"%s\" has already been defined!", id.c_str());
			return false;
		}

		m_userUint32Data[id] = value;
		return true;
	}
}

