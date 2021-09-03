/******************************************************************************
  * \attention
  *
  * <h2><center>&copy; COPYRIGHT 2021 STMicroelectronics</center></h2>
  *
  * Licensed under ST MIX MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        www.st.com/mix_myliberty
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
  * AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
******************************************************************************/

/*! \file
 *
 *  \author SRA
 *
 *  \brief Provides NDEF methods and definitions to access NFC Forum Tags
 *
 *  NDEF provides several functionalities required to perform NFC NDEF activities.
 *  <br>The NDEF encapsulates the different tag technologies (T2T, T3T, T4AT, T4BT, T5T)
 *  into a common and easy to use interface.
 *
 *  It provides interfaces to Detect, Read, Write and Format NDEF.
 *
 *  The most common interfaces are:
 *    <br>&nbsp; ndefPollerContextInitialization()
 *    <br>&nbsp; ndefPollerNdefDetect()
 *    <br>&nbsp; ndefPollerReadRawMessage()
 *    <br>&nbsp; ndefPollerWriteRawMessage()
 *    <br>&nbsp; ndefPollerTagFormat()
 *    <br>&nbsp; ndefPollerWriteMessage()
 *
 *
 *  An NDEF read usage example is provided here: \ref ndef_example_read.c
 *  \example ndef_example_read.c
 *
 *  An NDEF write usage example is provided here: \ref ndef_example_write.c
 *  \example ndef_example_write.c
 *
 * \addtogroup NDEF
 * @{
 *
 */


#ifndef NDEF_POLLER_H
#define NDEF_POLLER_H

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "st_errno.h"
#include "rfal_nfca.h"
#include "rfal_nfcb.h"
#include "rfal_nfcf.h"
#include "rfal_nfcv.h"
#include "rfal_nfc.h"
#include "rfal_isoDep.h"
#include "rfal_t4t.h"
#include "ndef_record.h"
#include "ndef_message.h"


/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */

#define NDEF_CC_BUF_LEN             17U                                                /*!< CC buffer len. Max len = 17 in case of T4T v3                */
#define NDEF_NFCV_SUPPORTED_CMD_LEN  4U                                                /*!< Ext sys info supported commands list len                     */

#define NDEF_SHORT_VFIELD_MAX_LEN  254U                                                /*!< Max V-field length for 1-byte Length encoding                 */
#define NDEF_TERMINATOR_TLV_LEN      1U                                                /*!< Terminator TLV size                                          */
#define NDEF_TERMINATOR_TLV_T     0xFEU                                                /*!< Terminator TLV T=FEh                                         */

#define NDEF_T2T_READ_RESP_SIZE     16U                                                /*!< Size of the READ response i.e. four blocks                   */

#define NDEF_T3T_BLOCK_SIZE         16U                                                /*!< size for a block in t3t                                      */
#define NDEF_T3T_MAX_NB_BLOCKS       4U                                                /*!< size for a block in t3t                                      */
#define NDEF_T3T_MAX_RX_SIZE      ((NDEF_T3T_BLOCK_SIZE*NDEF_T3T_MAX_NB_BLOCKS) + 16U) /*!< size for receive 4 blocks of 16 + UID + HEADER + CHECKSUM    */
#define NDEF_T3T_MAX_TX_SIZE      ((NDEF_T3T_BLOCK_SIZE*NDEF_T3T_MAX_NB_BLOCKS) + 16U) /*!< size for send Check 4 blocks of 16 + UID + HEADER + CHECKSUM */

#define NDEF_T5T_TxRx_BUFF_HEADER_SIZE        1U                                       /*!< Request Flags/Responses Flags size                           */
#define NDEF_T5T_TxRx_BUFF_FOOTER_SIZE        2U                                       /*!< CRC size                                                     */

#define NDEF_T5T_TxRx_BUFF_SIZE               \
          (32U +  NDEF_T5T_TxRx_BUFF_HEADER_SIZE + NDEF_T5T_TxRx_BUFF_FOOTER_SIZE)     /*!< T5T working buffer size                                      */

/*
 ******************************************************************************
 * GLOBAL MACROS
 ******************************************************************************
 */

#define ndefBytes2Uint16(hiB, loB)          ((uint16_t)((((uint32_t)(hiB)) << 8U) | ((uint32_t)(loB))))                                                  /*!< convert 2 bytes to a u16 */

#define ndefMajorVersion(V)                 ((uint8_t)((V) >>  4U))    /*!< Get major version */
#define ndefMinorVersion(V)                 ((uint8_t)((V) & 0xFU))    /*!< Get minor version */

/*
 ******************************************************************************
 * NDEF FEATURES CONFIGURATION
 ******************************************************************************
 */


/*
 ******************************************************************************
 * GLOBAL TYPES
 ******************************************************************************
 */

/*! NDEF device type */
typedef enum {
  NDEF_DEV_NONE          = 0x00U,                            /*!< Device type undef                                  */
  NDEF_DEV_T1T           = 0x01U,                            /*!< Device type T1T                                    */
  NDEF_DEV_T2T           = 0x02U,                            /*!< Device type T2T                                    */
  NDEF_DEV_T3T           = 0x03U,                            /*!< Device type T3T                                    */
  NDEF_DEV_T4T           = 0x04U,                            /*!< Device type T4AT                                   */
  NDEF_DEV_T5T           = 0x05U,                            /*!< Device type T5T                                    */
} ndefDeviceType;

/*! NDEF states  */
typedef enum {
  NDEF_STATE_INVALID     = 0x00U,                            /*!< Invalid state (e.g. no CC)                         */
  NDEF_STATE_INITIALIZED = 0x01U,                            /*!< State Initialized (no NDEF message)                */
  NDEF_STATE_READWRITE   = 0x02U,                            /*!< Valid NDEF found. Read/Write capability            */
  NDEF_STATE_READONLY    = 0x03U,                            /*!< Valid NDEF found. Read only                        */
} ndefState;

/*! NDEF Information */
typedef struct {
  uint8_t                  majorVersion;                     /*!< Major version                                      */
  uint8_t                  minorVersion;                     /*!< Minor version                                      */
  uint32_t                 areaLen;                          /*!< Area Len for NDEF storage                          */
  uint32_t                 areaAvalableSpaceLen;             /*!< Remaining Space in case a propTLV is present       */
  uint32_t                 messageLen;                       /*!< NDEF message Len                                   */
  ndefState                state;                            /*!< Tag state e.g. NDEF_STATE_INITIALIZED              */
} ndefInfo;

/*! NFCV (Extended) System Information  */
typedef struct {
  uint16_t                 numberOfBlock;                    /*!< Number of block                                    */
  uint8_t                  UID[RFAL_NFCV_UID_LEN];           /*!< UID Value                                          */
  uint8_t                  supportedCmd[NDEF_NFCV_SUPPORTED_CMD_LEN];/*!< Supported Commands list                    */
  uint8_t                  infoFlags;                        /*!< Information flags                                  */
  uint8_t                  DFSID;                            /*!< DFSID Value                                        */
  uint8_t                  AFI;                              /*!< AFI Value                                          */
  uint8_t                  blockSize;                        /*!< Block Size Value                                   */
  uint8_t                  ICRef;                            /*!< IC Reference                                       */
} ndefSystemInformation;

/*! T1T Capability Container  */
typedef struct {
  uint8_t                  magicNumber;                      /*!< Magic number e.g. E1h                              */
  uint8_t                  majorVersion;                     /*!< Major version i.e 1                                */
  uint8_t                  minorVersion;                     /*!< Minor version i.e 2                                */
  uint16_t                 tagMemorySize;                    /*!< Tag Memory Size (TMS)                              */
  uint8_t                  readAccess;                       /*!< NDEF READ access condition                         */
  uint8_t                  writeAccess;                      /*!< NDEF WRITE access condition                        */
} ndefCapabilityContainerT1T;

/*! T2T Capability Container  */
typedef struct {
  uint8_t                  magicNumber;                      /*!< Magic number e.g. E1h                              */
  uint8_t                  majorVersion;                     /*!< Major version i.e 1                                */
  uint8_t                  minorVersion;                     /*!< Minor version i.e 2                                */
  uint8_t                  size;                             /*!< Size. T2T_Area_Size = Size * 8                     */
  uint8_t                  readAccess;                       /*!< NDEF READ access condition                         */
  uint8_t                  writeAccess;                      /*!< NDEF WRITE access condition                        */
} ndefCapabilityContainerT2T;

/*! T3T Attribute info block  */
typedef struct {
  uint8_t                  majorVersion;                     /*!< Major version i.e 1                                */
  uint8_t                  minorVersion;                     /*!< Minor version i.e 2                                */
  uint8_t                  nbR;                              /*!< Nbr: number of blocks read in one CHECK cmd        */
  uint8_t                  nbW;                              /*!< Nbr: number of blocks written in one UPDATE cmd    */
  uint16_t                 nMaxB;                            /*!< NmaxB: max number of blocks for NDEF data          */
  uint8_t                  writeFlag;                        /*!< WriteFlag indicates completion of previous NDEF    */
  uint8_t                  rwFlag;                           /*!< RWFlag indicates whether the NDEF can be updated   */
  uint32_t                 Ln;                               /*!< Ln size of the actual stored NDEF data in bytes    */
} ndefAttribInfoBlockT3T;

/*! T4T Capability Container  */
typedef struct {
  uint16_t                 ccLen;                            /*!< CCFILE Len                                         */
  uint8_t                  vNo;                              /*!< Mapping version                                    */
  uint16_t                 mLe;                              /*!< Max data size that can be read using a ReadBinary  */
  uint16_t                 mLc;                              /*!< Max data size that can be sent using a single cmd  */
  uint8_t                  fileId[2];                        /*!< NDEF File Identifier                               */
  uint32_t                 fileSize;                         /*!< NDEF File Size                                     */
  uint8_t                  readAccess;                       /*!< NDEF File READ access condition                    */
  uint8_t                  writeAccess;                      /*!< NDEF File WRITE access condition                   */
} ndefCapabilityContainerT4T;

/*! T5T Capability Container  */
typedef struct {
  uint8_t                  ccLen;                            /*!< CC Len                                             */
  uint8_t                  magicNumber;                      /*!< Magic number i.e. E1h or E2h                       */
  uint8_t                  majorVersion;                     /*!< Major version i.e 1                                */
  uint8_t                  minorVersion;                     /*!< Minor version i.e 0                                */
  uint8_t                  readAccess;                       /*!< NDEF READ access condition                         */
  uint8_t                  writeAccess;                      /*!< NDEF WRITE access condition                        */
  bool                     specialFrame;                     /*!< Use Special Frames for Write-alike commands        */
  bool                     lockBlock;                        /*!< (EXTENDED_)LOCK_SINGLE_BLOCK supported             */
  bool                     multipleBlockRead;                /*!< (EXTENDED_)READ_MULTIPLE_BLOCK supported           */
  bool                     mlenOverflow;                     /*!< memory size exceeds 2040 bytes (Android)           */
  uint16_t                 memoryLen;                        /*!< MLEN (Memory Len). T5T_Area size = 8 * MLEN (bytes)*/
} ndefCapabilityContainerT5T;

/*! Generic Capability Container  */
typedef union {
  ndefCapabilityContainerT1T   t1t;                          /*!< T1T Capability Container                           */
  ndefCapabilityContainerT2T   t2t;                          /*!< T2T Capability Container                           */
  ndefAttribInfoBlockT3T       t3t;                          /*!< T3T Attribute Information Block                    */
  ndefCapabilityContainerT4T   t4t;                          /*!< T4T Capability Container                           */
  ndefCapabilityContainerT5T   t5t;                          /*!< T5T Capability Container                           */
} ndefCapabilityContainer;

/*! NDEF T1T sub context structure */
typedef struct {
  void *rfu;                                                 /*!< RFU                                                */
} ndefT1TContext;

/*! NDEF T2T sub context structure */
typedef struct {
  uint8_t                     currentSecNo;                      /*!< Current sector number                          */
  uint8_t                     cacheBuf[NDEF_T2T_READ_RESP_SIZE]; /*!< Cache buffer                                   */
  uint32_t                    cacheAddr;                         /*!< Address of cached data                         */
  uint32_t                    offsetNdefTLV;                     /*!< NDEF TLV message offset                        */
} ndefT2TContext;

/*! NDEF T3T sub context structure */
typedef struct {
  uint8_t                      txbuf[NDEF_T3T_MAX_TX_SIZE];         /*!< Tx buffer dedicated for T3T internal operations         */
  uint8_t                      rxbuf[NDEF_T3T_MAX_RX_SIZE];         /*!< Rx buffer dedicated for T3T internal operations         */
  rfalNfcfBlockListElem        listBlocks[NDEF_T3T_MAX_NB_BLOCKS];  /*!< block number list for T3T internal operations           */
} ndefT3TContext;

/*! NDEF T4T sub context structure */
typedef struct {
  uint8_t                      curMLe;                       /*!< Current MLe. Default Fh until CC file is read      */
  uint8_t                      curMLc;                       /*!< Current MLc. Default Dh until CC file is read      */
  bool                         mv1Flag;                      /*!< Mapping version 1 flag                             */
  rfalIsoDepApduBufFormat      cApduBuf;                     /*!< Command-APDU buffer                                */
  rfalIsoDepApduBufFormat      rApduBuf;                     /*!< Response-APDU buffer                               */
  rfalT4tRApduParam            respAPDU;                     /*!< Response-APDU params                               */
  rfalIsoDepBufFormat          tmpBuf;                       /*!< I-Block temporary buffer                           */
  uint16_t                     rApduBodyLen;                 /*!< Response Body Len                                  */
} ndefT4TContext;

/*! NDEF T5T sub context structure */
typedef struct {
  uint8_t                     *pAddressedUid;                /*!< Pointer to UID in Addr mode or NULL selected one   */
  uint32_t                     TlvNDEFOffset;                /*!< NDEF TLV message offset                            */
  uint8_t                      blockLen;                     /*!< T5T BLEN parameter                                 */
  ndefSystemInformation        sysInfo;                      /*!< System Information (when supported)                */
  bool                         sysInfoSupported;             /*!< System Information Supported flag                  */
  bool                         legacySTHighDensity;          /*!< Legacy ST High Density flag                        */
  uint8_t                      txrxBuf[NDEF_T5T_TxRx_BUFF_SIZE];  /*!< Tx Rx Buffer                                  */
} ndefT5TContext;

/*
 ******************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 ******************************************************************************
 */


#endif /* NDEF_POLLER_H */

/**
  * @}
  *
  */
