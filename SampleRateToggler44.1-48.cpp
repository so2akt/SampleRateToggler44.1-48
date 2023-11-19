#include <wchar.h>
#include "mmdeviceapi.h"
#include "Propidl.h"
#include "functiondiscoverykeys_devpkey.h"

int main()
{
	HRESULT hRes = NULL;
	IMMDeviceEnumerator *pDevEnum = NULL;
	IMMDeviceCollection *pDev = NULL;
	UINT nCount = 0;
	IMMDevice *pDevId = NULL;
	LPWSTR wstrEndpointId = NULL;
	IPropertyStore *pPropStore = NULL;
	PROPVARIANT friendlyName;

	hRes = CoInitialize(NULL);
	if (FAILED(hRes))	/* TODO: refactor error proc*/
	{
		return EXIT_FAILURE;
	}

	hRes = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
		CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID*)&pDevEnum);
	if (FAILED(hRes))	/* TODO: refactor error proc*/
	{
		CoUninitialize();
		return EXIT_FAILURE;
	}

	/* get list of active input audio devices */
	hRes = pDevEnum->EnumAudioEndpoints(eCapture/* eRender: output, eCapture: input */,
		DEVICE_STATE_ACTIVE, &pDev);
	if (FAILED(hRes))	/* TODO: refactor error proc*/
	{
		pDevEnum->Release();
		CoUninitialize();
		return EXIT_FAILURE;
	}

	hRes = pDev->GetCount(&nCount);
	if (FAILED(hRes))	/* TODO: refactor error proc*/
	{
		pDev->Release();
		pDevEnum->Release();
		CoUninitialize();
		return EXIT_FAILURE;
	}
	for (UINT i = 0; i < nCount; i++)
	{
		hRes = pDev->Item(i, &pDevId);
		if (FAILED(hRes))	/* TODO: refactor error proc*/
		{
			pDev->Release();
			pDevEnum->Release();
			CoUninitialize();
			return EXIT_FAILURE;
		}

		hRes = pDevId->GetId(&wstrEndpointId);
		if (FAILED(hRes))	/* TODO: refactor error proc*/
		{
			pDevId->Release();
			pDev->Release();
			pDevEnum->Release();
			CoUninitialize();
			return EXIT_FAILURE;
		}

		hRes = pDevId->OpenPropertyStore(STGM_READ, &pPropStore);
		if (FAILED(hRes))	/* TODO: refactor error proc*/
		{
			pDevId->Release();
			pDev->Release();
			pDevEnum->Release();
			CoUninitialize();
			return EXIT_FAILURE;
		}

		/* get friendly names on Sound settings */
		PropVariantInit(&friendlyName);
		hRes = pPropStore->GetValue(PKEY_Device_FriendlyName, &friendlyName);
		if (FAILED(hRes))	/* TODO: refactor error proc*/
		{
			pPropStore->Release();
			pDevId->Release();
			pDev->Release();
			pDevEnum->Release();
			CoUninitialize();
			return EXIT_FAILURE;
		}
		wprintf(L"Device No. %d: %ls\n", i, friendlyName.pwszVal);	/* FIXME: show multi-byte letters properly */
		PropVariantClear(&friendlyName);
	}

	pPropStore->Release();
	pDevId->Release();
	pDev->Release();
	pDevEnum->Release();
	CoUninitialize();

	return EXIT_SUCCESS;
}