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
#include "xbox\Network.h"
#include "utils\Log.h"
#include "guilib\GUIWindowManager.h"
#include "guilib\GUIUserMessages.h"
#include "music\tags\MusicInfoTag.h"

#include "NptStrings.h"
#include "Platinum.h"
#include "PltSyncMediaBrowser.h"
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
|   CUPnPCleaner class
+---------------------------------------------------------------------*/
class CUPnPCleaner : public NPT_Thread
{
public:
    CUPnPCleaner(CUPnP* upnp) : NPT_Thread(true), m_UPnP(upnp) {}
    void Run() {
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
|   static
+---------------------------------------------------------------------*/
CUPnP* CUPnP::upnp = NULL;

/*----------------------------------------------------------------------
|   CUPnP::CUPnP
+---------------------------------------------------------------------*/
CUPnP::CUPnP() :
    m_MediaBrowser(NULL),
    m_CtrlPointHolder(new CCtrlPointReferenceHolder())
{
    // initialize upnp in broadcast listening mode for xbox
    m_UPnP = new PLT_UPnP(1900, true);

    // keep main IP around
    m_IP = g_application.getNetwork().m_networkinfo.ip;
    NPT_List<NPT_IpAddress> list;
    if (NPT_SUCCEEDED(PLT_UPnPMessageHelper::GetIPAddresses(list))) {
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

    delete m_UPnP;
    delete m_CtrlPointHolder;
}

/*----------------------------------------------------------------------
|   CUPnP::GetInstance
+---------------------------------------------------------------------*/
CUPnP*
CUPnP::GetInstance()
{
    if (!upnp) {
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
    if (upnp) {
        CUPnP* _upnp = upnp;
        upnp = NULL;

        if (bWait) {
            delete _upnp;
        } else {
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
    for (PLT_PersonRoles::Iterator it = object.m_People.artists.GetFirstItem(); it; it++) {
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
