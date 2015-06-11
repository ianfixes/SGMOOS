#pragma once
#include "atlscrl.h"
#include "atlmisc.h"

template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CZoomWindowImpl : public CWindowImpl<T, TBase, TWinTraits>, public CScrollImpl< T >
{
public:
  BEGIN_MSG_MAP(CZoomWindowImpl< T >)
    MESSAGE_HANDLER(WM_VSCROLL, CScrollImpl< T >::OnVScroll)
    MESSAGE_HANDLER(WM_HSCROLL, CScrollImpl< T >::OnHScroll)
    MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
#if !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400))
    MESSAGE_HANDLER(m_uMsgMouseWheel, CScrollImpl< T >::OnMouseWheel)
#endif //(_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)
    MESSAGE_HANDLER(WM_SETTINGCHANGE, CScrollImpl< T >::OnSettingChange)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_PRINTCLIENT, OnPaint)
    ALT_MSG_MAP(1)
    COMMAND_ID_HANDLER(ID_SCROLL_UP, CScrollImpl< T >::OnScrollUp)
    COMMAND_ID_HANDLER(ID_SCROLL_DOWN, CScrollImpl< T >::OnScrollDown)
    COMMAND_ID_HANDLER(ID_SCROLL_PAGE_UP, CScrollImpl< T >::OnScrollPageUp)
    COMMAND_ID_HANDLER(ID_SCROLL_PAGE_DOWN, CScrollImpl< T >::OnScrollPageDown)
    COMMAND_ID_HANDLER(ID_SCROLL_TOP, CScrollImpl< T >::OnScrollTop)
    COMMAND_ID_HANDLER(ID_SCROLL_BOTTOM, CScrollImpl< T >::OnScrollBottom)
    COMMAND_ID_HANDLER(ID_SCROLL_LEFT, CScrollImpl< T >::OnScrollLeft)
    COMMAND_ID_HANDLER(ID_SCROLL_RIGHT, CScrollImpl< T >::OnScrollRight)
    COMMAND_ID_HANDLER(ID_SCROLL_PAGE_LEFT, CScrollImpl< T >::OnScrollPageLeft)
    COMMAND_ID_HANDLER(ID_SCROLL_PAGE_RIGHT, CScrollImpl< T >::OnScrollPageRight)
    COMMAND_ID_HANDLER(ID_SCROLL_ALL_LEFT, CScrollImpl< T >::OnScrollAllLeft)
    COMMAND_ID_HANDLER(ID_SCROLL_ALL_RIGHT, CScrollImpl< T >::OnScrollAllRight)
  END_MSG_MAP()
    
  CZoomWindowImpl() : 
    m_dZoomScale(1.0),
    m_OrigSizeAll(m_sizeAll),
    m_OrigSizeLine(m_sizeLine),
    m_OrigSizePage(m_sizePage)

  { }
  
  
  void SetScrollSize(int cx, int cy, BOOL bRedraw = TRUE)
  {
    m_dZoomScale = 1;
    m_OrigSizeAll = m_sizeAll = CSize(cx,cy);
    RecalcBars(bRedraw, TRUE);

    m_OrigSizeLine = m_sizeLine;
    m_OrigSizePage = m_sizePage;
    ZoomLevelChanged();
  } 
  
  void SetScrollSize(SIZE size, BOOL bRedraw = TRUE)
  {
    SetScrollSize(size.cx, size.cy, bRedraw);
  }
  
  LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
  {
    bHandled = FALSE;

		m_sizeClient.cx = GET_X_LPARAM(lParam);
		m_sizeClient.cy = GET_Y_LPARAM(lParam);

    RecalcBars(TRUE);

    return 1;
  }


  LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
  {
    T* pT = static_cast<T*>(this);
    ATLASSERT(::IsWindow(pT->m_hWnd));
    if(wParam != NULL)
    {
      CDCHandle dc = (HDC)wParam;
      PrepareDC(dc);
      pT->DoPaint(dc);
    }
    else
    {
      CPaintDC dc(pT->m_hWnd);
      PrepareDC(dc);
      pT->DoPaint(dc.m_hDC);
    }
    return 0;
  }
  
  LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
  {
    const double eps = 0.000001;
    //Special logic for CONTROL and mousewheel which is zooming.
    int nFlags = LOWORD(wParam);
    
    if (nFlags == MK_CONTROL)
    {
      T* pT = static_cast<T*>(this);
      ATLASSERT(::IsWindow(pT->m_hWnd));
      
#if (_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)
      uMsg;
      int zDelta = (int)(short)HIWORD(wParam);
#else
      int zDelta = (uMsg == WM_MOUSEWHEEL) ? (int)(short)HIWORD(wParam) : (int)wParam;
#endif //!((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400))
      
      CPoint ptMouse;
      ptMouse.x = GET_X_LPARAM(lParam); 
      ptMouse.y = GET_Y_LPARAM(lParam);
      ScreenToClient(&ptMouse);
      DPtoLP(&ptMouse);
      
      if (zDelta > 0)
      {
        if ( m_dZoomScale >= 1.0 - eps )
        {
          SetZoomLevel( (long) m_dZoomScale + 1.0  );
        }
        else 
        {
          long denom = (long) ( 1.0 / m_dZoomScale ) - 1;
          if ( denom == 0 ) 
            denom = 1;
          SetZoomLevel( 1.0 / denom );
        }
      }
      else 
      {
        if ( m_dZoomScale > 1.0 )
          SetZoomLevel( (long) m_dZoomScale - 1.0  );
        else 
        {
          long denom = (long) ( 1.0 / m_dZoomScale) + 1;
          if ( denom == 0 ) 
            denom = 1;
          SetZoomLevel( 1.0 / denom);
        }
      }
      UpdateViewPort( &ptMouse ); 
    }
    else if (nFlags == MK_SHIFT)
    {
      
      ATLTRACE("Got a shift key!\n");
    }
    else
      return CScrollImpl<T>::OnMouseWheel(uMsg, wParam, lParam, bHandled);
    
    return TRUE;
  } 
  
  // zooming functions
  void ZoomIn(POINT* point = NULL, double delta = 0.25) 
  {
    SetZoomLevel( m_dZoomScale + delta);
    UpdateViewPort( point );
  }
  void ZoomOut(POINT* point = NULL, double delta = 0.25) 
  {
    SetZoomLevel( m_dZoomScale - delta);
    UpdateViewPort( point );
  }
  
  double GetZoomLevel() { return m_dZoomScale;}
  
  virtual void ZoomLevelChanged(void) {};
  virtual void SetZoomLevel(double zoomLevel) { m_dZoomScale = zoomLevel /** 10000 / 10000*/; ATLTRACE("ZoomLevel: %f\n", m_dZoomScale);};
  
  // Zooming utility functions
  void DPtoLP (LPPOINT lpPoints, int nCount = 1)
  {
    // Convert to logical units
    // Called from View when no DC is available
    CClientDC dc(this->m_hWnd);
    PrepareDC(dc); 
    dc.DPtoLP(lpPoints, nCount);
  }
  
  void LPtoDP (LPPOINT lpPoints, int nCount = 1)
  {
    CClientDC dc(this->m_hWnd);
    PrepareDC(dc);
    dc.LPtoDP(lpPoints, nCount);
  }
  
  void ClientToDevice(CPoint &point)
  {
    point += m_ptOffset;
  }
  
  void ClientToDevice(CRect& rect)
  {
    rect.OffsetRect( CPoint(m_ptOffset));
  }
  
  void ClientToLogical(CPoint &point)
  {
    DPtoLP(&point);
  }
  
  void ClientToLogical(CRect& rect)
  {
    DPtoLP((POINT*) &rect, 2);
  }
  
  void DeviceToClient(CPoint& point)
  {
    point -= m_ptOffset;
  }
  
  void DeviceToClient(CRect& rect)
  {
    rect.OffsetRect( - CPoint(m_ptOffset));
  }
  
  void LogicalToClient(CPoint& point)
  {
    LPtoDP(&point);
  }
  
  void LogicalToClient(CRect& rect)
  {
    LPtoDP( (POINT*) & rect, 2);
  }
  
  
protected:
  
  virtual void PrepareDC(HDC hDC)
  {
    SetMapMode(hDC, MM_ANISOTROPIC);
    SetWindowExtEx(hDC, m_OrigSizeAll.cx, m_OrigSizeAll.cy, NULL);  //Set up the logical window
    
    SetViewportExtEx(hDC, m_sizeAll.cx, m_sizeAll.cy, NULL); // in device coordinates
    SetViewportOrgEx(hDC, -m_ptOffset.x, -m_ptOffset.y, NULL);
  }
  
  
  
  virtual void UpdateViewPort(POINT* ptNewCenter)
  {
    if ( m_dZoomScale < 1.0 / 32.0 )
    {
      m_dZoomScale = 1.0 / 32.0; 
      MessageBeep(0);
    }
    else if ( m_dZoomScale > 32 )
    {
      MessageBeep(0);
      m_dZoomScale = 32;
    }
    CPoint ptCenter;
    if ( !ptNewCenter )
    {
      ptCenter = GetLogicalCenter();
    }
    else
    {
      ptCenter = *ptNewCenter;
    }
    
    m_sizeAll.cx = (long) (m_OrigSizeAll.cx * m_dZoomScale);
    m_sizeAll.cy = (long) (m_OrigSizeAll.cy * m_dZoomScale);
    RecalcBars();
    CenterOnLogicalPoint(ptCenter);
    ZoomLevelChanged();
    Invalidate();
    
  }
  
  CPoint GetLogicalCenter()
  {
    CPoint pt;
    pt.x = m_sizeClient.cx < m_sizeAll.cx ? m_sizeClient.cx / 2 : m_sizeAll.cx / 2;
    pt.y = m_sizeClient.cy < m_sizeAll.cy ? m_sizeClient.cy / 2 : m_sizeAll.cy / 2;
    // Convert the point to logical coordinates
    DPtoLP(&pt);
    return pt;
    
  }
  
  void CenterOnLogicalPoint(CPoint ptLogCenter)
  {
    LPtoDP(&ptLogCenter);
    ClientToDevice(ptLogCenter);
    CenterOnPoint(ptLogCenter);
  }
  
  
  void CenterOnPoint(CPoint ptCenter)   // in device coordinates
  {
    
    int xDesired = ptCenter.x - m_sizeClient.cx  / 2;
    int yDesired = ptCenter.y - m_sizeClient.cy  / 2;
    
    DWORD dwStyle = GetStyle();
    
    if ((dwStyle & WS_HSCROLL) == 0 || xDesired < 0)
    {
      xDesired = 0;
    }
    else
    {
      if (xDesired >= m_sizeAll.cx)
        xDesired = m_sizeAll.cx - 1;
    }
    
    if ((dwStyle & WS_VSCROLL) == 0 || yDesired < 0)
    {
      yDesired = 0;
    }
    else
    {
      if ( yDesired >= m_sizeAll.cy  )
        yDesired = m_sizeAll.cy - 1;
    }
    
    ATLASSERT(xDesired >= 0);
    ATLASSERT(yDesired >= 0);
    
    SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_POS;
    
    si.nPos = xDesired;
    SetScrollInfo(SB_HORZ, &si, true);
    
    si.nPos = yDesired;
    SetScrollInfo(SB_VERT, &si, true);
    
    GetScrollInfo(SB_HORZ, &si);
    m_ptOffset.x = si.nPos;
    
    GetScrollInfo(SB_VERT, &si);
    m_ptOffset.y = si.nPos;
    
    
  }


  virtual void RecalcBars(BOOL bRedraw = TRUE, BOOL bResetPos = FALSE)
  { 
    
    
    T* pT = static_cast<T*>(this);
    ATLASSERT(::IsWindow(pT->m_hWnd));

    m_sizeLine.cx = CalcLineOrPage(0, m_sizeAll.cx, 100);
    m_sizeLine.cy = CalcLineOrPage(0, m_sizeAll.cy, 100);
    m_sizePage.cx = CalcLineOrPage(0, m_sizeAll.cx, 10);
    m_sizePage.cy = CalcLineOrPage(0, m_sizeAll.cy, 10);
    
    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);

    if ( CSize(m_sizeAll) == CSize(0,0) )
    {
      si.fMask = SIF_RANGE;
      si.nMin = si.nMax = 0;
      pT->SetScrollInfo( SB_VERT, &si, bRedraw);
      pT->SetScrollInfo( SB_HORZ, &si, bRedraw);
    }
    else
    {
      bool bUpdate = false;
      // first the vertical scroll
      si.fMask = SIF_ALL;

      pT->GetScrollInfo (SB_VERT, &si);
      si.nMin = 0;
      si.nMax = m_sizeAll.cy - 1;
      si.nPage = m_sizeClient.cy;
      if ( bResetPos )
        si.nPos = 0;
      int oldpos = si.nPos;
      pT->SetScrollInfo (SB_VERT, &si, bRedraw);
      pT->GetScrollInfo (SB_VERT, &si);
      if ( si.nPos != oldpos )
        bUpdate = true;
      m_ptOffset.y = si.nPos;
      
      // then the horizontal scroll
      pT->GetScrollInfo (SB_HORZ, &si);
      si.nMin = 0;
      si.nMax = m_sizeAll.cx - 1;
      si.nPage = m_sizeClient.cx;
      if ( bResetPos )
        si.nPos = 0;
      oldpos = si.nPos,
      pT->SetScrollInfo (SB_HORZ, &si, bRedraw);
      pT->GetScrollInfo (SB_HORZ, &si);
      if ( si.nPos != oldpos )
        bUpdate = true;
      m_ptOffset.x = si.nPos;

      if ( bUpdate )
      { 
        Invalidate();
        UpdateWindow();
      }
    }
  }
  
  
  double m_dZoomScale;
  SIZE   m_OrigSizeAll;
  SIZE   m_OrigSizeLine;
  SIZE   m_OrigSizePage;
  
};
