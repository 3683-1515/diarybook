#if _MSC_VER > 1000
#pragma once
#endif

#define APP_NAME "내 마음속의 숲"

#include <windows.h>
#include <port1632.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <time.h>

#define IDC_STATIC					-1

#define iddMain 					100
#define iddBacks 					101
#define iddCreatePlan				102

#define IDC_NOTEBOOKBITMAP 			110
#define IDC_PLANNERBITMAP 			111

#define IDC_PAGEINDICATOR 			200
#define IDC_MONTHCALENDAR 			201
#define IDC_PLANLISTHEADER 			202
#define IDC_YESTERDAY 				206
#define IDC_TODAY 					207
#define IDC_GOTOYESTERDAY 			208
#define IDC_GOTOTOMORROW 			209
#define IDC_GOTODIARY 				210
#define IDC_GOTOPLANNER 			211
#define IDC_GOTOTODO 				212
#define IDC_PREVYEAR				213
#define IDC_YEAR					214
#define IDC_NEXTYEAR				215
#define IDC_PREVMONTH				216
#define IDC_MONTH					217
#define IDC_NEXTMONTH				218
#define IDC_GOTOTIMER				219

#define DIARY_START 				501
#define IDC_WEATHERYESTERDAY 		501
#define IDC_WEATHERTODAY 			502
#define IDC_DIARYYESTERDAY 			503
#define IDC_DIARYTODAY 				504
#define IDC_WEATHER1 				505
#define IDC_WEATHER2 				506
#define DIARY_END 					506

#define PLANNER_START 				601
#define IDC_PLANLIST 				601
#define IDC_ADDPLAN 				602
#define IDC_DELETEPLAN 				603
#define IDC_PLANNERLABEL1			604
#define IDC_PLANNERLABEL2			605
#define IDC_PLANNERLABEL3			606
#define IDC_PLANNERLABEL4			607
#define IDC_PLANNERLABEL5			608
#define IDC_PLANNERLABEL6			609
#define IDC_PLANTITLE				610
#define IDC_PLANSTART				611
#define IDC_PLANEND					612
#define IDC_PLANLOCATION			613
#define IDC_PLANIMPORTANCE			614
#define IDC_PLANIMPORTANCEUPDOWN	615
#define IDC_PLANNOTES				616
#define IDC_SAVEPLAN				617
#define IDC_RESETPLANFIELD			618
#define PLANNER_END 				618

#define TIMER_START					701
#define IDC_TIMERPROGRESS			701
#define IDC_TIMERINDICATOR			702
#define IDC_TIMERNOTES				703
#define IDC_TIMERNAME				704
#define IDC_TIMERHOURSEDIT			705
#define IDC_TIMERMINUTESEDIT		706
#define IDC_CREATETIMER 			707
#define IDC_STARTTIMER				708
#define IDC_HOURUPDOWN				709
#define IDC_MINUTEUPDOWN			710
#define IDC_SECONDUPDOWN			711
#define IDC_TIMERLIST				712
#define IDC_TIMERSECONDSEDIT		713
#define IDC_TIMERNOTESLABEL			714
#define IDC_TIMERNAMELABEL			715
#define IDC_TIMERTIMELABEL			716
#define IDC_STOPTIMER				717
#define TIMER_END					717
#define timer 						30

#define ID_ICON_MAIN 				300

#define IDC_NOTEBOOK_BMP 			400
#define IDC_PLANNER_BMP 			401
#define IDC_TIMER_BMP				402

#define DIARY 	0
#define PLANNER 1
#define TODO 	2
#define TIMER 	3

#define NUMERIC(c) ((c) >= 48 && (c) <= 57)
#define NUMBER(c) ((c) - 48)

#define FALLBACK(x,y) (!(x) ? (y) : (x))

#define ERR_NOT_FR_FILE				1
#define ERR_FEATURE_MISMATCH		2
#define ERR_MALFORMED_DATA_HEAD		3
#define ERR_INVALID_DATE			4
#define ERR_CONTENT_LENGTH_MISMATCH	5
#define ERR_UNEXPENDED_EOF			6
#define ERR_INVALID_IMPORTANCE		7

isLeapYear(WORD year);
WORD dayCount(WORD month, WORD year);
addDays(SYSTEMTIME *d, INT days, SYSTEMTIME *dest);
copyDate(SYSTEMTIME *d, SYSTEMTIME *dest);
pageIndicator(TCHAR *buf, INT page, INT last);
writePageIndicator(HWND hdlg, INT ctrl, INT page, INT last);
invalidate(HWND hdlg, INT ctl);
checkTabs(TCHAR* str);

openDiary(INT year, INT month, INT day, INT *weather, TCHAR *buf);
saveDiary(HWND hdlg, INT year, INT month, INT day, INT weather, TCHAR *content);

savePlan(HWND hdlg, INT year, INT month, INT day);
openPlan(HWND hdlg, INT year, INT month, INT day, TCHAR *title);
getPlanList(HWND hdlg, INT year, INT month, INT day);
deletePlan(HWND hdlg, INT year, INT month, INT day, TCHAR *title);
clearPlanData(HWND hdlg, BOOL clearTitle);
getImportance(HWND hdlg);

createTimer(HWND hdlg);
startTimer(HWND hdlg);
reloadTimerList(HWND hdlg);
openTimer(HWND hdlg);
processTimer(HWND hdlg);

setScreen(HWND hdlg, INT mode);
reloadContents(HWND hdlg);

typedef struct _TIMERDATA {
   TCHAR name[105];
   TCHAR content[1044];
   INT hour;
   INT minute;
   INT second;
} TIMERDATA;

typedef struct _DIARIES {
	WORD year;
	WORD month;
	WORD day;
	INT weather;
	TCHAR* content;
} DIARIES;

typedef struct _DIARYBOOK {
	DIARIES* diaries;
	LONG length;
} DIARYBOOK;

typedef struct _PLANS {
	WORD year;
	WORD month;
	WORD day;
	TCHAR* title;
	INT startHour;
	INT startMinute;
	INT endHour;
	INT endMinute;
	TCHAR* location;
	INT importance;
	TCHAR* notes;
} PLANS;

typedef struct _PLANBOOK {
	PLANS* plans;
	LONG length;
} PLANBOOK;

readDiary(char* filename, DIARYBOOK* diary);
writeDiary(char* filename, DIARYBOOK* diary);
initDiary();
readPlanner(char* filename, PLANBOOK* calendar);
writePlanner(char* filename, PLANBOOK* calendar);
initPlanner();

