#include "plnmgr2.h"

extern char* filename;
DIARYBOOK diary;
long long lastSaved = 0;

initDiary()
{
	extern int lastFileError;
	return readDiary(filename, &diary);
}

openDiary(year, month, day, weather, buf)
	INT year;
	INT month;
	INT day;
	INT *weather;
	TCHAR *buf;
{
	DIARIES* dd;
	INT i;
	for(i=0; i<diary.length; i++) {
		dd = &(diary.diaries[i]);
		if(dd->year == year && dd->month == month && dd->day == day) {
			wcscpy(buf, dd->content);
			*weather = dd->weather;
			return TRUE;
		}
	}
	return FALSE;
}

saveDiary(hdlg, year, month, day, weather, content)
	HWND hdlg;
	INT year;
	INT month;
	INT day;
	INT weather;
	TCHAR *content;
{
	DIARIES* dd;
	INT i;
	BOOL saved = FALSE;
	for(i=0; i<diary.length; i++) {
		dd = &(diary.diaries[i]);
		if(dd->year == year && dd->month == month && dd->day == day) {
			free(dd->content);
			dd->content = (TCHAR*) malloc(wcslen(content) * sizeof(TCHAR) + 2);
			wcscpy(dd->content, content);
			dd->weather = weather;
			saved = TRUE;
			break;
		}
	}
	if(!saved) {
		diary.diaries = (DIARIES*) realloc(diary.diaries, (++diary.length) * sizeof(DIARIES));
		diary.diaries[diary.length-1].year = (WORD)year;
		diary.diaries[diary.length-1].month = (WORD)month;
		diary.diaries[diary.length-1].day = (WORD)day;
		diary.diaries[diary.length-1].weather = weather;
		diary.diaries[diary.length-1].content = (TCHAR*) malloc(wcslen(content) * sizeof(TCHAR) + 2);
		wcscpy(diary.diaries[diary.length-1].content, content);
	}
	if(lastSaved < (long long)time(0) - 10) {
		writeDiary(filename, &diary);
		lastSaved = (long long)time(0);
	}
	return TRUE;
}

openTextDiary(year, month, day, weather, buf)
	INT year;
	INT month;
	INT day;
	INT *weather;
	TCHAR *buf;
{
	INT i;
	INT y, m, d, w;
	FILE *f = fopen(filename, "rb+");
	TCHAR content[32009];
	char _content[32009];
	if(!f) {
		f = fopen(filename, "wb");
		fprintf(f, "");
		fclose(f);
		return FALSE;
	}
	while(~fscanf(f, "%i %i %i %i %[^\t\0]", &y, &m, &d, &w, _content)) {
		if(y == year && m == month && d == day) {
			mbstowcs(content, _content, 32000);
			wsprintf(buf, L"%ls", content);
			*weather = w;
			fclose(f);
			return TRUE;
		}
	}
	*weather = 0;
	fclose(f);
	return FALSE;
}

saveTextDiary(hdlg, year, month, day, weather, content)
	HWND hdlg;
	INT year;
	INT month;
	INT day;
	INT weather;
	TCHAR *content;
{
	INT i;
	INT y, m, d, w;
	FILE *f;
	FILE *dest;
	char _content[32009];
	char _save[32009];
	INT ret = 0;
	if(!checkTabs(content)) {
		MessageBox(hdlg, L"탭 문자가 들어가면 안 됩니다.", L"저장 오류", MB_ICONERROR | MB_OK);
		return FALSE;
	}
	f = fopen(filename, "rb+");
	dest = fopen("TMPSAVE.FRD", "wb");
	if(!f) {
		f = fopen(filename, "wb");
		fprintf(f, "");
		fclose(f);
		return FALSE;
	}
	while(~fscanf(f, "%i %i %i %i %[^\t\0]", &y, &m, &d, &w, _content)) {
		if(y == year && m == month && d == day) {
			continue;
		} else {
			fprintf(dest, "%i %i %i %i %s\t", y, m, d, w, _content);
		}
	}
	fclose(f);
	wcstombs(_save, content, 32000);
	fprintf(dest, "%i %i %i %i %s", year, month, day, weather, _save);
	fclose(dest);
	
	ret += remove(filename);
	ret += rename("TMPSAVE.FRD", filename);
	if(ret) {
		return FALSE;
	}
	return TRUE;
}
