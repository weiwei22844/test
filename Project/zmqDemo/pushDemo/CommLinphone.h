#ifndef COMMLINPHONE_H
#define COMMLINPHONE_H
#define LINPHONE_LISPORT  7755
#define QTSERV_LISPORT    7756
#ifdef WIN32
#define LINPHONE_IP "192.168.1.106"
#else
#define LINPHONE_IP QHostAddress::LocalHost
#endif

typedef enum{
    PHONE_STATUS=0,

    PHONE_SET=100,
}PACK_TYPE;

typedef enum _LinphoneCallState{
    LinphoneCallIdle,					/**<Initial call state 0*/
    LinphoneCallIncomingReceived,       /**<This is a new incoming call 1*/
    LinphoneCallOutgoingInit,           /**<An outgoing call is started 2*/
    LinphoneCallOutgoingProgress,       /**<An outgoing call is in progress 3*/
    LinphoneCallOutgoingRinging,        /**<An outgoing call is ringing at remote end 4*/
    LinphoneCallOutgoingEarlyMedia,     /**<An outgoing call is proposed early media 5*/
    LinphoneCallConnected,              /**<Connected, the call is answered 6*/
    LinphoneCallStreamsRunning,         /**<The media streams are established and running 7*/
    LinphoneCallPausing,                /**<The call is pausing at the initiative of local end 8*/
    LinphoneCallPaused,                 /**< The call is paused, remote end has accepted the pause 9*/
    LinphoneCallResuming,               /**<The call is being resumed by local end 10*/
    LinphoneCallRefered,                /**<The call is being transfered to another party, resulting in a new outgoing call to follow immediately 11*/
    LinphoneCallError,                  /**<The call encountered an error 12*/
    LinphoneCallEnd,                    /**<The call ended normally 13*/
    LinphoneCallPausedByRemote,         /**<The call is paused by remote end 14*/
    LinphoneCallUpdatedByRemote,        /**<The call's parameters are updated, used for example when video is asked by remote 15*/
    LinphoneCallIncomingEarlyMedia,     /**<We are proposing early media to an incoming call 16*/
    LinphoneCallUpdated,                /**<The remote accepted the call update initiated by us 17*/
    LinphoneCallReleased                /**< The call object is no more retained by the core 18*/
} LinphoneCallState;

typedef struct _CommPack
{
    unsigned char nVersion;
    unsigned char nType;
    unsigned short payLoadSize;
}CommPack;

typedef struct _TLV
{
    unsigned short type;
    unsigned short len;
    char value[1];
}TLV;

typedef struct _BellPkgHead{
    unsigned char ucVersion;
    unsigned char ucCode;
    unsigned char HdLen;
    unsigned char Reserved;
    unsigned int uiTotalLen;
}BellPkgHead;

typedef struct _BellPkg{
    BellPkgHead header;
    TLV tlvs[1];
}BellPkg;

#endif // COMMLINPHONE_H
