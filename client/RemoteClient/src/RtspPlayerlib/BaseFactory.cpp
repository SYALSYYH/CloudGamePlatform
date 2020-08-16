#include "stdafx.h"
#include "BaseFactory.h"

BaseFactory::BaseFactory(void)
{
}

bool BaseFactory::Initialize(HWND hwnd, int width, int height)
{
	imgheight = height;
	imgwidth = width;
	m_hwnd = hwnd;
	m_pRenderTarget = nullptr;
	m_pBitmap = nullptr;
	CreateDeviceResources();
	return true;
}

BaseFactory::~BaseFactory(void)
{
}

HRESULT BaseFactory::CreateDeviceResources()
{
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);
	HRESULT hr = S_OK;

	if (!m_pRenderTarget)
	{
		GetClientRect(m_hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU
			(
			rc.right - rc.left,
			rc.bottom - rc.top
			);
		// Create a Direct2D render target.
		hr = m_pDirect2dFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(m_hwnd, size, D2D1_PRESENT_OPTIONS_IMMEDIATELY),//�������������ò��ȴ���ֱͬ����Ĭ�ϴ�ֱͬ��ʱ���ˢ��Ƶ��Ϊ�Կ�ˢ��Ƶ�ʣ�һ��60FPS
			&m_pRenderTarget
			);
		//����λͼ
		D2D1_SIZE_U imgsize = D2D1::SizeU(imgwidth, imgheight);
		D2D1_PIXEL_FORMAT pixelFormat =  //λͼ���ظ�ʽ����
		{
			DXGI_FORMAT_B8G8R8A8_UNORM, //�ò�������ͼ�������������ظ�ʽ����ΪRGBA���ɸ�����Ҫ��Ϊ��ĸ�ʽ��ֻ�Ǻ�������ݿ���Ҫ����Ӧ�ĵ���
			D2D1_ALPHA_MODE_IGNORE
		};
		D2D1_BITMAP_PROPERTIES prop =  //λͼ������Ϣ����
		{
			pixelFormat,
			imgsize.width,
			imgsize.height
		};
		long pitch = imgsize.width * 4;
		imgdata = new char[imgsize.width * imgsize.height * 4];
		memset(imgdata, 0, imgsize.width * imgsize.height * 4);
		m_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);//����ͼ��Ϊ�����ģʽ
		m_pRenderTarget->CreateBitmap(imgsize, imgdata, pitch, &prop, &m_pBitmap);

		imgrect.left = 0;
		imgrect.right = imgwidth;
		imgrect.top = 0;
		imgrect.bottom = imgheight;
	}
	return hr;
}

void BaseFactory::DiscardDeviceResources()
{
	delete[] imgdata;
	SafeRelease(&m_pRenderTarget);
	m_pDirect2dFactory->Release();
	m_pBitmap->Release();
}

void BaseFactory::OnRender(char *data)//����ͼ�ε�ָ���ؼ���
{
	m_pRenderTarget->BeginDraw();//����ʾˢ��Ƶ���й�ϵ
	m_pBitmap->CopyFromMemory(&imgrect, data, imgwidth * 4);
	m_pRenderTarget->DrawBitmap(m_pBitmap, D2D1::RectF(0, 0, rc.right - rc.left, rc.bottom - rc.top));//�þ��δ�С���ܵ�"�����ı���Ӧ�ú�������Ŀ�Ĵ�С:xxx%"��Ӱ��
	m_pRenderTarget->EndDraw();
}
