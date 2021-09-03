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
 *
 * \addtogroup NDEF
 * @{
 *
 */


#ifndef NDEF_CLASS_H
#define NDEF_CLASS_H

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "st_errno.h"
#include "nfc_utils.h"
#include "rfal_nfc.h"
#include "rfal_nfca.h"
#include "rfal_nfcb.h"
#include "rfal_nfcf.h"
#include "rfal_nfcv.h"
#include "rfal_isoDep.h"
#include "rfal_t2t.h"
#include "rfal_t4t.h"
#include "ndef_poller.h"
#include "ndef_record.h"
#include "ndef_message.h"
#include "ndef_types.h"
#include "ndef_type_wifi.h"
#include "ndef_types_mime.h"
#include "ndef_types_rtd.h"


/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */

#define NDEF_MAX_RECORD          10U    /*!< Maximum number of records */

#define NDEF_SYSINFO_FLAG_DFSID_POS                           (0U)                       /*!< Info flags DFSID flag position                     */
#define NDEF_SYSINFO_FLAG_AFI_POS                             (1U)                       /*!< Info flags AFI flag position                       */
#define NDEF_SYSINFO_FLAG_MEMSIZE_POS                         (2U)                       /*!< Info flags Memory Size flag position               */
#define NDEF_SYSINFO_FLAG_ICREF_POS                           (3U)                       /*!< Info flags IC reference flag position              */
#define NDEF_SYSINFO_FLAG_MOI_POS                             (4U)                       /*!< Info flags MOI flag position                       */
#define NDEF_SYSINFO_FLAG_CMDLIST_POS                         (5U)                       /*!< Info flags Command List flag position              */
#define NDEF_SYSINFO_FLAG_CSI_POS                             (6U)                       /*!< Info flags CSI flag position                       */
#define NDEF_SYSINFO_FLAG_LEN_POS                             (7U)                       /*!< Info flags Length  position                        */

#define NDEF_CMDLIST_READSINGLEBLOCK_POS                      (0U)                       /*!< Cmd List: ReadSingleBlock position                 */
#define NDEF_CMDLIST_WRITESINGLEBLOCK_POS                     (1U)                       /*!< Cmd List: WriteSingleBlock position                */
#define NDEF_CMDLIST_LOCKSINGLEBLOCK_POS                      (2U)                       /*!< Cmd List: LockSingleBlock position                 */
#define NDEF_CMDLIST_READMULTIPLEBLOCKS_POS                   (3U)                       /*!< Cmd List: ReadMultipleBlocks position              */
#define NDEF_CMDLIST_WRITEMULTIPLEBLOCKS_POS                  (4U)                       /*!< Cmd List: WriteMultipleBlocks position             */
#define NDEF_CMDLIST_SELECT_POS                               (5U)                       /*!< Cmd List: SelectSupported position                 */
#define NDEF_CMDLIST_RESETTOREADY_POS                         (6U)                       /*!< Cmd List: ResetToReady position                    */
#define NDEF_CMDLIST_GETMULTIPLEBLOCKSECSTATUS_POS            (7U)                       /*!< Cmd List: GetMultipleBlockSecStatus position       */

#define NDEF_CMDLIST_WRITEAFI_POS                             (0U)                       /*!< Cmd List: WriteAFI position                        */
#define NDEF_CMDLIST_LOCKAFI_POS                              (1U)                       /*!< Cmd List: LockAFI position                         */
#define NDEF_CMDLIST_WRITEDSFID_POS                           (2U)                       /*!< Cmd List: WriteDSFID position                      */
#define NDEF_CMDLIST_LOCKDSFID_POS                            (3U)                       /*!< Cmd List: LockDSFID position                       */
#define NDEF_CMDLIST_GETSYSTEMINFORMATION_POS                 (4U)                       /*!< Cmd List: GetSystemInformation position            */
#define NDEF_CMDLIST_CUSTOMCMDS_POS                           (5U)                       /*!< Cmd List: CustomCmds position                      */
#define NDEF_CMDLIST_FASTREADMULTIPLEBLOCKS_POS               (6U)                       /*!< Cmd List: FastReadMultipleBlocks position          */

#define NDEF_CMDLIST_EXTREADSINGLEBLOCK_POS                   (0U)                       /*!< Cmd List: ExtReadSingleBlock position              */
#define NDEF_CMDLIST_EXTWRITESINGLEBLOCK_POS                  (1U)                       /*!< Cmd List: ExtWriteSingleBlock position             */
#define NDEF_CMDLIST_EXTLOCKSINGLEBLOCK_POS                   (2U)                       /*!< Cmd List: ExtLockSingleBlock position              */
#define NDEF_CMDLIST_EXTREADMULTIPLEBLOCKS_POS                (3U)                       /*!< Cmd List: ExtReadMultipleBlocks position           */
#define NDEF_CMDLIST_EXTWRITEMULTIPLEBLOCKS_POS               (4U)                       /*!< Cmd List: ExtWriteMultipleBlocks position          */
#define NDEF_CMDLIST_EXTGETMULTIPLEBLOCKSECSTATUS_POS         (5U)                       /*!< Cmd List: ExtGetMultipleBlockSecStatus position    */
#define NDEF_CMDLIST_FASTEXTENDEDREADMULTIPLEBLOCKS_POS       (6U)                       /*!< Cmd List: FastExtendedReadMultipleBlocks position  */

#define ndefT5TSysInfoDFSIDPresent(infoFlags)                          (((infoFlags) >> NDEF_SYSINFO_FLAG_DFSID_POS)   & 0x01U)           /*!< Returns DFSID presence flag                        */
#define ndefT5TSysInfoAFIPresent(infoFlags)                            (((infoFlags) >> NDEF_SYSINFO_FLAG_AFI_POS)     & 0x01U)           /*!< Returns AFI presence flag                          */
#define ndefT5TSysInfoMemSizePresent(infoFlags)                        (((infoFlags) >> NDEF_SYSINFO_FLAG_MEMSIZE_POS) & 0x01U)           /*!< Returns Memory size presence flag                  */
#define ndefT5TSysInfoICRefPresent(infoFlags)                          (((infoFlags) >> NDEF_SYSINFO_FLAG_ICREF_POS)   & 0x01U)           /*!< Returns IC Reference presence flag                 */
#define ndefT5TSysInfoMOIValue(infoFlags)                              (((infoFlags) >> NDEF_SYSINFO_FLAG_MOI_POS)     & 0x01U)           /*!< Returns MOI value                                  */
#define ndefT5TSysInfoCmdListPresent(infoFlags)                        (((infoFlags) >> NDEF_SYSINFO_FLAG_CMDLIST_POS) & 0x01U)           /*!< Returns Command List presence flag                 */
#define ndefT5TSysInfoCSIPresent(infoFlags)                            (((infoFlags) >> NDEF_SYSINFO_FLAG_CSI_POS)     & 0x01U)           /*!< Returns CSI presence flag                          */
#define ndefT5TSysInfoLenValue(infoFlags)                              (((infoFlags) >> NDEF_SYSINFO_FLAG_LEN_POS)     & 0x01U)           /*!< Returns Info flag len value                        */

#define ndefT5TSysInfoReadSingleBlockSupported(cmdList)                (((cmdList)[0] >> NDEF_CMDLIST_READSINGLEBLOCK_POS)                & 0x01U) /*!< Returns ReadSingleBlock support flag                 */
#define ndefT5TSysInfoWriteSingleBlockSupported(cmdList)               (((cmdList)[0] >> NDEF_CMDLIST_WRITESINGLEBLOCK_POS)               & 0x01U) /*!< Returns WriteSingleBlock support flag                */
#define ndefT5TSysInfoLockSingleBlockSupported(cmdList)                (((cmdList)[0] >> NDEF_CMDLIST_LOCKSINGLEBLOCK_POS)                & 0x01U) /*!< Returns LockSingleBlock support flag                 */
#define ndefT5TSysInfoReadMultipleBlocksSupported(cmdList)             (((cmdList)[0] >> NDEF_CMDLIST_READMULTIPLEBLOCKS_POS)             & 0x01U) /*!< Returns ReadMultipleBlocks support flag              */
#define ndefT5TSysInfoWriteMultipleBlocksSupported(cmdList)            (((cmdList)[0] >> NDEF_CMDLIST_WRITEMULTIPLEBLOCKS_POS)            & 0x01U) /*!< Returns WriteMultipleBlocks support flag             */
#define ndefT5TSysInfoSelectSupported(cmdList)                         (((cmdList)[0] >> NDEF_CMDLIST_SELECT_POS)                         & 0x01U) /*!< Returns SelectSupported support flag                 */
#define ndefT5TSysInfoResetToReadySupported(cmdList)                   (((cmdList)[0] >> NDEF_CMDLIST_RESETTOREADY_POS)                   & 0x01U) /*!< Returns ResetToReady support flag                    */
#define ndefT5TSysInfoGetMultipleBlockSecStatusSupported(cmdList)      (((cmdList)[0] >> NDEF_CMDLIST_GETMULTIPLEBLOCKSECSTATUS_POS)      & 0x01U) /*!< Returns GetMultipleBlockSecStatus support flag       */

#define ndefT5TSysInfoWriteAFISupported(cmdList)                       (((cmdList)[1] >> NDEF_CMDLIST_WRITEAFI_POS)                       & 0x01U) /*!< Returns WriteAFI support flag                        */
#define ndefT5TSysInfoLockAFISupported(cmdList)                        (((cmdList)[1] >> NDEF_CMDLIST_LOCKAFI_POS)                        & 0x01U) /*!< Returns LockAFI support flag                         */
#define ndefT5TSysInfoWriteDSFIDSupported(cmdList)                     (((cmdList)[1] >> NDEF_CMDLIST_WRITEDSFID_POS)                     & 0x01U) /*!< Returns WriteDSFID support flag                      */
#define ndefT5TSysInfoLockDSFIDSupported(cmdList)                      (((cmdList)[1] >> NDEF_CMDLIST_LOCKDSFID_POS)                      & 0x01U) /*!< Returns LockDSFID support flag                       */
#define ndefT5TSysInfoGetSystemInformationSupported(cmdList)           (((cmdList)[1] >> NDEF_CMDLIST_GETSYSTEMINFORMATION_POS)           & 0x01U) /*!< Returns GetSystemInformation support flag            */
#define ndefT5TSysInfoCustomCmdsSupported(cmdList)                     (((cmdList)[1] >> NDEF_CMDLIST_CUSTOMCMDS_POS)                     & 0x01U) /*!< Returns CustomCmds support flag                      */
#define ndefT5TSysInfoFastReadMultipleBlocksSupported(cmdList)         (((cmdList)[1] >> NDEF_CMDLIST_FASTREADMULTIPLEBLOCKS_POS)         & 0x01U) /*!< Returns FastReadMultipleBlocks support flag          */

#define ndefT5TSysInfoExtReadSingleBlockSupported(cmdList)             (((cmdList)[2] >> NDEF_CMDLIST_EXTREADSINGLEBLOCK_POS)             & 0x01U) /*!< Returns ExtReadSingleBlock support flag              */
#define ndefT5TSysInfoExtWriteSingleBlockSupported(cmdList)            (((cmdList)[2] >> NDEF_CMDLIST_EXTWRITESINGLEBLOCK_POS)            & 0x01U) /*!< Returns ExtWriteSingleBlock support flag             */
#define ndefT5TSysInfoExtLockSingleBlockSupported(cmdList)             (((cmdList)[2] >> NDEF_CMDLIST_EXTLOCKSINGLEBLOCK_POS)             & 0x01U) /*!< Returns ExtLockSingleBlock support flag              */
#define ndefT5TSysInfoExtReadMultipleBlocksSupported(cmdList)          (((cmdList)[2] >> NDEF_CMDLIST_EXTREADMULTIPLEBLOCKS_POS)          & 0x01U) /*!< Returns ExtReadMultipleBlocks support flag           */
#define ndefT5TSysInfoExtWriteMultipleBlocksSupported(cmdList)         (((cmdList)[2] >> NDEF_CMDLIST_EXTWRITEMULTIPLEBLOCKS_POS)         & 0x01U) /*!< Returns ExtWriteMultipleBlocks support flag          */
#define ndefT5TSysInfoExtGetMultipleBlockSecStatusSupported(cmdList)   (((cmdList)[2] >> NDEF_CMDLIST_EXTGETMULTIPLEBLOCKSECSTATUS_POS)   & 0x01U) /*!< Returns ExtGetMultipleBlockSecStatus support flag    */
#define ndefT5TSysInfoFastExtendedReadMultipleBlocksSupported(cmdList) (((cmdList)[2] >> NDEF_CMDLIST_FASTEXTENDEDREADMULTIPLEBLOCKS_POS) & 0x01U) /*!< Returns FastExtendedReadMultipleBlocks support flag  */

#define ndefT5TisT5TDevice(device) ((device)->type == RFAL_NFC_LISTEN_TYPE_NFCV)

/*
 ******************************************************************************
 * GLOBAL MACROS
 ******************************************************************************
 */


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


/*
 ******************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 ******************************************************************************
 */

class NdefClass {

  public:

    NdefClass(RfalNfcClass *rfal_nfc) : rfal_nfc(rfal_nfc)
    {
      memset(&device, 0, sizeof(rfalNfcDevice));
      state = NDEF_STATE_INVALID;
      memset(&cc, 0, sizeof(ndefCapabilityContainer));
      messageLen = 0;
      messageOffset = 0;
      areaLen = 0;
      memset(ccBuf, 0, ((sizeof(uint8_t)) * NDEF_CC_BUF_LEN));
      memset(&subCtx.t1t, 0, sizeof(ndefT1TContext));
      ndefRecordPoolIndex = 0;
      memset(ndefRecordPool, 0, (sizeof(ndefRecord) * NDEF_MAX_RECORD));
    }

    /*
    ******************************************************************************
    * NDEF POLLER FUNCTION PROTOTYPES
    ******************************************************************************
    */

    /*!
     *****************************************************************************
     * \brief Handle NDEF context activation
     *
     * This method performs the initialization of the NDEF context.
     * It must be called after a successful
     * anti-collision procedure and prior to any NDEF procedures such as NDEF
     * detection procedure.
     *
     * \param[in]   dev    : ndef Device
     *
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefPollerContextInitialization(rfalNfcDevice *dev);


    /*!
     *****************************************************************************
     * \brief NDEF Detection procedure
     *
     * This method performs the NDEF Detection procedure
     *
     * \param[out]  info   : ndef Information (optional parameter, NULL may be used when no NDEF Information is needed)
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : Detection failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefPollerNdefDetect(ndefInfo *info);


    /*!
     *****************************************************************************
     * \brief Read data
     *
     * This method reads arbitrary length data
     *
     * \param[in]   offset : file offset of where to start reading data
     * \param[in]   len    : requested len
     * \param[out]  buf    : buffer to place the data read from the tag
     * \param[out]  rcvdLen: received length
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : read failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefPollerReadBytes(uint32_t offset, uint32_t len, uint8_t *buf, uint32_t *rcvdLen);


    /*!
     *****************************************************************************
     * \brief  Write data
     *
     * This method writes arbitrary length data from the current selected file
     *
     * \param[in]   offset : file offset of where to start writing data
     * \param[in]   buf    : data to write
     * \param[in]   len    : buf len
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : read failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefPollerWriteBytes(uint32_t offset, const uint8_t *buf, uint32_t len);


    /*!
     *****************************************************************************
     * \brief Read raw NDEF message
     *
     * This method reads a raw NDEF message.
     * Prior to NDEF Read procedure, a successful ndefPollerNdefDetect()
     * has to be performed.
     *
     *
     * \param[out]  buf    : buffer to place the NDEF message
     * \param[in]   bufLen : buffer length
     * \param[out]  rcvdLen: received length
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : read failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefPollerReadRawMessage(uint8_t *buf, uint32_t bufLen, uint32_t *rcvdLen);


    /*!
     *****************************************************************************
     * \brief Write raw NDEF message
     *
     * This method writes a raw NDEF message.
     * Prior to NDEF Write procedure, a successful ndefPollerNdefDetect()
     * has to be performed.
     *
     * \param[in]   buf    : raw message buffer
     * \param[in]   bufLen : buffer length
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : write failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefPollerWriteRawMessage(const uint8_t *buf, uint32_t bufLen);


    /*!
     *****************************************************************************
     * \brief Format Tag
     *
     * This method format a tag to make it ready for NDEF storage.
     * cc and options parameters usage is described in each technology method
     * (ndefT[2345]TPollerTagFormat)
     *
     * \param[in]   cc      : Capability Container
     * \param[in]   options : specific flags
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : write failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefPollerTagFormat(const ndefCapabilityContainer *cc, uint32_t options);


    /*!
     *****************************************************************************
     * \brief Write NDEF message length
     *
     * This method writes the NLEN field
     *
     * \param[in]   rawMessageLen: len
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : write failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefPollerWriteRawMessageLen(uint32_t rawMessageLen);


    /*!
    *****************************************************************************
    * \brief Write an NDEF message
    *
    * Write the NDEF message to the tag
    *
    * \param[in] message: Message to write
    *
    * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
    * \return ERR_REQUEST      : write failed
    * \return ERR_PARAM        : Invalid parameter
    * \return ERR_PROTO        : Protocol error
    * \return ERR_NONE         : No error
    *****************************************************************************
    */
    ReturnCode ndefPollerWriteMessage(const ndefMessage *message);


    /*!
     *****************************************************************************
     * \brief Check Presence
     *
     * This method check whether an NFC tag is still present in the operating field
     *
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefPollerCheckPresence();


    /*!
     *****************************************************************************
     * \brief Check Available Space
     *
     * This method check whether a NFC tag has enough space to write a message of a given length
     *
     * \param[in]   messageLen: message length
     *
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_NOMEM        : not enough space
     * \return ERR_NONE         : Enough space for message of messageLen length
     *****************************************************************************
     */
    ReturnCode ndefPollerCheckAvailableSpace(uint32_t messageLen);


    /*!
     *****************************************************************************
     * \brief Begin Write Message
     *
     * This method sets the L-field to 0 (T1T, T2T, T4T, T5T) or set the WriteFlag (T3T) and sets the message offset to the proper value according to messageLen
     *
     * \param[in]   messageLen: message length
     *
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_NOMEM        : not enough space
     * \return ERR_NONE         : Enough space for message of messageLen length
     *****************************************************************************
     */
    ReturnCode ndefPollerBeginWriteMessage(uint32_t messageLen);


    /*!
     *****************************************************************************
     * \brief End Write Message
     *
     * This method updates the L-field value after the message has been written and resets the WriteFlag (for T3T only)
     *
     * \param[in]   messageLen: message length
     *
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_NOMEM        : not enough space
     * \return ERR_NONE         : Enough space for message of messageLen length
     *****************************************************************************
     */
    ReturnCode ndefPollerEndWriteMessage(uint32_t messageLen);


    /*
    ******************************************************************************
    * NDEF T2T POLLER FUNCTION PROTOTYPES
    ******************************************************************************
    */


    /*!
     *****************************************************************************
     * \brief Handle T2T NDEF context activation
     *
     * This method performs the initialization of the NDEF context and handles
     * the activation of the ISO-DEP layer. It must be called after a successful
     * anti-collision procedure and prior to any NDEF procedures such as NDEF
     * detection procedure.
     *
     * \param[in]   dev    : ndef Device
     *
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT2TPollerContextInitialization(rfalNfcDevice *dev);


    /*!
     *****************************************************************************
     * \brief T2T NDEF Detection procedure
     *
     * This method performs the T2T NDEF Detection procedure
     *
     * \param[out]  info   : ndef Information (optional parameter, NULL may be used when no NDEF Information is needed)
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : Detection failed (application or ccfile not found)
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT2TPollerNdefDetect(ndefInfo *info);


    /*!
     *****************************************************************************
     * \brief T2T Read data from tag memory
     *
     * This method reads arbitrary length data from the current selected file
     *
     * \param[in]   len    : requested len
     * \param[in]   offset : file offset of where to start reading data
     * \param[out]  buf    : buffer to place the data read from the tag
     * \param[out]  rcvdLen: received length
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : read failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT2TPollerReadBytes(uint32_t offset, uint32_t len, uint8_t *buf, uint32_t *rcvdLen);


    /*!
     *****************************************************************************
     * \brief T2T write data to tag memory
     *
     * This method reads arbitrary length data from the current selected file
     *
     * \param[in]   offset : file offset of where to start writing data
     * \param[in]   buf    : data to write
     * \param[in]   len    : buf len
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : read failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT2TPollerWriteBytes(uint32_t offset, const uint8_t *buf, uint32_t len);


    /*!
     *****************************************************************************
     * \brief T2T Read raw NDEF message
     *
     * This method reads a raw NDEF message from the current selected file.
     * Prior to NDEF Read procedure, a successful ndefT2TPollerNdefDetect()
     * has to be performed.
     *
     * \param[out]  buf    : buffer to place the NDEF message
     * \param[in]   bufLen : buffer length
     * \param[out]  rcvdLen: received length
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : read failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT2TPollerReadRawMessage(uint8_t *buf, uint32_t bufLen, uint32_t *rcvdLen);


    /*!
     *****************************************************************************
     * \brief T2T Write raw NDEF message
     *
     * This method writes a raw NDEF message in the current selected file.
     * Prior to NDEF Write procedure, a successful ndefT2TPollerNdefDetect()
     * has to be performed.
     *
     * \warning Current selected file must not be changed between NDEF Detect
     * procedure and NDEF Write procedure. If another file is selected before
     * NDEF Write procedure, it is user responsibility to re-select NDEF file
     * or to call ndefT2TPollerNdefDetect() to restore proper context.
     *
     * \param[in]   buf    : raw message buffer
     * \param[in]   bufLen : buffer length
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : write failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT2TPollerWriteRawMessage(const uint8_t *buf, uint32_t bufLen);


    /*!
     *****************************************************************************
     * \brief T2T Write NDEF message length
     *
     * This method writes the NLEN field (V2 mapping) or the ENLEN (V3 mapping).
     *
     * \param[in]   rawMessageLen: len
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : write failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT2TPollerWriteRawMessageLen(uint32_t rawMessageLen);


    /*!
     *****************************************************************************
     * \brief T2T Format Tag
     *
     * This method formats a tag to make it ready for NDEF storage.
     * The Capability Container block is written only for virgin tags.
     * If the cc parameter is not provided (i.e. NULL), a default one is used
     * with T2T_AreaSize = 48 bytes.
     * Beware that formatting is on most tags a one time operation (OTP bits!!!!)
     * Doing a wrong format may render your tag unusable.
     * options parameter is not used for T2T Tag Format method
     *
     * \param[in]   cc     : Capability Container
     * \param[in]   options: specific flags
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : write failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT2TPollerTagFormat(const ndefCapabilityContainer *cc, uint32_t options);


    /*!
     *****************************************************************************
     * \brief T2T Check Presence
     *
     * This method checks whether a T2T tag is still present in the operating field
     *
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT2TPollerCheckPresence();


    /*!
     *****************************************************************************
     * \brief T2T Check Available Space
     *
     * This method checks whether a T2T tag has enough space to write a message of a given length
     *
     * \param[in]   messageLen: message length
     *
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_NOMEM        : not enough space
     * \return ERR_NONE         : Enough space for message of messageLen length
     *****************************************************************************
     */
    ReturnCode ndefT2TPollerCheckAvailableSpace(uint32_t messageLen);


    /*!
     *****************************************************************************
     * \brief T2T Begin Write Message
     *
     * This method sets the L-field to 0 and sets the message offset to the proper value according to messageLen
     *
     * \param[in]   messageLen: message length
     *
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_NOMEM        : not enough space
     * \return ERR_NONE         : Enough space for message of messageLen length
     *****************************************************************************
     */
    ReturnCode ndefT2TPollerBeginWriteMessage(uint32_t messageLen);


    /*!
     *****************************************************************************
     * \brief T2T End Write Message
     *
     * This method updates the L-field value after the message has been written
     *
     * \param[in]   messageLen: message length
     *
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_NOMEM        : not enough space
     * \return ERR_NONE         : Enough space for message of messageLen length
     *****************************************************************************
     */
    ReturnCode ndefT2TPollerEndWriteMessage(uint32_t messageLen);


    /*
    ******************************************************************************
    * NDEF T3T POLLER FUNCTION PROTOTYPES
    ******************************************************************************
    */


    /*!
     *****************************************************************************
     * \brief Handle T3T NDEF context activation
     *
     * This method performs the initialization of the NDEF context and handles
     * the activation of the ISO-DEP layer. It must be called after a successful
     * anti-collision procedure and prior to any NDEF procedures such as NDEF
     * detection procedure.
     *
     * \param[in]   dev    : ndef Device
     *
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT3TPollerContextInitialization(rfalNfcDevice *dev);


    /*!
     *****************************************************************************
     * \brief T3T NDEF Detection procedure
     *
     * This method performs the T3T NDEF Detection procedure
     *
     * \param[out]  info   : ndef Information (optional parameter, NULL may be used when no NDEF Information is needed)
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : Detection failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT3TPollerNdefDetect(ndefInfo *info);


    /*!
     *****************************************************************************
     * \brief T3T Read data from file
     *
     * This method reads arbitrary length data from the current selected file
     *
     * \param[in]   len    : requested len
     * \param[in]   offset : file offset of where to start reading data
     * \param[out]  buf    : buffer to place the data read from the tag
     * \param[out]  rcvdLen: received length
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : read failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT3TPollerReadBytes(uint32_t offset, uint32_t len, uint8_t *buf, uint32_t *rcvdLen);


    /*!
     *****************************************************************************
     * \brief T3T write data to file
     *
     * This method reads arbitrary length data from the current selected file
     *
     * \param[in]   offset : file offset of where to start writing data
     * \param[in]   buf    : data to write
     * \param[in]   len    : buf len
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : read failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT3TPollerWriteBytes(uint32_t offset, const uint8_t *buf, uint32_t len);


    /*!
     *****************************************************************************
     * \brief T3T Read raw NDEF message
     *
     * This method reads a raw NDEF message from the current selected file.
     * Prior to NDEF Read procedure, a successful ndefT3TPollerNdefDetect()
     * has to be performed.
     *
     * \param[out]  buf    : buffer to place the NDEF message
     * \param[in]   bufLen : buffer length
     * \param[out]  rcvdLen: received length
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : read failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT3TPollerReadRawMessage(uint8_t *buf, uint32_t bufLen, uint32_t *rcvdLen);


    /*!
     *****************************************************************************
     * \brief T3T Write raw NDEF message
     *
     * This method writes a raw NDEF message in the current selected file.
     * Prior to NDEF Write procedure, a successful ndefT3TPollerNdefDetect()
     * has to be performed.
     *
     * \warning Current selected file must not be changed between NDEF Detect
     * procedure and NDEF Write procedure. If another file is selected before
     * NDEF Write procedure, it is user responsibility to re-select NDEF file
     * or to call ndefT3TPollerNdefDetect() to restore proper context.
     *
     * \param[in]   buf    : raw message buffer
     * \param[in]   bufLen : buffer length
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : write failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT3TPollerWriteRawMessage(const uint8_t *buf, uint32_t bufLen);


    /*!
     *****************************************************************************
     * \brief T3T Write NDEF message length
     *
     * This method writes the NLEN field (V2 mapping) or the ENLEN (V3 mapping).
     *
     * \param[in]   rawMessageLen: len
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : write failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT3TPollerWriteRawMessageLen(uint32_t rawMessageLen);


    /*!
     *****************************************************************************
     * \brief T3T Format Tag
     *
     * This method formats a tag to make it ready for NDEF storage.
     * cc parameter contains the Attribute Information Block fields
     * If cc parameter is not provided (i.e. NULL), this method assumes
     * that the AIB is already present.
     * options parameter is not used for T3T Tag Format method
     *
     *
     * \param[in]   cc     : Capability Container
     * \param[in]   options: specific flags
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : write failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT3TPollerTagFormat(const ndefCapabilityContainer *cc, uint32_t options);


    /*!
     *****************************************************************************
     * \brief T3T Check Presence
     *
     * This method checks whether a T3T tag is still present in the operating field
     *
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT3TPollerCheckPresence();


    /*!
     *****************************************************************************
     * \brief T3T Check Available Space
     *
     * This method checks whether a T3T tag has enough space to write a message of a given length
     *
     * \param[in]   messageLen: message length
     *
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_NOMEM        : not enough space
     * \return ERR_NONE         : Enough space for message of messageLen length
     *****************************************************************************
     */
    ReturnCode ndefT3TPollerCheckAvailableSpace(uint32_t messageLen);


    /*!
     *****************************************************************************
     * \brief T3T Begin Write Message
     *
     * This method sets the WriteFlag to the appropriate value before starting to write the NDEF message
     *
     * \param[in]   messageLen: message length
     *
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_NOMEM        : not enough space
     * \return ERR_NONE         : Enough space for message of messageLen length
     *****************************************************************************
     */
    ReturnCode ndefT3TPollerBeginWriteMessage(uint32_t messageLen);


    /*!
     *****************************************************************************
     * \brief T3T End Write Message
     *
     * This method updates the Ln field of the AIB and resets the WriteFlag
     *
     * \param[in]   messageLen: message length
     *
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_NOMEM        : not enough space
     * \return ERR_NONE         : Enough space for message of messageLen length
     *****************************************************************************
     */
    ReturnCode ndefT3TPollerEndWriteMessage(uint32_t messageLen);


    /*
    ******************************************************************************
    * NDEF T4T POLLER FUNCTION PROTOTYPES
    ******************************************************************************
    */


    /*!
     *****************************************************************************
     * \brief Handle T4T NDEF context activation
     *
     * This method performs the initialization of the NDEF context and handles
     * the activation of the ISO-DEP layer. It must be called after a successful
     * anti-collision procedure and prior to any NDEF procedures such as NDEF
     * detection procedure.
     *
     * \param[in]   dev    : ndef Device
     *
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT4TPollerContextInitialization(rfalNfcDevice *dev);


    /*!
     *****************************************************************************
     * \brief T4T NDEF Detection procedure
     *
     * This method performs the T4T NDEF Detection procedure
     *
     * \param[out]  info   : ndef Information (optional parameter, NULL may be used when no NDEF Information is needed)
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : Detection failed (application or ccfile not found)
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT4TPollerNdefDetect(ndefInfo *info);


    /*!
     *****************************************************************************
     * \brief T4T Select NDEF Tag Application
     *
     * This method sends the Select NDEF tag application. If V2 Tag Application
     * is not found, a Select NDEF tag V1 application is sent/
     *
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : Application not found
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT4TPollerSelectNdefTagApplication();


    /*!
     *****************************************************************************
     * \brief T4T Select File
     *
     * This method sends a Select File Command-APDU.
     *
     * The following fields of the ndef Context must be filled up before calling
     * this method:
     * - devType: device type
     * - subCtx.t4t.mv1Flag: Mapping version 1 flag
     *
     * \param[in]   fileId : file identifier
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : File not found
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT4TPollerSelectFile(const uint8_t *fileId);


    /*!
     *****************************************************************************
     * \brief T4T ReadBinary
     *
     * This method reads the data from the tag using a single
     * ReadBinary command
     *
     * \param[in]   offset : file offset of where to star reading data; valid range 0000h-7FFFh
     * \param[in]   len    : requested len (short field coding)
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : read failed (SW1SW2 <> 9000h)
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT4TPollerReadBinary(uint16_t offset, uint8_t len);


    /*!
     *****************************************************************************
     * \brief T4T ReadBinary with ODO
     *
     * This method reads the data from the tag using a single
     * ReadBinary ODO command
     *
     * \param[in]   offset : file offset of where to star reading data; valid range 0000h-7FFFh
     * \param[in]   len    : requested len (short field coding)
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : read failed (SW1SW2 <> 9000h)
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT4TPollerReadBinaryODO(uint32_t offset, uint8_t len);


    /*!
     *****************************************************************************
     * \brief T4T Read data from file
     *
     * This method reads arbitrary length data from the current selected file
     *
     * \param[in]   len    : requested len
     * \param[in]   offset : file offset of where to start reading data
     * \param[out]  buf    : buffer to place the data read from the tag
     * \param[out]  rcvdLen: received length
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : read failed (SW1SW2 <> 9000h)
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT4TPollerReadBytes(uint32_t offset, uint32_t len, uint8_t *buf, uint32_t *rcvdLen);

    /*!
     *****************************************************************************
     * \brief T4T WriteBinary
     *
     * This method writes the data to the tag using a single
     * WriteBinary command
     *
     * \param[in]   offset : file offset of where to star reading data; valid range 0000h-7FFFh
     * \param[in]   data   : data to be written
     * \param[in]   len    : data length
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : read failed (SW1SW2 <> 9000h)
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT4TPollerWriteBinary(uint16_t offset, const uint8_t *data, uint8_t len);


    /*!
     *****************************************************************************
     * \brief T4T WriteBinary with ODO
     *
     * This method writes the data to the tag using a single
     * WriteBinary ODO command
     *
     * \param[in]   offset : file offset of where to star reading data; valid range 0000h-7FFFh
     * \param[in]   data   : data to be written
     * \param[in]   len    : data length
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : read failed (SW1SW2 <> 9000h)
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT4TPollerWriteBinaryODO(uint32_t offset, const uint8_t *data, uint8_t len);

    /*!
     *****************************************************************************
     * \brief T4T write data to file
     *
     * This method reads arbitrary length data from the current selected file
     *
     * \param[in]   offset : file offset of where to start writing data
     * \param[in]   buf    : data to write
     * \param[in]   len    : buf len
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : read failed (SW1SW2 <> 9000h)
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT4TPollerWriteBytes(uint32_t offset, const uint8_t *buf, uint32_t len);


    /*!
     *****************************************************************************
     * \brief T4T Read raw NDEF message
     *
     * This method reads a raw NDEF message from the current selected file.
     * Prior to NDEF Read procedure, a successful ndefT4TPollerNdefDetect()
     * has to be performed.
     *
     * \warning Current selected file must not be changed between NDEF Detect
     * procedure and NDEF Read procedure. If another file is selected before
     * NDEF Read procedure, it is user responsibility to re-select NDEF file
     * or to call ndefT4TPollerNdefDetect() to restore proper context.
     *
     * \param[out]  buf    : buffer to place the NDEF message
     * \param[in]   bufLen : buffer length
     * \param[out]  rcvdLen: received length
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : read failed (SW1SW2 <> 9000h)
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT4TPollerReadRawMessage(uint8_t *buf, uint32_t bufLen, uint32_t *rcvdLen);


    /*!
     *****************************************************************************
     * \brief T4T Write raw NDEF message
     *
     * This method writes a raw NDEF message in the current selected file.
     * Prior to NDEF Write procedure, a successful ndefT4TPollerNdefDetect()
     * has to be performed.
     *
     * \warning Current selected file must not be changed between NDEF Detect
     * procedure and NDEF Write procedure. If another file is selected before
     * NDEF Write procedure, it is user responsibility to re-select NDEF file
     * or to call ndefT4TPollerNdefDetect() to restore proper context.
     *
     * \param[in]   buf    : raw message buffer
     * \param[in]   bufLen : buffer length
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : write failed (SW1SW2 <> 9000h)
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT4TPollerWriteRawMessage(const uint8_t *buf, uint32_t bufLen);


    /*!
     *****************************************************************************
     * \brief T4T Write NDEF message length
     *
     * This method writes the NLEN field (V2 mapping) or the ENLEN (V3 mapping).
     *
     * \param[in]   rawMessageLen: len
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : write failed (SW1SW2 <> 9000h)
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT4TPollerWriteRawMessageLen(uint32_t rawMessageLen);


    /*!
     *****************************************************************************
     * \brief T4T Format Tag
     *
     * This method formats a tag to make it ready for NDEF storage. In case of T4T,
     * it writes NLEN/ENLEN=0 to the NDEF File.
     * cc and options parameters are not used for T4T Tag Format method.
     *
     * \param[in]   cc     : Capability Container
     * \param[in]   options: specific flags
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : write failed (SW1SW2 <> 9000h)
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT4TPollerTagFormat(const ndefCapabilityContainer *cc, uint32_t options);


    /*!
     *****************************************************************************
     * \brief T4T Check Presence
     *
     * This method checks whether a T4T tag is still present in the operating field
     *
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT4TPollerCheckPresence();


    /*!
     *****************************************************************************
     * \brief T4T Check Available Space
     *
     * This method checks whether a T4T tag has enough space to write a message of a given length
     *
     * \param[in]   messageLen: message length
     *
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_NOMEM        : not enough space
     * \return ERR_NONE         : Enough space for message of messageLen length
     *****************************************************************************
     */
    ReturnCode ndefT4TPollerCheckAvailableSpace(uint32_t messageLen);


    /*!
     *****************************************************************************
     * \brief T4T Begin Write Message
     *
     * This method sets the L-field to 0
     *
     * \param[in]   messageLen: message length
     *
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_NOMEM        : not enough space
     * \return ERR_NONE         : Enough space for message of messageLen length
     *****************************************************************************
     */
    ReturnCode ndefT4TPollerBeginWriteMessage(uint32_t messageLen);


    /*!
     *****************************************************************************
     * \brief T4T End Write Message
     *
     * This method updates the L-field value after the message has been written
     *
     * \param[in]   messageLen: message length
     *
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_NOMEM        : not enough space
     * \return ERR_NONE         : Enough space for message of messageLen length
     *****************************************************************************
     */
    ReturnCode ndefT4TPollerEndWriteMessage(uint32_t messageLen);


    /*
    ******************************************************************************
    * NDEF T5T POLLER FUNCTION PROTOTYPES
    ******************************************************************************
    */


    /*!
     *****************************************************************************
     * \brief Handle T5T NDEF context activation
     *
     * This method performs the initialisation of the NDEF context.
     * It must be called after a successful
     * anticollition procedure and prior to any NDEF procedures such as NDEF
     * detection procedure.
     *
     * \param[in]   dev    : ndef Device
     *
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT5TPollerContextInitialization(rfalNfcDevice *dev);


    /*!
     *****************************************************************************
     * \brief T5T NDEF Detection procedure
     *
     * This method performs the T5T NDEF Detection procedure
     *
     *
     * \param[out]  info   : ndef Information (optional parameter, NULL may be used when no NDEF Information is needed)
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : Detection failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT5TPollerNdefDetect(ndefInfo *info);


    /*!
     *****************************************************************************
     * \brief T5T Read data from tag memory
     *
     * This method reads arbitrary length data from tag memory
     *
     * \param[in]   offset : file offset of where to start reading data
     * \param[in]   len    : requested len
     * \param[out]  buf    : buffer to place the data read from the tag
     * \param[out]  rcvdLen: received length
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : read failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT5TPollerReadBytes(uint32_t offset, uint32_t len, uint8_t *buf, uint32_t *rcvdLen);


    /*!
     *****************************************************************************
     * \brief T5T write data to tag memory
     *
     * This method write arbitrary length data from to tag memory
     *
     * \param[in]   offset : file offset of where to start writing data
     * \param[in]   buf    : data to write
     * \param[in]   len    : buf len
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : read failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT5TPollerWriteBytes(uint32_t offset, const uint8_t *buf, uint32_t len);


    /*!
     *****************************************************************************
     * \brief T5T Read raw NDEF message
     *
     * This method reads a raw NDEF message from the current selected file.
     * Prior to NDEF Read procedure, a successful ndefT5TPollerNdefDetect()
     * has to be performed.
     *
     * \param[out]  buf    : buffer to place the NDEF message
     * \param[in]   bufLen : buffer length
     * \param[out]  rcvdLen: received length
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : read failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT5TPollerReadRawMessage(uint8_t *buf, uint32_t bufLen, uint32_t *rcvdLen);


    /*!
     *****************************************************************************
     * \brief T5T Write raw NDEF message
     *
     * This method writes a raw NDEF message in the current selected file.
     * Prior to NDEF Write procedure, a successful ndefT5TPollerNdefDetect()
     * has to be performed.
     *
     * \param[in]   buf    : raw message buffer
     * \param[in]   bufLen : buffer length
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : write failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT5TPollerWriteRawMessage(const uint8_t *buf, uint32_t bufLen);


    /*!
     *****************************************************************************
     * \brief T5T Write NDEF message length
     *
     * This method writes the L field to the tag memory
     *
     * \param[in]   rawMessageLen: len
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : write failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT5TPollerWriteRawMessageLen(uint32_t rawMessageLen);


    /*!
     *****************************************************************************
     * \brief T5T Format Tag
     *
     * This method format a tag to make it ready for NDEF storage.
     * When the cc parameter is provided, the Capability Container is initialized
     * with the value provided by the user.
     * When the cc parameter is not provided (i.e. NULL), this method
     * retrieves the appropriate CC field from (Extended) Get System Information
     * or returns ERR_REQUEST if (Extended) Get System Information is not supported.
     * The option parameter is used in case of NULL cc parameter to choose
     * between Android CC format (default when option = 0)
     * or NFC Forum Format (option = NDEF_T5T_FORMAT_OPTION_NFC_FORUM)
     *
     *
     * \param[in]   cc      : Capability Container
     * \param[in]   options : specific flags
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_REQUEST      : write failed
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT5TPollerTagFormat(const ndefCapabilityContainer *cc, uint32_t options);


    /*!
     *****************************************************************************
     * \brief T5T Check Presence
     *
     * This method check whether a T5T tag is still present in the operating field
     *
     *
     * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_PROTO        : Protocol error
     * \return ERR_NONE         : No error
     *****************************************************************************
     */
    ReturnCode ndefT5TPollerCheckPresence();


    /*!
     *****************************************************************************
     * \brief T5T Check Available Space
     *
     * This method check whether a T5T tag has enough space to write a message of a given length
     *
     * \param[in]   messageLen: message length
     *
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_NOMEM        : not enough space
     * \return ERR_NONE         : Enough space for message of messageLen length
     *****************************************************************************
     */
    ReturnCode ndefT5TPollerCheckAvailableSpace(uint32_t messageLen);


    /*!
     *****************************************************************************
     * \brief T5T Begin Write Message
     *
     * This method sets the L-field to 0 and sets the message offset to the proper value according to messageLen
     *
     * \param[in]   messageLen: message length
     *
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_NOMEM        : not enough space
     * \return ERR_NONE         : Enough space for message of messageLen length
     *****************************************************************************
     */
    ReturnCode ndefT5TPollerBeginWriteMessage(uint32_t messageLen);


    /*!
     *****************************************************************************
     * \brief T5T End Write Message
     *
     * This method updates the L-field value after the message has been written
     *
     * \param[in]   messageLen: message length
     *
     * \return ERR_PARAM        : Invalid parameter
     * \return ERR_NOMEM        : not enough space
     * \return ERR_NONE         : Enough space for message of messageLen length
     *****************************************************************************
     */
    ReturnCode ndefT5TPollerEndWriteMessage(uint32_t messageLen);


    /*
    ******************************************************************************
    * NDEF MESSAGE FUNCTION PROTOTYPES
    ******************************************************************************
    */


    /*!
     *****************************************************************************
     * Initialize an empty NDEF message
     *
     * \param[in,out] message to initialize
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefMessageInit(ndefMessage *message);


    /*!
     *****************************************************************************
     * Get NDEF message information
     *
     * Return the message information
     *
     * \param[in]  message
     * \param[out] info: e.g. message length in bytes, number of records
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefMessageGetInfo(const ndefMessage *message, ndefMessageInfo *info);


    /*!
     *****************************************************************************
     * Get the number of NDEF message records
     *
     * Return the number of records in the given message
     *
     * \param[in] message
     *
     * \return number of records
     *****************************************************************************
     */
    uint32_t ndefMessageGetRecordCount(const ndefMessage *message);


    /*!
     *****************************************************************************
     * Append a record to an NDEF message
     *
     * \param[in]     record:  Record to append
     * \param[in,out] message: Message to be appended with the given record
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefMessageAppend(ndefMessage *message, ndefRecord *record);


    /*!
     *****************************************************************************
     * Decode a raw buffer to an NDEF message
     *
     * Convert a raw buffer to a message
     *
     * \param[in]  bufPayload: Payload buffer to convert into message
     * \param[out] message:    Message created from the raw buffer
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefMessageDecode(const ndefConstBuffer *bufPayload, ndefMessage *message);


    /*!
     *****************************************************************************
     * Encode an NDEF message to a raw buffer
     *
     * Convert a message to a raw buffer
     *
     * \param[in]     message:    Message to convert
     * \param[in,out] bufPayload: Output buffer to store the converted message
     *                            The input length provides the output buffer allocated
     *                            length, used for parameter check to avoid overflow.
     *                            In case the buffer provided is too short, it is
     *                            updated with the required buffer length.
     *                            On success, it is updated with the actual buffer
     *                            length used to contain the converted message.
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefMessageEncode(const ndefMessage *message, ndefBuffer *bufPayload);


    /*
    ******************************************************************************
    * NDEF RECORD FUNCTION PROTOTYPES
    ******************************************************************************
    */


    /*!
     *****************************************************************************
     * Reset an NDEF record
     *
     * This function clears every record field
     *
     * \param[in,out] record to reset
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRecordReset(ndefRecord *record);


    /*!
     *****************************************************************************
     * Initialize an NDEF record
     *
     * This function initializes all record fields
     *
     * \param[in,out] record:     Record to initialize
     * \param[in]     tnf:        TNF type
     * \param[in]     bufType:    Type buffer
     * \param[in]     bufId:      Id buffer
     * \param[in]     bufPayload: Payload buffer
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRecordInit(ndefRecord *record, uint8_t tnf, const ndefConstBuffer8 *bufType, const ndefConstBuffer8 *bufId, const ndefConstBuffer *bufPayload);


    /*!
     *****************************************************************************
     * Get NDEF record header length
     *
     * Return the length of header for the given record
     *
     * \param[in] record
     *
     * \return header length in bytes
     *****************************************************************************
     */
    uint32_t ndefRecordGetHeaderLength(const ndefRecord *record);


    /*!
     *****************************************************************************
     * Get NDEF record length
     *
     * Return the length of the given record, needed to store it as a raw buffer
     *
     * \param[in] record
     *
     * \return record length in bytes
     *****************************************************************************
     */
    uint32_t ndefRecordGetLength(const ndefRecord *record);


    /*!
     *****************************************************************************
     * Set NDEF record type
     *
     * Set the type for the given record
     *
     * \param[in,out] record:  Record to set the type
     * \param[in]     tnf:     TNF type
     * \param[in]     bufType: Type buffer
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRecordSetType(ndefRecord *record, uint8_t tnf, const ndefConstBuffer8 *bufType);


    /*!
     *****************************************************************************
     * Get NDEF record type
     *
     * Return the type for the given record
     *
     * \param[in]  record:  Record to get the type from
     * \param[out] tnf:     Pointer to the TNF type
     * \param[out] bufType: Type string buffer
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRecordGetType(const ndefRecord *record, uint8_t *tnf, ndefConstBuffer8 *bufType);

    /*!
     *****************************************************************************
     * Check the record type matches a given type
     *
     * \param[in]  record:  Record to get the type from
     * \param[out] tnf:     the TNF type to compare with
     * \param[out] bufType: Type string buffer to compare with
     *
     * \return true or false
     *****************************************************************************
     */
    bool ndefRecordTypeMatch(const ndefRecord *record, uint8_t tnf, const ndefConstBuffer8 *bufType);


    /*!
     *****************************************************************************
     * Set NDEF record Id
     *
     * Set the Id for the given NDEF record
     *
     * \param[in]  record: Record to set the Id
     * \param[out] bufId:  Id buffer
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRecordSetId(ndefRecord *record, const ndefConstBuffer8 *bufId);


    /*!
     *****************************************************************************
     * Get NDEF record Id
     *
     * Return the Id for the given NDEF record
     *
     * \param[in]  record: Record to get the Id from
     * \param[out] bufId:  Id buffer
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRecordGetId(const ndefRecord *record, ndefConstBuffer8 *bufId);


    /*!
     *****************************************************************************
     * Set NDEF record payload
     *
     * Set the payload for the given record, update the SR bit accordingly
     *
     * \param[in,out] record:     Record to set the payload
     * \param[in]     bufPayload: Payload buffer
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRecordSetPayload(ndefRecord *record, const ndefConstBuffer *bufPayload);


    /*!
     *****************************************************************************
     * Get NDEF record payload
     *
     * Return the payload for the given record
     *
     * \param[in]  record:     Record to get the payload from
     * \param[out] bufPayload: Payload buffer
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRecordGetPayload(const ndefRecord *record, ndefConstBuffer *bufPayload);


    /*!
     *****************************************************************************
     * Decode a raw buffer to create an NDEF record
     *
     * Convert a raw buffer to a record
     *
     * \param[in]  bufPayload: Payload buffer to convert into record
     * \param[out] record:     Record created from the raw buffer
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRecordDecode(const ndefConstBuffer *bufPayload, ndefRecord *record);


    /*!
     *****************************************************************************
     * Encode an NDEF record header to a raw buffer
     *
     * Convert a record header to a raw buffer. It is made of:
     * "header byte" (1 byte), type length (1 byte),
     * payload length (4 bytes), Id length (1 byte).
     * Total 7 bytes.
     *
     * \param[in]     record:    Record header to convert
     * \param[in,out] bufHeader: Output buffer to store the converted record header
     *                           The input length provides the output buffer allocated
     *                           length, used for parameter check to avoid overflow.
     *                           In case the buffer provided is too short, it is
     *                           updated with the required buffer length.
     *                           On success, it is updated with the actual buffer
     *                           length used to contain the converted record.
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRecordEncodeHeader(const ndefRecord *record, ndefBuffer *bufHeader);


    /*!
     *****************************************************************************
     * Encode an NDEF record to a raw buffer
     *
     * Convert a record to a raw buffer
     *
     * \param[in]     record:    Record to convert
     * \param[in,out] bufRecord: Output buffer to store the converted record
     *                           The input length provides the output buffer allocated
     *                           length, used for parameter check to avoid overflow.
     *                           In case the buffer provided is too short, it is
     *                           updated with the required buffer length.
     *                           On success, it is updated with the actual buffer
     *                           length used to contain the converted record.
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRecordEncode(const ndefRecord *record, ndefBuffer *bufRecord);


    /*!
     *****************************************************************************
     * Get NDEF record payload length
     *
     * Return the length of payload for the given record
     *
     * \param[in] record
     *
     * \return payload length in bytes
     *****************************************************************************
     */
    uint32_t ndefRecordGetPayloadLength(const ndefRecord *record);


    /*!
     *****************************************************************************
     * Return a payload elementary item needed to build the complete record payload
     *
     * Call this function to get either the first payload item, or the next one.
     * Returns the next payload item, call it until it returns NULL.
     *
     * \param[in]  record:         Record
     * \param[out] bufPayloadItem: The payload item returned
     * \param[in]  begin:          Tell to return the first payload item or the next one
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    const uint8_t *ndefRecordGetPayloadItem(const ndefRecord *record, ndefConstBuffer *bufPayloadItem, bool begin);


    /*
    ******************************************************************************
    * NDEF WIFI RECORD FUNCTION PROTOTYPES
    ******************************************************************************
    */


    /*!
     *****************************************************************************
     * Initialize a Wifi configuration
     *
     * \param[out] wifi:       wifi type to initialize
     * \param[in]  wifiConfig: wifi configuration to use
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefWifi(ndefType *wifi, const ndefTypeWifi *wifiConfig);


    /*!
     *****************************************************************************
     * Get Wifi configuration
     *
     * \param[in]  wifi:       wifi type to get information from
     * \param[out] wifiConfig: wifi configuration
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefGetWifi(const ndefType *wifi, ndefTypeWifi *wifiConfig);


    /*!
     *****************************************************************************
     * Convert an NDEF record to a wifi type
     *
     * \param[in]  record: Record to convert
     * \param[out] wifi:   The converted wifi type
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRecordToWifi(const ndefRecord *record, ndefType *wifi);


    /*!
     *****************************************************************************
     * Convert a wifi type to an NDEF record
     *
     * \param[in]  wifi:   wifi type to convert
     * \param[out] record: The converted type
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefWifiToRecord(const ndefType *wifi, ndefRecord *record);


    /*
    ******************************************************************************
    * NDEF MIME RECORD FUNCTION PROTOTYPES
    ******************************************************************************
    */


    /***************
     * Media
     ***************
     */

    /*!
     *****************************************************************************
     * Initialize a Media type
     *
     * \param[out] media:      Media type to initialize
     * \param[in]  bufType:    Type buffer
     * \param[in]  bufPayload: Payload buffer
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefMedia(ndefType *media, const ndefConstBuffer8 *bufType, const ndefConstBuffer *bufPayload);


    /*!
     *****************************************************************************
     * Get Media type content
     *
     * \param[in]  media:      Media type to get information from
     * \param[out] bufType:    Type buffer
     * \param[out] bufPayload: Payload buffer
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefGetMedia(const ndefType *media, ndefConstBuffer8 *bufType, ndefConstBuffer *bufPayload);


    /*!
     *****************************************************************************
     * Convert an NDEF record to a Media type
     *
     * \param[in]  record: Record to convert
     * \param[out] media:  The converted Media type
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRecordToMedia(const ndefRecord *record, ndefType *media);


    /*!
     *****************************************************************************
     * Convert a Media type to an NDEF record
     *
     * \param[in]  media:  Type to convert
     * \param[out] record: The converted type
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefMediaToRecord(const ndefType *media, ndefRecord *record);


    /***************
     * vCard
     ***************
     */

    /*!
     *****************************************************************************
     * Initialize a vCard
     *
     * \param[out] vCard:              vCard type to initialize
     * \param[in]  bufVCardInput:      vCard buffer content
     * \param[in]  bufVCardInputCount: Number of vCard entries
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefVCard(ndefType *vCard, const ndefVCardInput *bufVCardInput, uint32_t bufVCardInputCount);


    /*!
     *****************************************************************************
     * Get vCard content
     *
     * \param[in]  vCard:      vCard type to get information from
     * \param[out] bufType:    Type buffer to extract
     * \param[out] bufSubType: SubType buffer extracted. Can be NULL is not required
     * \param[out] bufValue:   Value buffer extracted
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefGetVCard(const ndefType *vCard, const ndefConstBuffer *bufType, ndefConstBuffer *bufSubType, ndefConstBuffer *bufValue);


    /*!
     *****************************************************************************
     * Convert an NDEF record to a vCard
     *
     * \param[in]  record: Record to convert
     * \param[out] vCard:  The converted vCard type
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRecordToVCard(const ndefRecord *record, ndefType *vCard);


    /*!
     *****************************************************************************
     * Convert a vCard type to an NDEF record
     *
     * \param[in]  vCard:  vCard type to convert
     * \param[out] record: The converted type
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefVCardToRecord(const ndefType *vCard, ndefRecord *record);


    /*
    ******************************************************************************
    * NDEF RTD RECORD FUNCTION PROTOTYPES
    ******************************************************************************
    */


    /***************
     * Empty type
     ***************
     */

    /*!
     *****************************************************************************
     * Initialize an Empty type
     *
     * \param[out] empty: Type to initialize
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefEmptyType(ndefType *empty);


    /*!
     *****************************************************************************
     * Convert an NDEF record to an Empty type
     *
     * \param[in]  record: Record to convert
     * \param[out] empty:  The converted type
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRecordToEmptyType(const ndefRecord *record, ndefType *empty);


    /*!
     *****************************************************************************
     * Convert an Empty type to an NDEF record
     *
     * \param[in]  empty:  Type to convert
     * \param[out] record: The converted type
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefEmptyTypeToRecord(const ndefType *empty, ndefRecord *record);


    /*********************
     * Device Information
     *********************
     */

    /*!
     *****************************************************************************
     * Initialize a RTD Device Information type
     *
     * \param[out] devInfo:          Type to initialize
     * \param[in]  devInfoData:      Device Information data
     * \param[in]  devInfoDataCount: Number of Device Information data
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRtdDeviceInfo(ndefType *devInfo, const ndefDeviceInfoEntry *devInfoData, uint8_t devInfoDataCount);


    /*!
     *****************************************************************************
     * Get RTD Device Information type content
     *
     * \param[in]  devInfo:     Type to get information from
     * \param[out] devInfoData: Device Information data
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefGetRtdDeviceInfo(const ndefType *devInfo, ndefTypeRtdDeviceInfo *devInfoData);


    /*!
     *****************************************************************************
     * Convert an NDEF record to a Device Information RTD type
     *
     * \param[in]  record:  Record to convert
     * \param[out] devInfo: The converted type
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRecordToRtdDeviceInfo(const ndefRecord *record, ndefType *devInfo);


    /*!
     *****************************************************************************
     * Convert a Device Information RTD type to an NDEF record
     *
     * \param[in]  devInfo: Type to convert
     * \param[out] record:  The converted type
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRtdDeviceInfoToRecord(const ndefType *devInfo, ndefRecord *record);


    /***************
     * Text
     ***************
     */


    /*!
     *****************************************************************************
     * Initialize a Text RTD type
     *
     * \param[out] text:            Type to initialize
     * \param[out] utfEncoding:     UTF-8/UTF-16
     * \param[in]  bufLanguageCode: ISO/IANA language code buffer
     * \param[in]  bufSentence:     Actual text buffer
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRtdText(ndefType *text, uint8_t utfEncoding, const ndefConstBuffer8 *bufLanguageCode, const ndefConstBuffer *bufSentence);


    /*!
     *****************************************************************************
     * Get RTD Text type content
     *
     * \param[in]  text:            Type to get information from
     * \param[out] utfEncoding:     UTF-8/UTF-16
     * \param[out] bufLanguageCode: ISO/IANA language code buffer
     * \param[out] bufSentence:     Actual text buffer
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefGetRtdText(const ndefType *text, uint8_t *utfEncoding, ndefConstBuffer8 *bufLanguageCode, ndefConstBuffer *bufSentence);


    /*!
     *****************************************************************************
     * Convert an NDEF record to a Text type
     *
     * \param[in]  record: Record to convert
     * \param[out] text:   The converted type
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRecordToRtdText(const ndefRecord *record, ndefType *text);


    /*!
     *****************************************************************************
     * Convert a Text RTD type to an NDEF record
     *
     * \param[in]  text:   Type to convert
     * \param[out] record: The converted type
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRtdTextToRecord(const ndefType *text, ndefRecord *record);


    /***************
     * URI
     ***************
     */

    /*!
     *****************************************************************************
     * Initialize a URI RTD type
     *
     * \param[out] uri:          Type to initialize
     * \param[in]  protocol:     URI protocol
     * \param[in]  bufUriString: URI string buffer
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRtdUri(ndefType *uri, uint8_t protocol, const ndefConstBuffer *bufUriString);


    /*!
     *****************************************************************************
     * Get URI RTD type content
     *
     * \param[in]  uri:          Type to get information from
     * \param[out] bufProtocol:  URI protocol buffer
     * \param[out] bufUriString: URI string buffer
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefGetRtdUri(const ndefType *uri, ndefConstBuffer *bufProtocol, ndefConstBuffer *bufUriString);


    /*!
     *****************************************************************************
     * Convert an NDEF record to a URI RTD type
     *
     * \param[in]  record: Record to convert
     * \param[out] uri:    The converted type
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRecordToRtdUri(const ndefRecord *record, ndefType *uri);


    /*!
     *****************************************************************************
     * Convert a URI RTD type to an NDEF record
     *
     * \param[in]  uri:    Type to convert
     * \param[out] record: The converted type
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRtdUriToRecord(const ndefType *uri, ndefRecord *record);


    /*******************
     * AAR External Type
     *******************
     */

    /*!
     *****************************************************************************
     * Initialize an RTD Android Application Record External type
     *
     * \param[out] aar:        Type to initialize
     * \param[in]  bufPayload: Payload buffer
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRtdAar(ndefType *aar, const ndefConstBuffer *bufPayload);


    /*!
     *****************************************************************************
     * Get RTD Android Application Record type content
     *
     * \param[in]  aar:          Type to get information from
     * \param[out] bufAarString: AAR string buffer
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefGetRtdAar(const ndefType *aar, ndefConstBuffer *bufAarString);


    /*!
     *****************************************************************************
     * Convert an NDEF record to an RTD Android Application Record External type
     *
     * \param[in]  record: Record to convert
     * \param[out] aar:    The converted AAR external type
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRecordToRtdAar(const ndefRecord *record, ndefType *aar);


    /*!
     *****************************************************************************
     * Convert an RTD Android Application Record External type to an NDEF record
     *
     * \param[in]  aar:    AAR External type to convert
     * \param[out] record: The converted type
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRtdAarToRecord(const ndefType *aar, ndefRecord *record);


    /*
    ******************************************************************************
    * NDEF TYPES RECORD FUNCTION PROTOTYPES
    ******************************************************************************
    */


    /*!
     *****************************************************************************
     * Return an NDEF record type Id
     *
     * \param[in]  record: Record to get the type from
     * \param[out] typeId: The record RTD type
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRecordTypeStringToTypeId(const ndefRecord *record, ndefTypeId *typeId);


    /*!
     *****************************************************************************
     * Convert a record to a supported type
     *
     * \param[in]  record: Record to read
     * \param[out] type:   Type to store the converted record
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRecordToType(const ndefRecord *record, ndefType *type);


    /*!
     *****************************************************************************
     * Convert a supported type to a record
     *
     * \param[in]  type:    Pointer to the type to read data from
     * \param[out] record:  Record to fill
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefTypeToRecord(const ndefType *type, ndefRecord *record);


    /*!
     *****************************************************************************
     * Set the NDEF specific structure to process NDEF types
     *
     * \param[in]  record: Record
     * \param[out] type:   NDEF type structure
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    ReturnCode ndefRecordSetNdefType(ndefRecord *record, const ndefType *type);


    /*!
     *****************************************************************************
     * Get the NDEF type structure of this record
     *
     * \param[in] record: Record
     *
     * \return ERR_NONE if successful or a standard error code
     *****************************************************************************
     */
    const ndefType *ndefRecordGetNdefType(const ndefRecord *record);

    rfalNfcDevice                device;                       /*!< ndef Device                                        */
    ndefDeviceType               type;                         /*!< ndef Device type                                   */
    ndefState                    state;                        /*!< Tag state e.g. NDEF_STATE_INITIALIZED              */
    ndefCapabilityContainer      cc;                           /*!< Capability Container                               */
    uint32_t                     messageLen;                   /*!< NDEF message len                                   */
    uint32_t                     messageOffset;                /*!< NDEF message offset                                */
    uint32_t                     areaLen;                      /*!< Area Len for NDEF storage                          */
    uint8_t                      ccBuf[NDEF_CC_BUF_LEN];       /*!< buffer for CC                                      */
    union {
      ndefT1TContext t1t;                                    /*!< T1T context                                        */
      ndefT2TContext t2t;                                    /*!< T2T context                                        */
      ndefT3TContext t3t;                                    /*!< T3T context                                        */
      ndefT4TContext t4t;                                    /*!< T4T context                                        */
      ndefT5TContext t5t;                                    /*!< T5T context                                        */
    } subCtx;                                                  /*!< Sub-context union                                  */


  protected:

    ndefDeviceType ndefPollerGetDeviceType(rfalNfcDevice *dev);
    ReturnCode ndefT2TPollerReadBlock(uint16_t blockAddr, uint8_t *buf);
    ReturnCode ndefT2TPollerWriteBlock(uint16_t blockAddr, const uint8_t *buf);
    ReturnCode ndefT3TPollerReadBlocks(uint16_t blockNum, uint8_t nbBlocks, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen);
    ReturnCode ndefT3TPollerReadAttributeInformationBlock();
    ReturnCode ndefT3TPollerWriteBlocks(uint16_t blockNum, uint8_t nbBlocks, const uint8_t *dataBlocks);
    ReturnCode ndefT3TPollerWriteAttributeInformationBlock();
    void ndefT4TInitializeIsoDepTxRxParam(rfalIsoDepApduTxRxParam *isoDepAPDU);
    ReturnCode ndefT4TTransceiveTxRx(rfalIsoDepApduTxRxParam *isoDepAPDU);
    ReturnCode ndefT4TReadAndParseCCFile();
    ReturnCode ndefT5TPollerReadSingleBlock(uint16_t blockNum, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen);
    ReturnCode ndefT5TGetSystemInformation(bool extended);
    ReturnCode ndefT5TWriteCC();
    ReturnCode ndefT5TPollerWriteSingleBlock(uint16_t blockNum, const uint8_t *wrData);
    ReturnCode ndefT5TPollerReadMultipleBlocks(uint16_t firstBlockNum, uint8_t numOfBlocks, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen);
    ndefRecord *ndefAllocRecord(void);
    ReturnCode ndefRecordPayloadEncode(const ndefRecord *record, ndefBuffer *bufPayload);
    ReturnCode ndefPayloadToWifi(const ndefConstBuffer *bufPayload, ndefType *wifi);
    ReturnCode ndefVCardFindMarker(const ndefConstBuffer *bufPayload, const ndefConstBuffer *bufMarker, uint32_t *offset);
    ReturnCode ndefVCardExtractLine(const ndefConstBuffer *bufPayload, const ndefConstBuffer *bufMarker, ndefConstBuffer *bufLine);
    ReturnCode ndefVCardParseLine(const ndefConstBuffer *bufLine, ndefVCardEntry *vCardEntry);
    bool ndefIsVCardTypeFound(const ndefTypeVCard *vCard, const uint8_t *typeString);
    ReturnCode ndefPayloadToVcard(const ndefConstBuffer *bufPayload, ndefType *vCard);
    ReturnCode ndefPayloadToRtdDeviceInfo(const ndefConstBuffer *bufDevInfo, ndefType *devInfo);
    ReturnCode ndefPayloadToRtdText(const ndefConstBuffer *bufText, ndefType *text);
    ReturnCode ndefRtdUriProtocolAutodetect(uint8_t *protocol, ndefConstBuffer *bufUriString);
    ReturnCode ndefPayloadToRtdUri(const ndefConstBuffer *bufUri, ndefType *uri);
    ReturnCode ndefTypeStringToTypeId(uint8_t tnf, const ndefConstBuffer8 *bufTypeString, ndefTypeId *typeId);

    RfalNfcClass *rfal_nfc;

    uint8_t ndefRecordPoolIndex;
    ndefRecord ndefRecordPool[NDEF_MAX_RECORD];
};

#endif /* NDEF_CLASS_H */

/**
  * @}
  *
  */
