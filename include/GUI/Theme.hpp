#pragma once

#include <map>
#include "Defines.hpp"

class TiXmlNode;

namespace gui {

	class Theme 
	{
	public:
		Theme();
		Theme(const std::string& filename);
		~Theme();

		void LoadFromFile(const std::string& filename);
		void LoadFromXml(TiXmlNode* node);

		void SaveToFile(const std::string& filename);
		void SaveToXml(TiXmlNode* node) const;
		void Save();
		const std::string& GetFilePath() const;

		bool HasProperty(const std::string& property);

		sf::Color GetColor(const std::string& id) const;
		sf::Image* GetImage(const std::string& id) const;
		uint32 GetUserData(const std::string& id) const;

		bool AddImage(const std::string& id, const std::string& path);
		bool AddUserData(const std::string& id, uint32 value);
	private:
		typedef std::map<std::string,uint32> ColorMap;
		typedef std::map<std::string,sf::Image*> ImageMap;
		typedef std::map<std::string,std::pair<std::string,sf::Image*> >UserImageMap;
		
		std::string m_filepath;
		mutable ImageMap m_images;
		mutable ColorMap m_colors;

		//user data. won't get saved in the .sgt file, but rather in the xml //
		mutable ColorMap m_userUint32Data;
		mutable UserImageMap m_userImages;

		void _Load(TiXmlNode* node);
	};
}