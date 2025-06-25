#include <userint.h>
#include "EX8.h"
#include "def.h"

static int com1, com2, analyzer;

static int com1ThreadID, com2ThreadID, writeLockHandle, readLockHandle;

static double rate = 1.0;

static int usePolling = 0, useErrorCorrection = 0 , isString = 1, isNumeric = 0, isBinary = 0;

static comBox box1, box2;

static imageBox outImage, inImage;
static int parity;

int main (int argc, char *argv[])
{
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	
	if ((com1 = LoadPanel (0, "EX8.uir", COM1)) < 0)
		return -1;
	if ((com2 = LoadPanel (0, "EX8.uir", COM2)) < 0)
		return -1;
	if ((analyzer = LoadPanel (0, "EX8.uir", ANALYZER)) < 0)
		return -1;
	
	initilizeComPorts("COM1", "COM2", 9600, 0, 7, 1, Q_SIZE, Q_SIZE);
	
	DisplayPanel (com1);
	DisplayPanel (analyzer);
	
	RunUserInterface ();
	
	DiscardPanel (com1);
	DiscardPanel (com2);
	DiscardPanel (analyzer);
	return 0;
}

int CVICALLBACK SendCallback (int panel, int control, int event,
							  void *callbackData, int eventData1, int eventData2)
{
    if (event != EVENT_COMMIT)
        return 0;

    if (isString && !usePolling)
    {
		
		SetCtrlAttribute(com1, COM1_TIMER, ATTR_ENABLED, 0); // stopping timer   
		   
	    char inBuffer[BUFFER_SIZE], outBuffer[BUFFER_SIZE];
	    GetCtrlVal(com1, COM1_OUTPUT_STRING, inBuffer);

	    // Add terminating character (e.g., '\n') if expected by receiver
	    strcat(outBuffer, "\n");

	    // Write to COM1
	    int bytesWritten = ComWrt(COM_PORT_1, outBuffer, (int)strlen(outBuffer));
	    
	    if (bytesWritten < 0)
	        MessagePopup("COM Write Error", "Failed to send data from COM1.");
			 
	    int bytesRead = ComRd (COM_PORT_2, inBuffer, bytesWritten);
		 
	    if (bytesRead < 0)
	    	MessagePopup("COM Read Error", "Failed to read data from COM2.");
		 
		int binArray[BUFFER_SIZE];  
		int bitCount = StrtoBin(inBuffer,binArray) - 8; // removing '/n'

		DeleteGraphPlot (analyzer, ANALYZER_DIGGRAPH, -1, VAL_IMMEDIATE_DRAW);  // Clear old
		PlotDigitalLines (analyzer, ANALYZER_DIGGRAPH, binArray, bitCount, VAL_UNSIGNED_INTEGER,  1);
			
    }
	
    else if (isNumeric)
    {
	    double num = 0.0;
	    char buffer[BUFFER_SIZE];
		comBox numericBox = {0};

	    GetCtrlVal(com1, COM1_NUMERIC, &num);
	    sprintf(buffer, "%.2f\0", num); // Add newline for termination

	    int bytesWritten = ComWrt(COM_PORT_1, buffer, (int)strlen(buffer));

	    if (bytesWritten < 0)
	        MessagePopup("COM Write Error", "Failed to send numeric data from COM1.");
		
		numericBox.panel = com2;
		numericBox.inControl = COM2_INPUT_STRING;
		
		InstallComCallback (COM_PORT_2, LWRS_RXFLAG, 0, 0, readInterrupt, &numericBox);
		ProcessSystemEvents();
    }
	
    else if (isBinary)
    {
    if (!outImage.filename) 
	{
        MessagePopup("Error", "Please load an image first");
        return 0;
    }
		
	resetImage(&inImage);
	
	InstallComCallback (COM_PORT_2, LWRS_RECEIVE, 16, 0, receiveImage, &inImage); // using interrupt for reading - prevents queue overflow
	
	CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, SendImageAsync, &outImage, NULL);
	}
	
	return 0;
}

void CVICALLBACK CommTypeCallBack (int menuBar, int menuItem, void *callbackData,
								   int panel)
{
	// Uncheck all types
    SetMenuBarAttribute(menuBar, MENUBAR_COMM_TYPE_STRING,     ATTR_CHECKED, 0);
    SetMenuBarAttribute(menuBar, MENUBAR_COMM_TYPE_NUMERIC,    ATTR_CHECKED, 0);
    SetMenuBarAttribute(menuBar, MENUBAR_COMM_TYPE_BIN_IMAGE,  ATTR_CHECKED, 0);
	
	dimAll(com1, com2);
	
	isString = isNumeric = isBinary = 0;
	
    // Enable controls depending on menu selection ---
    switch (menuItem)
    {
        case MENUBAR_COMM_TYPE_STRING:
        {
			SetMenuBarAttribute(menuBar, menuItem, ATTR_CHECKED, 1);
			GetMenuBarAttribute(menuBar, MENUBAR_COMM_TYPE_STRING, ATTR_CHECKED, &isString);
            setStringDisplay(com1, com2);
            break;
        }

        case MENUBAR_COMM_TYPE_NUMERIC:
        {
			HidePanel(analyzer);
			DisplayPanel(com2);
			SetMenuBarAttribute(menuBar, menuItem, ATTR_CHECKED, 1);
			GetMenuBarAttribute(menuBar, MENUBAR_COMM_TYPE_NUMERIC, ATTR_CHECKED, &isNumeric);
            setNumericDisplay(com1, com2);
            break;
        }

        case MENUBAR_COMM_TYPE_BIN_IMAGE:
        {
			HidePanel(analyzer);
			DisplayPanel(com2);
			SetMenuBarAttribute(menuBar, menuItem, ATTR_CHECKED, 1);
			GetMenuBarAttribute(menuBar, MENUBAR_COMM_TYPE_BIN_IMAGE, ATTR_CHECKED, &isBinary);
            setImageDisplay(com1, com2);
            break;
        }
    }

   
}

int CVICALLBACK pollingCallBack (int panel, int control, int event,
								 void *callbackData, int eventData1, int eventData2)
{
    if (event != EVENT_COMMIT)
        return 0;
	
    GetCtrlVal(com1, COM1_POLLING, &usePolling);
	
	// Hides/Displays relevent panel
	HidePanel(usePolling ? analyzer : com2); 
	DisplayPanel(usePolling ? com2 : analyzer);

    // Re-apply display logic
    dimAll(com1, com2);
    setStringDisplay(com1, com2);  // Re-show the right controls for polling mode
	
	if (usePolling) // polling checked
	{
		// creating locks for threads
		CmtNewLock (NULL, 0, &writeLockHandle);
		CmtNewLock (NULL, 0, &readLockHandle);
		
		// initilizing comBoxes 
		box1.panel = com1;
		box1.port = COM_PORT_1;
		box1.inControl = COM1_INPUT_STRING;
		box1.outControl = COM1_OUTPUT_STRING;
		
		box2.panel = com2;
		box2.port = COM_PORT_2;
		box2.inControl = COM2_INPUT_STRING;
		box2.outControl = COM2_OUTPUT_STRING;
		
		// starting timer
	    SetCtrlAttribute(com1, COM1_TIMER, ATTR_ENABLED, 1); 
	}
	
	return 0;
}

int CVICALLBACK timerCallBack (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
    if (event != EVENT_TIMER_TICK)
        return 0;
	
	// Schedule threads to do the work - will allow working in background
	CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, comThread, &box1, &com1ThreadID);
	CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, comThread, &box2, &com2ThreadID);
	return 0;
}

int CVICALLBACK rateCallback (int panel, int control, int event,
							  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(com1, COM1_RATE, &rate);  // Get polling interval from numeric control
			SetCtrlAttribute(com1, COM1_TIMER, ATTR_INTERVAL, rate);
			break;
	}
	return 0;
}

int CVICALLBACK errorCorrectionCallBack (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	 if (event != EVENT_COMMIT)
        return 0;
        
    GetCtrlVal(panel, control, &useErrorCorrection);
	
    if (useErrorCorrection) 
	{
		CloseCom (COM_PORT_1);
		CloseCom (COM_PORT_2);
		SetCtrlAttribute(panel, COM1_PARITY, ATTR_DIMMED, 0);
		GetCtrlVal(panel, COM1_PARITY, &parity);
        initilizeComPorts("COM1", "COM2", 9600, parity, 7, 1, 512, 512);
    }
	
	else
	{
		CloseCom (COM_PORT_1);
		CloseCom (COM_PORT_2);
		SetCtrlAttribute(panel, COM1_PARITY, ATTR_DIMMED, 1);
        initilizeComPorts("COM1", "COM2", 9600, 0, 7, 1, 512, 512); // 0 parity bits
	}
	
	return 0;
}


int CVICALLBACK loadImageCallBack (int panel, int control, int event,
								   void *callbackData, int eventData1, int eventData2)
{
    if (event != EVENT_COMMIT)
        return 0;
	
	resetImage(&outImage);
        
    int status = FileSelectPopup(".\images", "*.*","*.jpg;*.jpeg;*.png;*.bmp;", 
                                "Select Image File", VAL_SELECT_BUTTON, 0, 1, 1, 1, outImage.filename);
    
    if (status == VAL_EXISTING_FILE_SELECTED) {
        if (loadImageFile(&outImage) == 0) {
            char message[256];
			int   sizeBytes = 0;
			double sizeKB   = 0.0;
			if (GetFileInfo(outImage.filename, &sizeBytes) == 1) 
			{   /* success */
			    sizeKB = sizeBytes / 1024.0;                         /* KiB */
			}
            sprintf(message, "Image loaded: %dx%d, %.2f KB", outImage.width, outImage.height, sizeKB);
            MessagePopup("Success", message);
        }
    }
    
    return 0;
}

void CVICALLBACK QuitCallBack (int menuBar, int menuItem, void *callbackData,
							   int panel)
{
	CmtWaitForThreadPoolFunctionCompletion ( DEFAULT_THREAD_POOL_HANDLE, com1ThreadID ,0);
	CmtWaitForThreadPoolFunctionCompletion ( DEFAULT_THREAD_POOL_HANDLE, com2ThreadID , 0);
	CloseCom(COM_PORT_1);
	CloseCom(COM_PORT_2);
	CmtDiscardLock (readLockHandle);
	CmtDiscardLock (writeLockHandle);
	QuitUserInterface (0);
}

int CVICALLBACK panelCallback (int panel, int event, void *callbackData,
							   int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_GOT_FOCUS:
			

			break;
			
		case EVENT_LOST_FOCUS:

			break;
			
		case EVENT_CLOSE:
			CmtWaitForThreadPoolFunctionCompletion (DEFAULT_THREAD_POOL_HANDLE, com1ThreadID , 0);
			CmtWaitForThreadPoolFunctionCompletion (DEFAULT_THREAD_POOL_HANDLE, com2ThreadID , 0);
			CloseCom(COM_PORT_1);
			CloseCom(COM_PORT_2);
			CmtDiscardLock (readLockHandle);
			CmtDiscardLock (writeLockHandle);
			QuitUserInterface (0);
			break;
	}
	return 0;
}

