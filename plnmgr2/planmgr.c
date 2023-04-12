#include "plnmgr2.h"

extern char* calfn;
PLANBOOK calendar;

initPlanner()
{
	extern int lastFileError;
	return readPlanner(calfn, &calendar);
}

clearPlanData(hdlg, clearTitle)
	HWND hdlg;
	BOOL clearTitle;
{
	SetDlgItemInt(hdlg, IDC_PLANIMPORTANCE, 1, TRUE);
	if(clearTitle) SetDlgItemText(hdlg, IDC_PLANTITLE, L"");
	SetDlgItemText(hdlg, IDC_PLANLOCATION, L"");
	SetDlgItemText(hdlg, IDC_PLANNOTES, L"");
	SetDlgItemText(hdlg, IDC_PLANSTART, L"");
	SetDlgItemText(hdlg, IDC_PLANEND, L"");
}

getImportance(hdlg)
	HWND hdlg;
{
	TCHAR pibuf[9];
	INT importance;
	
	GetDlgItemText(hdlg, IDC_PLANIMPORTANCE, pibuf, 3);
	if(!NUMERIC(pibuf[0]) || (pibuf[1] && !NUMERIC(pibuf[1]))) {
		return FALSE;
	} else {
		importance = pibuf[1] ? (NUMBER(pibuf[0]) * 10 + NUMBER(pibuf[1])) : NUMBER(pibuf[0]);
		if(importance < 1 || importance > 10) {
			return FALSE;
		}
		return importance;
	}
}

getPlanList(hdlg, year, month, day)
	HWND hdlg;
	INT year;
	INT month;
	INT day;
{
	INT i;
	PLANS* plan;
	for(i=0; i<calendar.length; i++) {
		plan = &(calendar.plans[i]);
		if(plan->year == year && plan->month == month && plan->day == day) {
			SendMessage(GetDlgItem(hdlg, IDC_PLANLIST), LB_ADDSTRING, (WPARAM)0, (LPARAM)plan->title);
		}
	}
	return TRUE;
}

savePlan(hdlg, year, month, day)
	HWND hdlg;
	INT year;
	INT month;
	INT day;
{
	TCHAR ptitle[105];
	TCHAR ploc[105];
	TCHAR psbuf[9];
	TCHAR pebuf[9];
	TCHAR pibuf[9];
	TCHAR pnotes[1030];
	TCHAR ipt[105];
	char pts[105];
	char pns[1030];
	char pls[105];
	INT importance;
	INT sh, sm, eh, em;
	
	INT i;
	BOOL saved = FALSE;
	INT y, m, d, shf, smf, ehf, emf, imf;
	FILE *f;
	FILE *dest;
	char tf[105];
	char notef[1044];
	char locf[105];
	INT ret = 0;
	PLANS* plan;
	
	memset(ptitle, 0, sizeof(ptitle));
	GetDlgItemText(hdlg, IDC_PLANTITLE, ptitle, 100);
	if(!ptitle[0]) {
		MessageBox(hdlg, L"제목을 입력하십시오.", L"일정 만들기", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	
	memset(ploc, 0, sizeof(ploc));
	GetDlgItemText(hdlg, IDC_PLANLOCATION, ploc, 100);
	
	memset(psbuf, 0, sizeof(psbuf));
	GetDlgItemText(hdlg, IDC_PLANSTART, psbuf, 6);
	if(swscanf(psbuf, L"%d:%d", &sh, &sm) != 2) {
		MessageBox(hdlg, L"시작 시간이 잘못되었습니다. 1:23처럼 입력하십시오.", L"일정 만들기", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	memset(pebuf, 0, sizeof(pebuf));
	GetDlgItemText(hdlg, IDC_PLANEND, pebuf, 6);
	if(swscanf(pebuf, L"%d:%d", &eh, &em) != 2) {
		MessageBox(hdlg, L"종료 시간이 잘못되었습니다. 1:23처럼 입력하십시오.", L"일정 만들기", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	
	memset(pibuf, 0, sizeof(pibuf));
	importance = getImportance(hdlg);
	if(!importance) {
		MessageBox(hdlg, L"중요도가 잘못되었습니다. 중요도는 숫자로 1부터 10까지입니다.", L"일정 만들기", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	
	memset(pnotes, 0, sizeof(pnotes));
	GetDlgItemText(hdlg, IDC_PLANNOTES, pnotes, 1024);
	
	for(i=0; i<calendar.length; i++) {
		plan = &(calendar.plans[i]);
		if(plan->year == year && plan->month == month && plan->day == day && !wcscmp(plan->title, ptitle)) {
			plan->importance = importance;
			plan->startHour = sh;
			plan->startMinute = sm;
			plan->endHour = eh;
			plan->endMinute = em;
			
			free(plan->location);
			plan->location = (TCHAR*) malloc(wcslen(ploc) * sizeof(TCHAR) + 2);
			wcscpy(plan->location, ploc);
			
			free(plan->notes);
			plan->notes = (TCHAR*) malloc(wcslen(pnotes) * sizeof(TCHAR) + 2);
			wcscpy(plan->notes, pnotes);
			
			saved = TRUE;
			break;
		}
	}
	if(!saved) {
		calendar.plans = (PLANS*) realloc(calendar.plans, (++calendar.length) * sizeof(PLANS));
		plan = &(calendar.plans[calendar.length-1]);
		
		plan->year = (WORD)year;
		plan->month = (WORD)month;
		plan->day = (WORD)day;
		
		plan->importance = importance;
		plan->startHour = sh;
		plan->startMinute = sm;
		plan->endHour = eh;
		plan->endMinute = em;
		
		plan->title = (TCHAR*) malloc(wcslen(ptitle) * sizeof(TCHAR) + 2);
		wcscpy(plan->title, ptitle);
		
		plan->location = (TCHAR*) malloc(wcslen(ploc) * sizeof(TCHAR) + 2);
		wcscpy(plan->location, ploc);
		
		plan->notes = (TCHAR*) malloc(wcslen(pnotes) * sizeof(TCHAR) + 2);
		wcscpy(plan->notes, pnotes);
	}
	writePlanner(calfn, &calendar);
	if(!SendMessage(GetDlgItem(hdlg, IDC_PLANLIST), LB_GETCURSEL, (WPARAM)0, (LPARAM)0)) {
		clearPlanData(hdlg, TRUE);
		reloadContents(hdlg);
	}
	return TRUE;
}

openPlan(hdlg, year, month, day, title)
	HWND hdlg;
	INT year;
	INT month;
	INT day;
	TCHAR *title;
{
	INT i;
	TCHAR st[7], et[7];
	PLANS* plan;
	
	if(!wcscmp(L"<새 일정 만들기...>", title)) {
		EnableWindow(GetDlgItem(hdlg, IDC_PLANTITLE), TRUE);
		clearPlanData(hdlg, TRUE);
		return TRUE;
	}
	EnableWindow(GetDlgItem(hdlg, IDC_PLANTITLE), FALSE);
	
	for(i=0; i<calendar.length; i++) {
		plan = &(calendar.plans[i]);
		if(plan->year == year && plan->month == month && plan->day == day && !wcscmp(plan->title, title)) {
			SetDlgItemInt(hdlg, IDC_PLANIMPORTANCE, plan->importance, TRUE);
			wsprintf(st, L"%i:%02i", plan->startHour, plan->startMinute);
			wsprintf(et, L"%i:%02i", plan->endHour, plan->endMinute);
			SetDlgItemText(hdlg, IDC_PLANTITLE, plan->title);
			SetDlgItemText(hdlg, IDC_PLANLOCATION, plan->location);
			SetDlgItemText(hdlg, IDC_PLANNOTES, plan->notes);
			SetDlgItemText(hdlg, IDC_PLANSTART, st);
			SetDlgItemText(hdlg, IDC_PLANEND, et);
			return TRUE;
		}
	}
	return FALSE;
}

deletePlan(hdlg, year, month, day, title)
	HWND hdlg;
	INT year;
	INT month;
	INT day;
	TCHAR* title;
{
	INT i;
	PLANS* plan;
	
	for(i=0; i<calendar.length; i++) {
		plan = &(calendar.plans[i]);
		if(plan->year == year && plan->month == month && plan->day == day && !wcscmp(plan->title, title)) {
			wcscpy(plan->title, L"");
			break;
		}
	}
	writePlanner(calfn, &calendar);
	clearPlanData(hdlg, TRUE);
	EnableWindow(GetDlgItem(hdlg, IDC_PLANTITLE), TRUE);
	reloadContents(hdlg);
	return TRUE;
}

saveFilePlan(hdlg, year, month, day)
	HWND hdlg;
	INT year;
	INT month;
	INT day;
{
	TCHAR ptitle[105];
	TCHAR ploc[105];
	TCHAR psbuf[9];
	TCHAR pebuf[9];
	TCHAR pibuf[9];
	TCHAR pnotes[1030];
	TCHAR ipt[105];
	char pts[105];
	char pns[1030];
	char pls[105];
	INT importance;
	INT sh, sm, eh, em;
	
	INT i;
	INT y, m, d, shf, smf, ehf, emf, imf;
	FILE *f;
	FILE *dest;
	char tf[105];
	char notef[1044];
	char locf[105];
	INT ret = 0;
	
	memset(ptitle, 0, sizeof(ptitle));
	GetDlgItemText(hdlg, IDC_PLANTITLE, ptitle, 100);
	if(!ptitle[0] || !checkTabs(ptitle)) {
		MessageBox(hdlg, L"제목이 올바르지 않습니다.", L"일정 만들기", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	
	memset(ploc, 0, sizeof(ploc));
	GetDlgItemText(hdlg, IDC_PLANLOCATION, ploc, 100);
	if(!checkTabs(ploc)) {
		MessageBox(hdlg, L"장소에 탭 문자가 들어갈 수 없습니다.", L"일정 만들기", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	if(!ploc[0]) ploc[0] = '-', ploc[1] = 0;
	
	memset(psbuf, 0, sizeof(psbuf));
	GetDlgItemText(hdlg, IDC_PLANSTART, psbuf, 6);
	if(swscanf(psbuf, L"%d:%d", &sh, &sm) != 2) {
		MessageBox(hdlg, L"시작 시간이 잘못되었습니다. 1:23처럼 입력하십시오.", L"일정 만들기", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	memset(pebuf, 0, sizeof(pebuf));
	GetDlgItemText(hdlg, IDC_PLANEND, pebuf, 6);
	if(swscanf(pebuf, L"%d:%d", &eh, &em) != 2) {
		MessageBox(hdlg, L"종료 시간이 잘못되었습니다. 1:23처럼 입력하십시오.", L"일정 만들기", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	
	memset(pibuf, 0, sizeof(pibuf));
	importance = getImportance(hdlg);
	if(!importance) {
		MessageBox(hdlg, L"중요도가 잘못되었습니다. 중요도는 숫자로 1부터 10까지입니다.", L"일정 만들기", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	
	memset(pnotes, 0, sizeof(pnotes));
	GetDlgItemText(hdlg, IDC_PLANNOTES, pnotes, 1024);
	if(!checkTabs(pnotes)) {
		MessageBox(hdlg, L"메모에 탭 문자가 포함되면 안 됩니다.", L"일정 만들기", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	if(!pnotes[0]) pnotes[0] = '-', pnotes[1] = 0;
	
openfile:
	f = fopen(calfn, "rb+");
	dest = fopen("TMPSAVE2.FRP", "wb");
	if(!f) {
		f = fopen(calfn, "wb");
		fprintf(f, "");
		fclose(f);
		goto openfile;
	}
	wcstombs(pts, ptitle, 100);
	wcstombs(pns, pnotes, 1024);
	wcstombs(pls, ploc, 100);
	while(~fscanf(f, "%i %i %i %[^\t]\t%i %i %i %i %i%[^\t]\t%[^\t\0]", &y, &m, &d, tf, &shf, &smf, &ehf, &emf, &imf, locf, notef)) {
		mbstowcs(ipt, tf, 100);
		if(y == year && m == month && d == day && !wcscmp(ipt, ptitle)) {
			continue;
		} else {
			fprintf(dest, "%i %i %i %s\t%i %i %i %i %i %s\t%s\t", y, m, d, tf, shf, smf, ehf, emf, imf, locf, notef);
		}
	}
	
	fclose(f);
	fprintf(dest, "%i %i %i %s\t%i %i %i %i %i %s\t%s", year, month, day, pts, sh, sm, eh, em, importance, pls, pns);
	fclose(dest);
	
	ret += remove(calfn);
	ret += rename("TMPSAVE2.FRP", calfn);
	if(ret) {
		return FALSE;
	}
	if(!SendMessage(GetDlgItem(hdlg, IDC_PLANLIST), LB_GETCURSEL, (WPARAM)0, (LPARAM)0)) {
		clearPlanData(hdlg, TRUE);
		reloadContents(hdlg);
	}
	
	return TRUE;
}

openFilePlan(hdlg, year, month, day, title)
	HWND hdlg;
	INT year;
	INT month;
	INT day;
	TCHAR *title;
{
	FILE *f = fopen(calfn, "rb+");
	char tf[105];
	char ts[105];
	char locs[105];
	char notes[1044];
	
	INT y, m, d, importance, sh, sm, eh, em;
	TCHAR *ptitle = title;
	TCHAR ploc[105];
	TCHAR pnotes[1044];
	TCHAR st[7], et[7];
	
	if(!wcscmp(L"<새 일정 만들기...>", title)) {
		EnableWindow(GetDlgItem(hdlg, IDC_PLANTITLE), TRUE);
		clearPlanData(hdlg, TRUE);
		fclose(f);
		return TRUE;
	}
	EnableWindow(GetDlgItem(hdlg, IDC_PLANTITLE), FALSE);
	
	if(!f) {
		f = fopen(calfn, "wb");
		fprintf(f, "");
		fclose(f);
		return TRUE;
	}
	
	while(~fscanf(f, "%i %i %i %[^\t]\t%i %i %i %i %i%[^\t]\t%[^\t\0]", &y, &m, &d, tf, &sh, &sm, &eh, &em, &importance, locs, notes)) {
		wcstombs(ts, title, 100);
		if(y == year && m == month && d == day && !strcmp(ts, tf)) {
			mbstowcs(ploc, locs, 100);
			mbstowcs(pnotes, notes, 1024);
			SetDlgItemInt(hdlg, IDC_PLANIMPORTANCE, importance, TRUE);
			wsprintf(st, L"%i:%02i", sh, sm);
			wsprintf(et, L"%i:%02i", eh, em);
			SetDlgItemText(hdlg, IDC_PLANTITLE, ptitle);
			SetDlgItemText(hdlg, IDC_PLANLOCATION, ploc);
			SetDlgItemText(hdlg, IDC_PLANNOTES, pnotes);
			SetDlgItemText(hdlg, IDC_PLANSTART, st);
			SetDlgItemText(hdlg, IDC_PLANEND, et);
			fclose(f);
			return TRUE;
		}
	}
	fclose(f);
	return FALSE;
}

deleteFilePlan(hdlg, year, month, day, title)
	HWND hdlg;
	INT year;
	INT month;
	INT day;
	TCHAR *title;
{
	TCHAR *ptitle = title;
	TCHAR ploc[105];
	TCHAR psbuf[9];
	TCHAR pebuf[9];
	TCHAR pibuf[9];
	TCHAR pnotes[1030];
	TCHAR ipt[105];
	char pts[105];
	char pns[1030];
	char pls[105];
	INT importance;
	INT sh, sm, eh, em;
	
	INT i;
	INT y, m, d, shf, smf, ehf, emf, imf;
	FILE *f;
	FILE *dest;
	char tf[105];
	char notef[1044];
	char locf[105];
	INT ret = 0;
	
	f = fopen(calfn, "rb+");
	dest = fopen("TMPSAVE2.FRP", "wb");
	if(!f) {
		f = fopen(calfn, "wb");
		fprintf(f, "");
		fclose(f);
		return TRUE;
	}
	wcstombs(pts, ptitle, 100);
	wcstombs(pns, pnotes, 1024);
	wcstombs(pls, ploc, 100);
	while(~fscanf(f, "%i %i %i %[^\t]\t%i %i %i %i %i%[^\t]\t%[^\t\0]", &y, &m, &d, tf, &shf, &smf, &ehf, &emf, &imf, locf, notef)) {
		mbstowcs(ipt, tf, 100);
		if(y == year && m == month && d == day && !wcscmp(ipt, ptitle)) {
			continue;
		} else {
			fprintf(dest, "%i %i %i %s\t%i %i %i %i %i %s\t%s\t", y, m, d, tf, shf, smf, ehf, emf, imf, locf, notef);
		}
	}
	fclose(f);
	fclose(dest);
	
	ret += remove(calfn);
	ret += rename("TMPSAVE2.FRP", calfn);
	if(ret) {
		return FALSE;
	}
	
	clearPlanData(hdlg, TRUE);
	reloadContents(hdlg);
	
	return TRUE;
}

getFilePlanList(hdlg, year, month, day)
	HWND hdlg;
	INT year;
	INT month;
	INT day;
{
	FILE *f = fopen(calfn, "rb+");
	char tf[105];
	INT y, m, d;
	TCHAR ptitle[105];
	if(!f) {
		f = fopen(calfn, "wb");
		fprintf(f, "");
		fclose(f);
		return TRUE;
	}
	
	while(~fscanf(f, "%i %i %i %[^\t]\t%*i %*i %*i %*i %*i%*[^\t]\t%*[^\t\0]", &y, &m, &d, tf)) {
		if(y == year && m == month && d == day) {
			mbstowcs(ptitle, tf, 100);
			SendMessage(GetDlgItem(hdlg, IDC_PLANLIST), LB_ADDSTRING, (WPARAM)0, (LPARAM)ptitle);
		}
	}
	fclose(f);
	return TRUE;
}

