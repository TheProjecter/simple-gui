#include "../include/gui/Debug.hpp"
#include <iostream>
#include <stdarg.h>
#include <string>

std::ofstream gui::Debug::s_outdebug;
std::ofstream gui::Debug::s_outerror;
gui::Debug* gui::Debug::s_instance = NULL;
gui::Debug::FunctionsMap gui::Debug::s_functionTime;

namespace gui {

	Debug::Debug(void)
	{
		s_outdebug.open("debug.txt",std::ios::out);
		s_outerror.open("error.txt",std::ios::out);
	}

	Debug::~Debug(void)
	{
		if (s_outdebug.is_open()) {
			s_outdebug.close();
		}
		if (s_outerror.is_open()) {
			s_outerror.close();
		}
	}

	void Debug::outDebug(const char* format, ...)
	{
		if(!format) return;
		if(!s_outdebug.is_open()) {
			s_outdebug.open("debug.txt",std::ios::out);
		}
		va_list args;
		va_start(args, format);

		char buffer[256];
		vsnprintf(buffer,256,format, args);  //vsprintf_s

		std::string info = buffer;
	#ifdef _DEBUG
		std::cout << info << std::endl;
	#endif // _DEBUG
		s_outdebug << info << std::endl;
		s_outdebug.flush();

	}


	void Debug::outError(const char* format, ...)
	{
		if(!format) return;
		if(!s_outerror.is_open()) {
			s_outerror.open("error.txt",std::ios::out);
		}
		va_list args;
		va_start(args, format);

		char buffer[256];
		vsnprintf(buffer,256,format, args);  //vsprintf_s

		std::string info = buffer;

		std::cout << info << std::endl;

		s_outerror << info << std::endl;
		s_outerror.flush();

	}

	Debug& Debug::getInstance()
	{
		if(!s_instance) {
			s_instance = new Debug;
			return *s_instance;
		}
		return *s_instance;
	}

	gui::UnitTest::UnitTest( const std::string& functionName )
	{
		m_name = functionName;
		Debug::FunctionsMap::iterator it = Debug::s_functionTime.find(functionName);
		if(it == Debug::s_functionTime.end()) {
			Debug::s_functionTime[functionName] = 0;
		}
	}

	gui::UnitTest::~UnitTest()
	{
		float elapsed = m_clock.GetElapsedTime()*1000; //milliseconds
		Debug::FunctionsMap::iterator it = Debug::s_functionTime.find(m_name);
		if(it != Debug::s_functionTime.end()) {
			it->second += elapsed;
		}
	}

	void gui::Debug::outUnitTests()
	{
		float totalTime = 0.f;
		outDebug("===========+++SHOWING UNIT TESTING+++============");
		outDebug("=================================================");
		for(FunctionsMap::iterator it = s_functionTime.begin(); it != s_functionTime.end(); it++) {
			outDebug("Function: \"%s\" took: %.2f ms!", it->first.c_str(),it->second);
			totalTime += it->second;
		}
		outDebug("=================================================");
		outDebug("Total recorded function time elapsed: %.2f", totalTime);
		outDebug("=================================================");
		for(FunctionsMap::iterator it = s_functionTime.begin(); it != s_functionTime.end(); it++) {
			outDebug("Function: \"%s\" took: %.2f%% of time!", it->first.c_str(),(it->second/totalTime)*100);
		}
		outDebug("\n");
	}

}
