#include "pch.h"

#include "Application.h"

using winrt::com_ptr;
using winrt::check_hresult;

int main(int argc, char** argv)
{
    winrt::init_apartment();
    try {
        Application app(argc, argv);
        app.run();
    }
    catch (const std::exception& exception) {
        std::cerr << exception.what() << '\n';
    }
    catch (const winrt::hresult_error& exception) {
        std::wstring message{ exception.message() };
        std::wcerr << message << '\n';
    }
}
