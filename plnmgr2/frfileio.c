#include "plnmgr2.h"
#define ANY_CHAR (0xff)
#define _ 0x00

int lastFileError = 0;

const unsigned char frMagic[] = { 'F', 'R', 0x07, 0xD8, 0x03, 0x31 };

const unsigned char frDiaryHeader[] = {
	0x01,  										/* 헤더 시작 */
	DIARY,										/* 일기장 파일임을 나타냄 */
	ANY_CHAR, ANY_CHAR, ANY_CHAR, ANY_CHAR,  	/* 일기의 개수 */
	ANY_CHAR, 									/* 저장한 프로그램 버전 */
	0x02,										/* 헤더 끝 */
};
const unsigned char frEmptyDiaryHeader[] = {
	0x01,
	DIARY,
	0x00, 0x00, 0x00, 0x00,
	0x01,
	0x02,
};
const unsigned char frDiaryDataHead[] = {
	ANY_CHAR, ANY_CHAR,			/* 년 */
	ANY_CHAR, 					/* 월 */
	ANY_CHAR, 					/* 일 */
	ANY_CHAR,					/* 날씨 */
	ANY_CHAR, ANY_CHAR, 		/* 일기 글자 바이트 수 */
};
const unsigned char frDiaryDataFoot[] = {
	0x00, 						/* 널 */
};

const unsigned char frPlanHeader[] = {
	0x01,  										/* 헤더 시작 */
	PLANNER,									/* 일정 파일임을 나타냄 */
	ANY_CHAR, ANY_CHAR, ANY_CHAR, ANY_CHAR,  	/* 일정 개수 */
	ANY_CHAR, 									/* 저장한 프로그램 버전 */
	0x02,										/* 헤더 끝 */
};
const unsigned char frEmptyPlanHeader[] = {
	0x01,
	PLANNER,
	0x00, 0x00, 0x00, 0x00,
	0x01,
	0x02,
};
const unsigned char frPlanDataHead[] = {
	ANY_CHAR, ANY_CHAR,			/* 년 */
	ANY_CHAR, 					/* 월 */
	ANY_CHAR, 					/* 일 */
};
const unsigned char frPlanDataFoot[] = {
	0x00, 						/* 널 */
};

unsigned char magic[7];
unsigned char header[19];
unsigned char dhead[9];
unsigned char dfoot[3];

short fromOneByte(a)
	unsigned char a;
{
	short ret = a;
	return ret;
}

short fromTwoBytes(a, b)
	unsigned char a;
	unsigned char b;
{
	short ret;
	ret = a;
	ret = (ret << 8) + b;
	return ret;
}

long fromFourBytes(a, b, c, d)
	unsigned char a;
	unsigned char b;
	unsigned char c;
	unsigned char d;
{
	long ret;
	ret = a;
	ret = (ret << 8) + b;
	ret = (ret << 8) + c;
	ret = (ret << 8) + d;
	return ret;
}

toOneByte(byte, a)
	char byte;
	unsigned char* a;
{
	*a = byte;
}

toTwoBytes(bytes, a, b)
	short bytes;
	unsigned char* a;
	unsigned char* b;
{
	*a = (unsigned char)(bytes >> 8);
	*b = (unsigned char)(bytes - ((*a) * 0x100));
}

toFourBytes(bytes, a, b, c, d)
	long bytes;
	unsigned char* a;
	unsigned char* b;
	unsigned char* c;
	unsigned char* d;
{
	*a = (unsigned char)((((bytes) >> 8) >> 8) >> 8);
	*b = (unsigned char)(((bytes - (*a) * 0x100 * 0x100 * 0x100) >> 8) >> 8);
	*c = (unsigned char)((bytes - (*a) * 0x100 * 0x100 * 0x100 - (*b) * 0x100 * 0x100) >> 8);
	*d = (unsigned char)((bytes - (*a) * 0x100 * 0x100 * 0x100 - (*b) * 0x100 * 0x100) - (*c) * 0x100);
}

compare(_s1, _s2)
	unsigned char* _s1;
	unsigned char* _s2;
{
	unsigned char *s1 = _s1, *s2 = _s2;
	while(*s1 && *s2) {
		if(*s1 != ANY_CHAR && *s2 != ANY_CHAR && *s1 != *s2) {
			return 0;
		}
		s1++, s2++;
	}
	return 1;
}

readDiary(filename, diary)
	char* filename;
	DIARYBOOK* diary;
{
	FILE *f;
	long diaryCount, i, k;
	int length;
	WORD year, month, day;
	int weather;
	short wcb;
	char wc[4];
	
	f = fopen(filename, "rb");
	if(!f) {
		f = fopen(filename, "wb");
		fwrite(frMagic, sizeof(frMagic), 1, f);
		fwrite(frEmptyDiaryHeader, sizeof(frEmptyDiaryHeader), 1, f);
		diary->length = 0;
		fclose(f);
		return 0;
	}
	if(fread(magic, 1, 6, f) != 6 || !compare(magic, frMagic)) {
		return ERR_NOT_FR_FILE;
	}
	if(fread(header, 1, 8, f) != 8 || !compare(header, frDiaryHeader)) {
		return ERR_FEATURE_MISMATCH;
	}
	diaryCount = fromFourBytes(header[2], header[3], header[4], header[5]);
	diary->length = diaryCount;
	diary->diaries = (DIARIES*) malloc(sizeof(DIARIES) * diaryCount);
	for(i=0; i<diaryCount; i++) {
		if(fread(dhead, 1, 7, f) != 7 || !compare(frDiaryDataHead, dhead)) {
			diary->length = 0;
			free(diary->diaries);
			return ERR_MALFORMED_DATA_HEAD;
		}
		year  = fromTwoBytes(dhead[0], dhead[1]);
		month = fromOneByte(dhead[2]);
		day   = fromOneByte(dhead[3]);
		if(year < 1901 || year > 9999 || month < 1 || month > 12 || day < 1 || day > dayCount(month, year)) {
			diary->length = 0;
			free(diary->diaries);
			return ERR_INVALID_DATE;
		}
		diary->diaries[i].year  = year;
		diary->diaries[i].month = month;
		diary->diaries[i].day   = day;
		weather = fromOneByte(dhead[4]);
		if(weather < 0 || weather > 3) weather = 0;
		diary->diaries[i].weather = weather;
		length = fromTwoBytes(dhead[5], dhead[6]);
		(diary->diaries[i]).content = (TCHAR*) malloc(length * sizeof(TCHAR) + 2);
		wcscpy(diary->diaries[i].content, L"");
		for(k=0; k<length; k++) {
			if(fread(wc, 1, 2, f) != 2) goto lengthError;
			wcb = fromTwoBytes(wc[0], wc[1]);
			if(!wcb) {
lengthError:
				diary->length = 0;
				free(diary->diaries[i].content);
				free(diary->diaries);
				return ERR_CONTENT_LENGTH_MISMATCH;
			}
			diary->diaries[i].content[k] = wcb;
		}
		dfoot[0] = 0;
		fread(dfoot, 1, 1, f);
		if(dfoot[0]) {
			diary->length = 0;
			free(diary->diaries[i].content);
			free(diary->diaries);
			return ERR_CONTENT_LENGTH_MISMATCH;
		}
		diary->diaries[i].content[length] = 0;
	}
	fclose(f);
	return 0;
}

writeDiary(filename, diary)
	char* filename;
	DIARYBOOK* diary;
{
	FILE *f;
	long diaryCount, i, k;
	short length;
	WORD year, month, day;
	int weather;
	int clength;
	int ret = 0;
	char diaryHeader[] = {
		0x01,
		DIARY,
		_, _, _, _,
		0x01,
		0x02,
	};
	char buf[24];
	char wc[4];
	char nul[1] = { 0 };
	DIARIES* dd;
	diaryCount = diary->length;
	toFourBytes(diaryCount, diaryHeader+2, diaryHeader+3, diaryHeader+4, diaryHeader+5);
	
	f = fopen("TMPSAVE.FRB", "wb");
	fwrite(frMagic, sizeof(frMagic), 1, f);
	fwrite(diaryHeader, sizeof(diaryHeader), 1, f);
	for(i=0; i<diaryCount; i++) {
		dd = &(diary->diaries[i]);
		toTwoBytes(dd->year, buf+0, buf+1);
		toOneByte(dd->month, buf+2);
		toOneByte(dd->day, buf+3);
		toOneByte(dd->weather, buf+4);
		clength = wcslen(dd->content);
		toTwoBytes(clength, buf+5, buf+6);
		fwrite(buf, 7, 1, f);
		for(k=0; k<clength; k++) {
			toTwoBytes(dd->content[k], wc+0, wc+1);
			fwrite(wc, 2, 1, f);
		}
		fwrite(nul, 1, 1, f);
	}
	fclose(f);
	
	ret += remove(filename);
	ret += rename("TMPSAVE.FRB", filename);
	return ret;
}

readPlanner(filename, calendar)
	char* filename;
	PLANBOOK* calendar;
{
	FILE *f;
	long planCount, i, k;
	int length;
	WORD year, month, day;
	int sh, sm, eh, em, importance;
	short wcb;
	char wc[4];
	char byte[6];
	
	f = fopen(filename, "rb");
	if(!f) {
		f = fopen(filename, "wb");
		fwrite(frMagic, sizeof(frMagic), 1, f);
		fwrite(frEmptyPlanHeader, sizeof(frEmptyPlanHeader), 1, f);
		fclose(f);
		calendar->length = 0;
		return 0;
	}
	if(fread(magic, 1, 6, f) != 6 || !compare(magic, frMagic)) {
		return ERR_NOT_FR_FILE;
	}
	if(fread(header, 1, 8, f) != 8 || !compare(header, frPlanHeader)) {
		return ERR_FEATURE_MISMATCH;
	}
	planCount = fromFourBytes(header[2], header[3], header[4], header[5]);
	calendar->length = planCount;
	calendar->plans = (PLANS*) malloc(sizeof(PLANS) * planCount);
	for(i=0; i<planCount; i++) {
		if(fread(dhead, 1, 4, f) != 4 || !compare(frPlanDataHead, dhead)) {
			calendar->length = 0;
			free(calendar->plans);
			return ERR_MALFORMED_DATA_HEAD;
		}
		year  = fromTwoBytes(dhead[0], dhead[1]);
		month = fromOneByte(dhead[2]);
		day   = fromOneByte(dhead[3]);
		if(year < 1901 || year > 9999 || month < 1 || month > 12 || day < 1 || day > dayCount(month, year)) {
			calendar->length = 0;
			free(calendar->plans);
			return ERR_INVALID_DATE;
		}
		calendar->plans[i].year  = year;
		calendar->plans[i].month = month;
		calendar->plans[i].day   = day;
		
		length = 0;
		calendar->plans[i].title = (TCHAR*) malloc(2 * sizeof(TCHAR));
		while(1) {
			if(fread(wc, 1, 2, f) != 2) {
				calendar->length = 0;
				free(calendar->plans[i].title);
				free(calendar->plans);
				return ERR_CONTENT_LENGTH_MISMATCH;
			}
			wcb = fromTwoBytes(wc[0], wc[1]);
			if(!wcb) {
				calendar->plans[i].title[length] = 0;
				break;
			}
			length++;
			calendar->plans[i].title = (TCHAR*) realloc(calendar->plans[i].title, length * sizeof(TCHAR) + 2);
			calendar->plans[i].title[length-1] = wcb;
		}
		goto readTime;
		
invalidDateError:
		calendar->length = 0;
		free(calendar->plans[i].title);
		free(calendar->plans);
		return ERR_INVALID_DATE;
		
readTime:
		if(fread(byte, 1, 1, f) != 1) goto invalidDateError;
		sh = fromOneByte(byte[0]);
		if(fread(byte, 1, 1, f) != 1) goto invalidDateError;
		sm = fromOneByte(byte[0]);
		if(fread(byte, 1, 1, f) != 1) goto invalidDateError;
		eh = fromOneByte(byte[0]);
		if(fread(byte, 1, 1, f) != 1) goto invalidDateError;
		em = fromOneByte(byte[0]);
		if(sh < 0 || sh > 23 || eh < 0 || eh > 23 || sm < 0 || sm > 59 || em < 0 || em > 59)
			goto invalidDateError;
		
		calendar->plans[i].startHour = sh;
		calendar->plans[i].startMinute = sm;
		calendar->plans[i].endHour = eh;
		calendar->plans[i].endMinute = em;
		
		length = 0;
		calendar->plans[i].location = (TCHAR*) malloc(2 * sizeof(TCHAR));
		while(1) {
			if(fread(wc, 1, 2, f) != 2) {
				calendar->length = 0;
				free(calendar->plans[i].title);
				free(calendar->plans[i].location);
				free(calendar->plans);
				return ERR_CONTENT_LENGTH_MISMATCH;
			}
			wcb = fromTwoBytes(wc[0], wc[1]);
			if(!wcb) {
				calendar->plans[i].location[length] = 0;
				break;
			}
			length++;
			calendar->plans[i].location = (TCHAR*) realloc(calendar->plans[i].location, length * sizeof(TCHAR) + 2);
			calendar->plans[i].location[length-1] = wcb;
		}
		
		if(fread(byte, 1, 1, f) != 1) {
			calendar->length = 0;
			free(calendar->plans[i].title);
			free(calendar->plans[i].location);
			free(calendar->plans);
			return ERR_UNEXPENDED_EOF;
		}
		importance = fromOneByte(byte[0]);
		if(importance < 1 || importance > 10) {
			calendar->length = 0;
			free(calendar->plans[i].title);
			free(calendar->plans[i].location);
			free(calendar->plans);
			return ERR_INVALID_IMPORTANCE;
		}
		calendar->plans[i].importance = importance;
		
		length = 0;
		calendar->plans[i].notes = (TCHAR*) malloc(2 * sizeof(TCHAR));
		while(1) {
			if(fread(wc, 1, 2, f) != 2) {
				calendar->length = 0;
				free(calendar->plans[i].title);
				free(calendar->plans[i].location);
				free(calendar->plans[i].notes);
				free(calendar->plans);
				return ERR_CONTENT_LENGTH_MISMATCH;
			}
			wcb = fromTwoBytes(wc[0], wc[1]);
			if(!wcb) {
				calendar->plans[i].notes[length] = 0;
				break;
			}
			length++;
			calendar->plans[i].notes = (TCHAR*) realloc(calendar->plans[i].notes, length * sizeof(TCHAR) + 2);
			calendar->plans[i].notes[length-1] = wcb;
		}
	}
	fclose(f);
	return 0;
}

writePlanner(filename, calendar)
	char* filename;
	PLANBOOK* calendar;
{
	FILE *f = fopen("TMPSAVE2.FRB", "wb");
	PLANS* plan;
	int i, ret = 0, pos;
	int length, sublen = 0;
	char byte[4];
	char nul[1] = { 0 };
	char wc[4];
	char planHeader[] = {
		0x01,
		PLANNER,
		_, _, _, _,
		0x01,
		0x02,
	};
	fwrite(frMagic, sizeof(frMagic), 1, f);
	length = calendar->length;
	for(i=0; i<calendar->length; i++) {
		plan = &(calendar->plans[i]);
		if(!(plan->title[0])) sublen++;
	}
	toFourBytes(length - sublen, planHeader+2, planHeader+3, planHeader+4, planHeader+5);
	fwrite(planHeader, sizeof(planHeader), 1, f);
	for(i=0; i<length; i++) {
		plan = &(calendar->plans[i]);
		if(!(plan->title[0])) continue;
		
		toTwoBytes(plan->year, byte+0, byte+1);
		toOneByte(plan->month, byte+2);
		toOneByte(plan->day, byte+3);
		fwrite(byte, 4, 1, f);
		
		pos = 0;
		while(1) {
			if(!(plan->title[pos])) {
				fwrite(nul, 1, 1, f);
				fwrite(nul, 1, 1, f);
				break;
			}
			toTwoBytes(plan->title[pos], wc+0, wc+1);
			fwrite(wc, 2, 1, f);
			pos++;
		}
		
		toOneByte(plan->startHour, byte+0);
		toOneByte(plan->startMinute, byte+1);
		toOneByte(plan->endHour, byte+2);
		toOneByte(plan->endMinute, byte+3);
		fwrite(byte, 4, 1, f);
		
		pos = 0;
		while(1) {
			if(!(plan->location[pos])) {
				fwrite(nul, 1, 1, f);
				fwrite(nul, 1, 1, f);
				break;
			}
			toTwoBytes(plan->location[pos], wc+0, wc+1);
			fwrite(wc, 2, 1, f);
			pos++;
		}
		
		toOneByte(plan->importance, byte+0);
		fwrite(byte, 1, 1, f);
		
		pos = 0;
		while(1) {
			if(!(plan->notes[pos])) {
				fwrite(nul, 1, 1, f);
				fwrite(nul, 1, 1, f);
				break;
			}
			toTwoBytes(plan->notes[pos], wc+0, wc+1);
			fwrite(wc, 2, 1, f);
			pos++;
		}
	}
	fclose(f);
	
	ret += remove(filename);
	ret += rename("TMPSAVE2.FRB", filename);
	return ret;
}

