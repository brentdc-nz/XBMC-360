#include "Application.h"

CApplication g_application;

void __cdecl main()
{
	g_application.Create();

	while(1)
	{
		g_application.Run();
	}
}