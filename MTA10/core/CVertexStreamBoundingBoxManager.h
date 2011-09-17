/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:
*  PURPOSE:
*  DEVELOPERS:  D3DOCD
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "CBox.h"

//
//  Used to track blocks of vertices in a vertex buffer
//
class CRangeBoundsMap
{
    struct SItem
    {
        uint uiStart;
        uint uiLength;
        CBox box;
    };

    std::map < uint64, SItem > itemMap;

    uint64 MakeKey ( uint64 uiStart, uint64 uiLength ) const
    {
        return uiStart << 32 | uiLength;
    }
public:

    void SetRange ( const uint uiStart, const uint uiLength, const CBox& boundingBox )
    {
        const uint64 key = MakeKey ( uiStart, uiLength );
        SItem item = { 0, 0, boundingBox };
        MapSet ( itemMap, key, item );
    }

    bool IsRangeSet ( const uint uiStart, const uint uiLength, CBox& outBoundingBox ) const
    {
        const uint64 key = MakeKey ( uiStart, uiLength );
        if ( const SItem* pItem = MapFind ( itemMap, key ) )
        {
            outBoundingBox = pItem->box;
            return true;
        }
        return false;
    }

    void UnsetRange ( const uint uiStart, const uint uiLength )
    {
        for ( std::map < uint64, SItem >::iterator iter = itemMap.begin () ; iter != itemMap.end () ; )
        {
            const SItem& item = iter->second;

            // Detect overlap of the ranges
            if ( ( uiStart + uiLength >= item.uiStart ) && ( item.uiStart + item.uiLength > uiStart ) )
                itemMap.erase ( iter++ );
            else
                ++iter;
        }
    }
};


//
// SStreamBoundsInfo
//
struct SStreamBoundsInfo
{
    CRangeBoundsMap                 ConvertedRanges;
};


struct SCurrentStateInfo2
{
    // Info to DrawIndexPrimitive
    struct
    {
        D3DPRIMITIVETYPE                PrimitiveType;
        INT                             BaseVertexIndex;
        UINT                            MinVertexIndex;
        UINT                            NumVertices;
        UINT                            startIndex;
        UINT                            primCount;
    } args;

    // Render state
    struct
    {
        IDirect3DVertexBuffer9*         pStreamData;
        UINT                            OffsetInBytes;
        UINT                            Stride;
        WORD                            elementOffset;
    } stream;

    struct
    {
        IDirect3DVertexDeclaration9*    pVertexDeclaration;
        D3DVERTEXELEMENT9               elements[MAXD3DDECLLENGTH];
        UINT                            numElements;
        D3DVERTEXBUFFER_DESC            VertexBufferDesc;
    } decl;
};


//
// CVertexStreamBoundingBoxManager
//
class CVertexStreamBoundingBoxManager
{
public:
    ZERO_ON_NEW
                            CVertexStreamBoundingBoxManager      ( void );
    virtual                 ~CVertexStreamBoundingBoxManager     ( void );

    void                    OnDeviceCreate                      ( IDirect3DDevice9* pDevice );
    float                   GetDistanceSqToGeometry             ( D3DPRIMITIVETYPE PrimitiveType,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount );
    void                    OnVertexBufferDestroy               ( IDirect3DVertexBuffer9* pStreamData );
    void                    OnVertexBufferRangeInvalidated      ( IDirect3DVertexBuffer9* pStreamData, uint Offset, uint Size );

    static CVertexStreamBoundingBoxManager* GetSingleton         ( void );
protected:
    float                   CalcDistanceSq                      ( const SCurrentStateInfo2& state, const CBox& boundingBox );
    bool                    CheckCanDoThis                      ( SCurrentStateInfo2& current );
    bool                    GetVertexStreamBoundingBox          ( const SCurrentStateInfo2& state, CBox& outBoundingBox );
    bool                    ComputeVertexStreamBoundingBox      ( const SCurrentStateInfo2& current, uint ReadOffsetStart, uint ReadSize, CBox& outBoundingBox );

    SStreamBoundsInfo*      GetStreamBoundsInfo                 ( IDirect3DVertexBuffer9* pStreamData );
    SStreamBoundsInfo*      CreateStreamBoundsInfo              ( const SCurrentStateInfo2& current );

    IDirect3DDevice9*                           m_pDevice;
    std::map < void*, SStreamBoundsInfo >       m_StreamBoundsInfoMap;
    static CVertexStreamBoundingBoxManager*     ms_Singleton;
};
