/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
/*  This program is protected by German copyright law and international      */
/*  treaties. The use of this software including but not limited to its      */
/*  Source Code is subject to restrictions as agreed in the license          */
/*  agreement between you and Siemens.                                       */
/*  Copying or distribution is not allowed unless expressly permitted        */
/*  according to your license agreement with Siemens.                        */
/*****************************************************************************/
/*                                                                           */
/*  P r o j e c t         &P: PROFINET IO Runtime Software              :P&  */
/*                                                                           */
/*  P a c k a g e         &W: PROFINET IO Runtime Software              :W&  */
/*                                                                           */
/*  C o m p o n e n t     &C: PnIODDevkits                              :C&  */
/*                                                                           */
/*  F i l e               &F: iodapi_event.h                            :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  declaration of the user defined callback functions                       */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  H i s t o r y :                                                          */
/*                                                                           */
/*  Date        Version        Who  What                                     */
/*                                                                           */
/*****************************************************************************/
#ifndef _IODAPIEVENT_H
#define _IODAPIEVENT_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

    // *---------------------------------------------------------
    // *	AR management
    // *---------------------------------------------------------
    typedef struct 
    {
        PNIO_UINT16 ArNum;
        PNIO_UINT16 SessionKey;
        PNIO_UINT16 IocrProperties;  // specifies RT class 1, 2, 3 (all iocr's of an AR have the same value)
    } AR_PROP;


    //--------------------------------------------------------------------------------------
    // READ RECORD, WRITE RECORD:
    //--------------------------------------------------------------------------------------

    typedef struct
    {
	    PNIO_UINT32			DevHndl;        // device handle
	    PNIO_UINT32			Api;            // application process identifier
        PNIO_UINT16         ArNum;			// ar - handle
        PNIO_UINT16 		SessionKey;	    // ar session number
	    PNIO_UINT32			SequenceNum;    // CLRPC sequence number
	    PNIO_DEV_ADDR       Addr;		    // location (module/submodule)
        PNIO_UINT32			RecordIndex;    // record index
        PNIO_UINT32			BufLen;		    // record data length
        PNIO_VOID*          pRqHnd;         // Requestblock pointer, used as Handle 
    }  USR_REC_READ_RQ;

    typedef struct
    {
	    PNIO_UINT32			DevHndl;        // device handle
	    PNIO_UINT32			Api;            // application process identifier
        PNIO_UINT16         ArNum;			// ar - handle
        PNIO_UINT16 		SessionKey;	    // ar session number
	    PNIO_UINT32			SequenceNum;    // CLRPC sequence number
	    PNIO_DEV_ADDR       Addr;		    // location (module/submodule)
        PNIO_UINT32			RecordIndex;    // record index
        PNIO_UINT32			BufLen;		    // record data length	
	    PNIO_UINT8			*pBuffer;		// record data buffer pointer
        PNIO_VOID*          pRqHnd;         // Requestblock pointer, used as Handle 
    }  USR_REC_WRITE_RQ;

    PNIO_EXTRN PNIO_UINT32  usr_rec_read_response_async (void);
    PNIO_EXTRN PNIO_UINT32  usr_rec_write_response_async (void);



    //--------------------------------------------------------------------------------------
    // iPAR server example 
    //--------------------------------------------------------------------------------------
    // ****** example for special features (must be modified by user)
    #define EXAMPLE_IPAR_SERVER     1       // 1: support iPar - example data on record index 50, 0: else


    // ***** EXAMPLE IPAR:  upload&storage alarm ********
    #define EXAMPL_IPAR_SLOTNUM             1                           // slot number
    #define EXAMPL_IPAR_SUBSLOTNUM          1                           // subslot number, must be 1
    #define EXAMPL_IPAR_REC_IND             50                          // free defined by the user
    #define EXAMPL_IPAR_UPL_LEN             4                           // length of ipar data for upload
    #define EXAMPL_IPAR_RTV_LEN             4                           // length of ipar data for retrieval (0: no limit)


    //--------------------------------------------------------------------------------------
    // structure of special records 
    //--------------------------------------------------------------------------------------
    // *--------------------------------------
    // * record 0x8030, see IEC61158-6  "IsochronousModeData"
    // *--------------------------------------
    typedef  ATTR_PNIO_PACKED_PRE struct
    {
        REC_IO_BLOCKHDR     BlockHeader;
        PNIO_UINT8          Padding[2];
        PNIO_UINT16         SlotNumber;
        PNIO_UINT16         SubSlotNumber;
        PNIO_UINT16         ControllerApplicationCycleFactor;
        PNIO_UINT16         TimeDataCycle;
        PNIO_UINT32         TimeIOInput;         // value in nsec
        PNIO_UINT32         TimeIOOutput;        // value in nsec
        PNIO_UINT32         TimeIOInputValid;    // value in nsec
        PNIO_UINT32         TimeIOOutputValid;   // value in nsec
    }  ATTR_PNIO_PACKED REC8030_ISOMDATA;  


    //--------------------------------------------------------------------------------------
    // SPI1 RLSPI
    //--------------------------------------------------------------------------------------
	//e  
    //---PN原始报文c
    typedef struct
    {//PN报文结构c
	    PNIO_UINT8			buffer[16];
	    struct{
	    	PNIO_UINT8 		header;
	    	PNIO_UINT8		cmd;
	    	PNIO_UINT16		data[6];
	    	PNIO_UINT16 	crc;
	    }msg;			
	    

    }  TYPED_PN_MSG;	
    	
	
	//---PNM参数c
    typedef struct
    {//PNM参数结构c
	    PNIO_UINT16		FunCode[12];	//功能码c
	    PNIO_UINT16		Value[12];		//参数值c
	    

    }  TYPED_PNM_MSG;
    	
	
	//---状态结构c
	
//===1
//    #define		M_PNM_STEP_NONE		0			
//    #define		M_PNM_STEP_HAND		1
//    #define		M_PNM_STEP_BUILD	2
//    #define		M_PNM_STEP_FIRST	3
//    #define		M_PNM_STEP_RUN		4
//    #define		M_PNM_STEP_ERROR	5
//===2
	typedef enum
	{
    	M_PNM_STEP_NONE		=0,			
    	M_PNM_STEP_HAND		=1,
    	M_PNM_STEP_BUILD5	=2,
    	M_PNM_STEP_BUILD6	=3,
    	M_PNM_STEP_BUILD7	=4,
    	M_PNM_STEP_FIRST	=5,
    	M_PNM_STEP_RUN		=6,
    	M_PNM_STEP_ERROR	=7		
	}TYP_STEP;
	
	
	
//===e	
	
    typedef struct
    {
//===1
//	    PNIO_UINT8			PNM_Step;			//ttt 初始化清零C
//
//===2
	    TYP_STEP			PNM_Step;			//ttt 初始化清零C


//===E
	    PNIO_UINT8			PNIO_Num;			//PN数据配置长度,2,4,6,8,10,12,c	//ttt 初始化清零,断开清零c
	    PNIO_UINT8			SPI_WrGroupNum;		//SPI数据对齐条数,1,2,3,4,//0-3,1-4 	//ttt 初始化清零,断开清零c
	    PNIO_UINT8			SPI_RdGroupNum;		//SPI数据对齐条数c	//ttt 初始化清零,断开清零c
	    


    }  TYPED_RLSPI_STATUS;	
	
	//---
    typedef struct
    {
	    union
	    {
	    	PNIO_UINT16		buf16[8];
	    	PNIO_UINT8		buf8[16];
	    	struct{
	    		PNIO_UINT8 		header;
	    		PNIO_UINT8		cmd;
	    		PNIO_UINT16		data[6];
	    		PNIO_UINT16 	crc;
	    	}msg;	    	
	    }SPI_Rx_buf;							//1 SPI缓存,Rx为CRC前c
	    union
	    {
	    	PNIO_UINT16		buf16[8];
	    	PNIO_UINT8		buf8[16];
	    	struct{
	    		PNIO_UINT8 		header;
	    		PNIO_UINT8		cmd;
	    		PNIO_UINT16		data[6];
	    		PNIO_UINT16 	crc;
	    	}msg;	    	
	    }SPI_Tx_buf;    
	    
	    
	    TYPED_PN_MSG		PN_RxMsg;			//接收报文,c
	    TYPED_PN_MSG		PN_TxMsg;
	    
	    TYPED_PNM_MSG		PNM_Write_Liset;	//1 写参数原始结构c	//ttt 初始化清零,断开清零c
	    TYPED_PNM_MSG		PNM_Read_Liset;
	    PNIO_UINT8			PNM_Wr_Index[12];	//1 Fun new Index,*3
	    PNIO_UINT8			PNM_Rd_Index[12];
	    
	    
	    TYPED_RLSPI_STATUS	PNM_Status;			//数据长度,步骤c



    }  TYPED_RLSPI;	

	

//e

    typedef struct
    {
	    PNIO_UINT8			SPI_WrGroupCount;		//Wr计数,0,1,2,3c	//ttt 初始化清零,断开清零c
	    PNIO_UINT8			SPI_RdGroupCount;




    }  TYPED_RLUSR;	






#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
