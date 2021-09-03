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
 *  \brief Provides NDEF methods and definitions to access NFC Forum T4T
 *
 *  This module provides an interface to perform as a NFC Reader/Writer
 *  to handle a Type 4 Tag T4T
 *
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "ndef_class.h"

/*
 ******************************************************************************
 * ENABLE SWITCH
 ******************************************************************************
 */

/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */

#define NDEF_T4T_MAPPING_VERSION_2_0  0x20U                    /*!< Mapping version 2.0                   */
#define NDEF_T4T_MAPPING_VERSION_3_0  0x30U                    /*!< Mapping version 3.0                   */

#define NDEF_T4T_MAX_RAPDU_BODY_LEN                            256U                          /*!< Maximum Response-APDU response body length (short field coding) */
#define NDEF_T4T_MAX_RAPDU_LEN (NDEF_T4T_MAX_RAPDU_BODY_LEN + RFAL_T4T_MAX_RAPDU_SW1SW2_LEN) /*!< Maximum Response-APDU Length (short field coding)               */

#define NDEF_T4T_MAX_CAPDU_BODY_LEN                            255U                          /*!< Maximum Command-APDU data length (short field coding)           */
#define NDEF_T4T_MAX_CAPDU_LEN ( RFAL_T4T_MAX_CAPDU_PROLOGUE_LEN \
                               + RFAL_T4T_LC_LEN \
                               + NDEF_T4T_MAX_CAPDU_BODY_LEN \
                               + RFAL_T4T_LC_LEN )                                           /*!< Maximum Command-APDU Length (short field coding)                */


#define NDEF_T4T_FID_SIZE              2U        /*!< File Id size                                      */
#define NDEF_T4T_WRITE_ODO_PREFIX_SIZE 7U        /*!< Size of ODO for Write Binary: 54 03 xxyyzz 53 Ld  */

#define NDEF_T4T_DEFAULT_MLC      0x000DU        /*!< Defauit Max Lc value before reading CCFILE values */
#define NDEF_T4T_DEFAULT_MLE      0x000FU        /*!< Defauit Max Le value before reading CCFILE values */

#define NDEF_T4T_OFFSET_MAX       0x7FFFU        /*!< Maximum offset value for ReadBinary               */
#define NDEF_T4T_ODO_OFFSET_MAX 0xFFFFFEU        /*!< Maximum offset value for ReadBinary with ODO      */

#define NDEF_T4T_CCFILEV2_LEN         15U        /*!< CCFILE Len mapping version V2                     */
#define NDEF_T4T_CCFILEV3_LEN         17U        /*!< CCFILE Len mapping version V3                     */

#define NDEF_T4T_NDEF_CTLV_T        0x04U        /*!< NDEF-File_Ctrl_TLV T field value                  */
#define NDEF_T4T_ENDEF_CTLV_T       0x06U        /*!< ENDEF-File_Ctrl_TLV T field value                 */

#define NDEF_T4T_NDEF_CTLV_L        0x06U        /*!< NDEF-File_Ctrl_TLV T field value                  */
#define NDEF_T4T_ENDEF_CTLV_L       0x08U        /*!< ENDEF-File_Ctrl_TLV T field value                 */

#define NDEF_T4T_MIN_VALID_MLE    0x000FU        /*!< Min valid MLe. MLe valid range 000Fh-FFFFh        */
#define NDEF_T4T_MIN_VALID_MLC    0x000DU        /*!< Min valid MLc. MLc valid range 000Dh-FFFFh        */

#define NDEF_T4T_NLEN_LEN              2U        /*!< NLEN LEN (mapping version v2.0): 2 bytes          */
#define NDEF_T4T_ENLEN_LEN             4U        /*!< ENLEN LEN (mapping version v3.0): 4 bytes         */

#define NDEF_T4T_MIN_NLEN              3U        /*!< Minimum non null NLEN value. TS T4T v1.0 B        */

#define NDEF_T4T_MV2_MAX_OFSSET   0x7FFFU        /*!< ReadBinary maximum Offset (offset range 0000-7FFFh)*/

#define NDEF_T4T_MAX_MLE             255U        /*!< Maximum MLe value supported in this implementation (short field coding). Le=0 (MLe=256) not supported by some tag. */
#define NDEF_T4T_MAX_MLC             255U        /*!< Maximum MLc value supported in this implementation (short field coding).                                           */

/*
 ******************************************************************************
 * GLOBAL TYPES
 ******************************************************************************
 */

/*
 ******************************************************************************
 * GLOBAL MACROS
 ******************************************************************************
 */

#define ndefT4TIsReadAccessGranted(r)  ( ((r) == 0x00U) || (((r) >= 0x80U) && ((r) <= 0xFEU)) ) /*!< Read access status  */
#define ndefT4TIsWriteAccessGranted(w) ( ((w) == 0x00U) || (((w) >= 0x80U) && ((w) <= 0xFEU)) ) /*!< Write access status */

#define ndefT4TisT4TDevice(device) ((((device)->type == RFAL_NFC_LISTEN_TYPE_NFCA) && ((device)->dev.nfca.type == RFAL_NFCA_T4T)) || ((device)->type == RFAL_NFC_LISTEN_TYPE_NFCB))

/*
 ******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************
 */

/*
 ******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 ******************************************************************************
 */

/*
 ******************************************************************************
 * GLOBAL FUNCTIONS
 ******************************************************************************
 */

/*******************************************************************************/
void NdefClass::ndefT4TInitializeIsoDepTxRxParam(rfalIsoDepApduTxRxParam *isoDepAPDU)
{
  /* Initialize the ISO-DEP protocol transceive context */
  isoDepAPDU->txBuf        = &subCtx.t4t.cApduBuf;
  isoDepAPDU->DID          = device.proto.isoDep.info.DID;
  isoDepAPDU->FWT          = device.proto.isoDep.info.FWT;
  isoDepAPDU->dFWT         = device.proto.isoDep.info.dFWT;
  isoDepAPDU->FSx          = device.proto.isoDep.info.FSx;
  isoDepAPDU->ourFSx       = RFAL_ISODEP_FSX_KEEP;
  isoDepAPDU->rxBuf        = &subCtx.t4t.rApduBuf;
  isoDepAPDU->tmpBuf       = &subCtx.t4t.tmpBuf;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT4TTransceiveTxRx(rfalIsoDepApduTxRxParam *isoDepAPDU)
{
  ReturnCode               ret;

  /* Initialize respAPDU */
  subCtx.t4t.respAPDU.rApduBuf = &subCtx.t4t.rApduBuf;
  isoDepAPDU->rxLen                 = &subCtx.t4t.respAPDU.rcvdLen;

  ret = rfal_nfc->rfalIsoDepStartApduTransceive(*isoDepAPDU);
  if (ret == ERR_NONE) {
    do {
      /* Blocking implementation, T4T may define rather long timeouts */
      (rfal_nfc->getRfalRf())->rfalWorker();
      ret = rfal_nfc->rfalIsoDepGetApduTransceiveStatus();
    } while (ret == ERR_BUSY);
  }

  if (ret != ERR_NONE) {
    return ret;
  }

  ret = rfal_nfc->rfalT4TPollerParseRAPDU(&subCtx.t4t.respAPDU);
  subCtx.t4t.rApduBodyLen = subCtx.t4t.respAPDU.rApduBodyLen;

  return ret;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT4TReadAndParseCCFile()
{
  static const uint8_t RFAL_T4T_FID_CC[]      = {0xE1, 0x03};                                /*!< FID_CC-File               T4T 1.0  4.2   */

  ReturnCode           ret;
  uint8_t              dataIt;

  /* Select CCFILE TS T4T v1.0 7.2.1.3 */
  ret =  ndefT4TPollerSelectFile(RFAL_T4T_FID_CC);
  if (ret != ERR_NONE) {
    /* Conclude procedure TS T4T v1.0 7.2.1.4 */
    return ret;
  }

  /* Read CCFILE TS T4T v1.0 7.2.1.5 */
  /* read CCFILE assuming at least 15 bytes are available. If V3 found will read the extra bytes in a second step */
  ret = ndefT4TPollerReadBinary(0U, NDEF_T4T_CCFILEV2_LEN);
  if (ret != ERR_NONE) {
    /* Conclude procedure TS T4T v1.0 7.2.1.6 */
    return ret;
  }
  (void)ST_MEMCPY(ccBuf, subCtx.t4t.rApduBuf.apdu, NDEF_T4T_CCFILEV2_LEN);
  dataIt = 0;
  cc.t4t.ccLen = GETU16(&ccBuf[dataIt]);
  dataIt += (uint8_t)sizeof(uint16_t);
  cc.t4t.vNo   = ccBuf[dataIt];
  dataIt++;
  cc.t4t.mLe   = GETU16(&ccBuf[dataIt]);
  dataIt += (uint8_t)sizeof(uint16_t);
  cc.t4t.mLc   = GETU16(&ccBuf[dataIt]);
  dataIt += (uint8_t)sizeof(uint16_t);

  /* TS T4T v1.0 7.2.1.7 verify MLe and MLc are within the valid range */
  if ((cc.t4t.mLe < NDEF_T4T_MIN_VALID_MLE) || (cc.t4t.mLc < NDEF_T4T_MIN_VALID_MLC)) {
    /* Conclude procedure TS T4T v1.0 7.2.1.8 */
    return ERR_REQUEST;
  }

  subCtx.t4t.curMLe   = (uint8_t)MIN(cc.t4t.mLe, NDEF_T4T_MAX_MLE); /* Only short field codind supported */
  subCtx.t4t.curMLc   = (uint8_t)MIN(cc.t4t.mLc, NDEF_T4T_MAX_MLC); /* Only short field codind supported */

  /* TS T4T v1.0 7.2.1.7 and 4.3.2.4 verify support of mapping version */
  if (ndefMajorVersion(cc.t4t.vNo) > ndefMajorVersion(NDEF_T4T_MAPPING_VERSION_3_0)) {
    /* Conclude procedure TS T4T v1.0 7.2.1.8 */
    return ERR_REQUEST;
  }
  if (ndefMajorVersion(cc.t4t.vNo) == ndefMajorVersion(NDEF_T4T_MAPPING_VERSION_3_0)) {
    /* V3 found: read remainng bytes */
    ret = ndefT4TPollerReadBinary(NDEF_T4T_CCFILEV2_LEN, NDEF_T4T_CCFILEV3_LEN - NDEF_T4T_CCFILEV2_LEN);
    if (ret != ERR_NONE) {
      /* Conclude procedure TS T4T v1.0 7.2.1.6 */
      return ret;
    }
    (void)ST_MEMCPY(&ccBuf[NDEF_T4T_CCFILEV2_LEN], subCtx.t4t.rApduBuf.apdu, NDEF_T4T_CCFILEV3_LEN - NDEF_T4T_CCFILEV2_LEN);

    /* TS T4T v1.0 7.2.1.7 verify coding as in table 5 */
    if (ccBuf[dataIt] != NDEF_T4T_ENDEF_CTLV_T) {
      /* Conclude procedure TS T4T v1.0 7.2.1.8 */
      return ERR_REQUEST;
    }
    dataIt++;
    if (ccBuf[dataIt] < NDEF_T4T_ENDEF_CTLV_L) {
      /* Conclude procedure TS T4T v1.0 7.2.1.8 */
      return ERR_REQUEST;
    }
    dataIt++;
    cc.t4t.fileId[0U]   = ccBuf[dataIt];
    dataIt++;
    cc.t4t.fileId[1U]   = ccBuf[dataIt];
    dataIt++;
    cc.t4t.fileSize    = GETU32(&ccBuf[dataIt]);
    dataIt += (uint8_t)sizeof(uint32_t);
    cc.t4t.readAccess  = ccBuf[dataIt];
    dataIt++;
    cc.t4t.writeAccess = ccBuf[dataIt];
    dataIt++;
  } else {
    if (ccBuf[dataIt] != NDEF_T4T_NDEF_CTLV_T) {
      return ERR_REQUEST;
    }
    dataIt++;
    if (ccBuf[dataIt] < NDEF_T4T_NDEF_CTLV_L) {
      return ERR_REQUEST;
    }
    dataIt++;
    cc.t4t.fileId[0U]   = ccBuf[dataIt];
    dataIt++;
    cc.t4t.fileId[1U]   = ccBuf[dataIt];
    dataIt++;
    cc.t4t.fileSize    = ndefBytes2Uint16(ccBuf[dataIt], ccBuf[dataIt + 1U]);
    dataIt += (uint8_t)sizeof(uint16_t);
    cc.t4t.readAccess  = ccBuf[dataIt];
    dataIt++;
    cc.t4t.writeAccess = ccBuf[dataIt];
    dataIt++;
  }
  return ERR_NONE;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT4TPollerSelectNdefTagApplication()
{
  ReturnCode               ret;
  rfalIsoDepApduTxRxParam  isoDepAPDU;
  static const uint8_t NDEF_T4T_AID_NDEF[]    = {0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01};  /*!< AID_NDEF v2.0 or higher   T4T 1.0  4.3.3 */
  static const uint8_t NDEF_T4T_AID_NDEF_V1[] = {0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x00};  /*!< AID_NDEF v1.0             T4T 1.0  4.3.3 */

  if (!ndefT4TisT4TDevice(&device)) {
    return ERR_PARAM;
  }

  ndefT4TInitializeIsoDepTxRxParam(&isoDepAPDU);
  (void)rfal_nfc->rfalT4TPollerComposeSelectAppl(isoDepAPDU.txBuf, NDEF_T4T_AID_NDEF, (uint8_t)sizeof(NDEF_T4T_AID_NDEF), &isoDepAPDU.txBufLen);
  ret = ndefT4TTransceiveTxRx(&isoDepAPDU);

  if (ret == ERR_NONE) {
    /* application v2 or higher found */
    subCtx.t4t.mv1Flag = false;
    return ret;
  }

  if (ret != ERR_REQUEST) {
    return ret;
  }

  /* if v2 application not found, try v1 */
  (void)rfal_nfc->rfalT4TPollerComposeSelectAppl(isoDepAPDU.txBuf, NDEF_T4T_AID_NDEF_V1, (uint8_t)sizeof(NDEF_T4T_AID_NDEF_V1), &isoDepAPDU.txBufLen);
  ret = ndefT4TTransceiveTxRx(&isoDepAPDU);

  if (ret == ERR_NONE) {
    /* application v1 found */
    subCtx.t4t.mv1Flag = true;
  }
  return ret;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT4TPollerSelectFile(const uint8_t *fileId)
{
  ReturnCode               ret;
  rfalIsoDepApduTxRxParam  isoDepAPDU;

  if (!ndefT4TisT4TDevice(&device)) {
    return ERR_PARAM;
  }

  ndefT4TInitializeIsoDepTxRxParam(&isoDepAPDU);

  if (subCtx.t4t.mv1Flag) {
    (void)rfal_nfc->rfalT4TPollerComposeSelectFileV1Mapping(isoDepAPDU.txBuf, fileId, (uint8_t)sizeof(fileId), &isoDepAPDU.txBufLen);
  } else {
    (void)rfal_nfc->rfalT4TPollerComposeSelectFile(isoDepAPDU.txBuf, fileId, NDEF_T4T_FID_SIZE, &isoDepAPDU.txBufLen);
  }

  ret = ndefT4TTransceiveTxRx(&isoDepAPDU);

  return ret;
}


/*******************************************************************************/
ReturnCode NdefClass::ndefT4TPollerReadBinary(uint16_t offset, uint8_t len)
{
  ReturnCode               ret;
  rfalIsoDepApduTxRxParam  isoDepAPDU;

  if (!ndefT4TisT4TDevice(&device) || (len >  subCtx.t4t.curMLe) || (offset > NDEF_T4T_OFFSET_MAX)) {
    return ERR_PARAM;
  }

  ndefT4TInitializeIsoDepTxRxParam(&isoDepAPDU);
  (void)rfal_nfc->rfalT4TPollerComposeReadData(isoDepAPDU.txBuf, offset, len, &isoDepAPDU.txBufLen);
  ret = ndefT4TTransceiveTxRx(&isoDepAPDU);

  return ret;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT4TPollerReadBinaryODO(uint32_t offset, uint8_t len)
{
  ReturnCode               ret;
  rfalIsoDepApduTxRxParam  isoDepAPDU;

  if (!ndefT4TisT4TDevice(&device) || (len >  subCtx.t4t.curMLe)  || (offset > NDEF_T4T_ODO_OFFSET_MAX)) {
    return ERR_PARAM;
  }

  ndefT4TInitializeIsoDepTxRxParam(&isoDepAPDU);
  (void)rfal_nfc->rfalT4TPollerComposeReadDataODO(isoDepAPDU.txBuf, offset, len, &isoDepAPDU.txBufLen);
  ret = ndefT4TTransceiveTxRx(&isoDepAPDU);

  return ret;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT4TPollerReadBytes(uint32_t offset, uint32_t len, uint8_t *buf, uint32_t *rcvdLen)
{
  ReturnCode           ret;
  uint8_t              le;
  uint32_t             lvOffset = offset;
  uint32_t             lvLen    = len;
  uint8_t             *lvBuf    = buf;

  if (!ndefT4TisT4TDevice(&device) || (lvLen == 0U)) {
    return ERR_PARAM;
  }
  if (rcvdLen != NULL) {
    *rcvdLen = 0U;
  }

  do {
    le = (lvLen > subCtx.t4t.curMLe) ? subCtx.t4t.curMLe : (uint8_t)lvLen;
    if (lvOffset > NDEF_T4T_MV2_MAX_OFSSET) {
      ret = ndefT4TPollerReadBinaryODO(lvOffset, le);
    } else {
      ret = ndefT4TPollerReadBinary((uint16_t)lvOffset, le);
    }
    if (ret != ERR_NONE) {
      return ret;
    }
    if (subCtx.t4t.rApduBodyLen == 0U) {
      break; /* no more to read */
    }
    if (subCtx.t4t.rApduBodyLen >  lvLen) {
      return ERR_SYSTEM;
    }
    (void)ST_MEMCPY(lvBuf, subCtx.t4t.rApduBuf.apdu, subCtx.t4t.rApduBodyLen);
    lvBuf     = &lvBuf[subCtx.t4t.rApduBodyLen];
    lvOffset += subCtx.t4t.rApduBodyLen;
    lvLen    -= subCtx.t4t.rApduBodyLen;
    if (rcvdLen != NULL) {
      *rcvdLen += subCtx.t4t.rApduBodyLen;
    }
  } while (lvLen != 0U);

  return ERR_NONE;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT4TPollerContextInitialization(rfalNfcDevice *dev)
{
  if ((dev == NULL) || !ndefT4TisT4TDevice(dev)) {
    return ERR_PARAM;
  }

  (void)ST_MEMCPY(&device, dev, sizeof(device));

  state             = NDEF_STATE_INVALID;
  subCtx.t4t.curMLc = NDEF_T4T_DEFAULT_MLC;
  subCtx.t4t.curMLe = NDEF_T4T_DEFAULT_MLE;

  return ERR_NONE;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT4TPollerNdefDetect(ndefInfo *info)
{
  ReturnCode           ret;
  uint8_t             *nLen;
  uint8_t              nlenLen;

  if (info != NULL) {
    info->state                = NDEF_STATE_INVALID;
    info->majorVersion         = 0U;
    info->minorVersion         = 0U;
    info->areaLen              = 0U;
    info->areaAvalableSpaceLen = 0U;
    info->messageLen           = 0U;
  }

  if (!ndefT4TisT4TDevice(&device)) {
    return ERR_PARAM;
  }

  state = NDEF_STATE_INVALID;

  /* Select NDEF Tag application TS T4T v1.0 7.2.1.1 */
  ret =  ndefT4TPollerSelectNdefTagApplication();
  if (ret != ERR_NONE) {
    /* Conclude procedure TS T4T v1.0 7.2.1.2 */
    return ret;
  }

  /* TS T4T v1.0 7.2.1.3 and following */
  ret = ndefT4TReadAndParseCCFile();
  if (ret != ERR_NONE) {
    return ret;
  }
  nlenLen = (ndefMajorVersion(cc.t4t.vNo) == ndefMajorVersion(NDEF_T4T_MAPPING_VERSION_3_0)) ? NDEF_T4T_ENLEN_LEN : NDEF_T4T_NLEN_LEN;

  /* TS T4T v1.0 7.2.1.7 verify file READ access */
  if (!(ndefT4TIsReadAccessGranted(cc.t4t.readAccess))) {
    /* Conclude procedure TS T4T v1.0 7.2.1.8 */
    return ERR_REQUEST;
  }
  /* File size need at least be enough to store NLEN or ENLEN */
  if (cc.t4t.fileSize < nlenLen) {
    return ERR_REQUEST;
  }

  /* Select NDEF File TS T4T v1.0 7.2.1.9 */
  ret =  ndefT4TPollerSelectFile(cc.t4t.fileId);
  if (ret != ERR_NONE) {
    /* Conclude procedure TS T4T v1.0 7.2.1.10 */
    return ret;
  }
  /* Read NLEN/ENLEN TS T4T v1.0 7.2.1.11 */
  ret = ndefT4TPollerReadBinary(0U, nlenLen);
  if (ret != ERR_NONE) {
    /* Conclude procedure TS T4T v1.0 7.2.1.11 */
    return ret;
  }
  nLen = subCtx.t4t.rApduBuf.apdu;
  messageLen    = (nlenLen == NDEF_T4T_ENLEN_LEN) ?  GETU32(&nLen[0]) : (uint32_t)ndefBytes2Uint16(nLen[0], nLen[1]);
  messageOffset = nlenLen;
  areaLen       = cc.t4t.fileSize;

  if ((messageLen > (cc.t4t.fileSize - nlenLen)) || ((messageLen > 0U) && (messageLen < NDEF_T4T_MIN_NLEN))) {
    /* Conclude procedure TS T4T v1.0 7.2.1.11 */
    return ERR_REQUEST;
  }

  if (messageLen == 0U) {
    if (!(ndefT4TIsWriteAccessGranted(cc.t4t.writeAccess))) {
      /* Conclude procedure TS T4T v1.0 7.2.1.11 */
      return ERR_REQUEST;
    }
    state = NDEF_STATE_INITIALIZED;
  } else {
    state = (ndefT4TIsWriteAccessGranted(cc.t4t.writeAccess)) ? NDEF_STATE_READWRITE : NDEF_STATE_READONLY;
  }
  if (info != NULL) {
    info->state                = state;
    info->majorVersion         = ndefMajorVersion(cc.t4t.vNo);
    info->minorVersion         = ndefMinorVersion(cc.t4t.vNo);
    info->areaLen              = areaLen;
    info->areaAvalableSpaceLen = areaLen - messageOffset;
    info->messageLen           = messageLen;
  }

  return ERR_NONE;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT4TPollerReadRawMessage(uint8_t *buf, uint32_t bufLen, uint32_t *rcvdLen)
{
  ReturnCode           ret;

  if (!ndefT4TisT4TDevice(&device) || (buf == NULL)) {
    return ERR_PARAM;
  }
  /* TS T4T v1.0 7.2.2.1: T4T NDEF Detect should have been called before NDEF read procedure */
  /* Warning: current selected file must not be changed between NDEF Detect procedure and NDEF read procedure*/

  /* TS T4T v1.0 7.3.3.2: check presence of NDEF message */
  if (state <= NDEF_STATE_INITIALIZED) {
    /* Conclude procedure TS T4T v1.0 7.2.2.2 */
    return ERR_WRONG_STATE;
  }

  if (messageLen > bufLen) {
    return ERR_NOMEM;
  }

  /* TS T4T v1.0 7.3.3.3: read the NDEF message */
  ret = ndefT4TPollerReadBytes(messageOffset, messageLen, buf, rcvdLen);
  if (ret != ERR_NONE) {
    state = NDEF_STATE_INVALID;
  }

  return ret;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT4TPollerWriteBinary(uint16_t offset, const uint8_t *data, uint8_t len)
{
  ReturnCode               ret;
  rfalIsoDepApduTxRxParam  isoDepAPDU;

  if (!ndefT4TisT4TDevice(&device) || (len >  subCtx.t4t.curMLc) || (offset > NDEF_T4T_OFFSET_MAX)) {
    return ERR_PARAM;
  }

  ndefT4TInitializeIsoDepTxRxParam(&isoDepAPDU);
  (void)rfal_nfc->rfalT4TPollerComposeWriteData(isoDepAPDU.txBuf, offset, data, len, &isoDepAPDU.txBufLen);
  ret = ndefT4TTransceiveTxRx(&isoDepAPDU);

  return ret;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT4TPollerWriteBinaryODO(uint32_t offset, const uint8_t *data, uint8_t len)
{
  ReturnCode               ret;
  rfalIsoDepApduTxRxParam  isoDepAPDU;

  if (!ndefT4TisT4TDevice(&device) || (len >  subCtx.t4t.curMLc) || (offset > NDEF_T4T_ODO_OFFSET_MAX)) {
    return ERR_PARAM;
  }

  ndefT4TInitializeIsoDepTxRxParam(&isoDepAPDU);
  (void)rfal_nfc->rfalT4TPollerComposeWriteDataODO(isoDepAPDU.txBuf, offset, data, len, &isoDepAPDU.txBufLen);
  ret = ndefT4TTransceiveTxRx(&isoDepAPDU);

  return ret;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT4TPollerWriteBytes(uint32_t offset, const uint8_t *buf, uint32_t len)
{
  ReturnCode           ret;
  uint8_t              lc;
  uint32_t             lvOffset = offset;
  uint32_t             lvLen    = len;
  const uint8_t       *lvBuf    = buf;

  if (!ndefT4TisT4TDevice(&device) || (lvLen == 0U)) {
    return ERR_PARAM;
  }

  do {

    if (lvOffset > NDEF_T4T_MV2_MAX_OFSSET) {
      lc = (lvLen > ((uint32_t)subCtx.t4t.curMLc - NDEF_T4T_WRITE_ODO_PREFIX_SIZE)) ? (uint8_t)(subCtx.t4t.curMLc - NDEF_T4T_WRITE_ODO_PREFIX_SIZE) : (uint8_t)lvLen;
      ret = ndefT4TPollerWriteBinaryODO(lvOffset, lvBuf, lc);
    } else {
      lc = (lvLen > subCtx.t4t.curMLc) ? subCtx.t4t.curMLc : (uint8_t)lvLen;
      ret = ndefT4TPollerWriteBinary((uint16_t)lvOffset, lvBuf, lc);
    }
    if (ret != ERR_NONE) {
      return ret;
    }
    lvBuf     = &lvBuf[lc];
    lvOffset += lc;
    lvLen    -= lc;
  } while (lvLen != 0U);

  return ERR_NONE;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT4TPollerWriteRawMessageLen(uint32_t rawMessageLen)
{
  ReturnCode           ret;
  uint8_t              buf[NDEF_T4T_ENLEN_LEN];
  uint8_t              dataIt;

  if (!ndefT4TisT4TDevice(&device)) {
    return ERR_PARAM;
  }

  if ((state != NDEF_STATE_INITIALIZED) && (state != NDEF_STATE_READWRITE)) {
    return ERR_WRONG_STATE;
  }

  dataIt = 0U;
  if (ndefMajorVersion(cc.t4t.vNo) == ndefMajorVersion(NDEF_T4T_MAPPING_VERSION_3_0)) {
    buf[dataIt] = (uint8_t)(rawMessageLen >> 24U);
    dataIt++;
    buf[dataIt] = (uint8_t)(rawMessageLen >> 16U);
    dataIt++;
    buf[dataIt] = (uint8_t)(rawMessageLen >>  8U);
    dataIt++;
    buf[dataIt] = (uint8_t)(rawMessageLen);
    dataIt++;
  } else {
    buf[dataIt] = (uint8_t)(rawMessageLen >>  8U);
    dataIt++;
    buf[dataIt] = (uint8_t)(rawMessageLen);
    dataIt++;
  }

  ret = ndefT4TPollerWriteBytes(0U, buf, dataIt);
  return ret;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT4TPollerWriteRawMessage(const uint8_t *buf, uint32_t bufLen)
{
  ReturnCode           ret;

  if (!ndefT4TisT4TDevice(&device) || ((buf == NULL) && (bufLen != 0U))) {
    return ERR_PARAM;
  }

  /* TS T4T v1.0 7.2.3.1: T4T NDEF Detect should have been called before NDEF write procedure */
  /* Warning: current selected file must not be changed between NDEF Detect procedure and NDEF Write procedure*/

  /* TS T4T v1.0 7.3.3.2: check write access condition */
  if ((state != NDEF_STATE_INITIALIZED) && (state != NDEF_STATE_READWRITE)) {
    /* Conclude procedure TS T4T v1.0 7.2.3.2 */
    return ERR_WRONG_STATE;
  }

  /* TS T4T v1.0 7.2.3.3: check Mapping Version    */
  /* Done automatically inside underlying functions */

  /* TS T4T v1.0 7.2.3.4/8 verify length of the NDEF message */
  ret = ndefT4TPollerCheckAvailableSpace(bufLen);
  if (ret != ERR_NONE) {
    /* Conclude procedure TS T4T v1.0 7.2.3.4/8 */
    return ERR_PARAM;
  }

  /* TS T4T v1.0 7.2.3.5/9 Write value 0000h in NLEN field (resp. 00000000h in ENLEN field) */
  ret = ndefT4TPollerBeginWriteMessage(bufLen);
  if (ret != ERR_NONE) {
    state = NDEF_STATE_INVALID;
    /* Conclude procedure TS T4T v1.0 7.2.3.5/9 */
    return ret;
  }

  if (bufLen != 0U) {
    /* TS T4T v1.0 7.2.3.6/10 Write NDEF message) */
    ret = ndefT4TPollerWriteBytes(messageOffset, buf, bufLen);
    if (ret != ERR_NONE) {
      /* Conclude procedure TS T4T v1.0 7.2.3.6/10 */
      state = NDEF_STATE_INVALID;
      return ret;
    }

    /* TS T4T v1.0 7.2.3.7/11 Write value length in NLEN field (resp. in ENLEN field) */
    ret = ndefT4TPollerEndWriteMessage(bufLen);
    if (ret != ERR_NONE) {
      /* Conclude procedure TS T4T v1.0 7.2.3.7/11 */
      state = NDEF_STATE_INVALID;
      return ret;
    }
  }

  return ret;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT4TPollerTagFormat(const ndefCapabilityContainer *cc_p, uint32_t options)
{
  ReturnCode           ret;

  uint8_t              buf[NDEF_T4T_ENLEN_LEN];

  NO_WARNING(cc_p);
  NO_WARNING(options);

  if (!ndefT4TisT4TDevice(&device)) {
    return ERR_PARAM;
  }

  ret =  ndefT4TPollerSelectNdefTagApplication();
  if (ret != ERR_NONE) {
    return ret;
  }

  ret =  ndefT4TReadAndParseCCFile();
  if (ret != ERR_NONE) {
    return ret;
  }

  ret =  ndefT4TPollerSelectFile(cc.t4t.fileId);
  if (ret != ERR_NONE) {
    return ret;
  }
  (void)ST_MEMSET(buf, 0x00, sizeof(buf));
  ret = ndefT4TPollerWriteBytes(0U, buf, (ndefMajorVersion(cc.t4t.vNo) == ndefMajorVersion(NDEF_T4T_MAPPING_VERSION_3_0)) ? NDEF_T4T_ENLEN_LEN : NDEF_T4T_NLEN_LEN);
  return ret;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT4TPollerCheckPresence()
{
  rfalIsoDepApduTxRxParam  isoDepAPDU;
  ReturnCode               ret;

  if (!ndefT4TisT4TDevice(&device)) {
    return ERR_PARAM;
  }

  ndefT4TInitializeIsoDepTxRxParam(&isoDepAPDU);
  (void)rfal_nfc->rfalT4TPollerComposeReadData(isoDepAPDU.txBuf, 0, 1, &isoDepAPDU.txBufLen);

  /* Initialize respAPDU */
  subCtx.t4t.respAPDU.rApduBuf = &subCtx.t4t.rApduBuf;
  isoDepAPDU.rxLen                  = &subCtx.t4t.respAPDU.rcvdLen;

  (void)rfal_nfc->rfalIsoDepStartApduTransceive(isoDepAPDU);
  do {
    /* Blocking implementation, T4T may define rather long timeouts */
    (rfal_nfc->getRfalRf())->rfalWorker();
    ret = rfal_nfc->rfalIsoDepGetApduTransceiveStatus();
  } while (ret == ERR_BUSY);

  return ret;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT4TPollerCheckAvailableSpace(const uint32_t messageLen)
{
  uint8_t              nlenLen;

  if (!ndefT4TisT4TDevice(&device)) {
    return ERR_PARAM;
  }

  if (state == NDEF_STATE_INVALID) {
    return ERR_WRONG_STATE;
  }

  nlenLen = (ndefMajorVersion(cc.t4t.vNo) == ndefMajorVersion(NDEF_T4T_MAPPING_VERSION_3_0)) ? NDEF_T4T_ENLEN_LEN : NDEF_T4T_NLEN_LEN;
  if ((messageLen + (uint32_t)nlenLen) > cc.t4t.fileSize) {
    return ERR_NOMEM;
  }
  return ERR_NONE;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT4TPollerBeginWriteMessage(uint32_t messageLen)
{
  ReturnCode           ret;
  NO_WARNING(messageLen);

  if (!ndefT4TisT4TDevice(&device)) {
    return ERR_PARAM;
  }

  if ((state != NDEF_STATE_INITIALIZED) && (state != NDEF_STATE_READWRITE)) {
    return ERR_WRONG_STATE;
  }

  /* TS T4T v1.0 7.2.3.5/9 Write value 0000h in NLEN field (resp. 00000000h in ENLEN field) */
  ret = ndefT4TPollerWriteRawMessageLen(0U);
  if (ret != ERR_NONE) {
    /* Conclude procedure */
    state = NDEF_STATE_INVALID;
    return ret;
  }

  state = NDEF_STATE_INITIALIZED;

  return ERR_NONE;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT4TPollerEndWriteMessage(uint32_t messageLen)
{
  ReturnCode           ret;

  if (!ndefT4TisT4TDevice(&device)) {
    return ERR_PARAM;
  }

  if (state != NDEF_STATE_INITIALIZED) {
    return ERR_WRONG_STATE;
  }

  /* TS T4T v1.0 7.2.3.7/11 Write value length in NLEN field (resp. in ENLEN field) */
  ret = ndefT4TPollerWriteRawMessageLen(messageLen);
  if (ret != ERR_NONE) {
    /* Conclude procedure */
    state = NDEF_STATE_INVALID;
    return ret;
  }
  messageLen = messageLen;
  state = (messageLen == 0U) ? NDEF_STATE_INITIALIZED : NDEF_STATE_READWRITE;
  return ERR_NONE;
}
