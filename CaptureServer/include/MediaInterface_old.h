#pragma once

#include <stdint.h>
#include <Windows.h>

#define MAX_DATA_OBJ_COUNT  ( 64 )

#pragma pack(push, 8)

struct FrameData
{
    uint32_t   time ;
    uint32_t   size ;
    uint8_t *  buffer ;
    void *     context ;
} ;

struct ImageData
{
    //
    // YUV����
    //
    FrameData   image ;
    
    //
    // �������ݸ���
    //
    uint32_t    count ;
    
    //
    // ��������
    //
    FrameData   contexts[MAX_DATA_OBJ_COUNT] ;
} ;

struct NetStatusObj
{
    int16_t      nStatus ; //״ֵ̬
    int16_t      nRef  ;
    uint16_t     nSRtt ; // ���͵�rtt
    uint16_t     nPRtt ; // ���ն˵�rtt = 0 ˵����û���յ��Զ˵ķ���
} ;

#pragma pack(pop)

#ifndef __FRAME__BUFFER__

#define __FRAME__BUFFER__

#pragma pack(push, 1)

struct IFrame420PBuffer
{
    uint8_t *   m_YBuffer ; // y
    uint8_t *   m_UBuffer ; // u
    uint8_t *   m_VBuffer ; // v
    uint32_t    m_YLength ;
    uint32_t    m_ULength ;
    uint32_t    m_VLength ;
    uint32_t    m_UMemSize ;
    uint32_t    m_uPacked  ;
} ;

#pragma pack(pop)
#endif


class IVoiceEvent
{
#define VOICE_ERR_PLAY_DEVICE_START ( 0 )
#define VOICE_ERR_RECORD_DEVICE_START ( 1 )
public:
    virtual void __stdcall OnVoiceDevErrEvent( int nErrType, int nErrCode ) = 0 ;
    
    //
    // �����̳߳�ʼ��
    //
    virtual void __stdcall OnVoiceInit() = 0 ;
    
    //
    // �ɼ�����
    // @pbuffer
    // @usize  40���� 16k����  1280 bytes = 640������
    // @utime ʱ���
    //
    virtual void __stdcall OnVoiceDevBufferEvent(const FrameData * pPCMData ) = 0 ;
    virtual void __stdcall OnVoiceClean() = 0 ;
} ;



class IVoiceEngine
{
public:
	virtual void  __stdcall Release() = 0 ;
    
    //
    //  ������ѡ�����pcm����
    //
#define AUDIO_PARSER_MIC				( 0x0 )
#define AUDIO_PARSER_PCM                ( 0x2 )
#define AUDIO_PARSER_SYS				( 0x1 )


    //
    // @uMode = AUDIO_PARSER_PCM �����÷���ģʽ
    // Ĭ�Ͽ�������
    //
	virtual int  __stdcall Init( int nVoiceTypeId, IVoiceEvent * pVoiceEvent, uint32_t uMode ) = 0 ;
	virtual void  __stdcall Close() = 0 ;
    
    // bEnable = true �������ţ�= false �رղ���
	virtual void  __stdcall EnableRecord( bool bEnable , uint32_t uDelay ) = 0 ;
	virtual void  __stdcall EnablePlay(bool bEnable) = 0 ;
    virtual int   __stdcall Mute( bool bEnable ) = 0 ;
} ;




class IMediaClientEvent
{
public:
	//
	// nErrorCode = 0 �ɹ�
	// nErrorCode = 1 ʧ��
	//
#define LOGIN_ME_OK			( 0 )
#define LOGIN_ME_FAIL		( 1 )

    //
    // ��¼�������ɹ�
    //
	virtual void  __stdcall LoginEvent( int nErrorCode ) = 0 ;
    
    //
    // ������ʹ�ú��֪ͨ����ͨѶ�����л��л�
    //
	virtual void  __stdcall MediaServerEvent( int nAddress, int nPort ) = 0 ;
    
#define AUDIO_DATA_TYPE     ( 0 )
#define VIDEO_DATA_TYPE     ( 1 )
    
    //
    // ���û�����  @nDataType = AUDIO_DATA_TYPE/VIDEO_DATA_TYPE
    //
	virtual void  __stdcall UserEnterEvent( int nUserId , int nDataType ) = 0 ;
    
    //
    // ���û��뿪 @nDataType = AUDIO_DATA_TYPE/VIDEO_DATA_TYPE
    //
	virtual void  __stdcall UserLeaveEvent( int nUserId , int nDataType ) = 0 ;
    
    // ����Ͽ�
	virtual void  __stdcall CloseEvent( int nCloseCode ) = 0 ;
    
    //
    // ���������ص�
    // pNetStatus.nStatus
    
#define NET_STATUS_NORMAL       ( 0 )   // �е�
#define NET_STATUS_EXCELLENCE   ( 1 )   // �ܺ�
#define NET_STATUS_BAD          ( -1 )  // ��
#define NET_STATUS_VERY_BAD     ( -2 )  // �ǳ���
    
    virtual void  __stdcall NetStatusEvent( NetStatusObj * pNetStatus ) = 0 ;
};

class IMediaClient
{
public:
	virtual void  __stdcall Release() = 0 ;
    
    // @���ö˿��б�
	virtual void  __stdcall SetPortArray( uint16_t * pNetPortArray, uint32_t uPortCount) = 0 ;
    
    // ����ip�б�
	virtual void  __stdcall SetAddressArray( uint32_t * uAddressArray, uint32_t uAddressCount) =0 ;
    
    //
    // �����û���Ϣ @result =0 �ɹ�
    //
	virtual int   __stdcall SetUserInfo( uint64_t uRoomID, uint32_t uRoomKey, uint32_t uCID ) = 0 ;
    
    //
    // ��¼������ @result = 0 �ɹ�
    // @uLoginType = MEDIA_SERVER_MODE �Ƿ�������ģʽ
	//
#define MEDIA_MODE_SERVER		( 0x80000000 )
#define MEDIA_MODE_CLIENT	    ( 0 )

	virtual int   __stdcall EnterServer( uint32_t uLoginType, IMediaClientEvent * pMediaServerEvent ) = 0 ;
    
    //
    // �뿪������
    //
	virtual int   __stdcall LeaveServer() = 0 ;
    
    //
    // ��ȡʱ��� ����Ƶ ��Ƶʱ���һ��
    // @rc = 0 ������ֵ�����ܵ���0
    //
    virtual uint32_t __stdcall GetMediaTime() = 0 ;

	//
	// ����һ���û���½key
	// @uTimeOut = 0xffffffff ���ò��ͷţ�����ʹ��
	// 
#define FOR_EVER_TIME		( 0xffffffff )
	virtual int	__stdcall AddClientUserKey( uint32_t uCid , uint32_t uKey , uint32_t uTimeOut ) = 0 ;

	//
	// ɾ��һ���û���
	//
	virtual int __stdcall RemoveClientUserKey( uint32_t uCid ) = 0 ;
} ;



class IVideoUserEvent
{
public:

	//
	// ��ʼ�� @nwidth @nheight ����Ƶ�ߴ�
	// ����������ʼ�� opengl
    //
	virtual int32_t  __stdcall OnVideoInit( int nWidth, int nHeight ) = 0 ;
	
	//
	// ԭʼ�ӿ� ���߳����棬���������������
	//


//
// ��� SetZoomImageType = VF_I420
// ImageData->buffer = & IFrame420PBuffer
// Ĭ����ARGB
// ��� ImageType = VF_I420 ��ôʹ��
// IFrame420PBuffer * pVideoFrame = (IFrame420PBuffer *)(pImageData->image.buffer) ;
// ��Ⱦ pVideoFrame->YBuffer/UBuffer
//
	virtual int32_t  __stdcall OnVideoBuffer(const ImageData * pImageData ) = 0 ;
    
    //
    // ��ͬ�����ã���ֱ�ӷ�����Ϣ
    //
    virtual int32_t  __stdcall OnDataBuffer( FrameData * pFrameData, uint32_t uCount ) = 0 ;
    
    //
    // �˳�����
    //
	virtual void     __stdcall OnVideoClean() = 0 ;
};

class IVideoUser
{
public:
    //
    // @bSyncData = 0 ��ͬ���������� �����ص� OnDataBuffer ����
    //            = 1 ͬ������ ����Ƶ֡�ص������з��ض�Ӧ�ĸ������� Ĭ������
    //
    //
	virtual void  __stdcall Init( IVideoUserEvent * pVideoUserEvent, uint32_t bSyncData )  = 0 ;
    
#define VF_NV21         ( 1 )
#define VF_NV12         ( 2 )
#define VF_YV12         ( 3 )
#define VF_I420         ( 8 ) // ֧��
    
#define VF_RGB565       ( 4 ) // ֧��
#define VF_RGB24        ( 5 ) // Ŀǰ��֧��
#define VF_RGBA         ( 6 ) // ֧��
#define VF_RGBX         ( 7 ) // Ŀǰ��֧��
    
//
//
// �� SetZoomSize  ֮ǰ����
//
//
    virtual void  __stdcall SetZoomImageType(uint32_t uPixType) = 0 ;
    virtual void  __stdcall Close() = 0 ;
	virtual void  __stdcall SetData(void * pData) = 0 ;
	virtual void * __stdcall GetData() = 0 ;
	virtual uint32_t __stdcall GetUserId() = 0 ;
    
    // ��ȡ��Ƶ�ߴ�
	virtual void  __stdcall GetVideoInfo(int * nWidth, int * nHeight, int * nCodecId ) = 0 ;
    
    // �������Ŵ�С
    virtual void  __stdcall SetZoomSize(int nWidth, int nHeight ) = 0 ;
    
    //
    // �ͷŷ����֡������ OnVideoBuffer �ص���������Ƶ֡
    //
    virtual void __stdcall ReleaseFrameBuffer( FrameData * pFrameData ) = 0 ;
    
    //
    // ��ǰ��Ⱦ֡��
    //
    virtual uint32_t __stdcall GetLocalFrameCount() const  = 0 ;
    
    //
    //  �����Ǵ�0��ʼ�ļ��� ��Ҫ�Լ������ϴε�ֵ ���㵱ǰֵ
    //
    virtual uint32_t __stdcall GetErrFrameCount() const = 0 ;     // �������İ�����
    virtual uint32_t __stdcall GetAllFrameCount() const = 0  ;     // ����ɹ��İ�����
    virtual uint32_t __stdcall GetDropFrameCount() const = 0 ;    // ���ڿ��ٶ����İ�����
} ;

class IVideoRenderEvent
{
public:
	virtual void  __stdcall OnCreateVideoUser( IVideoUser * pVideoUser ) = 0 ;
	virtual void  __stdcall OnCloseVideoUser( IVideoUser * pVideoUser ) = 0 ;
	virtual void  __stdcall OnMyVideoBuffer( char * pBuffer , int nSize ) = 0 ;
} ;

	//
	//
	//
#define VIDEO_CAPTURE_INIT_EVENT		( 0x0010 )  // @nEventType ��
#define VIDEO_CAPTURE_CLEA_EVENT		( 0x0011 )  // @nEventType �ر�
#define VIDEO_CAPTURE_SUCC				( 0x0000 )  // @nErrorCode = 0 �ɹ�


/*
 ʱ����ӵ�¼ý���������tickΪ0 ��ʼ����
 */
class IVideoEngine
{
public:
	virtual void  __stdcall Release() = 0 ;

	virtual int  __stdcall Init( IVideoRenderEvent * pVideoRenderEvent ) = 0 ;
	virtual int  __stdcall Close() = 0 ;


    //
    // @nwidht nheight Ԥ���ߴ�
    // @nzoomwidth nzoomheight ʵ�ʱ���ߴ�
    // @rotate ��ת�Ƕ� 0 90 270
    //
	virtual int  __stdcall SetCaptureInfo( int nWidth, int nHeight , int nZoomWidth, int nZoomHeight,int Rotate ) = 0 ;

	//
	// @nDefFrames =0 def =10
	// @nMaxFrames =0 def =15
	//
	virtual int  __stdcall SetCaptureFrames(int nDefFrames, int nMaxFrames) = 0 ;
    
#define DEF_VIDEO_CODEC_TYPE        ( 0 ) // H264
#define VIDEO_CODEC_TYPE_OPENH264   ( 1 )
#define VIDEO_CODEC_TYPE_X264       ( 2 )
#define VIDEO_CODEC_TYPE_HARD264    ( 3 ) // Ӳ����
#define VIDEO_CODEC_TYPE_VP8        ( 4 )
#define VIDEO_CODEC_TYPE_VP9        ( 8 )
#define VIDEO_CODEC_TYPE_HIGH264    ( 16 )
#define VIDEO_CODEC_TYPE_FEC        ( 0x10000000 )
#define VIDEO_CODEC_TYPE_FEC_TAG    ( 0xFFFFFFF )
    
	//
	// @ hWnd  ����ʾ��ƵԤ�����ڵľ��
	// @nEncBitRate = 0 
	//
	virtual int  __stdcall StartCaptrueVideo( int nEncBitRate,  HWND hWnd ) = 0 ;
    
    //
    // @result = time ʱ��� 4�ֽ� < ��ʱ����������0 >
    //         = 0 ˵��ʧ�ܣ������߳�æµ����֡��ʧ����0��ʾ�ɹ�
    //
    virtual uint32_t  __stdcall ProcessEncodeBuffer( char * pEncBuffer , uint32_t uSize ) = 0 ;
    
	virtual int  __stdcall StopCaptrueVideo() = 0 ;
	virtual int  __stdcall EnablePlayVideo(bool bEnable) = 0 ;
	virtual int  __stdcall ShowVideoConfigWin(int nTop, int nLeft) = 0 ;
    virtual int __stdcall ResetCaptureInfo( int nZoomWidth, int nZoomHeight , int nRotate ) = 0 ;
    
#define MAX_VDATA_SIZE  ( 1260 )
    //
    // utime��ʱ��� @ProcessEncodeBuffer ����ֵ
    // @result = uSize succ...
    //         < 0  fail...
    //
    // ��������̵߳��ã����̵߳���
    //
    //
    virtual int32_t __stdcall PostDataBuffer( const void * pBuffer, uint32_t uSize, uint32_t uTime ) = 0 ;
    
    // ��ȡ��ǰ����֡��
    virtual uint32_t __stdcall GetCaptureFrameCount() = 0 ;
    
    // ��ȡ��ǰ��֡����������cpu����̫�ߵ���
    virtual uint32_t __stdcall GetLostFrameCount() = 0  ;

	//
	// ����264��������� GetTickCount() 
	//
	virtual int32_t	__stdcall SendH264Buffer( const void * pBuffer , uint32_t uSize , uint32_t uKeyFrame, uint32_t uTime )  = 0 ;

} ;

class IMediaInterface
{
public:
	virtual void  __stdcall Release() = 0 ;
    
    //
    // pLogDir = NULL  @"/sdcard/AR_Media_Sdk/"
    //
	virtual int  __stdcall Init(const char * pLogDir) = 0 ;
	virtual int  __stdcall Clean() = 0 ;
    
	virtual IMediaClient *  __stdcall CreateMediaClient() = 0 ;
	virtual IVoiceEngine *  __stdcall CreateVoiceEngine() = 0 ;
	virtual IVideoEngine *  __stdcall CreateVideoEngine() = 0 ;
    
    virtual int __stdcall  WriteLog( const char * pszLog, uint32_t uSize ) = 0  ;

};



#ifdef __cplusplus
extern "C"{
#endif
    
    IMediaInterface *  __stdcall CreateMediaEngine() ;
    
#ifdef __cplusplus
} ;
#endif


