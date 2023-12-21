//Custom Header
#include "engine.h"

#ifdef _DEBUG
#define MYDEBUG_MALLOC(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define MYDEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define MYDEBUG_NEW
#define MYDEBUG_MALLOC(s)
#endif // _DEBUG

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>  
#include <crtdbg.h>

#ifdef _DEBUG
#define new MYDEBUG_NEW
#define malloc(s) MYDEBUG_MALLOC(s)
#endif // _DEBUG

tre::Engine* pEngine;

int main()
{
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtMemState s1;
	_CrtMemState s2;
	_CrtMemState s3;

	_CrtMemCheckpoint(&s1);
	{
		tre::Engine e;
		pEngine = &e;
		e.init();
		e.run();
		e.close();
	}
	_CrtMemCheckpoint(&s2);

	if (_CrtMemDifference(&s3, &s1, &s2)) {
		_CrtMemDumpStatistics(&s3);
		_CrtMemDumpAllObjectsSince(&s1);
		_CrtDumpMemoryLeaks();
	}

	return 0;
}