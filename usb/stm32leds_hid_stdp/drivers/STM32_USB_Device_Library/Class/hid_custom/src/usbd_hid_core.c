/**
  ******************************************************************************
  * @file    usbd_hid_core.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   This file provides the HID core functions.
  *
  * @verbatim
  *      
  *          ===================================================================      
  *                                HID Class  Description
  *          =================================================================== 
  *           This module manages the HID class V1.11 following the "Device Class Definition
  *           for Human Interface Devices (HID) Version 1.11 Jun 27, 2001".
  *           This driver implements the following aspects of the specification:
  *             - The Boot Interface Subclass
  *             - The Mouse protocol
  *             - Usage Page : Generic Desktop
  *             - Usage : Joystick)
  *             - Collection : Application 
  *      
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *           
  *      
  *  @endverbatim
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "usbd_hid_core.h"
#include "usbd_desc.h"
#include "usbd_req.h"

// The hid core from ST is not flexible
#ifdef HID_REPORT_DESC_SIZE
#undef HID_REPORT_DESC_SIZE
#endif
#define HID_REPORT_DESC_SIZE 84 //23  31

#ifdef USB_HID_CONFIG_DESC_SIZ
#undef USB_HID_CONFIG_DESC_SIZ
#endif
#define USB_HID_CONFIG_DESC_SIZ 41

extern uint8_t OutBuffer[];
extern uint8_t HOSTD_OK;


static uint8_t  USBD_HID_Init (void  *pdev, 
                               uint8_t cfgidx);

static uint8_t  USBD_HID_DeInit (void  *pdev, 
                                 uint8_t cfgidx);

static uint8_t  USBD_HID_Setup (void  *pdev, 
                                USB_SETUP_REQ *req);

static uint8_t  *USBD_HID_GetCfgDesc (uint8_t speed, uint16_t *length);

static uint8_t  USBD_HID_DataIn (void  *pdev, uint8_t epnum);

static uint8_t  USBD_HID_DataOut (void  *pdev, uint8_t epnum);
/**
  * @}
  */ 

/** @defgroup USBD_HID_Private_Variables
  * @{
  */ 

USBD_Class_cb_TypeDef  USBD_HID_cb = 
{
  USBD_HID_Init,
  USBD_HID_DeInit,
  USBD_HID_Setup,
  NULL, /*EP0_TxSent*/  
  NULL, /*EP0_RxReady*/
  USBD_HID_DataIn, /*DataIn*/
  USBD_HID_DataOut, /*DataOut*/
  NULL, /*SOF */
  NULL,
  NULL,      
  USBD_HID_GetCfgDesc,
#ifdef USB_OTG_HS_CORE  
  USBD_HID_GetCfgDesc, /* use same config as per FS */
#endif  
};

static uint32_t  USBD_HID_AltSet  = 0;


static uint32_t  USBD_HID_Protocol = 0;


static uint32_t  USBD_HID_IdleState  = 0;



/* USB HID device Configuration Descriptor */
/***************  Дескриптор конфигурации ********************/
static uint8_t USBD_HID_CfgDesc[USB_HID_CONFIG_DESC_SIZ] =
{
  0x09,                              /* bLength Size of descriptor in bytes : =9 */
  USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType:  Configuration 0x02 */
  USB_HID_CONFIG_DESC_SIZ, /*wTotalLength:Total length of this configuration in bytes=41*/
  0x00,
  0x01,     /*bNumInterfaces: Number of interfaces in configuration =1 */
  0x01,     /*bConfigurationValue: This value is used for choosing this configuration */
  0x00,     /*iConfiguration: Index of a string descriptor of this configuration = 0 */
  0xE0,     /*bmAttributes: bus powered and Support Remote Wake-up */
  0x32,     /*MaxPower 100 mA: maximum consumption in mA (*2)  50*2 */

  /***************  Дескриптор интерфейса *******************************/
  /************** Descriptor  interface ****************/
  /* 09 */
  0x09,         /*bLength: Size of descriptor in bytes = 9 */
  USB_INTERFACE_DESCRIPTOR_TYPE,/*bDescriptorType: Interface descriptor type=0x04 */
  0x00,      /*bInterfaceNumber: Number of Interface*/
  0x00,      /*bAlternateSetting: Alternate setting*/
  0x02,      /*bNumEndpoints Number of end points in interface=2*/
  0x03,      /*bInterfaceClass: HID class device */
  0x00,      /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0x00,      /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  0,         /*iInterface: Index of string descriptor */

  /********************* HID-descriptor *************************************/

  /* 18 */
  0x09,         /*bLength: HID Descriptor size=9*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: type of descriptor - HID 0x21*/
  0x00,         /*bcdHID: HID class spec release HID 2.0*/
  0x02,
  0x00,         /*bCountryCode: Hardware target country=0*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow=1*/
  0x22,         /*bDescriptorType Тип дескриптора - report=22*/
  46,	// shortened length - 2 report id's
  // HID_REPORT_DESC_SIZE,     /*wItemLength: report-descriptor length = 84*/ //+++
  0x00,

 /******************** End point descriptors ************************/
  /******************** Descriptor endpoint ********************/
  /* 27 */
  0x07,           /*bLength: descriptor length in bytes =7 */
  USB_ENDPOINT_DESCRIPTOR_TYPE, /*bDescriptorType: endpoint descriptor (0x05)*/

  HID_IN_EP,    /*bEndpointAddress: 0x81=1000 0001
				  bits 0..3 defines # of of end point =1
				  bits 4..6 reserved and set to 0 (IN)
				  bit 7 defines a direction of end point 0 = Out, 1 = In  */
  0x03,         /*bmAttributes: bits 0..1 is a type of end point
                 00 = Control
                 01 = Isochronous
                 10 = Bulk
                 11 = Interrupt = Interrupt endpoint  */
  HID_IN_PACKET, /*wMaxPacketSize: Packet size of this end point =63*/ //+++
  0x00,
  0x01,          /*bInterval: Interval of polling for this endpoint.(1 ms)*/

  /******************** Descriptor endpoint ************************/

  /* 34 */
  0x07,          /*bLength: Length of endpoint in bytes =7 */
  USB_ENDPOINT_DESCRIPTOR_TYPE, /*bDescriptorType: endpoint descriptor (0x05)*/
  HID_OUT_EP,    /*bEndpointAddress:  0x01
                                    # of endpoint =1
                                    bit 7 is 0, endpoint direction = Out,*/

  0x03,            /*bmAttributes: Interrupt endpoint type*/
  HID_OUT_PACKET,  /*wMaxPacketSize:  Max packet size for this endpoint=63 */
  0x00,
  0x01,            /*bInterval: Polling interval for this endpoint. (1 ms)*/
  /* 41 */
} ;


/**************************ReportDesc*******************************/
//HID_REPORT_DESC_SIZE=23
static uint8_t HID_ReportDesc[HID_REPORT_DESC_SIZE]  =
{

		    0x06, 0x00, 0xff,              // USAGE_PAGE Vendor Page
		    0x09, 0x01,                    // USAGE (Vendor Usage 1)
		    0xa1, 0x01,                    // COLLECTION (Application)
//7
		    0x85, 0x01,                    //   REPORT_ID (1)
		    0x09, 0x01,                    //   USAGE (Vendor Usage 1)
		    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
		    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
		    0x75, 0x08,                    //   REPORT_SIZE (8)
		    0x95, 10,                      //   REPORT_COUNT (10)
		    0x85, 0x01,                    //   REPORT_ID (1)
		    0x09, 0x01,                    //   USAGE (Vendor Usage 1)
		    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
//26
		    0x85, 0x02,                    //   REPORT_ID (2)
		    0x09, 0x02,                    //   USAGE (Vendor Usage 2)
		    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
		    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
		    0x75, 0x08,                    //   REPORT_SIZE (8)
		    0x95, 10,                      //   REPORT_COUNT (10)
		    0x85, 0x02,                    //   REPORT_ID (2)
		    0x09, 0x02,                    //   USAGE (Vendor Usage 2)
		    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
// //45
//		    0x85, 0x03,                    //   REPORT_ID (3)
//		    0x09, 0x03,                    //   USAGE (Vendor Usage 3)
//		    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
//		    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
//		    0x75, 0x08,                    //   REPORT_SIZE (8)
//		    0x95, 63,                      //   REPORT_COUNT (63)
//		    0x85, 0x03,                    //   REPORT_ID (3)
//		    0x09, 0x03,                    //   USAGE (Vendor Usage 3)
//		    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
////64
//		    0x85, 0x04,                    //   REPORT_ID (4)
//		    0x09, 0x04,                    //   USAGE (Vendor Usage 4)
//		    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
//		    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
//		    0x75, 0x08,                    //   REPORT_SIZE (8)
//		    0x95, 63,                      //   REPORT_COUNT (63)
//		    0x85, 0x04,                    //   REPORT_ID (4)
//		    0x09, 0x04,                    //   USAGE (Vendor Usage 4)
//		    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
		    0xc0                           // END_COLLECTION

               // END_COLLECTION 84
};

/**
  * @brief  USBD_HID_Init
  *         Initialize the HID interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */

static uint8_t usb_rx_buffer[HID_OUT_PACKET];
static uint8_t  USBD_HID_Init (void  *pdev, 
                               uint8_t cfgidx)
{
  
  /* Open EP IN */
  DCD_EP_Open(pdev,
              HID_IN_EP,
              HID_IN_PACKET,
              USB_OTG_EP_INT);
  
  /* Open EP OUT */
  DCD_EP_Open(pdev,
              HID_OUT_EP,
              HID_OUT_PACKET,
              USB_OTG_EP_INT);
  DCD_EP_PrepareRx(pdev, HID_OUT_EP, usb_rx_buffer, HID_OUT_PACKET);
  
  return USBD_OK;
}

/**
  * @brief  USBD_HID_Init
  *         DeInitialize the HID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_HID_DeInit (void  *pdev, 
                                 uint8_t cfgidx)
{
  /* Close HID EPs */
  DCD_EP_Close (pdev , HID_IN_EP);
  DCD_EP_Close (pdev , HID_OUT_EP);
  
  
  return USBD_OK;
}

/**
  * @brief  USBD_HID_Setup
  *         Handle the HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_HID_Setup (void  *pdev, 
                                USB_SETUP_REQ *req)
{
  uint16_t len = 0;
  uint8_t  *pbuf = NULL;
  
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :  
    switch (req->bRequest)
    {
      
      
    case HID_REQ_SET_PROTOCOL:
      USBD_HID_Protocol = (uint8_t)(req->wValue);
      break;
      
    case HID_REQ_GET_PROTOCOL:
      USBD_CtlSendData (pdev, 
                        (uint8_t *)&USBD_HID_Protocol,
                        1);    
      break;
      
    case HID_REQ_SET_IDLE:
      USBD_HID_IdleState = (uint8_t)(req->wValue >> 8);
      break;
      
    case HID_REQ_GET_IDLE:
      USBD_CtlSendData (pdev, 
                        (uint8_t *)&USBD_HID_IdleState,
                        1);        
      break;

    default:
      USBD_CtlError (pdev, req);
      return USBD_FAIL; 
    }
    break;
    
  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR: 
      if( req->wValue >> 8 == HID_REPORT_DESC)
      {
        len = MIN(HID_REPORT_DESC_SIZE , req->wLength);
        pbuf = HID_ReportDesc;
      }
      else if( req->wValue >> 8 == HID_DESCRIPTOR_TYPE)
      {
        
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
        pbuf = USBD_HID_Desc;
#else
        pbuf = USBD_HID_CfgDesc + 0x12;
#endif
        len = MIN(USB_HID_DESC_SIZ , req->wLength);
      }
      
      USBD_CtlSendData (pdev, 
                        pbuf,
                        len);
      
      break;
      
    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev,
                        (uint8_t *)&USBD_HID_AltSet,
                        1);
      break;
      
    case USB_REQ_SET_INTERFACE :
      USBD_HID_AltSet = (uint8_t)(req->wValue);
      break;
    }
  }
  return USBD_OK;
}

/**
  * @brief  USBD_HID_SendReport 
  *         Send HID Report
  * @param  pdev: device instance
  * @param  buff: pointer to report
  * @retval status
  */
uint8_t USBD_HID_SendReport     (USB_OTG_CORE_HANDLE  *pdev, 
                                 uint8_t *report,
                                 uint16_t len)
{
  if (pdev->dev.device_status == USB_OTG_CONFIGURED )
  {
    DCD_EP_Tx (pdev, HID_IN_EP, report, len);
  }
  return USBD_OK;
}

/**
  * @brief  USBD_HID_GetCfgDesc 
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_HID_GetCfgDesc (uint8_t speed, uint16_t *length)
{
  *length = sizeof (USBD_HID_CfgDesc);
  return USBD_HID_CfgDesc;
}

/**
  * @brief  USBD_HID_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_HID_DataIn (void  *pdev,
                              uint8_t epnum)
{

  /* Ensure that the FIFO is empty before a new transfer, this condition could 
  be caused by  a new transfer before the end of the previous transfer */
  DCD_EP_Flush(pdev, HID_IN_EP);
  return USBD_OK;
}

/**
  * @brief  USBD_HID_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */

static uint8_t  USBD_HID_DataOut (void  *pdev,
                              uint8_t epnum)
{

  /* Ensure that the FIFO is empty before a new transfer, this condition could
  be caused by  a new transfer before the end of the previous transfer */
  int8_t i=0;
  while (i<11)
  {
	  OutBuffer[i]=usb_rx_buffer[i];
	  i++;
  }
  DCD_EP_PrepareRx(pdev, HID_OUT_EP, usb_rx_buffer, HID_OUT_PACKET);

  return USBD_OK;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

