/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CSyncDebug.cpp
*  PURPOSE:     Sync debugger
*  DEVELOPERS:  Christian Myhre Lundheim <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

#ifndef snprintf
#define snprintf _snprintf
#endif

// Debug mode only interface
#ifdef MTA_DEBUG

CSyncDebug::CSyncDebug ( CClientManager* pManager )
{
    // Init
    m_pManager = pManager;

    m_pPlayer = NULL;

    m_ulLastUpdateTime = CClientTime::GetTime ();
    m_uiLastPacketsSent = m_uiPacketsSent = g_pNet->GetPacketsSent ();
    m_uiLastPacketsReceived = m_uiPacketsReceived = g_pNet->GetGoodPacketsReceived () + g_pNet->GetBadPacketsReceived ();
    m_uiLastBitsReceived = m_uiBitsReceived = g_pNet->GetBitsReceived ();
    m_uiLastBitsReceived = m_uiBitsSent = g_pNet->GetBitsSent ();

    m_usFakeLagVariance = 0;
    m_usFakeLagPing = 0;
}


CSyncDebug::~CSyncDebug ( void )
{

}


void CSyncDebug::Unreference ( CClientPlayer& Player )
{
    // If this is the player we're debugging, unreference him from us
    if ( m_pPlayer == &Player )
    {
        m_pPlayer = NULL;
    }
}


void CSyncDebug::Attach ( CClientPlayer& Player )
{
    // Remember that player and do an update
    m_pPlayer = &Player;
    OnUpdate ();
}


void CSyncDebug::Detach ( void )
{
    m_pPlayer = NULL;
}


void CSyncDebug::OnPulse ( void )
{
    // Are we debugging some player?
    if ( m_pPlayer )
    {
        // Time to update?
        if ( CClientTime::GetTime () >= m_ulLastUpdateTime + 1000 )
        {
            OnUpdate ();
        }

        // Draw stats
        OnDraw ();
    }
}


void CSyncDebug::OnDraw ( void )
{
    // Grab his data
    CVector vecPosition;
    m_pPlayer->GetPosition ( vecPosition );
    float fCurrentRotation = m_pPlayer->GetCurrentRotation ();

    // Write it to a string
    SString strBuffer (
               "== PLAYER ==\n"
               "Nick: %s\n"
               "Position: %f %f %f\n"
               "Rotation: %f\n"
               "Is dead: %u\n"
               "Ping to server: %u\n"
               "Player syncs: %u\n"
               "Vehicle syncs: %u\n"
               "Key syncs: %u",

               m_pPlayer->GetNick (),
               vecPosition.fX, vecPosition.fY, vecPosition.fZ,
               fCurrentRotation,
               m_pPlayer->IsDead (),
               m_pPlayer->GetLatency (),
               m_pPlayer->GetPlayerSyncCount (),
               m_pPlayer->GetVehicleSyncCount (),
               m_pPlayer->GetKeySyncCount () );

    // Print it
    m_pManager->GetDisplayManager ()->DrawText2D ( strBuffer, CVector ( 0.02f, 0.31f, 0 ), 1.0f, 0xFFFFFFFF );


    // ******* GENERAL NET DATA *******
    // Bytes sent totally
    SString strBytesSent = GetDataUnit ( g_pNet->GetBitsSent () / 8 );

    // Bytes received totally
    SString strBytesRecv = GetDataUnit ( g_pNet->GetBitsReceived () / 8 );

    // Receive rate
    SString strRecvRate  = GetDataUnit ( ( m_uiBitsReceived - m_uiLastBitsReceived ) / 8 );

    // Send rate
    SString strSendRate  = GetDataUnit ( ( m_uiBitsSent - m_uiLastBitsSent ) / 8 );

    // Draw the background for global stats
    float fResWidth = static_cast < float > ( g_pCore->GetGraphics ()->GetViewportWidth () );
    float fResHeight = static_cast < float > ( g_pCore->GetGraphics ()->GetViewportHeight () );
    g_pGame->GetHud ()->Draw2DPolygon ( 0.75f * fResWidth, 0.30f * fResHeight,
                                        1.0f * fResWidth, 0.30f * fResHeight,
                                        0.75f * fResWidth, 0.80f * fResHeight,
                                        1.0f * fResWidth, 0.80f * fResHeight,
                                        0x78000000 );

    // Populate a string to print
    strBuffer.Format (
               "Ping: %u\n"
               "Fakelag: %u (%u)\n"
               "Packets recv: %u\n"
               "Packets sent: %u\n"
               "Bytes recv: %s\n"
               "Bytes sent: %s\n"
               "Datarate: %s/s / %s/s\n"
               "Packet rate: %u / %u",

               g_pNet->GetPing (),
               m_usFakeLagPing, m_usFakeLagVariance,

               g_pNet->GetGoodPacketsReceived () + g_pNet->GetBadPacketsReceived (),
               g_pNet->GetPacketsSent (),
               strBytesRecv.c_str (),
               strBytesSent.c_str (),
               strRecvRate.c_str (),
               strSendRate.c_str (),
               m_uiPacketsReceived - m_uiLastPacketsReceived,
               m_uiPacketsSent - m_uiLastPacketsSent );

    // Print it
    m_pManager->GetDisplayManager ()->DrawText2D ( strBuffer, CVector ( 0.76f, 0.31f, 0 ), 1.0f, 0xFFFFFFFF );
}


void CSyncDebug::OnUpdate ( void )
{
    m_ulLastUpdateTime = CClientTime::GetTime ();

    m_uiLastPacketsSent = m_uiPacketsSent;
    m_uiLastPacketsReceived = m_uiPacketsReceived;
    m_uiLastBitsReceived = m_uiBitsReceived;
    m_uiLastBitsSent = m_uiBitsSent;

    m_uiPacketsSent = g_pNet->GetPacketsSent ();
    m_uiPacketsReceived = g_pNet->GetGoodPacketsReceived () + g_pNet->GetBadPacketsReceived ();
    m_uiBitsReceived = g_pNet->GetBitsReceived ();
    m_uiBitsSent = g_pNet->GetBitsSent ();
}


#endif
