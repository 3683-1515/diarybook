#include "plnmgr2.h"

extern char* timfn;
TIMERDATA storage[10000];
INT cnt = 0;
INT now = 0;
INT mycounttime = 0;
FILE *timerData;

createTimer(hdlg)
	HWND hdlg;
{
	INT i;
	static char tname[105], tcontent[1044];
	TCHAR sender[2][10000];
	INT sender_time_int[3];

	GetDlgItemText(hdlg, IDC_TIMERNOTES, sender[0], 1024);
	if(!checkTabs(sender[0])) {
		MessageBox(hdlg, L"탭 문자가 포함될 수 없습니다.", L"오류", MB_OK | MB_ICONERROR);
		return TRUE;
	}
	GetDlgItemText(hdlg, IDC_TIMERNAME, sender[1], 100);
	if(!checkTabs(sender[1])) {
		MessageBox(hdlg, L"탭 문자가 포함될 수 없습니다.", L"오류", MB_OK | MB_ICONERROR);
		return TRUE;
	}
	sender_time_int[0] = GetDlgItemInt(hdlg, IDC_TIMERHOURSEDIT, 0, 0);
	sender_time_int[1] = GetDlgItemInt(hdlg, IDC_TIMERMINUTESEDIT, 0, 0);
	sender_time_int[2] = GetDlgItemInt(hdlg, IDC_TIMERSECONDSEDIT, 0, 0);
	for(i=0; i<2; i++) {
		if(!sender[i][0]) {
			MessageBox(hdlg, L"모든 칸을 채우지 않았거나 값이 잘못되었습니다.", L"오류", MB_OK | MB_ICONERROR);
			return TRUE;
		}
	}
	for(i=0; i<3; i++) {
		if(!i) {
			if(sender_time_int[i] > 24 || sender_time_int[i] < 0) {
				MessageBox(hdlg, L"시간이 잘못되었습니다.", L"오류", MB_OK | MB_ICONERROR);
				return TRUE;
			}
		} else {
			if(sender_time_int[i] > 60 || sender_time_int[i] < 0) {
				MessageBox(hdlg, L"시간이 잘못되었습니다.", L"오류", MB_OK | MB_ICONERROR);
				return TRUE;
			}
		}
	}
	wcstombs(tname, sender[1], 100);
	wcstombs(tcontent, sender[0], 1024);
	fprintf(timerData, "%i %s\t%s\t%i %i %i\t", ++cnt, tname, tcontent, sender_time_int[0], sender_time_int[1], sender_time_int[2]);
	fclose(timerData);
	return TRUE;
}

initTimer(hdlg)
	HWND hdlg;
{
	static TCHAR timebuf[15];
	swprintf(timebuf, L"%i:%02i:%02i", 
		GetDlgItemInt(hdlg, IDC_TIMERHOURSEDIT, 0, TRUE),
		GetDlgItemInt(hdlg, IDC_TIMERMINUTESEDIT, 0, TRUE),
		GetDlgItemInt(hdlg, IDC_TIMERSECONDSEDIT, 0, TRUE));
	SetDlgItemText(hdlg, IDC_TIMERINDICATOR, timebuf);
	invalidate(hdlg, IDC_TIMERINDICATOR);
	invalidate(hdlg, IDC_TIMERHOURSEDIT);
	invalidate(hdlg, IDC_TIMERMINUTESEDIT);
	invalidate(hdlg, IDC_TIMERSECONDSEDIT);
}

startTimer(hdlg)
	HWND hdlg;
{
	INT hours, minutes, seconds;
	static TCHAR timebuf[15];
	initTimer(hdlg);
	memset(timebuf, 0, sizeof(timebuf));
	GetDlgItemText(hdlg, IDC_TIMERINDICATOR, timebuf, 12);
	swscanf(timebuf, L"%d:%d:%d", &hours, &minutes, &seconds);
	now = 3600 * hours + 60 * minutes + seconds;
	SendMessage(GetDlgItem(hdlg, IDC_TIMERPROGRESS), PBM_SETRANGE32, 0, now);
	KillTimer(hdlg, timer);
	mycounttime = 0;
	if(!SetTimer(hdlg, timer, 1000, NULL)) {
		TCHAR buf[100];
		swprintf(buf, L"타이머 설정 오류 (코드: %X)", GetLastError());
		MessageBox(hdlg, buf, L"오류", MB_OK | MB_ICONERROR);
	}
}

reloadTimerList(hdlg)
	HWND hdlg;
{
	static char tname[105], tcontent[1044];
	cnt = 0;
	SendMessage(GetDlgItem(hdlg, IDC_TIMERLIST), LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
openTimerList:
	timerData = fopen(timfn, "a+");
	if(!timerData) {
		timerData = fopen(timfn, "wb");
		fprintf(timerData, "");
		fclose(timerData);
		goto openTimerList;
	}
	rewind(timerData);
	while(~fscanf(timerData, "%*i %[^\t]\t%[^\t]\t%i %i %i", tname, tcontent, &(storage[cnt].hour), &(storage[cnt].minute), &(storage[cnt].second))) {
		mbstowcs(storage[cnt].name, tname, 9999);
		mbstowcs(storage[cnt].content, tcontent, 9999);
		SendMessage(GetDlgItem(hdlg, IDC_TIMERLIST), LB_ADDSTRING, (WPARAM)0, (LPARAM)(storage[cnt].name));
		cnt++;
	}
	cnt--;
}

openTimer(hdlg)
	HWND hdlg;
{
	INT idx;
	static TCHAR timebuf[15];
	idx = (INT)SendMessage(GetDlgItem(hdlg, IDC_TIMERLIST), LB_GETCURSEL, 0, 0);
	if(idx != -1) {
		KillTimer(hdlg, timer);
		mycounttime = 0;
		SendMessage(GetDlgItem(hdlg, IDC_TIMERPROGRESS), PBM_SETPOS, 0, 0);
		SetDlgItemText(hdlg, IDC_TIMERINDICATOR, L"0:00:00");
		invalidate(hdlg, IDC_TIMERINDICATOR);
		
		SetDlgItemText(hdlg, IDC_TIMERNAME, storage[idx].name);
		SetDlgItemText(hdlg, IDC_TIMERNOTES, storage[idx].content);
		SetDlgItemInt(hdlg, IDC_TIMERHOURSEDIT, storage[idx].hour, 1);
		SetDlgItemInt(hdlg, IDC_TIMERMINUTESEDIT, storage[idx].minute, 1);
		SetDlgItemInt(hdlg, IDC_TIMERSECONDSEDIT, storage[idx].second, 1);
		
		initTimer(hdlg);
	}
}

processTimer(hdlg)
	HWND hdlg;
{
	static TCHAR timebuf[15];
	mycounttime++;
	swprintf(timebuf, L"%i:%02i:%02i", mycounttime / 3600, (mycounttime % 3600) / 60, mycounttime % 60);
	SetDlgItemText(hdlg, IDC_TIMERINDICATOR, timebuf);
	invalidate(hdlg, IDC_TIMERINDICATOR);
	SendMessage(GetDlgItem(hdlg, IDC_TIMERPROGRESS), PBM_SETPOS, mycounttime, 0);
	if(mycounttime >= now) {
		KillTimer(hdlg, timer);
		mycounttime = 0;
		Beep(750, 1000);
		MessageBox(hdlg, L"타이머가 종료되었습니다.", L"알림", MB_OK | MB_ICONEXCLAMATION);
	}
}

