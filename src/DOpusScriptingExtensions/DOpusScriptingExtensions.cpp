#include "pch.h"
#include "resource.h"
#include "DOpusScriptingExtensions_i.h"
#include "Utils/StringUtils.h"
#include "Utils/Exceptions.h"
#include "Utils/ComUtils.h"
#include "Utils/WinApiUtils.h"
#include "ProcessRunner/ProcessRunnerResult.h"
#include "ProcessRunner/ProcessRunner.h"
#include "FileMimeTypeDetector/LibMagicWrapper.h"
#include "FileMimeTypeDetector/FileMimeTypeDetectorResult.h"
#include "FileMimeTypeDetector/FileMimeTypeDetector.h"
#include "StringFormatter/StringFormatter.h"
#include "MediaInfoRetriever/MediaInfoRetriever.h"
#include "ExifTool/ExifToolWrapper.h"
#include "ExifTool/ExifTool.h"

class CDOpusScriptingExtensionsModule : public ATL::CAtlDllModuleT<CDOpusScriptingExtensionsModule>
{
public:
  DECLARE_LIBID(LIBID_DOpusScriptingExtensionsLib)
  DECLARE_REGISTRY_APPID_RESOURCEID(IDR_DOpusScriptingExtensions, "{010ccc04-f6ea-46e5-854d-67d1fb42c7f3}")
};

CDOpusScriptingExtensionsModule _AtlModule;

extern "C" BOOL WINAPI DllMain(HINSTANCE /* hInstance */, DWORD dwReason, LPVOID lpReserved)
{
  return _AtlModule.DllMain(dwReason, lpReserved);
}

// Used to determine whether the DLL can be unloaded by OLE.
_Use_decl_annotations_
STDAPI DllCanUnloadNow(void)
{
	return _AtlModule.DllCanUnloadNow();
}

// Returns a class factory to create an object of the requested type.
_Use_decl_annotations_
STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID* ppv)
{
	return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

// DllRegisterServer - Adds entries to the system registry.
_Use_decl_annotations_
STDAPI DllRegisterServer(void)
{
	// registers object, typelib and all interfaces in typelib
	HRESULT hr = _AtlModule.DllRegisterServer();
	return hr;
}

// DllUnregisterServer - Removes entries from the system registry.
_Use_decl_annotations_
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();
	return hr;
}

// DllInstall - Adds/Removes entries to the system registry per user per machine.
STDAPI DllInstall(BOOL bInstall, _In_opt_  LPCWSTR pszCmdLine)
{
	HRESULT hr = E_FAIL;
	static const wchar_t szUserSwitch[] = L"user";

	if (pszCmdLine != nullptr)
	{
		if (_wcsnicmp(pszCmdLine, szUserSwitch, _countof(szUserSwitch)) == 0)
		{
			ATL::AtlSetPerUserRegistration(true);
		}
	}

	if (bInstall)
	{
		hr = DllRegisterServer();
		if (FAILED(hr))
		{
			DllUnregisterServer();
		}
	}
	else
	{
		hr = DllUnregisterServer();
	}

	return hr;
}


