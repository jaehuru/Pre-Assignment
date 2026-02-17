// MFCApplication1Dlg.h: 헤더 파일
//

#pragma once
#include <atlimage.h> // CImage 사용을 위해 추가

// 사용자 정의 메시지
#define WM_USER_UPDATE_POINTS       (WM_USER + 1)
#define WM_USER_ANIMATION_COMPLETE  (WM_USER + 2)


// CMFCApplication1Dlg 대화 상자
class CMFCApplication1Dlg : public CDialogEx
{
// 생성입니다.
public:
	CMFCApplication1Dlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.
	~CMFCApplication1Dlg(); // 소멸자 추가

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCAPPLICATION1_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

    // 사용자 정의 멤버 변수
    CPoint m_points[3];                 // 클릭 지점 3개 저장
    int m_clickCount;                   // 유효한 클릭 수
    int m_smallCircleRadius;            // 클릭 지점 원의 반지름 (사용자 입력)
    int m_mainCircleThickness;          // 주 원의 두께 (사용자 입력)
    CPoint m_mainCircleCenter;          // 주 원의 중심
    double m_mainCircleRadius;          // 주 원의 반지름
    int m_draggingPointIndex;           // 드래그 중인 점의 인덱스 (-1이면 없음)
    bool m_isAnimating;                 // 랜덤 이동 애니메이션 중인지 여부

    // CImage를 사용한 더블 버퍼링
    CImage m_image;                     // 그리기 버퍼로 사용할 이미지
    CRect m_drawRect;                   // 그림 그리기 영역

    // 헬퍼 함수
    void DrawCircle(CDC* pDC, CPoint center, int radius, COLORREF color); // 윤곽선만 그리는 함수 (사용자 명명 규칙에 따라)
    void DrawClickCircle(CDC* pDC, CPoint center, int radius, COLORREF color); // 속이 채워진 원을 그리는 함수 (사용자 명명 규칙에 따라)
    void DrawThickCircle(CDC* pDC, CPoint center, int radius, int thickness, COLORREF color);
    bool CalculateCircumCircle(CPoint p1, CPoint p2, CPoint p3, CPoint& center, double& radius);
    bool IsPointInSmallCircle(CPoint testPoint, int pointIndex);
    void UpdatePointDisplay(int index); // UI에 점 좌표 업데이트
    void ClearPointDisplays();          // UI 점 좌표 초기화
    void UpdateControlStates();         // 버튼 활성화/비활성화 상태 업데이트

    // 워커 스레드 함수
    static UINT RandomMoveThread(LPVOID pParam);


	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
    afx_msg void OnDestroy(); // 소멸자에서 GDI 객체 정리
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnBnClickedInitializeButton();
    afx_msg void OnBnClickedRandomMoveButton();
    afx_msg LRESULT OnUpdatePoints(WPARAM wParam, LPARAM lParam); // 사용자 정의 메시지 핸들러
    afx_msg LRESULT OnAnimationComplete(WPARAM wParam, LPARAM lParam); // 사용자 정의 메시지 핸들러
	DECLARE_MESSAGE_MAP()
};
