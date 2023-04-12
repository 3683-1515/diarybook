#include "plnmgr2.h"

HINSTANCE hInst;
SYSTEMTIME d, yesterday;
INT scr = 0;
HANDLE bmNotebook, bmPlanner, bmTimer;
char* filename = "diary.frb";
char* calfn = "planner.frb";
char* timfn = "timer.frt";

getTime(hdlg, d)
	HWND hdlg;
	SYSTEMTIME *d;
{
	ZeroMemory(d, sizeof(SYSTEMTIME));
	SendMessage(GetDlgItem(hdlg, IDC_MONTHCALENDAR), MCM_GETCURSEL, 0, (LPARAM)d);
}

reloadContents(hdlg)
	HWND hdlg;
{
	TCHAR buf[20];
	TCHAR pagebuf[100];
	
	if(scr == DIARY) {
		TCHAR diary[32009] = L"";
		INT weather = 0;

		getTime(hdlg, &d);
		wsprintf(buf, L"%i�� %i�� %i��", d.wYear, d.wMonth, d.wDay);
		invalidate(hdlg, IDC_TODAY);
		SetWindowText(GetDlgItem(hdlg, IDC_TODAY), buf);
		openDiary(d.wYear, d.wMonth, d.wDay, &weather, diary);
		SetDlgItemText(hdlg, IDC_DIARYTODAY, diary);
		SendMessage(GetDlgItem(hdlg, IDC_WEATHERTODAY), CB_SETCURSEL, (WPARAM)weather, (LPARAM)0);
		
		addDays(&d, -1, &yesterday);
		wsprintf(buf, L"%i�� %i�� %i��", yesterday.wYear, yesterday.wMonth, yesterday.wDay);
		invalidate(hdlg, IDC_YESTERDAY);
		SetWindowText(GetDlgItem(hdlg, IDC_YESTERDAY), buf);
		wcscpy(diary, L"");
		openDiary(yesterday.wYear, yesterday.wMonth, yesterday.wDay, &weather, diary);
		SetDlgItemText(hdlg, IDC_DIARYYESTERDAY, diary);
		SendMessage(GetDlgItem(hdlg, IDC_WEATHERYESTERDAY), CB_SETCURSEL, (WPARAM)weather, (LPARAM)0);
	} else if(scr == PLANNER) {
		getTime(hdlg, &d);
		wsprintf(buf, L"%i�� %i�� %i��", d.wYear, d.wMonth, d.wDay);
		invalidate(hdlg, IDC_YESTERDAY);
		SetWindowText(GetDlgItem(hdlg, IDC_YESTERDAY), buf);
		invalidate(hdlg, IDC_TODAY);
		SetWindowText(GetDlgItem(hdlg, IDC_TODAY), L"���� �����");
		SendMessage(GetDlgItem(hdlg, IDC_PLANLIST), LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
		SendMessage(GetDlgItem(hdlg, IDC_PLANLIST), LB_ADDSTRING, (WPARAM)0, (LPARAM)L"<�� ���� �����...>");
		getPlanList(hdlg, d.wYear, d.wMonth, d.wDay);
		SendMessage(GetDlgItem(hdlg, IDC_PLANLIST), LB_SETCURSEL, (WPARAM)0, (LPARAM)0);
		clearPlanData(hdlg, TRUE);
	} else if(scr == TIMER) {
		SetWindowText(GetDlgItem(hdlg, IDC_YESTERDAY), L"����� Ÿ�̸�");
		SetWindowText(GetDlgItem(hdlg, IDC_TODAY), L"Ÿ�̸� ����");
	}
	
	pageIndicator(pagebuf, d.wDay, dayCount(d.wMonth, d.wYear));
	invalidate(hdlg, IDC_PAGEINDICATOR);
	SetWindowText(GetDlgItem(hdlg, IDC_PAGEINDICATOR), pagebuf);
	
	swprintf(buf, L"%i��", d.wYear);
	invalidate(hdlg, IDC_YEAR);
	SetWindowText(GetDlgItem(hdlg, IDC_YEAR), buf);
	swprintf(buf, L"%i��", d.wMonth);
	invalidate(hdlg, IDC_MONTH);
	SetWindowText(GetDlgItem(hdlg, IDC_MONTH), buf);
}

setScreen(hdlg, mode)
	HWND hdlg;
	INT mode;
{
	INT i;
	if(mode == scr) return FALSE;
	if(mode == DIARY) {
		SendMessage(GetDlgItem(hdlg, IDC_NOTEBOOKBITMAP), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bmNotebook);
		for(i=DIARY_START; i<=DIARY_END; i++)
			ShowWindow(GetDlgItem(hdlg, i), SW_SHOW);
		for(i=PLANNER_START; i<=PLANNER_END; i++)
			ShowWindow(GetDlgItem(hdlg, i), SW_HIDE);
		for(i=TIMER_START; i<=TIMER_END; i++)
			ShowWindow(GetDlgItem(hdlg, i), SW_HIDE);
	} else if(mode == PLANNER) {
		SendMessage(GetDlgItem(hdlg, IDC_NOTEBOOKBITMAP), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bmPlanner);
		for(i=PLANNER_START; i<=PLANNER_END; i++)
			ShowWindow(GetDlgItem(hdlg, i), SW_SHOW);
		for(i=DIARY_START; i<=DIARY_END; i++)
			ShowWindow(GetDlgItem(hdlg, i), SW_HIDE);
		for(i=TIMER_START; i<=TIMER_END; i++)
			ShowWindow(GetDlgItem(hdlg, i), SW_HIDE);
		clearPlanData(hdlg, TRUE);
	} else if(mode == TIMER) {
		SendMessage(GetDlgItem(hdlg, IDC_NOTEBOOKBITMAP), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bmTimer);
		for(i=TIMER_START; i<=TIMER_END; i++)
			ShowWindow(GetDlgItem(hdlg, i), SW_SHOW);
		for(i=DIARY_START; i<=DIARY_END; i++)
			ShowWindow(GetDlgItem(hdlg, i), SW_HIDE);
		for(i=PLANNER_START; i<=PLANNER_END; i++)
			ShowWindow(GetDlgItem(hdlg, i), SW_HIDE);
	}
	
	for(i=IDC_PAGEINDICATOR; i<=IDC_GOTOTIMER; i++) {
		ShowWindow(GetDlgItem(hdlg, i), SW_HIDE);
		ShowWindow(GetDlgItem(hdlg, i), SW_SHOW);
	}
	EnableWindow(GetDlgItem(hdlg, IDC_GOTOYESTERDAY), mode != TIMER);
	EnableWindow(GetDlgItem(hdlg, IDC_GOTOTOMORROW), mode != TIMER);
	UpdateWindow(hdlg);
	
	scr = mode;
	reloadContents(hdlg);
	
	return TRUE;
}

INT_PTR APIENTRY MainDlgProc(hdlg, uMsg, wParam, lParam)
	HWND hdlg;
	UINT uMsg;
	WPARAM wParam;
	LPARAM lParam;
{
	INT i;
	static TCHAR weather[][10] = {
		TEXT("����"),
		TEXT("��"),
		TEXT("�帲"),
		TEXT("��"),
	};
	static HFONT fNote;
	static HFONT fBold;
	static HFONT fNormal;
	static LPNMUPDOWN lpnmud;
	static TCHAR dbuf[32009];
	TCHAR ptbuf[105];
	TCHAR pibuf[9];
	INT ni;
	INT pli;
	static TCHAR timebuf[15];
	static char tname[105], tcontent[1044];
	static TCHAR pagebuf[100];
	
    switch(uMsg) {
		case WM_INITDIALOG: {
			HICON hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_ICON_MAIN), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
			if(hIcon) SendMessage(hdlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
			bmNotebook = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_NOTEBOOK_BMP), IMAGE_BITMAP, 0, 0, 0);
			bmPlanner = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_PLANNER_BMP), IMAGE_BITMAP, 0, 0, 0);
			bmTimer = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_TIMER_BMP), IMAGE_BITMAP, 0, 0, 0);
			
			for(i=0; i<4; i++) {
				SendMessage(GetDlgItem(hdlg, IDC_WEATHERYESTERDAY), CB_ADDSTRING, (WPARAM)0, (LPARAM)(weather[i]));
				SendMessage(GetDlgItem(hdlg, IDC_WEATHERTODAY), CB_ADDSTRING, (WPARAM)0, (LPARAM)(weather[i]));
			}
			SendMessage(GetDlgItem(hdlg, IDC_WEATHERYESTERDAY), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
			SendMessage(GetDlgItem(hdlg, IDC_WEATHERTODAY), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
			
			reloadContents(hdlg);
			
			fNote = CreateFont(16, 0, 0, 0, FW_DONTCARE, 0, 0, 0, HANGEUL_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Dotum"));
			fBold = CreateFont(12, 0, 0, 0, 700, 0, 0, 0, HANGEUL_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Gulim"));
			fNormal = CreateFont(12, 0, 0, 0, FW_DONTCARE, 0, 0, 0, HANGEUL_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Gulim"));
			SendMessage(GetDlgItem(hdlg, IDC_DIARYYESTERDAY), WM_SETFONT, (WPARAM)fNote, (LPARAM)TRUE);
			SendMessage(GetDlgItem(hdlg, IDC_DIARYTODAY), WM_SETFONT, (WPARAM)fNote, (LPARAM)TRUE);
			SendMessage(GetDlgItem(hdlg, IDC_YESTERDAY), WM_SETFONT, (WPARAM)fBold, (LPARAM)TRUE);
			SendMessage(GetDlgItem(hdlg, IDC_TODAY), WM_SETFONT, (WPARAM)fBold, (LPARAM)TRUE);
			ShowScrollBar(GetDlgItem(hdlg, IDC_DIARYYESTERDAY), SB_VERT, TRUE);
			ShowScrollBar(GetDlgItem(hdlg, IDC_DIARYTODAY), SB_VERT, TRUE);
			ShowScrollBar(GetDlgItem(hdlg, IDC_PLANNOTES), SB_VERT, TRUE);
			SendMessage(GetDlgItem(hdlg, IDC_YESTERDAY), EM_SETLIMITTEXT, (WPARAM)32000, (LPARAM)0);
			SendMessage(GetDlgItem(hdlg, IDC_TODAY), EM_SETLIMITTEXT, (WPARAM)32000, (LPARAM)0);
			SendMessage(GetDlgItem(hdlg, IDC_PLANIMPORTANCE), EM_SETLIMITTEXT, (WPARAM)2, (LPARAM)0);
			SendMessage(GetDlgItem(hdlg, IDC_PLANIMPORTANCEUPDOWN), UDM_SETRANGE, (WPARAM)0, MAKELPARAM(10, 1));
			SendMessage(GetDlgItem(hdlg, IDC_PLANTITLE), EM_SETLIMITTEXT, (WPARAM)100, (LPARAM)0);
			SendMessage(GetDlgItem(hdlg, IDC_PLANLOCATION), EM_SETLIMITTEXT, (WPARAM)100, (LPARAM)0);
			SendMessage(GetDlgItem(hdlg, IDC_PLANNOTES), EM_SETLIMITTEXT, (WPARAM)1024, (LPARAM)0);
			
			EnableWindow(GetDlgItem(hdlg,IDC_TIMERHOURSEDIT), FALSE);
			EnableWindow(GetDlgItem(hdlg,IDC_TIMERMINUTESEDIT), FALSE);
			EnableWindow(GetDlgItem(hdlg,IDC_TIMERSECONDSEDIT), FALSE);
			
			setScreen(hdlg, DIARY);
			
			for(i=PLANNER_START; i<=PLANNER_END; i++)
				ShowWindow(GetDlgItem(hdlg, i), SW_HIDE);
			for(i=TIMER_START; i<=TIMER_END; i++)
				ShowWindow(GetDlgItem(hdlg, i), SW_HIDE);
			
			reloadTimerList(hdlg);
		}
		return TRUE;

		case WM_CLOSE: {
			extern DIARYBOOK diary;
			writeDiary(filename, &diary);
			EndDialog(hdlg, 0);
		}
		return TRUE;
		
		case WM_TIMER: {
			switch((UINT)wParam) {
				case timer: {
					processTimer(hdlg);
				}
			}
		}
		return TRUE;

		case WM_COMMAND: {
			switch(GET_WM_COMMAND_ID(wParam, lParam)) {
				/* ��ø/�޷� ���� */
				case IDC_GOTOYESTERDAY: {
					addDays(&d, -1, &d);
					SendMessage(GetDlgItem(hdlg, IDC_MONTHCALENDAR), MCM_SETCURSEL, 0, (LPARAM)&d);
					reloadContents(hdlg);
				} break; case IDC_GOTOTOMORROW: {
					addDays(&d, 1, &d);
					SendMessage(GetDlgItem(hdlg, IDC_MONTHCALENDAR), MCM_SETCURSEL, 0, (LPARAM)&d);
					reloadContents(hdlg);
				} break;
				
				/* �ϱ� ���� */
				case IDC_DIARYTODAY:
				case IDC_DIARYYESTERDAY:
				case IDC_WEATHERTODAY:
				case IDC_WEATHERYESTERDAY: {
					if(GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE || GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE) {
						if(GET_WM_COMMAND_ID(wParam, lParam) == IDC_DIARYYESTERDAY || GET_WM_COMMAND_ID(wParam, lParam) == IDC_WEATHERYESTERDAY) {
							memset(dbuf, 0, sizeof(dbuf));
							GetDlgItemText(hdlg, IDC_DIARYYESTERDAY, dbuf, 32000);
							if(!saveDiary(hdlg, yesterday.wYear, yesterday.wMonth, yesterday.wDay, (INT)SendMessage(GetDlgItem(hdlg, IDC_WEATHERYESTERDAY), (UINT) CB_GETCURSEL, (WPARAM)0, (LPARAM)0), dbuf))
								MessageBox(hdlg, L"�ϱ����� ������ �� �����ϴ�.", L"�ϱ���", MB_OK | MB_ICONERROR);
						} else {
							memset(dbuf, 0, sizeof(dbuf));
							GetDlgItemText(hdlg, IDC_DIARYTODAY, dbuf, 32000);
							if(!saveDiary(hdlg, d.wYear, d.wMonth, d.wDay, (INT)SendMessage(GetDlgItem(hdlg, IDC_WEATHERTODAY), (UINT) CB_GETCURSEL, (WPARAM)0, (LPARAM)0), dbuf))
								MessageBox(hdlg, L"�ϱ����� ������ �� �����ϴ�.", L"�ϱ���", MB_OK | MB_ICONERROR);
						}
					}
				}
				
				/* ��� �̵� */
				break; case IDC_GOTODIARY:
					setScreen(hdlg, DIARY);
				break; case IDC_GOTOPLANNER:
					setScreen(hdlg, PLANNER);
				break; case IDC_GOTOTIMER:
					setScreen(hdlg, TIMER);
				
				/* ����ǥ */
				break; case IDC_PLANTITLE: {
					if(GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE) {
						invalidate(hdlg, IDC_TODAY);
						GetDlgItemText(hdlg, IDC_PLANTITLE, ptbuf, 100);
						if(!ptbuf[0]) {
							SetWindowText(GetDlgItem(hdlg, IDC_TODAY), L"���� �����");
							break;
						} else {
							SetWindowText(GetDlgItem(hdlg, IDC_TODAY), ptbuf);
						}
					}
				}
				break; case IDC_SAVEPLAN: {
					if(!savePlan(hdlg, d.wYear, d.wMonth, d.wDay))
						MessageBox(hdlg, L"������ ������ �� �����ϴ�.", L"����", MB_OK | MB_ICONERROR);
				}
				break; case IDC_PLANLIST: {
					if(GET_WM_COMMAND_CMD(wParam, lParam) == LBN_SELCHANGE) {
						pli = (INT)SendMessage(GetDlgItem(hdlg, IDC_PLANLIST), LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
						SendMessage(GetDlgItem(hdlg, IDC_PLANLIST), LB_GETTEXT, (WPARAM)pli, (LPARAM)ptbuf);
						openPlan(hdlg, d.wYear, d.wMonth, d.wDay, ptbuf);
					}
				}
				break; case IDC_DELETEPLAN: {
					pli = (INT)SendMessage(GetDlgItem(hdlg, IDC_PLANLIST), LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
					SendMessage(GetDlgItem(hdlg, IDC_PLANLIST), LB_GETTEXT, (WPARAM)pli, (LPARAM)ptbuf);
					if(!deletePlan(hdlg, d.wYear, d.wMonth, d.wDay, ptbuf))
						MessageBox(hdlg, L"������ ���� �� �����ϴ�.", L"����", MB_OK | MB_ICONERROR);
				}
				break; case IDC_PREVMONTH: {
					addDays(&d, -dayCount(FALLBACK(d.wMonth - 1, 12), d.wYear), &d);
					SendMessage(GetDlgItem(hdlg, IDC_MONTHCALENDAR), MCM_SETCURSEL, 0, (LPARAM)&d);
					reloadContents(hdlg);
				}
				break; case IDC_NEXTMONTH: {
					addDays(&d, dayCount((d.wMonth == 12 ? 1 : (d.wMonth + 1)), d.wYear), &d);
					SendMessage(GetDlgItem(hdlg, IDC_MONTHCALENDAR), MCM_SETCURSEL, 0, (LPARAM)&d);
					reloadContents(hdlg);
				}
				break; case IDC_PREVYEAR: {
					addDays(&d, -(isLeapYear(d.wYear) && (d.wMonth > 2 || d.wMonth == 2 && d.wDay == 29) ? 366 : 365), &d);
					SendMessage(GetDlgItem(hdlg, IDC_MONTHCALENDAR), MCM_SETCURSEL, 0, (LPARAM)&d);
					reloadContents(hdlg);
				}
				break; case IDC_NEXTYEAR: {
					addDays(&d, isLeapYear(d.wYear) && (d.wMonth > 2 || d.wMonth == 2 && d.wDay == 29) ? 366 : 365, &d);
					SendMessage(GetDlgItem(hdlg, IDC_MONTHCALENDAR), MCM_SETCURSEL, 0, (LPARAM)&d);
					reloadContents(hdlg);
				}
				break; case IDC_RESETPLANFIELD: {
					clearPlanData(hdlg, FALSE);
				}
				
				/* Ÿ�̸� */
				break; case IDC_CREATETIMER: {
					createTimer(hdlg);
					reloadTimerList(hdlg);
				} break; case IDC_TIMERLIST:
				          case IDC_STOPTIMER: {
					openTimer(hdlg);
				} break; case IDC_STARTTIMER: {
					startTimer(hdlg);
				}
			}
		}
		return TRUE;
		
		case WM_CTLCOLORSTATIC: {
			if (
				(HWND)lParam != GetDlgItem(hdlg, IDC_PLANLISTHEADER) && 
				(HWND)lParam != GetDlgItem(hdlg, IDC_PLANTITLE) && 
				(HWND)lParam != GetDlgItem(hdlg, IDC_TIMERHOURSEDIT) && 
				(HWND)lParam != GetDlgItem(hdlg, IDC_TIMERMINUTESEDIT) && 
				(HWND)lParam != GetDlgItem(hdlg, IDC_TIMERSECONDSEDIT)
			) {
				SetBkMode((HDC)wParam, TRANSPARENT);
				SetTextColor((HDC)wParam, RGB(0, 0, 0));
				return (INT_PTR)GetStockObject(HOLLOW_BRUSH);
			}
		}
		return TRUE;
		
		case WM_ERASEBKGND:
		return TRUE;

		case WM_NOTIFY: {
			INT n;
			
			switch (((LPNMHDR)lParam)->code) {
				case MCN_SELECT: {
					reloadContents(hdlg);
				}
				return TRUE;
				
				case UDN_DELTAPOS: {
					switch(GET_WM_COMMAND_ID(wParam,lParam)) {
						case IDC_PLANIMPORTANCEUPDOWN: {
							lpnmud = (LPNMUPDOWN)lParam;
							ni = getImportance(hdlg) + lpnmud->iDelta;
							if(ni < 1 || ni > 10) return FALSE;
							wsprintf(pibuf, L"%i", ni);
							SetDlgItemText(hdlg, IDC_PLANIMPORTANCE, pibuf);
							invalidate(hdlg, IDC_PLANIMPORTANCE);
						}
						return TRUE;
						
						case IDC_HOURUPDOWN: {
							n = GetDlgItemInt(hdlg, IDC_TIMERHOURSEDIT, 0, 0) - ((LPNMUPDOWN)lParam)->iDelta;
							SetDlgItemInt(hdlg, IDC_TIMERHOURSEDIT, n, 1);
							invalidate(hdlg, IDC_TIMERHOURSEDIT);
						}
						return TRUE;
						
						case IDC_MINUTEUPDOWN: {
							n = GetDlgItemInt(hdlg, IDC_TIMERMINUTESEDIT, 0, 0) - ((LPNMUPDOWN)lParam)->iDelta;
							SetDlgItemInt(hdlg, IDC_TIMERMINUTESEDIT, n, 1);
							invalidate(hdlg, IDC_TIMERMINUTESEDIT);
						}
						return TRUE;
						
						case IDC_SECONDUPDOWN: {
							n = GetDlgItemInt(hdlg, IDC_TIMERSECONDSEDIT, 0, 0) - ((LPNMUPDOWN)lParam)->iDelta;
							SetDlgItemInt(hdlg, IDC_TIMERSECONDSEDIT, n, 1);
							invalidate(hdlg, IDC_TIMERSECONDSEDIT);
						}
						return TRUE;
					}
				}
			}
		}
		return TRUE;
    }
    return FALSE;
}

MMain(hinst, hinstPrev, lpstrCmdLine, sw)
	INT err = 0;
	setlocale(0, "");
	
	if(err = initDiary())
		switch(err) {
			case ERR_NOT_FR_FILE:
				MessageBox(NULL, L"�ҷ������� �ϱ��� ������ �� Ǯ�׸����� ���Ǵ� ������ �ƴմϴ�.", L"�ϱ��� ����", MB_OK | MB_ICONERROR);
				return err;
			case ERR_FEATURE_MISMATCH:
				MessageBox(NULL, L"�ҷ������� ������ �ùٸ���, �ϱ����� �ƴ� �ٸ� ���(����, Ÿ�̸� ��)�� ���� ���Դϴ�.", L"�ϱ��� ����", MB_OK | MB_ICONERROR);
				return err;
			case ERR_MALFORMED_DATA_HEAD:
				MessageBox(NULL, L"�ϱ��� ������ �ջ�Ǿ����ϴ�.", L"�ϱ��� ����", MB_OK | MB_ICONERROR);
				return err;
			case ERR_INVALID_DATE:
				MessageBox(NULL, L"����� �ϱ� �� �ϳ��� ��¥�� �߸��Ǿ����ϴ�.", L"�ϱ��� ����", MB_OK | MB_ICONERROR);
				return err;
			case ERR_CONTENT_LENGTH_MISMATCH:
				MessageBox(NULL, L"�ϱ��� ������ �߸� ����Ǿ� �ֽ��ϴ�.", L"�ϱ��� ����", MB_OK | MB_ICONERROR);
				return err;
		}
	if(err = initPlanner()) {
		switch(err) {
			case ERR_NOT_FR_FILE:
				MessageBox(NULL, L"�ҷ������� ����ǥ�� �� Ǯ�׸����� ���Ǵ� ������ �ƴմϴ�.", L"����ǥ ����", MB_OK | MB_ICONERROR);
				return err;
			case ERR_FEATURE_MISMATCH:
				MessageBox(NULL, L"�ҷ������� ������ �ùٸ���, ����ǥ�� �ƴ� �ٸ� ���(�ϱ�, Ÿ�̸� ��)�� ���� ���Դϴ�.", L"����ǥ ����", MB_OK | MB_ICONERROR);
				return err;
			case ERR_MALFORMED_DATA_HEAD:
				MessageBox(NULL, L"����ǥ�� �ջ�Ǿ����ϴ�.", L"����ǥ ����", MB_OK | MB_ICONERROR);
				return err;
			case ERR_INVALID_DATE:
				MessageBox(NULL, L"�ϳ� �̻��� ������ ��¥�� �߸��Ǿ����ϴ�.", L"����ǥ ����", MB_OK | MB_ICONERROR);
				return err;
			case ERR_CONTENT_LENGTH_MISMATCH:
				MessageBox(NULL, L"���� ������ �߸� ����Ǿ� �ֽ��ϴ�.", L"����ǥ ����", MB_OK | MB_ICONERROR);
				return err;
			case ERR_UNEXPENDED_EOF:
				MessageBox(NULL, L"����ǥ�� ���� �߿� �ջ�Ǿ����ϴ�.", L"����ǥ ����", MB_OK | MB_ICONERROR);
				return err;
			case ERR_INVALID_IMPORTANCE:
				MessageBox(NULL, L"�߿䵵�� �߸� ��ϵǾ����ϴ�.", L"����ǥ ����", MB_OK | MB_ICONERROR);
				return err;
		}
	}
	
	hInst = hinst;
    return (INT)DialogBox(hInst, MAKEINTRESOURCE(iddMain), NULL, (DLGPROC)MainDlgProc);
}
