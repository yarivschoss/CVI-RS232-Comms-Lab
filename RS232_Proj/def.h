#ifndef  __DEF_H
#define  __DEF_H

// libraries imports
#include <cvirte.h>		
#include <userint.h>
#include <stdio.h>
#include <utility.h>
#include <string.h>
#include <rs232.h>
#include <math.h>
#include <formatio.h> 

// headers
#include "EX8.h"

#define COM_PORT_1 1
#define COM_PORT_2 2

#define BUFFER_SIZE 256
#define Q_SIZE 15000
#define CR 13


// handles
extern int writeLockHandle, readLockHandle;

// panels
extern int com1, com2, analyzer;



// structs
typedef struct {
	
  int panel, port, inControl, outControl;
	
} comBox; // COM box of parameters

typedef struct {
    char filename[MAX_PATHNAME_LEN];
	int bitmapID;
	int pixelDepth;
	int bytesPerRow;
	int width, height;
	int* colorTable;
	int colorTableSize;
	unsigned char* bits;
	int bitsSize;
	unsigned char* mask;
	int maskSize;
} imageBox; // Image transmission structure

// image variables
extern int useErrorCorrection;
extern imageBox inImage, outImage;

// initillizing functions

void initilizeComPorts(const char* port1, const char* port2, int baudRate, int parity, 
					   int dataBits, int stopBits, int inQueueSize, int outQueueSize)
{
	int status;

	// Open COM1
	status = OpenComConfig(COM_PORT_1, port1, baudRate, parity, dataBits, stopBits, inQueueSize, outQueueSize);
	if (status < 0)
	{
	    MessagePopup("COM1 Error", "Failed to open COM1.");
	}
	else
	{
	    FlushInQ(COM_PORT_1);  // Clear input buffer
	    FlushOutQ(COM_PORT_1); // Clear output buffer
	}

	// Open COM2
	status = OpenComConfig(COM_PORT_2, port2, baudRate, parity, dataBits, stopBits, inQueueSize, outQueueSize);
	if (status < 0)
	{
	    MessagePopup("COM2 Error", "Failed to open COM2.");
	}
	else
	{
	    FlushInQ(COM_PORT_2);
	    FlushOutQ(COM_PORT_2);
	}
}

// display functions

/***********************************************************************************/

void dimAll(int com1, int com2)
{
	// COM1 Controls
    SetCtrlAttribute(com1, COM1_OUTPUT_STRING, ATTR_DIMMED, 1);
    SetCtrlAttribute(com1, COM1_INPUT_STRING,  ATTR_DIMMED, 1);
    SetCtrlAttribute(com1, COM1_NUMERIC,       ATTR_DIMMED, 1);
    SetCtrlAttribute(com1, COM1_ERROR_CORRECTION,   ATTR_DIMMED, 1);
    SetCtrlAttribute(com1, COM1_PICTUREBUTTON,      ATTR_DIMMED, 1);
    SetCtrlAttribute(com1, COM1_TEXTMSG,            ATTR_DIMMED, 1);
    SetCtrlAttribute(com1, COM1_TEXTMSG_2,          ATTR_DIMMED, 1);
	SetCtrlAttribute(com1, COM1_POLLING, ATTR_DIMMED, 1);
    SetCtrlAttribute(com1, COM1_RATE,  ATTR_DIMMED, 1);
    SetCtrlAttribute(com1, COM1_SEND, ATTR_DIMMED, 1);

    // COM2 Controls
    SetCtrlAttribute(com2, COM2_INPUT_STRING,  ATTR_DIMMED, 1);
    SetCtrlAttribute(com2, COM2_OUTPUT_STRING, ATTR_DIMMED, 1);
    SetCtrlAttribute(com2, COM2_CANVAS,             ATTR_DIMMED, 1);
}

void setStringDisplay(int com1, int com2)
{
    // Enable COM1 string I/O
    SetCtrlAttribute(com1, COM1_OUTPUT_STRING, ATTR_DIMMED, 0);
	SetCtrlAttribute(com1, COM1_POLLING,  ATTR_DIMMED, 0);
	SetCtrlAttribute(com1, COM1_SEND, ATTR_DIMMED, 0);

    // Check if polling is enabled, to decide COM2 visibility
    int polling = 0;
    GetCtrlVal(com1, COM1_POLLING, &polling);
    if (polling)
    {
		SetCtrlAttribute(com1, COM1_SEND, ATTR_DIMMED, 1);
		SetCtrlAttribute(com1, COM1_RATE,  ATTR_DIMMED, 0);
		SetCtrlAttribute(com2, COM2_INPUT_STRING, ATTR_DIMMED, 0);
        SetCtrlAttribute(com2, COM2_OUTPUT_STRING, ATTR_DIMMED, 0);
		SetCtrlAttribute(com1, COM1_INPUT_STRING,  ATTR_DIMMED, 0);
    }
	if(!polling) 
	{
		HidePanel(com2);
		DisplayPanel (analyzer);
	}
}

void setNumericDisplay(int com1, int com2)
{
    SetCtrlAttribute(com1, COM1_NUMERIC, ATTR_DIMMED, 0);
	SetCtrlAttribute(com2, COM2_INPUT_STRING,  ATTR_DIMMED, 0);
	SetCtrlAttribute(com1, COM1_SEND, ATTR_DIMMED, 0);
}

void setImageDisplay(int com1, int com2)
{
    // Enable image-related controls in COM1 and COM2
    SetCtrlAttribute(com1, COM1_ERROR_CORRECTION, ATTR_DIMMED, 0);
    SetCtrlAttribute(com1, COM1_PICTUREBUTTON,    ATTR_DIMMED, 0);
    SetCtrlAttribute(com1, COM1_TEXTMSG,          ATTR_DIMMED, 0);
    SetCtrlAttribute(com1, COM1_TEXTMSG_2,        ATTR_DIMMED, 0);
    SetCtrlAttribute(com2, COM2_CANVAS,           ATTR_DIMMED, 0);
    SetCtrlAttribute(com1, COM1_SEND, ATTR_DIMMED, 0);
}

// help functions

/***********************************************************************************/

int StrtoBin(const char *str, int *binArray)
{
    int i, bit;
    int index = 0;

    int len = (int)strlen(str);

    for (i = 0; i < len; ++i)
    {
        unsigned char ch = (unsigned char)str[i];

        // Extract 8 bits (MSB to LSB)
        for (bit = 7; bit >= 0; --bit)
        {
            binArray[index++] = (ch >> bit) & 1;
        }
    }

    return index;
}

char* strdup(const char* src)
{
    if (src == NULL)
        return NULL;

    size_t len = strlen(src) + 1;  // +1 for null terminator
    char* dst = (char*)malloc(len);

    if (dst != NULL)
        memcpy(dst, src, len);

    return dst;
}

// w/r functions

/***********************************************************************************/

int read(int panel, int port ,int control) // reads from input buffer of port and displays to control - return number of bytes read (if failed returns -1)
{
	char inBuffer[BUFFER_SIZE] = {0};
	
	int QLen = GetInQLen(port);
	
	// if nothing in Queue - return 0
	if (!QLen)
		return 0;
	
	int bytesRead = ComRdTerm(port, inBuffer, QLen, 0);

    if (bytesRead > 0)
    {
        inBuffer[bytesRead] = '\0';

		CmtGetLock (readLockHandle); // lock
        SetCtrlVal(panel,  control, inBuffer);
		CmtReleaseLock (readLockHandle); // unlock
		
		return bytesRead;
    }

    return -1;
}

int write(int panel, int port ,int control) // writes to output buffer of port from control - return number of bytes written
{
	char outBuffer[BUFFER_SIZE] = {0}; // initillizing the string 
	
	CmtGetLock (writeLockHandle); // lock
	GetCtrlVal(panel, control, outBuffer);
	CmtReleaseLock (writeLockHandle); // unlock
	
	if (outBuffer[0] == 0) // string is empty
	return 0;
	
	strcat(outBuffer, "\n");
	
	return ComWrt(port, outBuffer, (int)strlen(outBuffer));
}

int writeChunks(int port, void* data, int totalSize)
{
	char buffer[Q_SIZE];
    int sent = 0, wrote;

    while (sent < totalSize)
    {
        int chunk = (totalSize - sent > Q_SIZE) ? Q_SIZE : totalSize - sent;
        memcpy(buffer, (char*)data + sent, chunk);

        wrote = ComWrt(port, buffer, chunk);
		if(!wrote) break;
		if (wrote < 0) return -1;
		
        sent += wrote;
    }

    return 0;
}

int readChunks(int port, void** dataPtr, int* bytesReceived, int totalSize)
{
    char  buffer[Q_SIZE];
    int   inQ      = GetInQLen(port);
    int   toRead;
    int   got;

    if (inQ <= 0)                    // nothing to read right now
        return 0;

    // only read as many as we still expect, capped at Q_SIZE:
    toRead = totalSize - *bytesReceived;
    if (toRead > Q_SIZE) 
        toRead = Q_SIZE;

    got = ComRd(port, buffer, toRead);
    if (got < 0) {
        MessagePopup("Error", "Failed to read from port");
        return -1;
    }
    if (got == 0)
        return 0;

    // grow (or malloc) the user buffer
    void *tmp = realloc(*dataPtr, *bytesReceived + got);
    if (!tmp) {
        MessagePopup("Error", "Memory reallocation failed");
        return -1;
    }
    *dataPtr = tmp;

    // append new bytes at the end
    memcpy((char*)(*dataPtr) + *bytesReceived, buffer, got);
    *bytesReceived += got;

    return got;
}

// callbacks

/***********************************************************************************/

int CVICALLBACK comThread(void *data) // thread function for polling communication
{
	comBox* box = (comBox*)data;
		
	write(box->panel, box->port, box->outControl);
	read(box->panel, box->port, box->inControl);

	return 0;
}

void CVICALLBACK readInterrupt (int portNumber, int eventMask, void *callbackData) // read interrupt for numeric communication 
{
  comBox* box = (comBox*)callbackData; 
  read(box->panel, portNumber, box->inControl);	
}

// image functions

/***********************************************************************************/

void displayImageOnPictureButton(const char* filename) // display image on picture button
{
        // Set the image as the picture button's bitmap
        SetCtrlAttribute(com1, COM1_PICTUREBUTTON, ATTR_IMAGE_FILE, filename);
        
        // Optional: Try to scale the image to fit the button (if supported)
        SetCtrlAttribute(com1, COM1_PICTUREBUTTON, ATTR_FIT_MODE, VAL_SIZE_TO_PICTURE);
}

int loadImageFile(imageBox* image) // retrurns 0 in success, -1 in failed 
{
    FILE* file = fopen(image->filename, "rb");
	
	int status = GetBitmapFromFile (image->filename, &image->bitmapID);
	
    if (!file || status != 0) {
        MessagePopup("Error", "Cannot open image file");
        return -1;
    }
	
	fclose(file);
	
    // Get image data
	GetBitmapInfo(image->bitmapID, &image->colorTableSize, &image->bitsSize, &image->maskSize);
	
	if (image->colorTableSize) image->colorTable = (int*)malloc(image->colorTableSize);
	if (image->bitsSize) image->bits = (unsigned char*)malloc(image->bitsSize);
	if (image->maskSize) image->mask = (unsigned char*)malloc(image->maskSize);
	
    GetBitmapData (image->bitmapID, &image->bytesPerRow, &image->pixelDepth, &image->width, &image->height, 
				   image->colorTable, image->bits, image->mask);
	

	 // Display image on the picture button
    displayImageOnPictureButton(image->filename);
	
    return 0;
}

void freeImageBox(imageBox* image)
{
    if (!image)
        return;

    if (image->colorTable) {
        free(image->colorTable);
        image->colorTable = NULL;
        image->colorTableSize = -1;
    }

    if (image->bits) {
        free(image->bits);
        image->bits = NULL;
        image->bitsSize = -1;
    }

    if (image->mask) {
        free(image->mask);
        image->mask = NULL;
        image->maskSize = -1;
    }

    // clear other fields
    image->bitmapID = -1;
    image->pixelDepth = -1;
    image->bytesPerRow = -1;
    image->width = -1;
    image->height = -1;
    image->filename[0] = '\0';  // clear filename string
}

void CVICALLBACK OnImageTransferDone (void *callbackData)
{
    imageBox *inImg  = &inImage;
    imageBox *outImg = &outImage;   // or cast callbackData if you passed it

	if(inImg->bitmapID == -1) return;
	
    int cW, cH;
	GetCtrlAttribute (com2, COM2_CANVAS, ATTR_WIDTH,  &cW);   /* canvas size (px) */
	GetCtrlAttribute (com2, COM2_CANVAS, ATTR_HEIGHT, &cH);

	double sx = (double)cW / inImage.width;     /* horizontal scale factor  */
	double sy = (double)cH / inImage.height;    /* vertical   scale factor  */
	double s  = (sx < sy) ? sx : sy;            /* fit inside, keep aspect  */

	int dW = (int)(inImage.width  * s + 0.5);   /* destination width  */
	int dH = (int)(inImage.height * s + 0.5);   /* destination height */
	int left = (cW - dW) / 2;                   /* centre inside canvas */
	int top  = (cH - dH) / 2;

	CanvasStartBatchDraw (com2, COM2_CANVAS);   /* avoids flicker        */
	CanvasClear        (com2, COM2_CANVAS, VAL_ENTIRE_OBJECT);
	CanvasDrawBitmap   (com2, COM2_CANVAS, inImage.bitmapID,
	                    VAL_ENTIRE_OBJECT,       /* source = full bitmap */
	                    MakeRect (top, left, dH, dW));
	CanvasEndBatchDraw (com2, COM2_CANVAS);

    freeImageBox(inImg);
    freeImageBox(outImg);
}

void CVICALLBACK receiveImage(int portNumber, int eventMask, void *callbackData) // Receive image via interrupt - creates bitmap when finished
{
	imageBox* image = (imageBox*)callbackData; 
	char buffer[Q_SIZE];
	static int colorCnt = 0, bitsCnt = 0, maskCnt = 0;
	
	int QLen = GetInQLen(portNumber);
	if(!QLen)
	{
		callbackData = (void*)image; 
		return;
	}
	
	if (image->bytesPerRow == -1)
	{
		ComRd(portNumber, buffer, sizeof(int));
		memcpy(&image->bytesPerRow, buffer, sizeof(int)); 
		QLen = GetInQLen(portNumber);
		if(!QLen)
		{
			callbackData = (void*)image; 
			return;
		}
	}
	
	if (image->pixelDepth == -1)
	{
		ComRd(portNumber, buffer, sizeof(int));
		memcpy(&image->pixelDepth, buffer, sizeof(int));
		QLen = GetInQLen(portNumber);
		if(!QLen)
		{
			callbackData = (void*)image; 
			return;
		}
	}
	
	if (image->width == -1)
	{
		ComRd(portNumber, buffer, sizeof(int));
		memcpy(&image->width, buffer, sizeof(int)); 
		QLen = GetInQLen(portNumber);
		if(!QLen)
		{
			callbackData = (void*)image; 
			return;
		}
	}
	
	if (image->height == -1)
	{
		ComRd(portNumber, buffer, sizeof(int));
		memcpy(&image->height, buffer, sizeof(int));
		QLen = GetInQLen(portNumber);
		if(!QLen)
		{
			callbackData = (void*)image; 
			return;
		}
	}

	if (image->colorTableSize == -1)
	{
		ComRd(portNumber, buffer, sizeof(int));
		memcpy(&image->colorTableSize, buffer, sizeof(int));
		QLen = GetInQLen(portNumber);
		if(!QLen)
		{
			callbackData = (void*)image; 
			return;
		}
	}
	
	if (image->bitsSize == -1)
	{
		ComRd(portNumber, buffer, sizeof(int));
		memcpy(&image->bitsSize, buffer, sizeof(int)); 
		QLen = GetInQLen(portNumber);
		if(!QLen)
		{
			callbackData = (void*)image; 
			return;
		}
	}
	
	if (image->maskSize == -1)
	{
		ComRd(portNumber, buffer, sizeof(int));
		memcpy(&image->maskSize, buffer, sizeof(int));
		QLen = GetInQLen(portNumber);
		if(!QLen)
		{
			callbackData = (void*)image; 
			return;
		}
	}
	
	while (colorCnt < image->colorTableSize) 
	{
    int n = readChunks(portNumber, (void**)&image->colorTable, &colorCnt, image->colorTableSize);
    if (n < 0)  return;  // error
    if (n == 0) return;  // queue emptied
	}
	
	while (bitsCnt < image->bitsSize)
	{
		int n = readChunks(portNumber, (void**)&image->bits, &bitsCnt ,image->bitsSize);
		if (n < 0)  return;  // error
    	if (n == 0) return;  // queue emptied
	}
	
	while (maskCnt < image->maskSize)
	{
	   int n = readChunks(portNumber, (void**)&image->mask, &maskCnt, image->maskSize);
	   if (n < 0)  return;  // error
       if (n == 0) return;  // queue emptied
	}
	
	if (NewBitmap (image->bytesPerRow, image->pixelDepth, image->width, image->height, 
			   image->colorTable, image->bits, image->mask, &image->bitmapID) < 0)
	{
		MessagePopup("Error", "bitmap creation failed");
		return;
	}
							
	callbackData = (void*)image;
	
	// reset counters for the next image
    colorCnt = bitsCnt = maskCnt = 0;
	
	InstallComCallback (COM_PORT_2, 0, 0, 0, 0, 0); //  Uninstall ComCallback
	// tell CVI to run OnImageTransferDone() inside UI thread once we're done
    PostDeferredCall (OnImageTransferDone, NULL);
	
	return;
}

int sendImage(imageBox* image) // retrurns 0 in success, -1 in failed
{
	int bytesWrote = 0;
	
    // Writing to COM1
    bytesWrote = ComWrt(COM_PORT_1, (char*)&image->bytesPerRow, sizeof(int));
    if (bytesWrote <= 0) return -1;

    bytesWrote = ComWrt(COM_PORT_1, (char*)&image->pixelDepth, sizeof(int));
    if (bytesWrote <= 0) return -1;

    bytesWrote = ComWrt(COM_PORT_1, (char*)&image->width, sizeof(int));
    if (bytesWrote <= 0) return -1;

    bytesWrote = ComWrt(COM_PORT_1, (char*)&image->height, sizeof(int));
    if (bytesWrote <= 0) return -1;
	
    bytesWrote = ComWrt(COM_PORT_1, (char*)&image->colorTableSize, sizeof(int));
    if (bytesWrote <= 0) return -1;

    bytesWrote = ComWrt(COM_PORT_1, (char*)&image->bitsSize, sizeof(int));
    if (bytesWrote <= 0) return -1;

    bytesWrote = ComWrt(COM_PORT_1, (char*)&image->maskSize, sizeof(int));
    if (bytesWrote <= 0) return -1;
	
    // Send arrays with chunking
	if (writeChunks(COM_PORT_1, image->colorTable, image->colorTableSize) < 0) return -1;
	if (writeChunks(COM_PORT_1, image->bits, image->bitsSize) < 0) return -1;
	if (writeChunks(COM_PORT_1, image->mask, image->maskSize) < 0) return -1;
    
    return 0;
}

int CVICALLBACK SendImageAsync (void *data)
{
    imageBox *img = (imageBox*)data;

    // writeChunks *without* ProcessSystemEvents
    int status = sendImage(img);

    return status;
}

void resetImage(imageBox *img)
{
	img->bitmapID       = -1;
    img->bytesPerRow    = -1;
    img->pixelDepth     = -1;
    img->width          = -1;
    img->height         = -1;
    img->colorTableSize = -1;
    img->bitsSize       = -1;
    img->maskSize       = -1;
    img->colorTable =   NULL; 
    img->bits =         NULL; 
    img->mask =         NULL;
}


#endif // __DEF_H