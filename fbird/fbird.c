#include <windows.h>
HINSTANCE hInst;

INT isLeapYear(year)
	WORD year;
{
	return year % 400 == 0 || (year % 4 == 0 && year % 100);
}

WORD dayCount(month, year)
	WORD month;
	WORD year;
{
	WORD day = 30;
	
	if(month == 2) {
		if(isLeapYear(year))
			day = 29;
		else
			day = 28;
	} else if(month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12)
		day = 31;
	else
		day = 30;
	
	return day;
}

copyDate(d, dest)
	SYSTEMTIME *d;
	SYSTEMTIME *dest;
{
	ZeroMemory(dest, sizeof(SYSTEMTIME));
	dest->wYear = d->wYear;
	dest->wMonth = d->wMonth;
	dest->wDayOfWeek = d->wDayOfWeek;
	dest->wDay = d->wDay;
	dest->wHour = d->wHour;
	dest->wMinute = d->wMinute;
	dest->wSecond = d->wSecond;
	dest->wMilliseconds = d->wMilliseconds;
}

addDays(d, days, dest)
	SYSTEMTIME *d;
	INT days;
	SYSTEMTIME *dest;
{
	INT i;
	WORD year, month, day;
	year = d->wYear;
	month = d->wMonth;
	day = d->wDay;
	copyDate(d, dest);
	
	if(days < 0) for(i=0; i<-days; i++) {
		day--;
		if(!day) {
			month--;
			if(!month) {
				year--;
				month = 12;
			}
			day = dayCount(month, year);
		}
	} else for(i=0; i<days; i++) {
		day++;
		if(day > dayCount(month, year)) {
			month++;
			if(month > 12) {
				year++;
				month = 1;
			}
			day = 1;
		}
	}
	dest->wYear = year;
	dest->wMonth = month;
	dest->wDay = day;
}

pageIndicator(buf, page, last)
	TCHAR *buf;
	INT page;
	INT last;
{
	INT i, pt;
	for(i=1, pt=0; i<=last; i++, pt+=2) {
		if(page == i) buf[pt] = 8226;
		else buf[pt] = 9702;
		buf[pt+1] = ' ';
	}
	buf[pt-1] = 0;
}

writePageIndicator(hdlg, ctrl, page, last)
	HWND hdlg;
	INT ctrl;
	INT page;
	INT last;
{
	INT i, pt;
	TCHAR pagebuf[40];
	for(i=1, pt=0; i<=last; i++, pt+=2) {
		if(page == i) pagebuf[pt] = 8226;
		else pagebuf[pt] = 9702;
		pagebuf[pt+1] = ' ';
	}
	pagebuf[pt-1] = 0;
	SetDlgItemText(hdlg, ctrl, (LPCWSTR)pagebuf);
}

invalidate(hdlg, ctl)
	HWND hdlg;
	INT ctl;
{
	RECT rect;
	HWND ctrl = GetDlgItem(hdlg, ctl);
	GetClientRect(ctrl, &rect);
	InvalidateRect(ctrl, &rect, FALSE);
	MapWindowPoints(ctrl, hdlg, (POINT*)&rect, 2);
	RedrawWindow(hdlg, &rect, 0, RDW_ERASE | RDW_INVALIDATE);
}

checkTabs(str)
	TCHAR* str;
{
	TCHAR *c = str;
	while(*c) {
		if(*c == '\t') return FALSE;
		c++;
	}
	return TRUE;
}

APIENTRY LibMain(hInstance, ulReason, lpReserved)
	HANDLE hInstance;
	ULONG ulReason;
	LPVOID lpReserved;
{
    hInst = hInstance;
	return TRUE;
}
