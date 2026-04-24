/*
* UPnP Support for XBMC
* Copyright (c) 2006 c0diq (Sylvain Rebaud)
* Portions Copyright (c) by the authors of libPlatinum
*
* http://www.plutinosoft.com/blog/category/platinum/
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "UPnP.h"
#include "Application.h"
#include "URL.h"
#include "Settings.h"
#include "GUISettings.h"
#include "FileItem.h"
#include "xbox\Network.h"
#include "utils\Log.h"
#include "utils\StringUtils.h"
#include "guilib\GUIWindowManager.h"
#include "guilib\GUIUserMessages.h"
#include "guilib\GUIInfoManager.h"
#include "music\tags\MusicInfoTag.h"
#include "video\VideoInfoTag.h"

#include "NptStrings.h"
#include "Platinum.h"
#include "PltMediaRenderer.h"
#include "PltSyncMediaBrowser.h"
#include "PltDidl.h"
#include "NptNetwork.h"

#ifdef _XBOX
#include <xtl.h>
#include <winsockx.h>
#include "NptXboxNetwork.h"
#endif

/*----------------------------------------------------------------------
|   CCtrlPointReferenceHolder class
+---------------------------------------------------------------------*/
class CCtrlPointReferenceHolder
{
public:
	PLT_CtrlPointReference m_CtrlPoint;
};

/*----------------------------------------------------------------------
|   CDeviceHostReferenceHolder class
+---------------------------------------------------------------------*/
class CDeviceHostReferenceHolder
{
public:
	PLT_DeviceHostReference m_Device;
};

/*----------------------------------------------------------------------
|   CRendererReferenceHolder class
+---------------------------------------------------------------------*/
class CRendererReferenceHolder
{
public:
	PLT_DeviceHostReference m_Device;
};

/*----------------------------------------------------------------------
|   CUPnPCleaner class
+---------------------------------------------------------------------*/
class CUPnPCleaner : public NPT_Thread
{
public:
	CUPnPCleaner(CUPnP* upnp) : NPT_Thread(true), m_UPnP(upnp) {}
	void Run()
	{
		delete m_UPnP;
	}

	CUPnP* m_UPnP;
};

/*----------------------------------------------------------------------
|   CMediaBrowser class
+---------------------------------------------------------------------*/
class CMediaBrowser : public PLT_SyncMediaBrowser,
					  public PLT_MediaContainerChangesListener
{
public:
	CMediaBrowser(PLT_CtrlPointReference& ctrlPoint)
		: PLT_SyncMediaBrowser(ctrlPoint, true)
	{
		SetContainerListener(this);
	}

	// PLT_MediaBrowser methods
	virtual bool OnMSAdded(PLT_DeviceDataReference& device)
	{
		CGUIMessage message(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_UPDATE_PATH);
		message.SetStringParam("upnp://");
		g_windowManager.SendThreadMessage(message);

		return PLT_SyncMediaBrowser::OnMSAdded(device);
	}

	virtual void OnMSRemoved(PLT_DeviceDataReference& device)
	{
		PLT_SyncMediaBrowser::OnMSRemoved(device);

		CGUIMessage message(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_UPDATE_PATH);
		message.SetStringParam("upnp://");
		g_windowManager.SendThreadMessage(message);
	}

	// PLT_MediaContainerChangesListener methods
	virtual void OnContainerChanged(PLT_DeviceDataReference& device,
									const char*              item_id,
									const char*              update_id)
	{
		NPT_String path = "upnp://"+device->GetUUID()+"/";
		if (!NPT_StringsEqual(item_id, "0")) {
			CStdString id = item_id;
			CURL::Encode(id);
			path += id.c_str();
			path += "/";
		}

		CGUIMessage message(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_UPDATE_PATH);
		message.SetStringParam(path.GetChars());
		g_windowManager.SendThreadMessage(message);
	}
};

/*----------------------------------------------------------------------
|   CUPnPRenderer
+---------------------------------------------------------------------*/
class CUPnPRenderer : public PLT_MediaRenderer
{
public:
	CUPnPRenderer(const char*  friendly_name,
	              bool         show_ip = false,
	              const char*  uuid = NULL,
	              unsigned int port = 0);

	void UpdateState();

	// AVTransport methods
	virtual NPT_Result OnNext(PLT_ActionReference& action);
	virtual NPT_Result OnPause(PLT_ActionReference& action);
	virtual NPT_Result OnPlay(PLT_ActionReference& action);
	virtual NPT_Result OnPrevious(PLT_ActionReference& action);
	virtual NPT_Result OnStop(PLT_ActionReference& action);
	virtual NPT_Result OnSeek(PLT_ActionReference& action);
	virtual NPT_Result OnSetAVTransportURI(PLT_ActionReference& action);

	// RenderingControl methods
	virtual NPT_Result OnSetVolume(PLT_ActionReference& action);
	virtual NPT_Result OnSetMute(PLT_ActionReference& action);

private:
	NPT_Result SetupServices(PLT_DeviceData& data);
	NPT_Result GetMetadata(NPT_String& meta);
	NPT_Result PlayMedia(const char* uri,
	                      const char* metadata = NULL,
	                      PLT_Action* action = NULL);
};

/*----------------------------------------------------------------------
|   CUPnPRenderer::CUPnPRenderer
+---------------------------------------------------------------------*/
CUPnPRenderer::CUPnPRenderer(const char*  friendly_name,
                             bool         show_ip /* = false */,
                             const char*  uuid /* = NULL */,
                             unsigned int port /* = 0 */) :
	PLT_MediaRenderer(friendly_name,
	                  show_ip,
	                  uuid,
	                  port)
{
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::SetupServices
+---------------------------------------------------------------------*/
NPT_Result
CUPnPRenderer::SetupServices(PLT_DeviceData& data)
{
	NPT_CHECK(PLT_MediaRenderer::SetupServices(data));

	// update what we can play
	PLT_Service* service = NULL;
	NPT_CHECK_FATAL(FindServiceByType("urn:schemas-upnp-org:service:ConnectionManager:1", service));
	service->SetStateVariable("SinkProtocolInfo"
		,"http-get:*:*:*"
		",xbmc-get:*:*:*"
		",http-get:*:audio/mpegurl:*"
		",http-get:*:audio/mpeg:*"
		",http-get:*:audio/mpeg3:*"
		",http-get:*:audio/mp3:*"
		",http-get:*:audio/basic:*"
		",http-get:*:audio/midi:*"
		",http-get:*:audio/ulaw:*"
		",http-get:*:audio/ogg:*"
		",http-get:*:audio/DVI4:*"
		",http-get:*:audio/G722:*"
		",http-get:*:audio/G723:*"
		",http-get:*:audio/G726-16:*"
		",http-get:*:audio/G726-24:*"
		",http-get:*:audio/G726-32:*"
		",http-get:*:audio/G726-40:*"
		",http-get:*:audio/G728:*"
		",http-get:*:audio/G729:*"
		",http-get:*:audio/G729D:*"
		",http-get:*:audio/G729E:*"
		",http-get:*:audio/GSM:*"
		",http-get:*:audio/GSM-EFR:*"
		",http-get:*:audio/L8:*"
		",http-get:*:audio/L16:*"
		",http-get:*:audio/LPC:*"
		",http-get:*:audio/MPA:*"
		",http-get:*:audio/PCMA:*"
		",http-get:*:audio/PCMU:*"
		",http-get:*:audio/QCELP:*"
		",http-get:*:audio/RED:*"
		",http-get:*:audio/VDVI:*"
		",http-get:*:audio/ac3:*"
		",http-get:*:audio/vorbis:*"
		",http-get:*:audio/speex:*"
		",http-get:*:audio/x-aiff:*"
		",http-get:*:audio/x-pn-realaudio:*"
		",http-get:*:audio/x-realaudio:*"
		",http-get:*:audio/x-wav:*"
		",http-get:*:audio/x-ms-wma:*"
		",http-get:*:audio/x-mpegurl:*"
		",http-get:*:application/x-shockwave-flash:*"
		",http-get:*:application/ogg:*"
		",http-get:*:application/sdp:*"
		",http-get:*:image/gif:*"
		",http-get:*:image/jpeg:*"
		",http-get:*:image/ief:*"
		",http-get:*:image/png:*"
		",http-get:*:image/tiff:*"
		",http-get:*:video/avi:*"
		",http-get:*:video/mpeg:*"
		",http-get:*:video/fli:*"
		",http-get:*:video/flv:*"
		",http-get:*:video/quicktime:*"
		",http-get:*:video/vnd.vivo:*"
		",http-get:*:video/vc1:*"
		",http-get:*:video/ogg:*"
		",http-get:*:video/mp4:*"
		",http-get:*:video/BT656:*"
		",http-get:*:video/CelB:*"
		",http-get:*:video/JPEG:*"
		",http-get:*:video/H261:*"
		",http-get:*:video/H263:*"
		",http-get:*:video/H263-1998:*"
		",http-get:*:video/H263-2000:*"
		",http-get:*:video/MPV:*"
		",http-get:*:video/MP2T:*"
		",http-get:*:video/MP1S:*"
		",http-get:*:video/MP2P:*"
		",http-get:*:video/BMPEG:*"
		",http-get:*:video/x-ms-wmv:*"
		",http-get:*:video/x-ms-avi:*"
		",http-get:*:video/x-flv:*"
		",http-get:*:video/x-fli:*"
		",http-get:*:video/x-ms-asf:*"
		",http-get:*:video/x-ms-asx:*"
		",http-get:*:video/x-ms-wmx:*"
		",http-get:*:video/x-ms-wvx:*"
		",http-get:*:video/x-msvideo:*"
		);
	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::UpdateState
+---------------------------------------------------------------------*/
void
CUPnPRenderer::UpdateState()
{
	PLT_Service *avt, *rct;
	if (NPT_FAILED(FindServiceByType("urn:schemas-upnp-org:service:AVTransport:1", avt)))
		return;
	if (NPT_FAILED(FindServiceByType("urn:schemas-upnp-org:service:RenderingControl:1", rct)))
		return;

	CStdString buffer;
	int volume;
	if (g_settings.m_bMute)
	{
		rct->SetStateVariable("Mute", "1");
		volume = g_settings.m_iPreMuteVolumeLevel;
	}
	else
	{
		rct->SetStateVariable("Mute", "0");
		volume = g_application.GetVolume();
	}

	buffer.Format("%d", volume);
	rct->SetStateVariable("Volume", buffer.c_str());

	buffer.Format("%d", 256 * (volume * 60 - 60) / 100);
	rct->SetStateVariable("VolumeDb", buffer.c_str());

	if (g_application.IsPlaying() || g_application.IsPaused())
	{
		if (g_application.IsPaused())
		{
			avt->SetStateVariable("TransportState", "PAUSED_PLAYBACK");
		}
		else
		{
			avt->SetStateVariable("TransportState", "PLAYING");
		}

		avt->SetStateVariable("TransportStatus", "OK");
		avt->SetStateVariable("TransportPlaySpeed", (const char*)NPT_String::FromInteger(g_application.GetPlaySpeed()));
		avt->SetStateVariable("NumberOfTracks", "1");
		avt->SetStateVariable("CurrentTrack", "1");

		buffer = g_infoManager.GetCurrentPlayTime(TIME_FORMAT_HH_MM_SS);
		avt->SetStateVariable("RelativeTimePosition", buffer.c_str());
//		buffer = CStringUtils::SecondsToTimeString((long)g_infoManager.GetTotalPlayTime(), TIME_FORMAT_HH_MM_SS); // Not correct, should be GetCurrentPlayTime
		avt->SetStateVariable("AbsoluteTimePosition", buffer.c_str());

		buffer = g_infoManager.GetDuration(TIME_FORMAT_HH_MM_SS);
		if (buffer.length() > 0)
		{
			avt->SetStateVariable("CurrentTrackDuration", buffer.c_str());
			avt->SetStateVariable("CurrentMediaDuration", buffer.c_str());
		}
		else
		{
			avt->SetStateVariable("CurrentTrackDuration", "00:00:00");
			avt->SetStateVariable("CurrentMediaDuration", "00:00:00");
		}

		avt->SetStateVariable("AVTransportURI", g_application.CurrentFile().c_str());
		avt->SetStateVariable("CurrentTrackURI", g_application.CurrentFile().c_str());

		NPT_String metadata;
		avt->GetStateVariableValue("AVTransportURIMetaData", metadata);
		// try to recreate the didl dynamically if not set
		if (metadata.IsEmpty())
		{
			GetMetadata(metadata);
		}
		avt->SetStateVariable("CurrentTrackMetadata", metadata);
		avt->SetStateVariable("AVTransportURIMetaData", metadata);
	}
	else
	{
		avt->SetStateVariable("TransportState", "STOPPED");
		avt->SetStateVariable("TransportPlaySpeed", "1");
		avt->SetStateVariable("NumberOfTracks", "0");
		avt->SetStateVariable("CurrentTrack", "0");
		avt->SetStateVariable("RelativeTimePosition", "00:00:00");
		avt->SetStateVariable("AbsoluteTimePosition", "00:00:00");
		avt->SetStateVariable("CurrentTrackDuration", "00:00:00");
		avt->SetStateVariable("CurrentMediaDuration", "00:00:00");
	}
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::GetMetadata
+---------------------------------------------------------------------*/
NPT_Result
CUPnPRenderer::GetMetadata(NPT_String& meta)
{
	NPT_Result res = NPT_FAILURE;
	const CFileItem &item = g_application.CurrentFileItem();
	NPT_String file_path;
	// TODO: CUPnPServer::BuildObject not yet ported
	//PLT_MediaObject* object = CUPnPServer::BuildObject(item, file_path, false);
	//if (object)
	//{
	//	res = PLT_Didl::ToDidl(*object, "*", meta);
	//	delete object;
	//}
	return res;
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::OnNext
+---------------------------------------------------------------------*/
NPT_Result
CUPnPRenderer::OnNext(PLT_ActionReference& action)
{
	g_application.getApplicationMessenger().PlayListPlayerNext();
	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::OnPause
+---------------------------------------------------------------------*/
NPT_Result
CUPnPRenderer::OnPause(PLT_ActionReference& action)
{
	if (!g_application.IsPaused())
		g_application.getApplicationMessenger().MediaPause();
	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::OnPlay
+---------------------------------------------------------------------*/
NPT_Result
CUPnPRenderer::OnPlay(PLT_ActionReference& action)
{
	if (g_application.IsPaused())
	{
		g_application.getApplicationMessenger().MediaPause();
	}
	else if (!g_application.IsPlaying())
	{
		NPT_String uri, meta;
		PLT_Service* service;
		// look for value set previously by SetAVTransportURI
		NPT_CHECK_SEVERE(FindServiceByType("urn:schemas-upnp-org:service:AVTransport:1", service));
		NPT_CHECK_SEVERE(service->GetStateVariableValue("AVTransportURI", uri));
		NPT_CHECK_SEVERE(service->GetStateVariableValue("AVTransportURIMetaData", meta));

		// if not set, use the current file being played
		PlayMedia(uri, meta);
	}
	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::OnPrevious
+---------------------------------------------------------------------*/
NPT_Result
CUPnPRenderer::OnPrevious(PLT_ActionReference& action)
{
	g_application.getApplicationMessenger().PlayListPlayerPrevious();
	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::OnStop
+---------------------------------------------------------------------*/
NPT_Result
CUPnPRenderer::OnStop(PLT_ActionReference& action)
{
	g_application.getApplicationMessenger().MediaStop();
	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::OnSetAVTransportURI
+---------------------------------------------------------------------*/
NPT_Result
CUPnPRenderer::OnSetAVTransportURI(PLT_ActionReference& action)
{
	NPT_String uri, meta;
	PLT_Service* service;
	NPT_CHECK_SEVERE(FindServiceByType("urn:schemas-upnp-org:service:AVTransport:1", service));

	NPT_CHECK_SEVERE(action->GetArgumentValue("CurrentURI", uri));
	NPT_CHECK_SEVERE(action->GetArgumentValue("CurrentURIMetaData", meta));

	// if not playing already, just keep around uri & metadata
	// and wait for play command
	if (!g_application.IsPlaying())
	{
		service->SetStateVariable("TransportState", "STOPPED");
		service->SetStateVariable("TransportStatus", "OK");
		service->SetStateVariable("TransportPlaySpeed", "1");
		service->SetStateVariable("AVTransportURI", uri);
		service->SetStateVariable("AVTransportURIMetaData", meta);

		NPT_CHECK_SEVERE(action->SetArgumentsOutFromStateVariable());
		return NPT_SUCCESS;
	}

	return PlayMedia(uri, meta, action.AsPointer());
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::PlayMedia
+---------------------------------------------------------------------*/
NPT_Result
CUPnPRenderer::PlayMedia(const char* uri, const char* meta, PLT_Action* action)
{
	PLT_Service* service;
	NPT_CHECK_SEVERE(FindServiceByType("urn:schemas-upnp-org:service:AVTransport:1", service));

	service->SetStateVariable("TransportState", "TRANSITIONING");
	service->SetStateVariable("TransportStatus", "OK");
	service->SetStateVariable("TransportPlaySpeed", "1");

	PLT_MediaObjectListReference list;
	PLT_MediaObject*             object = NULL;

	if (meta && NPT_SUCCEEDED(PLT_Didl::FromDidl(meta, list)))
	{
		list->Get(0, object);
	}

	if (object)
	{
		CFileItem item(uri, false);

		PLT_MediaItemResource* res = object->m_Resources.GetFirstItem();
		for (NPT_Cardinal i = 0; i < object->m_Resources.GetItemCount(); i++)
		{
			if (object->m_Resources[i].m_Uri == uri)
			{
				res = &object->m_Resources[i];
				break;
			}
		}
		for (NPT_Cardinal i = 0; i < object->m_Resources.GetItemCount(); i++)
		{
			if (object->m_Resources[i].m_ProtocolInfo.ToString().StartsWith("xbmc-get:"))
			{
				res = &object->m_Resources[i];
				item.SetPath(CStdString(res->m_Uri));
				break;
			}
		}

		if (res && res->m_ProtocolInfo.IsValid())
		{
			item.SetMimeType((const char*)res->m_ProtocolInfo.GetContentType());
		}

		item.m_dateTime.SetFromDateString((const char*)object->m_Date);
		item.m_strTitle = (const char*)object->m_Title;
		item.SetLabel((const char*)object->m_Title);
		item.SetLabelPreformated(true);
		item.SetThumbnailImage((const char*)object->m_ExtraInfo.album_art_uri);

		if (object->m_ObjectClass.type.StartsWith("object.item.audioItem"))
		{
			if (NPT_SUCCEEDED(CUPnP::PopulateTagFromObject(*item.GetMusicInfoTag(), *object, res)))
				item.SetLabelPreformated(false);
		}
		else if (object->m_ObjectClass.type.StartsWith("object.item.videoItem"))
		{
			if (NPT_SUCCEEDED(CUPnP::PopulateTagFromObject(*item.GetVideoInfoTag(), *object, res)))
				item.SetLabelPreformated(false);
		}

		g_application.getApplicationMessenger().MediaPlay(item);
	}
	else
	{
		g_application.getApplicationMessenger().MediaPlay((const char*)uri);
	}

	if (!g_application.IsPlaying())
	{
		service->SetStateVariable("TransportState", "STOPPED");
		service->SetStateVariable("TransportStatus", "ERROR_OCCURRED");
	}
	else
	{
		service->SetStateVariable("AVTransportURI", uri);
		service->SetStateVariable("AVTransportURIMetaData", meta);
	}

	if (action)
	{
		NPT_CHECK_SEVERE(action->SetArgumentsOutFromStateVariable());
	}
	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::OnSetVolume
+---------------------------------------------------------------------*/
NPT_Result
CUPnPRenderer::OnSetVolume(PLT_ActionReference& action)
{
	NPT_String volume;
	NPT_CHECK_SEVERE(action->GetArgumentValue("DesiredVolume", volume));
	g_application.SetVolume(atoi((const char*)volume));
	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::OnSetMute
+---------------------------------------------------------------------*/
NPT_Result
CUPnPRenderer::OnSetMute(PLT_ActionReference& action)
{
	NPT_String mute;
	NPT_CHECK_SEVERE(action->GetArgumentValue("DesiredMute", mute));
	if ((mute == "1") ^ g_settings.m_bMute)
		g_application.Mute();
	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::OnSeek
+---------------------------------------------------------------------*/
NPT_Result
CUPnPRenderer::OnSeek(PLT_ActionReference& action)
{
	if (!g_application.IsPlaying()) return NPT_ERROR_INVALID_STATE;

	NPT_String unit, target;
	NPT_CHECK_SEVERE(action->GetArgumentValue("Unit", unit));
	NPT_CHECK_SEVERE(action->GetArgumentValue("Target", target));

	if (!unit.Compare("REL_TIME"))
	{
		// converts target to seconds
		NPT_UInt32 seconds;
		NPT_CHECK_SEVERE(PLT_Didl::ParseTimeStamp(target, seconds));
		g_application.SeekTime((double)seconds);
	}

	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   static
+---------------------------------------------------------------------*/
CUPnP* CUPnP::upnp = NULL;

/*----------------------------------------------------------------------
|   CUPnP::CUPnP
+---------------------------------------------------------------------*/
CUPnP::CUPnP() :
	m_MediaBrowser(NULL),
	m_CtrlPointHolder(new CCtrlPointReferenceHolder()),
	m_RendererHolder(new CRendererReferenceHolder())
{
	// initialize upnp in broadcast listening mode for xbox
	m_UPnP = new PLT_UPnP(1900, true);

	// keep main IP around
	m_IP = g_application.getNetwork().m_networkinfo.ip;
	NPT_List<NPT_IpAddress> list;
	if (NPT_SUCCEEDED(PLT_UPnPMessageHelper::GetIPAddresses(list)))
	{
		m_IP = (*(list.GetFirstItem())).ToString();
	}

	CLog::Log(LOGNOTICE, "UPnP: Starting UPnP stack on %s", m_IP.c_str());

	// start upnp monitoring
	m_UPnP->Start();
}

/*----------------------------------------------------------------------
|   CUPnP::~CUPnP
+---------------------------------------------------------------------*/
CUPnP::~CUPnP()
{
	m_UPnP->Stop();
	StopClient();
	StopRenderer();

	delete m_UPnP;
	delete m_CtrlPointHolder;
	delete m_RendererHolder;
}

/*----------------------------------------------------------------------
|   CUPnP::GetInstance
+---------------------------------------------------------------------*/
CUPnP*
CUPnP::GetInstance()
{
	if (!upnp)
	{
		upnp = new CUPnP();
	}

	return upnp;
}

/*----------------------------------------------------------------------
|   CUPnP::ReleaseInstance
+---------------------------------------------------------------------*/
void
CUPnP::ReleaseInstance(bool bWait)
{
	if (upnp)
	{
		CUPnP* _upnp = upnp;
		upnp = NULL;

		if (bWait)
		{
			delete _upnp;
		}
		else
		{
			// since it takes a while to clean up
			// starts a detached thread to do this
			CUPnPCleaner* cleaner = new CUPnPCleaner(_upnp);
			cleaner->Start();
		}
	}
}

/*----------------------------------------------------------------------
|   CUPnP::StartClient
+---------------------------------------------------------------------*/
void
CUPnP::StartClient()
{
	if (!m_CtrlPointHolder->m_CtrlPoint.IsNull()) return;

	CLog::Log(LOGNOTICE, "UPnP: Starting client...");

	// create controlpoint, pass NULL to avoid sending a multicast search
	m_CtrlPointHolder->m_CtrlPoint = new PLT_CtrlPoint(NULL);

	// start it
	m_UPnP->AddCtrlPoint(m_CtrlPointHolder->m_CtrlPoint);

	// start browser
	m_MediaBrowser = new CMediaBrowser(m_CtrlPointHolder->m_CtrlPoint);

	// Xbox 360 can't receive multicast, but it can send it
	// Issue a search request on broadcast and multicast
	m_CtrlPointHolder->m_CtrlPoint->Discover(NPT_HttpUrl("255.255.255.255", 1900, "*"), "upnp:rootdevice", 1, 6000);
	m_CtrlPointHolder->m_CtrlPoint->Discover(NPT_HttpUrl("239.255.255.250", 1900, "*"), "upnp:rootdevice", 1, 6000);

	CLog::Log(LOGNOTICE, "UPnP: Client started, discovering devices...");
}

/*----------------------------------------------------------------------
|   CUPnP::StopClient
+---------------------------------------------------------------------*/
void
CUPnP::StopClient()
{
	if (m_CtrlPointHolder->m_CtrlPoint.IsNull()) return;

	CLog::Log(LOGNOTICE, "UPnP: Stopping client...");

	m_UPnP->RemoveCtrlPoint(m_CtrlPointHolder->m_CtrlPoint);
	m_CtrlPointHolder->m_CtrlPoint = NULL;

	delete m_MediaBrowser;
	m_MediaBrowser = NULL;
}

/*----------------------------------------------------------------------
|   JoinString
+---------------------------------------------------------------------*/
static const NPT_String JoinString(const NPT_List<NPT_String>& array, const NPT_String& delimiter)
{
	NPT_String result;

	for (NPT_List<NPT_String>::Iterator it = array.GetFirstItem(); it; it++)
		result += delimiter + (*it);

	if (result.IsEmpty())
		return "";
	else
		return result.SubString(delimiter.GetLength());
}

/*----------------------------------------------------------------------
|   CUPnP::PopulateTagFromObject
+---------------------------------------------------------------------*/
int CUPnP::PopulateTagFromObject(MUSIC_INFO::CMusicInfoTag& tag,
								 PLT_MediaObject&           object,
								 PLT_MediaItemResource*     resource /* = NULL */)
{
	tag.SetTitle((const char*)object.m_Title);
	tag.SetArtist((const char*)object.m_Creator);

	for (PLT_PersonRoles::Iterator it = object.m_People.artists.GetFirstItem(); it; it++)
	{
		if      (it->role == "")            tag.SetArtist((const char*)it->name);
		else if (it->role == "Performer")   tag.SetArtist((const char*)it->name);
		else if (it->role == "AlbumArtist") tag.SetAlbumArtist((const char*)it->name);
	}

	tag.SetTrackNumber(object.m_MiscInfo.original_track_number);
	tag.SetGenre((const char*)JoinString(object.m_Affiliation.genre, " / "));
	tag.SetAlbum((const char*)object.m_Affiliation.album);

	if (resource)
		tag.SetDuration(resource->m_Duration);

	tag.SetLoaded();
	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CUPnP::PopulateTagFromObject
+---------------------------------------------------------------------*/
int CUPnP::PopulateTagFromObject(CVideoInfoTag&         tag,
								 PLT_MediaObject&       object,
								 PLT_MediaItemResource* resource /* = NULL */)
{
	if(!object.m_Recorded.program_title.IsEmpty())
	{
		int episode;
		int season;
		int title = object.m_Recorded.program_title.Find(" : ");

		if(sscanf(object.m_Recorded.program_title, "S%2dE%2d", &episode, &season) == 2 && title >= 0)
		{
			tag.m_strTitle = object.m_Recorded.program_title.SubString(title + 3);
			tag.m_iEpisode = episode;
			tag.m_iSeason  = season;
		}
		else
		{
			tag.m_strTitle = object.m_Recorded.program_title;
			tag.m_iSeason  = object.m_Recorded.episode_number / 100;
			tag.m_iEpisode = object.m_Recorded.episode_number % 100;
		}
	}
	else
		tag.m_strTitle = object.m_Title;

	tag.m_strGenre    = JoinString(object.m_Affiliation.genre, " / ");
	tag.m_strDirector = object.m_People.director;
	tag.m_strTagLine  = object.m_Description.description;
	tag.m_strPlot     = object.m_Description.long_description;
	tag.m_strShowTitle = object.m_Recorded.series_title;

	if(resource)
	  tag.m_strRuntime.Format("%d",resource->m_Duration);

	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CUPnP::CreateRenderer
+---------------------------------------------------------------------*/
CUPnPRenderer*
CUPnP::CreateRenderer(int port /* = 0 */)
{
	CUPnPRenderer* device =
		new CUPnPRenderer("XBMC 360: Media Renderer",
		                  true,
		                  NULL,
		                  port);

	device->m_ModelName = "XBMC 360";
	device->m_ModelNumber = "1.0";
	device->m_ModelDescription = "XBMC 360 Media Center - Media Renderer";
	device->m_Manufacturer = "Team XBMC";

	return device;
}

/*----------------------------------------------------------------------
|   CUPnP::StartRenderer
+---------------------------------------------------------------------*/
void CUPnP::StartRenderer()
{
	if (!m_RendererHolder->m_Device.IsNull()) return;

	CLog::Log(LOGNOTICE, "UPnP: Starting renderer...");

	m_RendererHolder->m_Device = CreateRenderer(0);

	// tell controller to ignore ourselves from list of upnp servers
	if (!m_CtrlPointHolder->m_CtrlPoint.IsNull())
	{
		m_CtrlPointHolder->m_CtrlPoint->IgnoreUUID(m_RendererHolder->m_Device->GetUUID());
	}

	NPT_Result res = m_UPnP->AddDevice(m_RendererHolder->m_Device);

	if (NPT_SUCCEEDED(res))
	{
		CLog::Log(LOGNOTICE, "UPnP: Renderer started successfully");
	}
	else
	{
		CLog::Log(LOGERROR, "UPnP: Failed to start renderer");
		m_RendererHolder->m_Device = NULL;
	}
}

/*----------------------------------------------------------------------
|   CUPnP::StopRenderer
+---------------------------------------------------------------------*/
void CUPnP::StopRenderer()
{
	if (m_RendererHolder->m_Device.IsNull()) return;

	CLog::Log(LOGNOTICE, "UPnP: Stopping renderer...");

	m_UPnP->RemoveDevice(m_RendererHolder->m_Device);
	m_RendererHolder->m_Device = NULL;
}

/*----------------------------------------------------------------------
|   CUPnP::UpdateState
+---------------------------------------------------------------------*/
void CUPnP::UpdateState()
{
	if (!m_RendererHolder->m_Device.IsNull())
		((CUPnPRenderer*)m_RendererHolder->m_Device.AsPointer())->UpdateState();
}