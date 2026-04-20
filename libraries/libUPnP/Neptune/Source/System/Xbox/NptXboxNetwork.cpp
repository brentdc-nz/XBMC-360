/*****************************************************************
|
|  Neptune - Network :: Xbox Winsock Implementation
|
|  (c) 2001-2005 Gilles Boccon-Gibod
|  Author: Gilles Boccon-Gibod (bok@bok.net)
|
****************************************************************/

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include <xtl.h>
#include <winsockx.h>

#include "NptNetwork.h"
#include "NptXboxNetwork.h"

/*----------------------------------------------------------------------
|   static initializer
+---------------------------------------------------------------------*/
NPT_WinsockSystem::NPT_WinsockSystem() {
    // Xbox 360: use XNET_STARTUP_BYPASS_SECURITY to allow LAN broadcast/multicast
    XNetStartupParams xnsp = { sizeof(xnsp), 0 };
    xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;
    INT err = XNetStartup(&xnsp);

    WORD    wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);
    WSAStartup( wVersionRequested, &wsaData );
}
NPT_WinsockSystem::~NPT_WinsockSystem() {
    WSACleanup();
    XNetCleanup();
}
NPT_WinsockSystem NPT_WinsockSystem::Initializer;

/*----------------------------------------------------------------------
|       NPT_NetworkInterface::GetNetworkInterfaces
+---------------------------------------------------------------------*/
NPT_Result
NPT_NetworkInterface::GetNetworkInterfaces(NPT_List<NPT_NetworkInterface*>& interfaces)
{
    XNADDR xna;
    DWORD  state;
    do {
        state = XNetGetTitleXnAddr(&xna);
        Sleep(100);
    } while (state == XNET_GET_XNADDR_PENDING);

    if (state & XNET_GET_XNADDR_STATIC || state & XNET_GET_XNADDR_DHCP) {
        NPT_IpAddress primary_address(ntohl(xna.ina.s_addr));
        NPT_IpAddress netmask; /* no support for netmask */
        NPT_IpAddress broadcast_address(ntohl(xna.ina.s_addr));
        NPT_Flags     flags = NPT_NETWORK_INTERFACE_FLAG_BROADCAST;

        NPT_MacAddress mac;
        if (state & XNET_GET_XNADDR_ETHERNET) {
            mac.SetAddress(NPT_MacAddress::TYPE_ETHERNET, xna.abEnet, 6);
        }

        // create an interface object
        char iface_name[5];
        iface_name[0] = 'i';
        iface_name[1] = 'f';
        iface_name[2] = '0';
        iface_name[3] = '0';
        iface_name[4] = '\0';
        NPT_NetworkInterface* iface = new NPT_NetworkInterface(iface_name, mac, flags);

        // set the interface address
        NPT_NetworkInterfaceAddress iface_address(
            primary_address,
            broadcast_address,
            NPT_IpAddress::Any,
            netmask);
        iface->AddAddress(iface_address);  

        // add the interface to the list
        interfaces.Add(iface);  
    }

    return NPT_SUCCESS;
}

