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
 *  \brief Provides NDEF methods and definitions to access NFC-V Forum T5T
 *
 *  This module provides an interface to perform as a NFC-V Reader/Writer
 *  to handle a Type 5 Tag T5T
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

#define NDEF_T5T_CC_MAGIC_1_BYTE_ADDR_MODE                   0xE1U                       /*!< T5T CC Magic Number (1-byte Address Mode)           */
#define NDEF_T5T_CC_MAGIC_2_BYTE_ADDR_MODE                   0xE2U                       /*!< T5T CC Magic Number (2-byte Address Mode)           */
#define NDEF_T5T_CC_LEN_4_BYTES                                 4U                       /*!< T5T CC Length (4 bytes)                            */
#define NDEF_T5T_CC_LEN_8_BYTES                                 8U                       /*!< T5T CC Length (8 bytes)                            */
#define NDEF_T5T_FORMAT_OPTION_NFC_FORUM                        1U                       /*!< Format tag according to NFC Forum MLEN computation */

#define NDEF_T5T_UID_MANUFACTURER_ID_POS       6U    /*!< Manufacturer ID Offset in UID buffer (reverse)    */
#define NDEF_T5T_MANUFACTURER_ID_ST         0x02U    /*!< Manufacturer ID for ST                            */

#define NDEF_T5T_SYSINFO_MAX_LEN             22U     /*!< Max length for (Extended) Get System Info response                */

#define NDEF_T5T_MLEN_DIVIDER                  8U    /*!<  T5T_area size is measured in bytes is equal to 8 * MLEN          */

#define NDEF_T5T_TLV_L_3_BYTES_LEN             3U    /*!< TLV L Length: 3 bytes                             */
#define NDEF_T5T_TLV_L_1_BYTES_LEN             1U    /*!< TLV L Length: 1 bytes                             */
#define NDEF_T5T_TLV_T_LEN                     1U    /*!< TLV T Length: 1 bytes                             */

#define NDEF_T5T_MAX_BLOCK_1_BYTE_ADDR       256U    /*!< Max number of blocks for 1 byte addressing        */
#define NDEF_T5T_MAX_MLEN_1_BYTE_ENCODING    256U    /*!< MLEN max value for 1 byte encoding                */

#define NDEF_T5T_TL_MAX_SIZE  (NDEF_T5T_TLV_T_LEN \
                       + NDEF_T5T_TLV_L_3_BYTES_LEN) /*!< Max TL size                                       */

#define NDEF_T5T_TLV_NDEF                   0x03U    /*!< TLV flag NDEF value                               */
#define NDEF_T5T_TLV_PROPRIETARY            0xFDU    /*!< TLV flag PROPRIETARY value                        */
#define NDEF_T5T_TLV_TERMINATOR             0xFEU    /*!< TLV flag TERMINATOR value                         */
#define NDEF_T5T_TLV_RFU                    0x00U    /*!< TLV flag RFU value                                */

/*
 *****************************************************************************
 * GLOBAL TYPES
 ******************************************************************************
 */

/*
 ******************************************************************************
 * GLOBAL MACROS
 ******************************************************************************
 */


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
ReturnCode NdefClass::ndefT5TPollerReadBytes(uint32_t offset, uint32_t len, uint8_t *buf, uint32_t *rcvdLen)
{
  uint8_t         lastVal;
  uint8_t         status;
  uint16_t        res;
  uint16_t        nbRead;
  uint16_t        blockLen;
  uint16_t        startBlock;
  uint16_t        startAddr;
  ReturnCode      result     = ERR_PARAM;
  uint32_t        currentLen = len;
  uint32_t        lvRcvLen   = 0U;

  if ((subCtx.t5t.blockLen > 0U) && (buf != NULL) && (len > 0U)) {
    blockLen   = (uint16_t)subCtx.t5t.blockLen;
    if (blockLen == 0U) {
      return ERR_SYSTEM;
    }
    startBlock = (uint16_t)(offset / blockLen);
    startAddr  = (uint16_t)(startBlock * blockLen);

    res = ndefT5TPollerReadSingleBlock(startBlock, subCtx.t5t.txrxBuf, blockLen + 3U, &nbRead);
    if ((res == ERR_NONE) && (subCtx.t5t.txrxBuf[0U] == 0U) && (nbRead > 0U)) {
      nbRead = (uint16_t)(nbRead  + startAddr - (uint16_t)offset - 1U);
      if ((uint32_t) nbRead > currentLen) {
        nbRead = (uint16_t) currentLen;
      }
      if (nbRead > 0U) {
        (void)ST_MEMCPY(buf, &subCtx.t5t.txrxBuf[1U - startAddr + (uint16_t)offset], (uint32_t)nbRead);
      }
      lvRcvLen   += (uint32_t) nbRead;
      currentLen -= (uint32_t) nbRead;
      while (currentLen >= ((uint32_t)blockLen + 2U)) {
        startBlock++;
        lastVal = buf[lvRcvLen - 1U];
        res = ndefT5TPollerReadSingleBlock(startBlock, &buf[lvRcvLen - 1U], blockLen + 3U, &nbRead);
        status  = buf[lvRcvLen - 1U]; /* Keep status */
        buf[lvRcvLen - 1U] = lastVal; /* Restore previous value */
        if ((res == ERR_NONE) && (nbRead > 0U) && (status == 0U)) {
          lvRcvLen   += blockLen;
          currentLen -= blockLen;
        } else {
          break;
        }
      }
      while (currentLen > 0U) {
        startBlock++;
        res = ndefT5TPollerReadSingleBlock(startBlock, subCtx.t5t.txrxBuf, blockLen + 3U, &nbRead);
        if ((res == ERR_NONE) && (subCtx.t5t.txrxBuf[0U] == 0U) && (nbRead > 0U)) {
          -- nbRead; /* remove status char */
          if (nbRead > currentLen) {
            nbRead = (uint16_t)currentLen;
          }
          if (nbRead > 0U) {
            (void)ST_MEMCPY(&buf[lvRcvLen], & subCtx.t5t.txrxBuf[1U], nbRead);
          }
          lvRcvLen   += nbRead;
          currentLen -= nbRead;
        } else {
          break;
        }
      }
    }
  }
  if (currentLen == 0U) {
    result = ERR_NONE;
  }
  if (rcvdLen != NULL) {
    * rcvdLen = lvRcvLen;
  }
  return result;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT5TPollerContextInitialization(rfalNfcDevice *dev)
{
  ReturnCode    result;
  uint16_t      rcvLen;

  if ((dev == NULL) || !ndefT5TisT5TDevice(dev)) {
    return ERR_PARAM;
  }

  (void)ST_MEMCPY(&device, dev, sizeof(device));

  /* Reset info about the card */
  state                    = NDEF_STATE_INVALID;
  messageOffset            = 0U;
  messageLen               = 0U;
  subCtx.t5t.blockLen      = 0U;
  subCtx.t5t.pAddressedUid = device.dev.nfcv.InvRes.UID; /* By default work in addressed mode */
  subCtx.t5t.TlvNDEFOffset = 0U; /* Offset for TLV */

  subCtx.t5t.legacySTHighDensity = false;
  result = ndefT5TPollerReadSingleBlock(0U, subCtx.t5t.txrxBuf, (uint16_t)sizeof(subCtx.t5t.txrxBuf), &rcvLen);
  if ((result != ERR_NONE) && (device.dev.nfcv.InvRes.UID[NDEF_T5T_UID_MANUFACTURER_ID_POS] == NDEF_T5T_MANUFACTURER_ID_ST)) {
    /* Try High Density Legacy mode */
    subCtx.t5t.legacySTHighDensity = true;
    result = ndefT5TPollerReadSingleBlock(0U, subCtx.t5t.txrxBuf, (uint16_t)sizeof(subCtx.t5t.txrxBuf), &rcvLen);
    if (result != ERR_NONE) {
      return result;
    }
  }

  if ((rcvLen > 1U) && (subCtx.t5t.txrxBuf[0U] == (uint8_t) 0U)) {
    subCtx.t5t.blockLen = (uint8_t)(rcvLen - 1U);
  } else {
    return ERR_PROTO;
  }

  if (rfal_nfc->rfalNfcvPollerSelect((uint8_t)RFAL_NFCV_REQ_FLAG_DEFAULT, device.dev.nfcv.InvRes.UID)  == ERR_NONE) {
    subCtx.t5t.pAddressedUid = NULL; /* Switch to selected mode */
  }

  subCtx.t5t.sysInfoSupported = false;

  if (!subCtx.t5t.legacySTHighDensity) {
    /* Extended Get System Info */
    if (ndefT5TGetSystemInformation(true) == ERR_NONE) {
      subCtx.t5t.sysInfoSupported = true;
    }
  }
  if (!subCtx.t5t.sysInfoSupported) {
    /* Get System Info */
    if (ndefT5TGetSystemInformation(false) == ERR_NONE) {
      subCtx.t5t.sysInfoSupported = true;
    }
  }
  return result;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT5TPollerNdefDetect(ndefInfo *info)
{
  ReturnCode result;
  uint8_t    tmpBuf[NDEF_T5T_TL_MAX_SIZE];
  ReturnCode returnCode = ERR_REQUEST; /* Default return code */
  uint16_t   offset;
  uint16_t   length;
  uint32_t   TlvOffset;
  bool       bExit;
  uint32_t   rcvLen;

  if (!ndefT5TisT5TDevice(&device)) {
    return ERR_PARAM;
  }

  state                           = NDEF_STATE_INVALID;
  cc.t5t.ccLen                    = 0U;
  cc.t5t.memoryLen                = 0U;
  messageLen                      = 0U;
  messageOffset                   = 0U;

  if (info != NULL) {
    info->state                = NDEF_STATE_INVALID;
    info->majorVersion         = 0U;
    info->minorVersion         = 0U;
    info->areaLen              = 0U;
    info->areaAvalableSpaceLen = 0U;
    info->messageLen           = 0U;
  }

  result = ndefT5TPollerReadBytes(0U, 8U, ccBuf, &rcvLen);
  if ((result == ERR_NONE) && (rcvLen == 8U) && ((ccBuf[0] == (uint8_t)0xE1U) || (ccBuf[0] == (uint8_t)0xE2U))) {
    cc.t5t.magicNumber           =  ccBuf[0U];
    cc.t5t.majorVersion          = (ccBuf[1U] >> 6U) & 0x03U;
    cc.t5t.minorVersion          = (ccBuf[1U] >> 4U) & 0x03U;
    cc.t5t.readAccess            = (ccBuf[1U] >> 2U) & 0x03U;
    cc.t5t.writeAccess           = (ccBuf[1U] >> 0U) & 0x03U;
    cc.t5t.memoryLen             =  ccBuf[2U];
    cc.t5t.multipleBlockRead     = (((ccBuf[3U] >> 0U) & 0x01U) != 0U);
    cc.t5t.mlenOverflow          = (((ccBuf[3U] >> 2U) & 0x01U) != 0U);
    cc.t5t.lockBlock             = (((ccBuf[3U] >> 3U) & 0x01U) != 0U);
    cc.t5t.specialFrame          = (((ccBuf[3U] >> 4U) & 0x01U) != 0U);
    state                        = NDEF_STATE_INITIALIZED;

    if (cc.t5t.memoryLen != 0U) {
      cc.t5t.ccLen             = NDEF_T5T_CC_LEN_4_BYTES;
      if ((cc.t5t.memoryLen == 0xFFU) && cc.t5t.mlenOverflow) {
        if ((subCtx.t5t.sysInfoSupported == true) && (ndefT5TSysInfoMemSizePresent(subCtx.t5t.sysInfo.infoFlags) != 0U)) {
          cc.t5t.memoryLen = (uint16_t)((subCtx.t5t.sysInfo.numberOfBlock  * subCtx.t5t.sysInfo.blockSize) / NDEF_T5T_MLEN_DIVIDER);
        }
      }
    } else {
      cc.t5t.ccLen             = NDEF_T5T_CC_LEN_8_BYTES;
      cc.t5t.memoryLen         = ((uint16_t)ccBuf[6U] << 8U) + (uint16_t)ccBuf[7U];
    }
    if ((subCtx.t5t.sysInfoSupported == true) &&
        (ndefT5TSysInfoMemSizePresent(subCtx.t5t.sysInfo.infoFlags) != 0U) &&
        (cc.t5t.memoryLen == (uint16_t)((subCtx.t5t.sysInfo.numberOfBlock  * subCtx.t5t.sysInfo.blockSize) / NDEF_T5T_MLEN_DIVIDER)) &&
        (cc.t5t.memoryLen > 0U)) {
      cc.t5t.memoryLen--; /* remove CC area from memory len */
    }
    messageLen     = 0U;
    messageOffset  = cc.t5t.ccLen;
    TlvOffset = cc.t5t.ccLen;
    bExit     = false;
    do {
      result = ndefT5TPollerReadBytes(TlvOffset, NDEF_T5T_TL_MAX_SIZE, tmpBuf, &rcvLen);
      if ((result != ERR_NONE) || (rcvLen != NDEF_T5T_TL_MAX_SIZE)) {
        break;
      }
      offset = 2U;
      length = tmpBuf[1U];
      if (length == (NDEF_SHORT_VFIELD_MAX_LEN + 1U)) {
        /* Size is encoded in 1 + 2 bytes */
        length = (((uint16_t)tmpBuf[2U]) << 8U) + (uint16_t)tmpBuf[3U];
        offset += 2U;
      }
      if (tmpBuf[0U] == (uint8_t)NDEF_T5T_TLV_NDEF) {
        /* NDEF record return it */
        returnCode                    = ERR_NONE;  /* Default */
        subCtx.t5t.TlvNDEFOffset = TlvOffset; /* Offset for TLV */
        messageOffset            = TlvOffset + offset;
        messageLen               = length;
        TlvOffset = 0U;
        if (length == 0U) {
          /* Req 40 7.5.1.6 */
          if ((cc.t5t.readAccess == 0U) && (cc.t5t.writeAccess == 0U)) {
            state = NDEF_STATE_INITIALIZED;
          } else {
            state = NDEF_STATE_INVALID;
            returnCode = ERR_REQUEST; /* Default */
          }
          bExit = true;
        } else {
          if (cc.t5t.readAccess == 0U) {
            if (cc.t5t.writeAccess == 0U) {
              state = NDEF_STATE_READWRITE;
            } else {
              state = NDEF_STATE_READONLY;
            }
          }
          bExit = true;
        }
      } else if (tmpBuf[0U] == (uint8_t) NDEF_T5T_TLV_TERMINATOR) {
        /* NDEF end */
        TlvOffset = 0U;
        bExit     = true;
      } else if (tmpBuf[0U] == (uint8_t) NDEF_T5T_TLV_PROPRIETARY) {
        /* proprietary go next, nothing to do */
        TlvOffset += (uint32_t)offset + (uint32_t)length;
      } else {
        /* RFU value */
        TlvOffset = 0U;
        bExit = true;
      }
    } while ((TlvOffset > 0U) && (bExit == false));
  } else {
    /* No CCFile */
    returnCode = ERR_REQUEST;
    if (result != ERR_NONE) {
      returnCode = result;
    }
  }

  /* TS T5T v1.0 4.3.1.17 T5T_area size is measured in bytes is equal to 8 * MLEN */
  areaLen               = (uint32_t)cc.t5t.memoryLen * NDEF_T5T_MLEN_DIVIDER;
  if (info != NULL) {
    info->state                = state;
    info->majorVersion         = cc.t5t.majorVersion;
    info->minorVersion         = cc.t5t.minorVersion;
    info->areaLen              = areaLen;
    info->areaAvalableSpaceLen = (uint32_t)cc.t5t.ccLen + areaLen - messageOffset;
    info->messageLen           = messageLen;
  }
  return returnCode;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT5TPollerReadRawMessage(uint8_t *buf, uint32_t bufLen, uint32_t *rcvdLen)
{
  ReturnCode result;

  if (!ndefT5TisT5TDevice(&device) || (buf == NULL)) {
    return ERR_PARAM;
  }

  if (messageLen > bufLen) {
    return ERR_NOMEM;
  }

  result = ndefT5TPollerReadBytes(messageOffset, messageLen, buf, rcvdLen);
  return result;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT5TPollerWriteBytes(uint32_t offset, const uint8_t *buf, uint32_t len)
{
  ReturnCode      result = ERR_REQUEST;
  ReturnCode      res;
  uint16_t        nbRead;
  uint16_t        blockLen16;
  uint16_t        startBlock;
  uint16_t        startAddr ;
  const uint8_t *wrbuf      = buf;
  uint32_t        currentLen = len;

  if (!ndefT5TisT5TDevice(&device) || (len == 0U) || (subCtx.t5t.blockLen == 0U)) {
    return ERR_PARAM;
  }
  blockLen16 = (uint16_t)subCtx.t5t.blockLen;
  if (blockLen16 == 0U) {
    return ERR_SYSTEM;
  }
  startBlock = (uint16_t)(offset     / blockLen16);
  startAddr  = (uint16_t)(startBlock * blockLen16);

  if (startAddr != offset) {
    /* Unaligned start offset must read the first block before */
    res = ndefT5TPollerReadSingleBlock(startBlock, subCtx.t5t.txrxBuf, blockLen16 + 3U, &nbRead);
    if ((res == ERR_NONE) && (subCtx.t5t.txrxBuf[0U] == 0U) && (nbRead > 0U)) {
      nbRead = (uint16_t)((uint32_t)nbRead - 1U  + startAddr - offset);
      if (nbRead > (uint32_t) currentLen) {
        nbRead = (uint16_t) currentLen;
      }
      if (nbRead > 0U) {
        (void)ST_MEMCPY(&subCtx.t5t.txrxBuf[1U - startAddr + (uint16_t)offset], wrbuf, nbRead);
      }
      res = ndefT5TPollerWriteSingleBlock(startBlock, &subCtx.t5t.txrxBuf[1U]);
      if (res != ERR_NONE) {
        return res;
      }
    } else {
      if (res != ERR_NONE) {
        result = res;
      } else {
        result = ERR_PARAM;
      }
      return result;
    }
    currentLen -= nbRead;
    wrbuf       = &wrbuf[nbRead];
    startBlock++;
  }
  while (currentLen >= blockLen16) {
    res = ndefT5TPollerWriteSingleBlock(startBlock, wrbuf);
    if (res == ERR_NONE) {
      currentLen -= blockLen16;
      wrbuf       = &wrbuf[blockLen16];
      startBlock++;
    } else {
      result = res;
      break;
    }
  }
  if ((currentLen != 0U) && (currentLen < blockLen16)) {
    /* Unaligned end, must read the first block before */
    res = ndefT5TPollerReadSingleBlock(startBlock, subCtx.t5t.txrxBuf, blockLen16 + 3U, &nbRead);
    if ((res == ERR_NONE) && (subCtx.t5t.txrxBuf[0U] == 0U) && (nbRead > 0U)) {
      if (currentLen > 0U) {
        (void)ST_MEMCPY(&subCtx.t5t.txrxBuf[1U], wrbuf, currentLen);
      }
      res = ndefT5TPollerWriteSingleBlock(startBlock, &subCtx.t5t.txrxBuf[1U]);
      if (res != ERR_NONE) {
        result = res;
      } else {
        currentLen = 0U;
      }
    } else {
      if (res != ERR_NONE) {
        result = res;
      } else {
        result = ERR_PARAM;
      }
      return result;
    }
  }
  if (currentLen == 0U) {
    result = ERR_NONE;
  }
  return result;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT5TPollerWriteRawMessageLen(uint32_t rawMessageLen)
{
  uint8_t    TLV[8U];
  ReturnCode result = ERR_PARAM;
  uint8_t     len    = 0U;

  if (ndefT5TisT5TDevice(&device)) {
    if ((state != NDEF_STATE_INITIALIZED) && (state != NDEF_STATE_READWRITE)) {
      result = ERR_WRONG_STATE;
    } else {
      TLV[len] = NDEF_T5T_TLV_NDEF;
      len++;
      if (rawMessageLen <= NDEF_SHORT_VFIELD_MAX_LEN) {
        TLV[len] = (uint8_t) rawMessageLen;
        len++;
      } else {
        TLV[len] = (uint8_t)(rawMessageLen >> 8U);
        len++;
        TLV[len] = (uint8_t) rawMessageLen;
        len++;
      }
      if (rawMessageLen == 0U) {
        TLV[len] = NDEF_TERMINATOR_TLV_T; /* TLV terminator */
        len++;
      }

      result = ndefT5TPollerWriteBytes(subCtx.t5t.TlvNDEFOffset, TLV, len);
      if ((result == ERR_NONE) && (rawMessageLen != 0U)) {
        /* T5T need specific terminator */
        len = 0U;
        TLV[len] = NDEF_TERMINATOR_TLV_T; /* TLV terminator */
        len++;
        result = ndefT5TPollerWriteBytes(messageOffset + rawMessageLen, TLV, len);
      }
    }
  }
  return result;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT5TPollerWriteRawMessage(const uint8_t *buf, uint32_t bufLen)
{
  uint32_t   len = bufLen ;
  ReturnCode result;

  if (!ndefT5TisT5TDevice(&device) || (buf == NULL)) {
    return ERR_PARAM;
  }

  /* TS T5T v1.0 7.5.3.1/2: T5T NDEF Detect should have been called before NDEF write procedure */
  /* Warning: current tag content must not be changed between NDEF Detect procedure and NDEF Write procedure*/

  /* TS T5T v1.0 7.5.3.3: check write access condition */
  if ((state != NDEF_STATE_INITIALIZED) && (state != NDEF_STATE_READWRITE)) {
    /* Conclude procedure */
    return ERR_WRONG_STATE;
  }

  /* TS T5T v1.0 7.5.3.3: verify available space */
  result = ndefT5TPollerCheckAvailableSpace(bufLen);
  if (result != ERR_NONE) {
    /* Conclude procedures */
    return ERR_PARAM;
  }
  /* TS T5T v1.0 7.5.3.4: reset L-Field to 0 */
  /* and update messageOffset according to L-field len */
  result = ndefT5TPollerBeginWriteMessage(bufLen);
  if (result != ERR_NONE) {
    state = NDEF_STATE_INVALID;
    /* Conclude procedure */
    return result;
  }
  if (bufLen != 0U) {
    /* TS T5T v1.0 7.5.3.5: write new NDEF message */
    result = ndefT5TPollerWriteBytes(messageOffset, buf, len);
    if (result != ERR_NONE) {
      /* Conclude procedure */
      state = NDEF_STATE_INVALID;
      return result;
    }
    /* TS T5T v1.0 7.5.3.6 & 7.5.3.7: update L-Field and write Terminator TLV */
    result = ndefT5TPollerEndWriteMessage(len);
    if (result != ERR_NONE) {
      /* Conclude procedure */
      state = NDEF_STATE_INVALID;
      return result;
    }
  }
  return result;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT5TWriteCC()
{
  ReturnCode  ret;
  uint8_t    *buf;
  uint8_t     dataIt;

  if (!ndefT5TisT5TDevice(&device)) {
    return ERR_PARAM;
  }

  buf    = ccBuf;
  dataIt = 0U;
  /* Encode CC */
  buf[dataIt] = cc.t5t.magicNumber;                                                                /* Byte 0 */
  dataIt++;
  buf[dataIt] = (uint8_t)(((cc.t5t.majorVersion  & 0x03U) << 6) |                                  /* Byte 1 */
                          ((cc.t5t.minorVersion  & 0x03U) << 4) |                                  /*        */
                          ((cc.t5t.readAccess    & 0x03U) << 2) |                                  /*        */
                          ((cc.t5t.writeAccess   & 0x03U) << 0));                                  /*        */
  dataIt++;
  buf[dataIt] = (cc.t5t.ccLen == NDEF_T5T_CC_LEN_8_BYTES) ? 0U : (uint8_t)cc.t5t.memoryLen;   /* Byte 2 */
  dataIt++;
  buf[dataIt]   = 0U;                                                                                   /* Byte 3 */
  if (cc.t5t.multipleBlockRead) {
    buf[dataIt] |= 0x01U;  /* Byte 3  b0 MBREAD                */
  }
  if (cc.t5t.mlenOverflow)      {
    buf[dataIt] |= 0x04U;  /* Byte 3  b2 Android MLEN overflow */
  }
  if (cc.t5t.lockBlock)         {
    buf[dataIt] |= 0x08U;  /* Byte 3  b3 Lock Block            */
  }
  if (cc.t5t.specialFrame)      {
    buf[dataIt] |= 0x10U;  /* Byte 3  b4 Special Frame         */
  }
  dataIt++;
  if (cc.t5t.ccLen == NDEF_T5T_CC_LEN_8_BYTES) {
    buf[dataIt] = 0U;                                                                                 /* Byte 4 */
    dataIt++;
    buf[dataIt] = 0U;                                                                                 /* Byte 5 */
    dataIt++;
    buf[dataIt] = (uint8_t)(cc.t5t.memoryLen >> 8);                                              /* Byte 6 */
    dataIt++;
    buf[dataIt] = (uint8_t)(cc.t5t.memoryLen >> 0);                                              /* Byte 7 */
    dataIt++;
  }

  ret = ndefT5TPollerWriteBytes(0U, buf, cc.t5t.ccLen);
  return ret;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT5TPollerTagFormat(const ndefCapabilityContainer *cc_p, uint32_t options)
{
  uint16_t                 rcvdLen;
  ReturnCode               result;
  static const uint8_t     emptyNDEF[] = { 0x03U, 0x00U, 0xFEU, 0x00U};

  if (!ndefT5TisT5TDevice(&device)) {
    return ERR_PARAM;
  }

  /* Reset previous potential info about NDEF messages */
  messageLen               = 0U;
  messageOffset            = 0U;
  subCtx.t5t.TlvNDEFOffset = 0U;

  if (cc_p != NULL) {
    if ((cc_p->t5t.ccLen != NDEF_T5T_CC_LEN_8_BYTES) && (cc_p->t5t.ccLen != NDEF_T5T_CC_LEN_4_BYTES)) {
      return ERR_PARAM;
    }
    (void)ST_MEMCPY(&cc, cc_p, sizeof(ndefCapabilityContainer));
  } else {
    /* Try to find the appropriate cc values */
    cc.t5t.magicNumber  = NDEF_T5T_CC_MAGIC_1_BYTE_ADDR_MODE; /* E1 */
    cc.t5t.majorVersion = 1U;
    cc.t5t.minorVersion = 0U;
    cc.t5t.readAccess   = 0U;
    cc.t5t.writeAccess  = 0U;
    cc.t5t.lockBlock    = false;
    cc.t5t.specialFrame = false;
    cc.t5t.memoryLen    = 0U;
    cc.t5t.mlenOverflow = false;

    result = ndefT5TPollerReadMultipleBlocks(0U, 0U, subCtx.t5t.txrxBuf, (uint16_t)sizeof(subCtx.t5t.txrxBuf), &rcvdLen);
    cc.t5t.multipleBlockRead = (result ==  ERR_NONE) ? true : false;

    /* Try to retrieve the tag's size using getSystemInfo and GetExtSystemInfo */

    if ((subCtx.t5t.sysInfoSupported == true) && (ndefT5TSysInfoMemSizePresent(subCtx.t5t.sysInfo.infoFlags) != 0U)) {
      cc.t5t.memoryLen = (uint16_t)((subCtx.t5t.sysInfo.numberOfBlock  * subCtx.t5t.sysInfo.blockSize) / NDEF_T5T_MLEN_DIVIDER);

      if ((options & NDEF_T5T_FORMAT_OPTION_NFC_FORUM) == NDEF_T5T_FORMAT_OPTION_NFC_FORUM) { /* NFC Forum format */
        if (cc.t5t.memoryLen >= NDEF_T5T_MAX_MLEN_1_BYTE_ENCODING) {
          cc.t5t.ccLen =  NDEF_T5T_CC_LEN_8_BYTES;
        }
        if (cc.t5t.memoryLen > 0U) {
          cc.t5t.memoryLen--; /* remove CC area from memory len */
        }
      } else { /* Android format */
        cc.t5t.ccLen = NDEF_T5T_CC_LEN_4_BYTES;
        if (cc.t5t.memoryLen >= NDEF_T5T_MAX_MLEN_1_BYTE_ENCODING) {
          cc.t5t.mlenOverflow = true;
          cc.t5t.memoryLen    = 0xFFU;
        }
      }

      if (!subCtx.t5t.legacySTHighDensity && (subCtx.t5t.sysInfo.numberOfBlock > NDEF_T5T_MAX_BLOCK_1_BYTE_ADDR)) {
        cc.t5t.magicNumber = NDEF_T5T_CC_MAGIC_2_BYTE_ADDR_MODE; /* E2 */
      }
    } else {
      return ERR_REQUEST;
    }
  }

  result = ndefT5TWriteCC();
  if (result != ERR_NONE) {
    /* If write fails, try to use special frame if not yet used */
    if (!cc.t5t.specialFrame) {
      delay(20U); /* Wait to be sure that previous command has ended */
      cc.t5t.specialFrame = true; /* Add option flag */
      result = ndefT5TWriteCC();
      if (result != ERR_NONE) {
        cc.t5t.specialFrame = false; /* Add option flag */
        return result;
      }
    } else {
      return result;
    }
  }
  /* Update info about current NDEF */

  subCtx.t5t.TlvNDEFOffset = cc.t5t.ccLen;

  result = ndefT5TPollerWriteBytes(subCtx.t5t.TlvNDEFOffset, emptyNDEF, sizeof(emptyNDEF));
  if (result == ERR_NONE) {
    /* Update info about current NDEF */
    messageOffset = (uint32_t)cc.t5t.ccLen + 0x02U;
    state         = NDEF_STATE_INITIALIZED;
  }
  return result;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT5TPollerCheckPresence()
{
  ReturnCode          ret;
  uint16_t            blockAddr;
  uint16_t            rcvLen;

  if (!ndefT5TisT5TDevice(&device)) {
    return ERR_PARAM;
  }

  blockAddr = 0U;

  ret = ndefT5TPollerReadSingleBlock(blockAddr, subCtx.t5t.txrxBuf, (uint16_t)sizeof(subCtx.t5t.txrxBuf), &rcvLen);

  return ret;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT5TPollerCheckAvailableSpace(uint32_t messageLen)
{
  uint32_t            lLen;

  if (!ndefT5TisT5TDevice(&device)) {
    return ERR_PARAM;
  }

  if (state == NDEF_STATE_INVALID) {
    return ERR_WRONG_STATE;
  }

  lLen = (messageLen > NDEF_SHORT_VFIELD_MAX_LEN) ? NDEF_T5T_TLV_L_3_BYTES_LEN : NDEF_T5T_TLV_L_1_BYTES_LEN;

  if ((messageLen + subCtx.t5t.TlvNDEFOffset + NDEF_T5T_TLV_T_LEN + lLen) > (areaLen + cc.t5t.ccLen)) {
    return ERR_NOMEM;
  }
  return ERR_NONE;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT5TPollerBeginWriteMessage(uint32_t messageLen)
{
  ReturnCode           ret;
  uint32_t             lLen;

  if (!ndefT5TisT5TDevice(&device)) {
    return ERR_PARAM;
  }

  if ((state != NDEF_STATE_INITIALIZED) && (state != NDEF_STATE_READWRITE)) {
    return ERR_WRONG_STATE;
  }

  /* TS T5T v1.0 7.5.3.4: reset L-Field to 0 */
  ret = ndefT5TPollerWriteRawMessageLen(0U);
  if (ret != ERR_NONE) {
    /* Conclude procedure */
    state = NDEF_STATE_INVALID;
    return ret;
  }

  lLen                = (messageLen > NDEF_SHORT_VFIELD_MAX_LEN) ? NDEF_T5T_TLV_L_3_BYTES_LEN : NDEF_T5T_TLV_L_1_BYTES_LEN;
  messageOffset  = subCtx.t5t.TlvNDEFOffset;
  messageOffset += NDEF_T5T_TLV_T_LEN; /* T Len */
  messageOffset += lLen;               /* L Len */
  state          = NDEF_STATE_INITIALIZED;

  return ERR_NONE;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT5TPollerEndWriteMessage(uint32_t messageLen)
{
  ReturnCode           ret;

  if (!ndefT5TisT5TDevice(&device)) {
    return ERR_PARAM;
  }

  if (state != NDEF_STATE_INITIALIZED) {
    return ERR_WRONG_STATE;
  }

  /* TS T5T v1.0 7.5.3.6 & 7.5.3.7: update L-Field and write Terminator TLV */
  ret = ndefT5TPollerWriteRawMessageLen(messageLen);
  if (ret != ERR_NONE) {
    /* Conclude procedure */
    state = NDEF_STATE_INVALID;
    return ret;
  }
  messageLen = messageLen;
  state      = (messageLen == 0U) ? NDEF_STATE_INITIALIZED : NDEF_STATE_READWRITE;
  return ERR_NONE;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT5TPollerWriteSingleBlock(uint16_t blockNum, const uint8_t *wrData)
{
  ReturnCode                ret;
  uint8_t                   flags;

  if (!ndefT5TisT5TDevice(&device)) {
    return ERR_PARAM;
  }

  flags = cc.t5t.specialFrame ? ((uint8_t)RFAL_NFCV_REQ_FLAG_DEFAULT | (uint8_t)RFAL_NFCV_REQ_FLAG_OPTION) : (uint8_t)RFAL_NFCV_REQ_FLAG_DEFAULT;

  if (subCtx.t5t.legacySTHighDensity) {
    ret = rfal_nfc->rfalST25xVPollerM24LRWriteSingleBlock(flags, subCtx.t5t.pAddressedUid, blockNum, wrData, subCtx.t5t.blockLen);
  } else {
    if (blockNum < NDEF_T5T_MAX_BLOCK_1_BYTE_ADDR) {
      ret = rfal_nfc->rfalNfcvPollerWriteSingleBlock(flags, subCtx.t5t.pAddressedUid, (uint8_t)blockNum, wrData, subCtx.t5t.blockLen);
    } else {
      ret = rfal_nfc->rfalNfcvPollerExtendedWriteSingleBlock(flags, subCtx.t5t.pAddressedUid, blockNum, wrData, subCtx.t5t.blockLen);
    }
  }

  return ret;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT5TPollerReadMultipleBlocks(uint16_t firstBlockNum, uint8_t numOfBlocks, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen)
{
  ReturnCode                ret;

  if (!ndefT5TisT5TDevice(&device)) {
    return ERR_PARAM;
  }

  if (subCtx.t5t.legacySTHighDensity) {

    ret = rfal_nfc->rfalST25xVPollerM24LRReadMultipleBlocks((uint8_t)RFAL_NFCV_REQ_FLAG_DEFAULT, subCtx.t5t.pAddressedUid, firstBlockNum, numOfBlocks, rxBuf, rxBufLen, rcvLen);
  } else {
    if (firstBlockNum < NDEF_T5T_MAX_BLOCK_1_BYTE_ADDR) {
      ret = rfal_nfc->rfalNfcvPollerReadMultipleBlocks((uint8_t)RFAL_NFCV_REQ_FLAG_DEFAULT, subCtx.t5t.pAddressedUid, (uint8_t)firstBlockNum, numOfBlocks, rxBuf, rxBufLen, rcvLen);
    } else {
      ret = rfal_nfc->rfalNfcvPollerExtendedReadMultipleBlocks((uint8_t)RFAL_NFCV_REQ_FLAG_DEFAULT, subCtx.t5t.pAddressedUid, firstBlockNum, numOfBlocks, rxBuf, rxBufLen, rcvLen);
    }
  }

  return ret;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT5TPollerReadSingleBlock(uint16_t blockNum, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen)
{
  ReturnCode                ret;

  if (!ndefT5TisT5TDevice(&device)) {
    return ERR_PARAM;
  }

  if (subCtx.t5t.legacySTHighDensity) {

    ret = rfal_nfc->rfalST25xVPollerM24LRReadSingleBlock((uint8_t)RFAL_NFCV_REQ_FLAG_DEFAULT, subCtx.t5t.pAddressedUid, blockNum, rxBuf, rxBufLen, rcvLen);
  } else {
    if (blockNum < NDEF_T5T_MAX_BLOCK_1_BYTE_ADDR) {
      ret = rfal_nfc->rfalNfcvPollerReadSingleBlock((uint8_t)RFAL_NFCV_REQ_FLAG_DEFAULT, subCtx.t5t.pAddressedUid, (uint8_t)blockNum, rxBuf, rxBufLen, rcvLen);
    } else {
      ret = rfal_nfc->rfalNfcvPollerExtendedReadSingleBlock((uint8_t)RFAL_NFCV_REQ_FLAG_DEFAULT, subCtx.t5t.pAddressedUid, blockNum, rxBuf, rxBufLen, rcvLen);
    }
  }

  return ret;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT5TGetSystemInformation(bool extended)
{
  ReturnCode                ret;
  uint8_t                   rxBuf[NDEF_T5T_SYSINFO_MAX_LEN];
  uint16_t                  rcvLen;
  uint8_t                  *resp;

  if (!ndefT5TisT5TDevice(&device)) {
    return ERR_PARAM;
  }

  if (extended) {
    ret = rfal_nfc->rfalNfcvPollerExtendedGetSystemInformation((uint8_t)RFAL_NFCV_REQ_FLAG_DEFAULT, subCtx.t5t.pAddressedUid, (uint8_t)RFAL_NFCV_SYSINFO_REQ_ALL, rxBuf, (uint16_t)sizeof(rxBuf), &rcvLen);
  } else {
    ret = rfal_nfc->rfalNfcvPollerGetSystemInformation(subCtx.t5t.legacySTHighDensity ? ((uint8_t)RFAL_NFCV_REQ_FLAG_DEFAULT | (uint8_t)RFAL_NFCV_REQ_FLAG_PROTOCOL_EXT) : ((uint8_t)RFAL_NFCV_REQ_FLAG_DEFAULT), subCtx.t5t.pAddressedUid, rxBuf, (uint16_t)sizeof(rxBuf), &rcvLen);
  }

  if (ret != ERR_NONE) {
    return ret;
  }

  /* FIXME check buf rcvLen */
  resp = &rxBuf[0U];
  /* skip Flags */
  resp++;
  /* get Info flags */
  subCtx.t5t.sysInfo.infoFlags = *resp;
  resp++;
  if (extended && (ndefT5TSysInfoLenValue(subCtx.t5t.sysInfo.infoFlags) != 0U)) {
    return ERR_PROTO;
  }
  /* get UID */
  (void)ST_MEMCPY(subCtx.t5t.sysInfo.UID, resp, RFAL_NFCV_UID_LEN);
  resp = &resp[RFAL_NFCV_UID_LEN];
  if (ndefT5TSysInfoDFSIDPresent(subCtx.t5t.sysInfo.infoFlags) != 0U) {
    subCtx.t5t.sysInfo.DFSID = *resp;
    resp++;
  }
  if (ndefT5TSysInfoAFIPresent(subCtx.t5t.sysInfo.infoFlags) != 0U) {
    subCtx.t5t.sysInfo.AFI = *resp;
    resp++;
  }
  if (ndefT5TSysInfoMemSizePresent(subCtx.t5t.sysInfo.infoFlags) != 0U) {
    if (subCtx.t5t.legacySTHighDensity || extended) {
      /* LRIS64K/M24LR16/M24LR64 */
      subCtx.t5t.sysInfo.numberOfBlock =  *resp;
      resp++;
      subCtx.t5t.sysInfo.numberOfBlock |= (((uint16_t) * resp) << 8U);
      resp++;
    } else {
      subCtx.t5t.sysInfo.numberOfBlock = *resp;
      resp++;
    }
    subCtx.t5t.sysInfo.blockSize = *resp;
    resp++;
    /* Add 1 to get real values*/
    subCtx.t5t.sysInfo.numberOfBlock++;
    subCtx.t5t.sysInfo.blockSize++;
  }
  if (ndefT5TSysInfoICRefPresent(subCtx.t5t.sysInfo.infoFlags) != 0U) {
    subCtx.t5t.sysInfo.ICRef = *resp;
    resp++;
  }
  if (extended && (ndefT5TSysInfoCmdListPresent(subCtx.t5t.sysInfo.infoFlags) != 0U)) {
    subCtx.t5t.sysInfo.supportedCmd[0U] = *resp;
    resp++;
    subCtx.t5t.sysInfo.supportedCmd[1U] = *resp;
    resp++;
    subCtx.t5t.sysInfo.supportedCmd[2U] = *resp;
    resp++;
    subCtx.t5t.sysInfo.supportedCmd[3U] = *resp;
    resp++;
  }
  return ERR_NONE;
}
