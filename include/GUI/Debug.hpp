#pragma once

#include <fstream>
#include <map>
#include "../include/gui/Defines.hpp"

namespace gui {

	class Debug
	{
	public:
	friend class UnitTest;
		Debug(void);
		~Debug(void);
		static void outDebug(const char* text, ...);
		static void outError(const char* text, ...);
		static void outUnitTests();
		static Debug& getInstance();
	private:
		static Debug* s_instance;
		static std::ofstream s_outdebug;
		static std::ofstream s_outerror;
		typedef std::map<std::string, float> FunctionsMap;
		static FunctionsMap s_functionTime;
	};

	/* Dummy class for handling unit testing */
	//TODO: Improve unit testing..
	class UnitTest {
	public:
		UnitTest(const std::string&	functionName);
		~UnitTest();
	private:
		std::string m_name;
		sf::Clock m_clock;
	};

	#define sLog Debug::getInstance()
	#define debug_log sLog.outDebug("Line: %u | Debug: %s() said: ",__LINE__,__FUNCTION__);\
		sLog.outDebug
	#define error_log sLog.outError("Line: %u | ERROR: %s() said: ",__LINE__, __FUNCTION__);\
		sLog.outError

	#define UNIT_TEST UnitTest __unit_test__(__FUNCTION__);

}