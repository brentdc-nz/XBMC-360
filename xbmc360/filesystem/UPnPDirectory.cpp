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

#include "UPnPDirectory.h"
#include "FileItem.h"
#include "File.h"
#include "URL.h"
#include "network\UPnP.h"
#include "utils\Log.h"

#include "Platinum.h"
#include "PltSyncMediaBrowser.h"

// Pre-swap an item's remote thumb URL to its on-disk cached .tbn if one already
// exists. Items without a cached thumb keep the remote URL so CVideoThumbLoader
// (LoadRemoteThumb) can still download and cache it on first visit. This avoids
// the visible "thumbs flicker / slowly reappear" on window re-entry caused by
// the port lacking CLargeTextureManager (which in xbmc4xbox loads http URLs
// live while the background cache job runs).
static void PreSwapToCachedVideoThumb(CFileItem* pItem)
{
	if (!pItem || !pItem->HasThumbnail()) return;
	CStdString cached(pItem->GetCachedVideoThumb());
	if (!cached.IsEmpty() && XFILE::CFile::Exists(cached))
		pItem->SetThumbnailImage(cached);
}

using namespace XFILE;

/*----------------------------------------------------------------------
|   FindDeviceWait
+---------------------------------------------------------------------*/
static bool FindDeviceWait(CUPnP* upnp, const char* uuid, PLT_DeviceDataReference& device)
{
    bool client_started = upnp->IsClientStarted();
    upnp->StartClient();

    // look for device in our list
    // (and wait for it to respond for 5 secs if we're just starting upnp client)
    NPT_TimeStamp watchdog;
    NPT_System::GetCurrentTimeStamp(watchdog);
    watchdog += 5.f;

    for (;;) {
        if (NPT_SUCCEEDED(upnp->m_MediaBrowser->FindServer(uuid, device)) && !device.IsNull())
            break;

        // fail right away if device not found and upnp client was already running
        if (client_started)
            return false;

        // otherwise check if we've waited long enough without success
        NPT_TimeStamp now;
        NPT_System::GetCurrentTimeStamp(now);
        if (now > watchdog)
            return false;

        // sleep a bit and try again
        NPT_System::Sleep(NPT_TimeInterval(1, 0));
    }

    return !device.IsNull();
}

/*----------------------------------------------------------------------
|   CUPnPDirectory::GetFriendlyName
+---------------------------------------------------------------------*/
const char*
CUPnPDirectory::GetFriendlyName(const char* url)
{
    NPT_String path = url;
    if (!path.EndsWith("/")) path += "/";

    if (path.Left(7).Compare("upnp://", true) != 0) {
        return NULL;
    } else if (path.Compare("upnp://", true) == 0) {
        return "UPnP Media Servers (Auto-Discover)";
    }

    // look for nextslash
    int next_slash = path.Find('/', 7);
    if (next_slash == -1)
        return NULL;

    NPT_String uuid = path.SubString(7, next_slash-7);

    // look for device
    PLT_DeviceDataReference device;
    if(!FindDeviceWait(CUPnP::GetInstance(), uuid, device))
        return NULL;

    return (const char*)device->GetFriendlyName();
}

/*----------------------------------------------------------------------
|   CUPnPDirectory::GetResource
+---------------------------------------------------------------------*/
bool CUPnPDirectory::GetResource(const CURL& path, CFileItem &item)
{
    if(path.GetProtocol() != "upnp")
        return false;

    CUPnP* upnp = CUPnP::GetInstance();
    if(!upnp)
        return false;

    CStdString uuid   = path.GetHostName();
    CStdString object = path.GetFileName();
    object.TrimRight("/");
    CURL::Decode(object);

    PLT_DeviceDataReference device;
    if(!FindDeviceWait(upnp, uuid.c_str(), device)) {
        CLog::Log(LOGERROR, "CUPnPDirectory::GetResource - unable to find uuid %s", uuid.c_str());
        return false;
    }

    PLT_MediaObjectListReference list;
    if (NPT_FAILED(upnp->m_MediaBrowser->BrowseSync(device, object.c_str(), list, true))) {
        CLog::Log(LOGERROR, "CUPnPDirectory::GetResource - unable to find object %s", object.c_str());
        return false;
    }

    if (list.IsNull() || !list->GetItemCount()) {
        CLog::Log(LOGERROR, "CUPnPDirectory::GetResource - no items returned for object %s", object.c_str());
        return false;
    }

    PLT_MediaObjectList::Iterator entry = list->GetFirstItem();
    if (entry == 0)
        return false;

    if((*entry)->m_Resources.GetItemCount()) {
        PLT_MediaItemResource& resource = (*entry)->m_Resources[0];

        // store original path
        item.SetProperty("original_listitem_url", item.m_strPath);

        // set path to first resource URI
        item.m_strPath = (const char*) resource.m_Uri;
    }

    return true;
}

/*----------------------------------------------------------------------
|   CUPnPDirectory::GetDirectory
+---------------------------------------------------------------------*/
bool
CUPnPDirectory::GetDirectory(const CStdString& strPath, CFileItemList &items)
{
    CUPnP* upnp = CUPnP::GetInstance();

    // We accept upnp://devuuid/[item_id/]
    NPT_String path = strPath.c_str();
    if (!path.StartsWith("upnp://", true)) {
        return false;
    }

    if (path.Compare("upnp://", true) == 0) {
        upnp->StartClient();

        // root -> get list of devices
        const NPT_Lock<PLT_DeviceDataReferenceList>& devices = upnp->m_MediaBrowser->GetMediaServers();
        NPT_List<PLT_DeviceDataReference>::Iterator device = devices.GetFirstItem();
        while (device) {
            NPT_String name = (*device)->GetFriendlyName();
            NPT_String uuid = (*device)->GetUUID();

            CFileItemPtr pItem(new CFileItem((const char*)name));
            pItem->m_strPath = CStdString((const char*) "upnp://" + uuid + "/");
            pItem->m_bIsFolder = true;
            pItem->SetThumbnailImage((const char*)(*device)->GetIconUrl("image/jpeg"));

            items.Add(pItem);

            ++device;
        }
    } else {
        if (!path.EndsWith("/")) path += "/";

        // look for nextslash
        int next_slash = path.Find('/', 7);

        NPT_String uuid = (next_slash==-1)?path.SubString(7):path.SubString(7, next_slash-7);
        NPT_String object_id = (next_slash==-1)?"":path.SubString(next_slash+1);
        object_id.TrimRight("/");
        if (object_id.GetLength()) {
            CStdString tmp = (char*) object_id;
            CURL::Decode(tmp);
            object_id = tmp;
        }

        // try to find the device with wait on startup
        PLT_DeviceDataReference device;
        if (!FindDeviceWait(upnp, uuid, device))
            goto failure;

        // issue a browse request with object_id
        // if object_id is empty use "0" for root
        object_id = object_id.IsEmpty()?"0":object_id;

        // special case for Windows Media Connect and WMP11
        if (object_id == "0" && ((device->GetFriendlyName().Find("Windows Media Connect", 0, true) >= 0) ||
                                 (device->m_ModelName == "Windows Media Player Sharing"))) {
            // default to video
            object_id = "2";
        }

        // if error, return now, the device could have gone away
        PLT_MediaObjectListReference list;
        NPT_Result res = upnp->m_MediaBrowser->BrowseSync(device, object_id, list);
        if (NPT_FAILED(res)) goto failure;

        // empty list is ok
        if (list.IsNull()) goto cleanup;

        PLT_MediaObjectList::Iterator entry = list->GetFirstItem();
        while (entry) {
            NPT_String ObjectClass = (*entry)->m_ObjectClass.type.ToLowercase();

            CFileItemPtr pItem(new CFileItem((const char*)(*entry)->m_Title));
            pItem->SetLabelPreformated(true);
            pItem->m_bIsFolder = (*entry)->IsContainer();

            CStdString id = (char*) (*entry)->m_ObjectID;
            CURL::Encode(id);
            pItem->m_strPath = CStdString((const char*) "upnp://" + uuid + "/" + id.c_str());

            // if it's a container, format a string as upnp://uuid/object_id/
            if (pItem->m_bIsFolder) {
                pItem->m_strPath += "/";
            } else {
                if ((*entry)->m_Resources.GetItemCount()) {
                    PLT_MediaItemResource& resource = (*entry)->m_Resources[0];

                    // set size metadata
                    if (resource.m_Size != (NPT_LargeSize)-1) {
                        pItem->m_dwSize = resource.m_Size;
                    }
                }
            }

            // look for date
            if((*entry)->m_Description.date.GetLength()) {
                SYSTEMTIME time = {};
                sscanf((*entry)->m_Description.date, "%hu-%hu-%huT%hu:%hu:%hu",
                       &time.wYear, &time.wMonth, &time.wDay, &time.wHour, &time.wMinute, &time.wSecond);
                pItem->m_dateTime = time;
            }

            // if there is a thumbnail available set it here
            if((*entry)->m_ExtraInfo.album_art_uri.GetLength())
                pItem->SetThumbnailImage((const char*) (*entry)->m_ExtraInfo.album_art_uri);
            else if((*entry)->m_Description.icon_uri.GetLength())
                pItem->SetThumbnailImage((const char*) (*entry)->m_Description.icon_uri);

            // If we already have a cached .tbn on disk, swap the remote URL for
            // the local path up-front so the first GUI paint shows the thumb
            // without waiting for the background thumb loader.
            PreSwapToCachedVideoThumb(pItem.get());

            items.Add(pItem);

            ++entry;
        }
    }

cleanup:
    return true;

failure:
    return false;
}
