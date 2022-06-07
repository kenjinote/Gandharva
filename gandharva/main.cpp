#include "MainWindow.h"
#include "Profile.h"
#include "FilePath.h"
#include "Application.h"
#include <exception>


int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
try {
	::CoInitialize(nullptr);
	INITCOMMONCONTROLSEX icc;
		icc.dwSize = sizeof(icc);
		icc.dwICC  = ICC_COOL_CLASSES | ICC_WIN95_CLASSES;
	::InitCommonControlsEx(&icc);

	{	auto exeName = gandharva::Path::RemoveExtension(
			gandharva::Path::GetExePath()
		);

		gandharva::Profile::Init<gandharva::Profile::Config>(
			exeName + L"-config.txt"
		);
		gandharva::Profile::Init<gandharva::Profile::State>(
			exeName + L"-state.txt"
		);
	}

	gandharva::MainWindow window;

	window.RestorePrevState();
	auto exitStatus = gandharva::Application::Run();

	gandharva::Profile::Get<gandharva::Profile::Config>().Save();
	gandharva::Profile::Get<gandharva::Profile::State >().Save();

	return  exitStatus;
}
catch(std::exception& e)
{
	::MessageBoxA(0, e.what(), "ívñΩìIÉGÉâÅ[ - gandharva", MB_OK | MB_ICONERROR);
	return 1;
}
