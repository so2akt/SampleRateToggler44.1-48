#include <wchar.h>
#include "mmdeviceapi.h"
#include "Propidl.h"
#include "functiondiscoverykeys_devpkey.h"
#include "initguid.h"

/* refer to the following article: */
/* https://learn.microsoft.com/en-us/answers/questions/669471/how-to-control-enable-audio-enhancements-with-code */
DEFINE_GUID(CLSID_PolicyConfig, 0x870af99c, 0x171d, 0x4f9e, 0xaf, 0x0d, 0xe6, 0x3d, 0xf4, 0x0c, 0x2b, 0xc9);
MIDL_INTERFACE("f8679f50-850a-41cf-9c72-430f290290c8")
IPolicyConfig : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetMixFormat(PCWSTR pszDeviceName, WAVEFORMATEX * *ppFormat) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetDeviceFormat(PCWSTR pszDeviceName, bool bDefault, WAVEFORMATEX** ppFormat) = 0;
	virtual HRESULT STDMETHODCALLTYPE ResetDeviceFormat(PCWSTR pszDeviceName) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetDeviceFormat(PCWSTR pszDeviceName, WAVEFORMATEX* ppEndpointFormatFormat, WAVEFORMATEX* pMixFormat) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetProcessingPeriod(PCWSTR pszDeviceName, bool bDefault, PINT64 pmftDefaultPeriod, PINT64 pmftMinimumPeriod) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetProcessingPeriod(PCWSTR pszDeviceName, PINT64 pmftPeriod) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetShareMode(PCWSTR pszDeviceName, struct DeviceShareMode* pMode) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetShareMode(PCWSTR pszDeviceName, struct DeviceShareMode* pMode) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetPropertyValue(PCWSTR pszDeviceName, BOOL bFxStore, const PROPERTYKEY& pKey, PROPVARIANT* pv) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetPropertyValue(PCWSTR pszDeviceName, BOOL bFxStore, const PROPERTYKEY& pKey, PROPVARIANT* pv) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetDefaultEndpoint(PCWSTR pszDeviceName, ERole eRole) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetEndpointVisibility(PCWSTR pszDeviceName, bool bVisible) = 0;
};

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
	IPolicyConfig *pPolicyConfig = NULL;
	WAVEFORMATEX *pDevFormat = NULL, *pMixFormat = NULL;

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

		hRes = CoCreateInstance(CLSID_PolicyConfig, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pPolicyConfig));
		if (FAILED(hRes))	/* TODO: refactor error proc*/
		{
			pPropStore->Release();
			pDevId->Release();
			pDev->Release();
			pDevEnum->Release();
			CoUninitialize();
			return EXIT_FAILURE;
		}

		hRes = pPolicyConfig->GetDeviceFormat(wstrEndpointId,
			FALSE/* if TRUE, get Default Format on Restore Default click */,
			&pDevFormat);
		if (FAILED(hRes))	/* TODO: refactor error proc*/
		{
			pPolicyConfig->Release();
			pPropStore->Release();
			pDevId->Release();
			pDev->Release();
			pDevEnum->Release();
			CoUninitialize();
			return EXIT_FAILURE;
		}
		wprintf(L"\tWAVEFORMATEX Data from GetDeviceFormat\n");
		wprintf(L"\t\twFormatTag: %d\n", pDevFormat->wFormatTag);
		wprintf(L"\t\tnChannels: %d\n", pDevFormat->nChannels);
		wprintf(L"\t\tnSamplesPerSec: %ld\n", pDevFormat->nSamplesPerSec);
		wprintf(L"\t\tnAvgBytesPerSec: %ld\n", pDevFormat->nAvgBytesPerSec);
		wprintf(L"\t\tnBlockAlign: %ld\n", pDevFormat->nBlockAlign);
		wprintf(L"\t\twBitsPerSample: %d\n", pDevFormat->wBitsPerSample);
		wprintf(L"\t\tcbSize: %d\n", pDevFormat->cbSize);

		hRes = pPolicyConfig->GetMixFormat(wstrEndpointId, &pMixFormat);
		if (FAILED(hRes))	/* TODO: refactor error proc*/
		{
			pPolicyConfig->Release();
			pPropStore->Release();
			pDevId->Release();
			pDev->Release();
			pDevEnum->Release();
			CoUninitialize();
			return EXIT_FAILURE;
		}
		wprintf(L"\tWAVEFORMATEX Data from GetMixFormat\n");
		wprintf(L"\t\twFormatTag: %d\n", pMixFormat->wFormatTag);
		wprintf(L"\t\tnChannels: %d\n", pMixFormat->nChannels);
		wprintf(L"\t\tnSamplesPerSec: %ld\n", pMixFormat->nSamplesPerSec);
		wprintf(L"\t\tnAvgBytesPerSec: %ld\n", pMixFormat->nAvgBytesPerSec);
		wprintf(L"\t\tnBlockAlign: %ld\n", pMixFormat->nBlockAlign);
		wprintf(L"\t\twBitsPerSample: %d\n", pMixFormat->wBitsPerSample);
		wprintf(L"\t\tcbSize: %d\n", pMixFormat->cbSize);

		wprintf(L"\n");

		PropVariantClear(&friendlyName);
	}

	pPropStore->Release();
	pDevId->Release();
	pDev->Release();
	pDevEnum->Release();
	CoUninitialize();

	return EXIT_SUCCESS;
}