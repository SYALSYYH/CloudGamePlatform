#pragma once
#include "include/cef_app.h"
#include "include/cef_client.h"

#include <list>
using namespace std;

typedef CefRefPtr<CefBrowser>		BrowserPtr;
typedef std::vector<BrowserPtr>	BrowserList;

class CUserGameDlg;
class CCefBrowserEventHandler
	: public CefClient
	, public CefDisplayHandler			// ��ʾ�仯�¼�
	, public CefLoadHandler			    // ���ش����¼�
	, public CefLifeSpanHandler		    // ���������¼�
	//, public CefContextMenuHandler	// �����Ĳ˵��¼�
	//, public CefDialogHandler			// �Ի����¼�
	//, public CefDownloadHandler		// �����¼�
	//, public CefDragHandler			// ��ק�¼�
	//, public CefFindHandler			// �����¼�
	//, public ...
	//, public CefRequestHandler,
	//, public CefContextMenuHandler,
{
public:
	CCefBrowserEventHandler(CUserGameDlg* pMainFrame);
	virtual ~CCefBrowserEventHandler(void);
	BrowserPtr GetBrowser(HWND hWnd);

public:
	// CefClient �¼�������,���û�ж�Ӧ��������Ĭ��ʹ���ڲ�������
	virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE;
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE;
	virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE;

public:	
	// display handler method
	virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) OVERRIDE;
	virtual void OnAddressChange(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame,const CefString& url) OVERRIDE;

public:
	// load handler method
	virtual void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl) OVERRIDE;
	virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser,bool isLoading,bool canGoBack,bool canGoForward) OVERRIDE;

public:
	// display handler meethod
	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
	virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
	virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

public:
	// own method of cef browser event handler
	void CloseAllBrowser(bool bForceClose = true);
	void CloseBrowser(HWND hWnd);

protected:
	BrowserList					m_browser_list;
	CUserGameDlg*				m_pMainFrame;

	IMPLEMENT_REFCOUNTING(CCefBrowserEventHandler);
};
