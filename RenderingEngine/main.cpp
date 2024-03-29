//Custom Header
#include "engine.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>  
#include <crtdbg.h>

#include <memory>

std::shared_ptr<tre::Engine> pEngine;

int main()
{
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtMemState s1;
	_CrtMemState s2;
	_CrtMemState s3;

	_CrtMemCheckpoint(&s1);
	{
		pEngine = std::make_shared<tre::Engine>();
		pEngine->init();
		pEngine->run();
		pEngine->close();
	}
	_CrtMemCheckpoint(&s2);

	if (_CrtMemDifference(&s3, &s1, &s2)) {
		_CrtMemDumpStatistics(&s3);
		_CrtMemDumpAllObjectsSince(&s1);
		_CrtDumpMemoryLeaks();
	}

	return 0;
}