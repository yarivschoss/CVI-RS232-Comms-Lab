/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  ANALYZER                         1       /* callback function: panelCallback */
#define  ANALYZER_DIGGRAPH                2       /* control type: digitalGraph, callback function: (none) */

#define  COM1                             2       /* callback function: panelCallback */
#define  COM1_INPUT_STRING                2       /* control type: string, callback function: (none) */
#define  COM1_OUTPUT_STRING               3       /* control type: string, callback function: (none) */
#define  COM1_NUMERIC                     4       /* control type: numeric, callback function: (none) */
#define  COM1_ERROR_CORRECTION            5       /* control type: radioButton, callback function: errorCorrectionCallBack */
#define  COM1_POLLING                     6       /* control type: radioButton, callback function: pollingCallBack */
#define  COM1_SEND                        7       /* control type: command, callback function: SendCallback */
#define  COM1_PICTUREBUTTON               8       /* control type: pictButton, callback function: loadImageCallBack */
#define  COM1_TEXTMSG                     9       /* control type: textMsg, callback function: (none) */
#define  COM1_TEXTMSG_2                   10      /* control type: textMsg, callback function: (none) */
#define  COM1_PARITY                      11      /* control type: numeric, callback function: (none) */
#define  COM1_RATE                        12      /* control type: numeric, callback function: rateCallback */
#define  COM1_TIMER                       13      /* control type: timer, callback function: timerCallBack */

#define  COM2                             3       /* callback function: panelCallback */
#define  COM2_OUTPUT_STRING               2       /* control type: string, callback function: (none) */
#define  COM2_INPUT_STRING                3       /* control type: string, callback function: (none) */
#define  COM2_CANVAS                      4       /* control type: canvas, callback function: (none) */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

#define  MENUBAR                          1
#define  MENUBAR_COMM_TYPE                2
#define  MENUBAR_COMM_TYPE_STRING         3       /* callback function: CommTypeCallBack */
#define  MENUBAR_COMM_TYPE_NUMERIC        4       /* callback function: CommTypeCallBack */
#define  MENUBAR_COMM_TYPE_BIN_IMAGE      5       /* callback function: CommTypeCallBack */
#define  MENUBAR_QUIT                     6       /* callback function: QuitCallBack */


     /* Callback Prototypes: */

void CVICALLBACK CommTypeCallBack(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK errorCorrectionCallBack(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK loadImageCallBack(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK panelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK pollingCallBack(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK QuitCallBack(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK rateCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SendCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK timerCallBack(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif