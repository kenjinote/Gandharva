#include "Application.h"

namespace gandharva
{

UINT
Application::Run()
{
	MSG msg;

	while(::GetMessage(&msg, nullptr, 0, 0) > 0){
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	return (UINT) msg.wParam;

}


void
Application::PeekAndDispatchMessage()
{
	MSG msg;
	while(::PeekMessage(&msg, nullptr, 0, 0, TRUE)){
		if(msg.message == WM_QUIT){
			::ExitProcess((UINT)msg.wParam);
		}
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}


void
Application::FlushMessageQueue(UINT msgMin, UINT msgMax)
{
	MSG msg;
	while(::PeekMessage(&msg, nullptr, msgMin, msgMax, TRUE)){
		::DispatchMessage(&msg);
	}
}


} // namespace gandharva
