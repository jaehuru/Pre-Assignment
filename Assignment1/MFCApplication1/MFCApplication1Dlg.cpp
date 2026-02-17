// MFCApplication1Dlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "MFCApplication1.h"
#include "MFCApplication1Dlg.h"
#include "afxdialogex.h"
#include <cmath>
#include <random> 
#include <atlimage.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCApplication1Dlg 대화 상자

CMFCApplication1Dlg::CMFCApplication1Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCAPPLICATION1_DIALOG, pParent),
    m_clickCount(0),
    m_smallCircleRadius(10), 
    m_mainCircleThickness(2), 
    m_draggingPointIndex(-1),
    m_isAnimating(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    // m_points 초기화
    for (int i = 0; i < 3; ++i)
    {
        m_points[i] = CPoint(0, 0);
    }
}

CMFCApplication1Dlg::~CMFCApplication1Dlg()
{

}

void CMFCApplication1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_RADIUS, m_smallCircleRadius);
    DDX_Text(pDX, IDC_EDIT_THICKNESS, m_mainCircleThickness);
    // 반경과 두께가 음수가 되지 않도록 유효성 검사 추가
    if (pDX->m_bSaveAndValidate)
    {
        if (m_smallCircleRadius < 1) m_smallCircleRadius = 1;
        if (m_mainCircleThickness < 1) m_mainCircleThickness = 1;
    }
}

BEGIN_MESSAGE_MAP(CMFCApplication1Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_BN_CLICKED(IDC_BUTTON_INITIALIZE, &CMFCApplication1Dlg::OnBnClickedInitializeButton)
    ON_BN_CLICKED(IDC_BUTTON_RANDOM_MOVE, &CMFCApplication1Dlg::OnBnClickedRandomMoveButton)
    ON_MESSAGE(WM_USER_UPDATE_POINTS, &CMFCApplication1Dlg::OnUpdatePoints)
    ON_MESSAGE(WM_USER_ANIMATION_COMPLETE, &CMFCApplication1Dlg::OnAnimationComplete)
    ON_WM_DESTROY()
END_MESSAGE_MAP()


// CMFCApplication1Dlg 메시지 처리기

void CMFCApplication1Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

BOOL CMFCApplication1Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
    
    // 컨트롤의 초기값 설정 (DDX를 통해 멤버 변수로 로드)
    UpdateData(FALSE); 

    // 그리기 영역 초기화 (대화 상자의 클라이언트 영역 전체)
    GetClientRect(&m_drawRect);

    // CImage 초기화 (그리기 영역 크기에 맞게)
    m_image.Create(m_drawRect.Width(), m_drawRect.Height(), 24); // 24비트 컬러 이미지 생성 (RGB)

    ClearPointDisplays(); // UI 컨트롤 다시 사용
    UpdateControlStates();

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CMFCApplication1Dlg::OnDestroy()
{
    CDialogEx::OnDestroy();

    // CImage 객체 정리 (m_image.Destroy() 호출은 필요에 따라 명시적으로 가능하나, 소멸자에서 자동으로 정리됨)
}


void CMFCApplication1Dlg::UpdatePointDisplay(int index)
{
    if (index >= 0 && index < 3)
    {
        CString str;
        str.Format(_T("Point %d: (%d, %d)"), index + 1, m_points[index].x, m_points[index].y);
        UINT ids[] = { IDC_STATIC_POINT1, IDC_STATIC_POINT2, IDC_STATIC_POINT3 };
        GetDlgItem(ids[index])->SetWindowText(str); // 컨트롤 다시 사용
    }
}

void CMFCApplication1Dlg::ClearPointDisplays()
{
    for (int i = 0; i < 3; ++i)
    {
        CString str;
        str.Format(_T("Point %d: (-, -)"), i + 1);
        UINT ids[] = { IDC_STATIC_POINT1, IDC_STATIC_POINT2, IDC_STATIC_POINT3 };
        GetDlgItem(ids[i])->SetWindowText(str); // 컨트롤 다시 사용
    }
}


void CMFCApplication1Dlg::UpdateControlStates()
{
	// 랜덤 이동 버튼은 세 점이 모두 클릭되고 애니메이션이 진행 중이지 않을 때만 활성화
    GetDlgItem(IDC_BUTTON_RANDOM_MOVE)->EnableWindow(m_clickCount == 3 && !m_isAnimating);
	// 초기화 버튼은 클릭된 점이 하나라도 있을 때 활성화
    GetDlgItem(IDC_BUTTON_INITIALIZE)->EnableWindow(TRUE);
    // UI 컨트롤 다시 사용
    GetDlgItem(IDC_EDIT_RADIUS)->EnableWindow(!m_isAnimating);
    GetDlgItem(IDC_EDIT_THICKNESS)->EnableWindow(!m_isAnimating);
}

// 정원 그리기 (윤곽선만)
void CMFCApplication1Dlg::DrawCircle(CDC* pDC, CPoint center, int radius, COLORREF color)
{
    if (radius <= 0 || pDC == nullptr) return;

    int x = radius;
    int y = 0;
    int err = 0; // Decision parameter

    while (x >= y)
    {
        // 8 옥탄트 그리기
        pDC->SetPixel(center.x + x, center.y + y, color);
        pDC->SetPixel(center.x + y, center.y + x, color);
        pDC->SetPixel(center.x - y, center.y + x, color);
        pDC->SetPixel(center.x - x, center.y + y, color);
        pDC->SetPixel(center.x - x, center.y - y, color);
        pDC->SetPixel(center.x - y, center.y - x, color);
        pDC->SetPixel(center.x + y, center.y - x, color);
        pDC->SetPixel(center.x + x, center.y - y, color);

        if (err <= 0)
        {
            y += 1;
            err += 2 * y + 1;
        }
        if (err > 0)
        {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}

// 클릭 원 그리기 (속이 채워진 원)
void CMFCApplication1Dlg::DrawClickCircle(CDC* pDC, CPoint center, int radius, COLORREF color)
{
    if (radius <= 0 || pDC == nullptr) return;

    int r_sq = radius * radius;
    for (int y = -radius; y <= radius; y++)
    {
        int x_limit = static_cast<int>(sqrt(r_sq - y * y));
        for (int x = -x_limit; x <= x_limit; x++)
        {
            pDC->SetPixel(center.x + x, center.y + y, color);
        }
    }
}


// 두꺼운 원 그리기
void CMFCApplication1Dlg::DrawThickCircle(CDC* pDC, CPoint center, int radius, int thickness, COLORREF color)
{
    if (pDC == nullptr || thickness <= 0) return;

    int startRadius = radius - (thickness / 2);
    for (int i = 0; i < thickness; ++i)
    {
        int currentRadius = startRadius + i;
        if (currentRadius > 0)
        {
            DrawCircle(pDC, center, currentRadius, color); // DrawCircle (윤곽선)을 사용하여 윤곽선만 그림
        }
    }
}


void CMFCApplication1Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
        // CImage의 DC를 얻어옵니다.
        CDC* pImageDC = CDC::FromHandle(m_image.GetDC());

        // CImage 버퍼를 배경색으로 채웁니다.
        pImageDC->FillSolidRect(0, 0, m_drawRect.Width(), m_drawRect.Height(), GetSysColor(COLOR_BTNFACE));

        // 클릭 지점 원 그리기
        for (int i = 0; i < m_clickCount; ++i)
        {
            // 클라이언트 영역 기준으로 좌표 조정
            CPoint drawPoint = m_points[i];
            drawPoint.Offset(-m_drawRect.left, -m_drawRect.top);
            DrawClickCircle(pImageDC, drawPoint, m_smallCircleRadius, RGB(0, 0, 0));
        }

        // 세 점이 모두 클릭되었으면 정원 그리기
        if (m_clickCount == 3)
        {
            CPoint center;
            double radius;
            if (CalculateCircumCircle(m_points[0], m_points[1], m_points[2], center, radius))
            {
                m_mainCircleCenter = center;
                m_mainCircleRadius = radius;

                // 클라이언트 영역 기준으로 좌표 조정
                CPoint drawCenter = m_mainCircleCenter;
                drawCenter.Offset(-m_drawRect.left, -m_drawRect.top);

                DrawThickCircle(pImageDC, drawCenter, static_cast<int>(m_mainCircleRadius), m_mainCircleThickness, RGB(0, 0, 0));
            }
        }
        
        // CImage의 DC를 해제합니다.
        m_image.ReleaseDC();

        // 최종 이미지를 화면 DC에 BitBlt합니다.
        CPaintDC dc(this);
        m_image.BitBlt(dc.GetSafeHdc(), m_drawRect.left, m_drawRect.top, m_drawRect.Width(), m_drawRect.Height(), 0, 0, SRCCOPY);
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서 이 함수를 호출합니다.
HCURSOR CMFCApplication1Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// 점이 작은 원 안에 있는지 확인
bool CMFCApplication1Dlg::IsPointInSmallCircle(CPoint testPoint, int pointIndex)
{
    if (pointIndex < 0 || pointIndex >= m_clickCount) return false;

    // 클라이언트 영역 기준 좌표로 변환
    testPoint.Offset(m_drawRect.left, m_drawRect.top);

    double dist_sq = (m_points[pointIndex].x - testPoint.x) * (m_points[pointIndex].x - testPoint.x) +
                     (m_points[pointIndex].y - testPoint.y) * (m_points[pointIndex].y - testPoint.y);
    double radius_sq = static_cast<double>(m_smallCircleRadius) * m_smallCircleRadius;

    return dist_sq <= radius_sq;
}

// 세 점을 지나는 원의 중심과 반지름 계산 (외접원)
// 이 함수는 선형 대수학을 사용하여 세 점을 지나는 원의 방정식을 풉니다.
bool CMFCApplication1Dlg::CalculateCircumCircle(CPoint p1, CPoint p2, CPoint p3, CPoint& center, double& radius)
{
    double D = 2.0 * (p1.x * (p2.y - p3.y) + p2.x * (p3.y - p1.y) + p3.x * (p1.y - p2.y));

    // 세 점이 일직선 위에 있는 경우 (D가 0에 매우 가까운 경우)
    if (abs(D) < 1e-6) // 엡실론 값을 사용하여 부동 소수점 비교
    {
        // 원을 그릴 수 없음
        radius = 0;
        return false;
    }

    double P1_sq = static_cast<double>(p1.x) * p1.x + static_cast<double>(p1.y) * p1.y;
    double P2_sq = static_cast<double>(p2.x) * p2.x + static_cast<double>(p2.y) * p2.y;
    double P3_sq = static_cast<double>(p3.x) * p3.x + static_cast<double>(p3.y) * p3.y;

    double Ux = (P1_sq * (p2.y - p3.y) + P2_sq * (p3.y - p1.y) + P3_sq * (p1.y - p2.y)) / D;
    double Uy = (P1_sq * (p3.x - p2.x) + P2_sq * (p1.x - p3.x) + P3_sq * (p2.x - p1.x)) / D;

    center = CPoint((int)round(Ux), (int)round(Uy));
    radius = sqrt(pow(static_cast<double>(p1.x) - Ux, 2) + pow(static_cast<double>(p1.y) - Uy, 2));

    return true;
}

void CMFCApplication1Dlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    // 그리기 영역 내에서만 클릭을 처리
    // point는 스크린 좌표가 아닌 클라이언트 영역 전체 좌표이므로, m_drawRect.PtInRect(point)가 적절함.
    // 하지만 m_points에 저장될 때는 m_drawRect.left, m_drawRect.top을 기준으로 offset 시켜야 함.
    if (!m_drawRect.PtInRect(point))
    {
        CDialogEx::OnLButtonDown(nFlags, point);
        return;
    }

    // m_points는 그리기 영역 (m_drawRect) 내의 상대 좌표를 저장.
    // 따라서 클릭된 point도 그리기 영역 기준으로 변환해야 함.
    CPoint relativePoint = point;
    relativePoint.Offset(-m_drawRect.left, -m_drawRect.top);

    if (m_clickCount < 3)
    {
        m_points[m_clickCount] = relativePoint;
        UpdatePointDisplay(m_clickCount); // UI 컨트롤 다시 사용
        m_clickCount++;

        if (m_clickCount == 3)
        {
            UpdateData(TRUE); // 입력 컨트롤 다시 사용
            UpdateControlStates(); // 버튼 활성화/비활성화
        }
        InvalidateRect(m_drawRect); // 그리기 영역만 다시 그리기
    }
    else // 세 점이 모두 클릭된 후
    {
        // 드래그 시작 시점인지 확인
        for (int i = 0; i < 3; ++i)
        {
            if (IsPointInSmallCircle(relativePoint, i)) // IsPointInSmallCircle 내부에서 offset 처리됨
            {
                m_draggingPointIndex = i;
                SetCapture(); // 마우스 캡처
                break;
            }
        }
    }

    CDialogEx::OnLButtonDown(nFlags, point);
}

void CMFCApplication1Dlg::OnMouseMove(UINT nFlags, CPoint point)
{
    if (m_draggingPointIndex != -1 && (nFlags & MK_LBUTTON))
    {
        // 그리기 영역 내에서만 드래그 처리
        if (m_drawRect.PtInRect(point))
        {
            // 그리기 영역 기준으로 좌표 조정
            CPoint relativePoint = point;
            relativePoint.Offset(-m_drawRect.left, -m_drawRect.top);

            m_points[m_draggingPointIndex] = relativePoint;
            UpdatePointDisplay(m_draggingPointIndex); // UI 컨트롤 다시 사용
            InvalidateRect(m_drawRect); // 그리기 영역만 다시 그리기
        }
    }
    CDialogEx::OnMouseMove(nFlags, point);
}

void CMFCApplication1Dlg::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (m_draggingPointIndex != -1)
    {
        ReleaseCapture(); // 마우스 캡처 해제
        m_draggingPointIndex = -1;
        // 드래그가 끝났으므로 최종적으로 한번 더 그리기 영역을 갱신
        InvalidateRect(m_drawRect);
    }

    CDialogEx::OnLButtonUp(nFlags, point);
}

void CMFCApplication1Dlg::OnBnClickedInitializeButton()
{
    m_clickCount = 0;
    m_draggingPointIndex = -1;
    m_isAnimating = false; // 혹시 애니메이션 중이었다면 중지

    for (int i = 0; i < 3; ++i)
    {
        m_points[i] = CPoint(0, 0);
    }
    m_mainCircleCenter = CPoint(0, 0);
    m_mainCircleRadius = 0;

    ClearPointDisplays(); // UI 컨트롤 다시 사용
    UpdateControlStates();
    InvalidateRect(m_drawRect); // 그리기 영역 초기화
}

void CMFCApplication1Dlg::OnBnClickedRandomMoveButton()
{
    if (m_clickCount != 3 || m_isAnimating)
    {
        return; // 세 점이 모두 찍히지 않았거나 이미 애니메이션 중이면 실행 안 함
    }

    m_isAnimating = true;
    UpdateControlStates(); // 버튼 비활성화

    // 워커 스레드 시작
    AfxBeginThread(RandomMoveThread, this);
}

UINT CMFCApplication1Dlg::RandomMoveThread(LPVOID pParam)
{
    CMFCApplication1Dlg* pDlg = static_cast<CMFCApplication1Dlg*>(pParam);
    if (pDlg == nullptr)
    {
        return 1; // 오류 코드
    }

    random_device rd;
    mt19937 gen(rd());

    for (int i = 0; i < 10; ++i) // 총 10번 반복
    {
        // 새로운 랜덤 위치 생성
        // 그리기 영역(m_drawRect) 내에서 랜덤 좌표 생성
        // m_drawRect는 스크린 좌표를 가지고 있으므로, Width()와 Height()는 그리기 영역의 크기.
        // 이 크기 내에서 랜덤 좌표를 생성해야 함.
        uniform_int_distribution<> distribX(0, pDlg->m_drawRect.Width() - 1);
        uniform_int_distribution<> distribY(0, pDlg->m_drawRect.Height() - 1);

        CPoint newPoints[3];
        for (int j = 0; j < 3; ++j)
        {
            newPoints[j].x = distribX(gen);
            newPoints[j].y = distribY(gen);
        }

        // 메인 UI 스레드에 점 업데이트 메시지 전송
        // WPARAM에 업데이트된 점들을 CPoint* 배열로 전달 (메모리 관리 주의)
        // 여기서는 임시 배열을 사용하고, 메시지 핸들러에서 복사하는 방식
        CPoint* pNewPoints = new CPoint[3];
        for (int j = 0; j < 3; ++j)
        {
            pNewPoints[j] = newPoints[j];
        }
        pDlg->PostMessage(WM_USER_UPDATE_POINTS, reinterpret_cast<WPARAM>(pNewPoints), 0);

        Sleep(500); // 0.5초 대기 (초당 2회)
    }

    // 애니메이션 완료 메시지 전송
    pDlg->PostMessage(WM_USER_ANIMATION_COMPLETE, 0, 0);

    return 0; // 성공
}

LRESULT CMFCApplication1Dlg::OnUpdatePoints(WPARAM wParam, LPARAM lParam)
{
    CPoint* pNewPoints = reinterpret_cast<CPoint*>(wParam);
    if (pNewPoints != nullptr)
    {
        for (int i = 0; i < 3; ++i)
        {
            m_points[i] = pNewPoints[i];
            UpdatePointDisplay(i);
        }
        delete[] pNewPoints; // 워커 스레드에서 할당한 메모리 해제

        // 새로운 점들로 다시 그리기
        InvalidateRect(m_drawRect);
    }
    return 0;
}

LRESULT CMFCApplication1Dlg::OnAnimationComplete(WPARAM wParam, LPARAM lParam)
{
    m_isAnimating = false;
    UpdateControlStates(); // 버튼 다시 활성화
    return 0;
}
