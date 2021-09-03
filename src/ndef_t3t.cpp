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
 *  \brief Provides NDEF methods and definitions to access NFC Forum T3T
 *
 *  This module provides an interface to perform as a NFC Reader/Writer
 *  to handle a Type 3 Tag T3T
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
#define NDEF_T3T_MAX_DEVICE                  1U  /*!< T3T maximum number of device for detection         */
#define NDEF_T3T_SYSTEMCODE              0x12FCU /*!< SENSF_RES System Code for T3T TS T3T 1.0 7.1.1.1   */
#define NDEF_T3T_WRITEFLAG_ON               0xFU /*!< WriteFlag ON  value TS T3T 1.0 7.2.2.16            */
#define NDEF_T3T_WRITEFLAG_OFF              0x0U /*!< WriteFlag OFF value TS T3T 1.0 7.2.2.16            */
#define NDEF_T3T_AREA_OFFSET                 16U /*!< T3T Area starts at block #1                        */
#define NDEF_T3T_BLOCKLEN                    16U /*!< T3T block len is always 16                         */
#define NDEF_T3T_NBBLOCKSMAX                  4U /*!< T3T max nb of blocks per read/write                */
#define NDEF_T3T_FLAG_RW                      1U /*!< T3T read/write flag value                          */
#define NDEF_T3T_FLAG_RO                      0U /*!< T3T read only flag value                           */
#define NDEF_T3T_SENSFRES_NFCID2              2U /*!< T3T offset of UID in SENSFRES struct               */
#define NDEF_T3T_ATTRIB_INFO_CHECKSUM_LEN   0xEU /*!< T3T checksum len for attribute info to compute     */
#define NDEF_T3T_ATTRIB_INFO_OFFSET_VERSION 0x0U /*!< T3T attribute info offset of version               */
#define NDEF_T3T_ATTRIB_INFO_OFFSET_NBR       1U /*!< T3T attribute info offset of number of read        */
#define NDEF_T3T_ATTRIB_INFO_OFFSET_NBW       2U /*!< T3T attribute info offset of number of write       */
#define NDEF_T3T_ATTRIB_INFO_OFFSET_MAXB      3U /*!< T3T attribute info offset of MAXB                  */
#define NDEF_T3T_ATTRIB_INFO_OFFSET_FLAG_W    9U /*!< T3T attribute info offset of Write flag            */
#define NDEF_T3T_ATTRIB_INFO_OFFSET_FLAG_RW  10U /*!< T3T attribute info offset of Read/Write flag       */
#define NDEF_T3T_ATTRIB_INFO_OFFSET_FLAG_LN  11U /*!< T3T attribute info offset of LN field              */
#define NDEF_T3T_ATTRIB_INFO_VERSION_1_0   0x10U /*!< T3T attribute info full version number             */
#define NDEF_T3T_ATTRIB_INFO_BLOCK_NB         0U /*!< T3T attribute info block number                    */
#define NDEF_T3T_BLOCKNB_CONF              0x80U /*!< T3T TxRx config value for Read/Write block         */
#define NDEF_T3T_CHECK_NB_BLOCKS_LEN          1U /*!< T3T Length of the Nb of blocks in the CHECK reply  */

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

#define ndefT3TisT3TDevice(device) ((device)->type == RFAL_NFC_LISTEN_TYPE_NFCF)
#define ndefT3TIsWriteFlagON(writeFlag) ((writeFlag) == NDEF_T3T_WRITEFLAG_ON)

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
ReturnCode NdefClass::ndefT3TPollerReadBlocks(uint16_t blockNum, uint8_t nbBlocks, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen)
{
  ReturnCode                 ret;
  uint16_t                   requestedDataSize;
  rfalNfcfServBlockListParam servBlock;
  rfalNfcfBlockListElem     *listBlocks;
  uint8_t                    index;
  uint16_t                   rcvdLen = 0U;
  rfalNfcfServ               serviceCodeLst = 0x000BU; /* serviceCodeLst */

  if (!ndefT3TisT3TDevice(&device)) {
    return ERR_PARAM;
  }

  requestedDataSize = (uint16_t)nbBlocks * NDEF_T3T_BLOCK_SIZE;
  if (rxBufLen < requestedDataSize) {
    return ERR_PARAM;
  }

  listBlocks = subCtx.t3t.listBlocks;

  for (index = 0U; index < nbBlocks; index++) {
    /* Write each block number (16 bits per block address) */
    listBlocks[index].conf     = (uint8_t) NDEF_T3T_BLOCKNB_CONF;
    listBlocks[index].blockNum = (uint16_t)(blockNum + (uint16_t) index);
  }

  servBlock.numServ   = 1U;
  servBlock.servList  = &serviceCodeLst;
  servBlock.numBlock  = nbBlocks;
  servBlock.blockList = listBlocks;

  ret = rfal_nfc->rfalNfcfPollerCheck(device.dev.nfcf.sensfRes.NFCID2, &servBlock, subCtx.t3t.rxbuf, (uint16_t)sizeof(subCtx.t3t.rxbuf), &rcvdLen);
  if (ret != ERR_NONE) {
    return ret;
  }
  if (rcvdLen != (uint16_t)(NDEF_T3T_CHECK_NB_BLOCKS_LEN + requestedDataSize)) {
    return ERR_REQUEST;
  }
  if (requestedDataSize > 0U) {
    (void)ST_MEMCPY(rxBuf, &subCtx.t3t.rxbuf[NDEF_T3T_CHECK_NB_BLOCKS_LEN], requestedDataSize);
    if (rcvLen != NULL) {
      *rcvLen = requestedDataSize;
    }
  }
  return ERR_NONE;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT3TPollerReadBytes(uint32_t offset, uint32_t len, uint8_t *buf, uint32_t *rcvdLen)
{
  uint16_t        res;
  uint16_t        nbRead;
  ReturnCode      result     = ERR_NONE;
  uint32_t        currentLen = len;
  uint32_t        lvRcvLen   = 0U;
  const uint16_t  blockLen   = (uint16_t) NDEF_T3T_BLOCKLEN;
  uint16_t        startBlock = (uint16_t)(offset / blockLen);
  uint16_t        startAddr  = (uint16_t)(startBlock * blockLen);
  uint16_t        startOffset = (uint16_t)(offset - (uint32_t) startAddr);
  uint16_t        nbBlocks   = (uint16_t) NDEF_T3T_NBBLOCKSMAX;

  if (!ndefT3TisT3TDevice(&device) || (len == 0U)) {
    return ERR_PARAM;
  }
  if (state != NDEF_STATE_INVALID) {
    nbBlocks = cc.t3t.nbR;
  }

  if (startOffset != 0U) {
    /* Unaligned read, need to use a tmp buffer */
    res = ndefT3TPollerReadBlocks(startBlock, 1U /* One block */, subCtx.t3t.rxbuf, blockLen, &nbRead);
    if (res != ERR_NONE) {
      /* Check result */
      result = res;
    } else if (nbRead != NDEF_T3T_BLOCKLEN) {
      /* Check len */
      result = ERR_MEM_CORRUPT;
    } else {
      nbRead = (uint16_t)(nbRead - (uint16_t)startOffset);
      if ((uint32_t) nbRead > currentLen) {
        nbRead = (uint16_t) currentLen;
      }
      if (nbRead > 0U) {
        (void)ST_MEMCPY(buf, &subCtx.t3t.rxbuf[offset], (uint32_t)nbRead);
      }
      lvRcvLen   += (uint32_t) nbRead;
      currentLen -= (uint32_t) nbRead;
      startBlock++;
    }
  }

  while ((currentLen >= (uint32_t)blockLen) && (result == ERR_NONE)) {
    if (currentLen < ((uint32_t)blockLen * nbBlocks)) {
      /* Reduce the nb of blocks to read */
      nbBlocks = (uint16_t)(currentLen / blockLen);
    }
    res = ndefT3TPollerReadBlocks(startBlock, (uint8_t)nbBlocks, subCtx.t3t.rxbuf, blockLen * nbBlocks, &nbRead);
    if (res != ERR_NONE) {
      /* Check result */
      return res;
    } else if (nbRead != (blockLen * nbBlocks)) {
      /* Check len */
      return ERR_MEM_CORRUPT;
    } else {
      (void)ST_MEMCPY(&buf[lvRcvLen], subCtx.t3t.rxbuf, (uint32_t)currentLen);
      lvRcvLen   += nbRead;
      currentLen -= nbRead;
      startBlock += nbBlocks;
    }
  }
  if ((currentLen > 0U) && (result == ERR_NONE)) {
    /* Unaligned read, need to use a tmp buffer */
    res = ndefT3TPollerReadBlocks(startBlock, 1U /* One block */, subCtx.t3t.rxbuf, blockLen, &nbRead);
    if (res != ERR_NONE) {
      /* Check result */
      return res;
    } else if (nbRead != NDEF_T3T_BLOCKLEN) {
      /* Check len */
      return ERR_MEM_CORRUPT;
    } else {
      if (currentLen > 0U) {
        (void)ST_MEMCPY(&buf[lvRcvLen], subCtx.t3t.rxbuf, (uint32_t)currentLen);
      }
      lvRcvLen   += (uint32_t) currentLen;
      currentLen -= (uint32_t) currentLen;
    }
  }
  if ((currentLen == 0U) && (result == ERR_NONE)) {
    result = ERR_NONE;
  }
  if (rcvdLen != NULL) {
    *rcvdLen = lvRcvLen;
  }
  return result;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT3TPollerReadAttributeInformationBlock()
{
  /* Follow 7.4.1 NDEF Detection Procedure */
  ReturnCode   retcode;
  uint8_t     *rxbuf;
  uint16_t     checksum_received;
  uint16_t     checksum_computed = 0U;
  uint16_t     rcvLen            = 0U;
  uint8_t      i;

  if (!ndefT3TisT3TDevice(&device)) {
    return ERR_PARAM;
  }
  rxbuf   = ccBuf;
  retcode = ndefT3TPollerReadBlocks(NDEF_T3T_ATTRIB_INFO_BLOCK_NB, 1U /* One block */, rxbuf, NDEF_T3T_BLOCK_SIZE, &rcvLen);
  if ((retcode != ERR_NONE) && (rcvLen != NDEF_T3T_BLOCK_SIZE)) {
    return retcode;
  }
  /* Now compute checksum */
  for (i = 0U; i < NDEF_T3T_ATTRIB_INFO_CHECKSUM_LEN; i++) {
    checksum_computed += (uint16_t) rxbuf[i];
  }
  checksum_received = ((uint16_t)rxbuf[NDEF_T3T_ATTRIB_INFO_CHECKSUM_LEN] << 8U) + (uint16_t)rxbuf[NDEF_T3T_ATTRIB_INFO_CHECKSUM_LEN + 1U];
  if (checksum_received !=  checksum_computed) {
    return ERR_REQUEST;
  }

  /* Now copy the attribute struct */
  cc.t3t.majorVersion  = (rxbuf[NDEF_T3T_ATTRIB_INFO_OFFSET_VERSION] >> 4U);
  cc.t3t.minorVersion  = (rxbuf[NDEF_T3T_ATTRIB_INFO_OFFSET_VERSION] & 0xFU);
  cc.t3t.nbR           = rxbuf[NDEF_T3T_ATTRIB_INFO_OFFSET_NBR];
  cc.t3t.nbW           = rxbuf[NDEF_T3T_ATTRIB_INFO_OFFSET_NBW];
  cc.t3t.nMaxB         = ((uint16_t)rxbuf[NDEF_T3T_ATTRIB_INFO_OFFSET_MAXB] << 8U) + (uint16_t)rxbuf[NDEF_T3T_ATTRIB_INFO_OFFSET_MAXB + 1U];
  cc.t3t.writeFlag     = rxbuf[NDEF_T3T_ATTRIB_INFO_OFFSET_FLAG_W];
  cc.t3t.rwFlag        = rxbuf[NDEF_T3T_ATTRIB_INFO_OFFSET_FLAG_RW];
  cc.t3t.Ln            = ((uint32_t)rxbuf[NDEF_T3T_ATTRIB_INFO_OFFSET_FLAG_LN + 0U] << 0x10U)
                         | ((uint32_t)rxbuf[NDEF_T3T_ATTRIB_INFO_OFFSET_FLAG_LN + 1U] << 0x8U)
                         | (uint32_t)rxbuf[NDEF_T3T_ATTRIB_INFO_OFFSET_FLAG_LN + 2U];
  return ERR_NONE;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT3TPollerContextInitialization(rfalNfcDevice *dev)
{
  if ((dev == NULL) || !ndefT3TisT3TDevice(dev)) {
    return ERR_PARAM;
  }

  (void)ST_MEMCPY(&device, dev, sizeof(device));

  state                   = NDEF_STATE_INVALID;

  return ERR_NONE;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT3TPollerNdefDetect(ndefInfo *info)
{
  ReturnCode        retcode;
  rfalFeliCaPollRes pollRes[NDEF_T3T_MAX_DEVICE];
  uint8_t           devCnt     = NDEF_T3T_MAX_DEVICE;
  uint8_t           collisions = 0U;

  if (info != NULL) {
    info->state                = NDEF_STATE_INVALID;
    info->majorVersion         = 0U;
    info->minorVersion         = 0U;
    info->areaLen              = 0U;
    info->areaAvalableSpaceLen = 0U;
    info->messageLen           = 0U;
  }

  if (!ndefT3TisT3TDevice(&device)) {
    return ERR_PARAM;
  }
  state = NDEF_STATE_INVALID;

  /* TS T3T v1.0 7.4.1.1 the Reader/Writer SHALL send a SENSF_REQ Command with System Code set to 12FCh. */
  retcode = rfal_nfc->rfalNfcfPollerPoll(RFAL_FELICA_1_SLOT, NDEF_T3T_SYSTEMCODE, (uint8_t)RFAL_FELICA_POLL_RC_NO_REQUEST, pollRes, &devCnt, &collisions);
  if (retcode != ERR_NONE) {
    /* TS T3T v1.0 7.4.1.2 Conclude procedure. */
    return retcode;
  }

  /* Check if UID of the first card is the same */
  if (ST_BYTECMP(&(pollRes[0U][NDEF_T3T_SENSFRES_NFCID2]), device.dev.nfcf.sensfRes.NFCID2, RFAL_NFCF_NFCID2_LEN) != 0) {
    return ERR_REQUEST; /* Wrong UID */
  }

  /* TS T3T v1.0 7.4.1.3 The Reader/Writer SHALL read the Attribute Information Block using the CHECK Command. */
  /* TS T3T v1.0 7.4.1.4 The Reader/Writer SHALL verify the value of Checksum of the Attribute Information Block. */
  retcode = ndefT3TPollerReadAttributeInformationBlock();
  if (retcode != ERR_NONE) {
    return retcode;
  }

  /* TS T3T v1.0 7.4.1.6 The Reader/Writer SHALL check if it supports the NDEF mapping version number based on the rules given in Section 7.3. */
  if (cc.t3t.majorVersion != ndefMajorVersion(NDEF_T3T_ATTRIB_INFO_VERSION_1_0)) {
    return ERR_REQUEST;
  }

  messageLen     = cc.t3t.Ln;
  messageOffset  = NDEF_T3T_AREA_OFFSET;
  areaLen        = (uint32_t)cc.t3t.nMaxB * NDEF_T3T_BLOCK_SIZE;
  state          = NDEF_STATE_INITIALIZED;
  if (messageLen > 0U) {
    if (cc.t3t.rwFlag == NDEF_T3T_FLAG_RW) {
      state = NDEF_STATE_READWRITE;
    } else {
      if (cc.t3t.rwFlag == NDEF_T3T_FLAG_RO) {
        state = NDEF_STATE_READONLY;
      }
    }
  }

  if (info != NULL) {
    info->state                = state;
    info->majorVersion         = cc.t3t.majorVersion;
    info->minorVersion         = cc.t3t.minorVersion;
    info->areaLen              = areaLen;
    info->areaAvalableSpaceLen = areaLen;
    info->messageLen           = messageLen;
  }

  return ERR_NONE;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT3TPollerReadRawMessage(uint8_t *buf, uint32_t bufLen, uint32_t *rcvdLen)
{
  ReturnCode ret;

  if (!ndefT3TisT3TDevice(&device) || (buf == NULL)) {
    return ERR_PARAM;
  }

  /* TS T3T v1.0 7.4.2: This procedure assumes that the Reader/Writer has successfully performed the NDEF detection procedure. */
  /* Warning: current tag content must not be changed between NDEF Detect procedure and NDEF read procedure*/
  if (state <= NDEF_STATE_INITIALIZED) {
    return ERR_WRONG_STATE;
  }
  /* TS T3T v1.0 7.4.2.1: If the WriteFlag remembered during the NDEF detection procedure is set to ON, the NDEF data may be inconsistent ...*/
  if (ndefT3TIsWriteFlagON(cc.t3t.writeFlag)) {
    /*  TS T3T v1.0 7.4.2.1: ... the Reader/Writer SHALL conclude the NDEF read procedure*/
    return ERR_WRONG_STATE;
  }

  if (messageLen > bufLen) {
    return ERR_NOMEM;
  }

  /*  TS T3T v1.0 7.4.2.2: Read NDEF data */
  ret = ndefT3TPollerReadBytes(messageOffset, messageLen, buf, rcvdLen);
  if (ret != ERR_NONE) {
    state = NDEF_STATE_INVALID;
  }
  return ret;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT3TPollerWriteBlocks(uint16_t blockNum, uint8_t nbBlocks, const uint8_t *dataBlocks)
{
  ReturnCode                 ret;
  rfalNfcfServBlockListParam servBlock;
  rfalNfcfBlockListElem     *listBlocks;
  uint8_t                    index;
  rfalNfcfServ               serviceCodeLst = 0x0009U;

  if (!ndefT3TisT3TDevice(&device)) {
    return ERR_PARAM;
  }

  listBlocks = subCtx.t3t.listBlocks;

  for (index = 0U; index < nbBlocks; index++) {
    /* Write each block number (16 bits per block address) */
    listBlocks[index].conf     = (uint8_t) NDEF_T3T_BLOCKNB_CONF;
    listBlocks[index].blockNum = (uint16_t)(blockNum + (uint16_t) index);
  }
  servBlock.numServ   = 1U;
  servBlock.servList  = &serviceCodeLst;
  servBlock.numBlock  = nbBlocks;
  servBlock.blockList = listBlocks;

  ret = rfal_nfc->rfalNfcfPollerUpdate(device.dev.nfcf.sensfRes.NFCID2, &servBlock, subCtx.t3t.txbuf, (uint16_t)sizeof(subCtx.t3t.txbuf), dataBlocks, subCtx.t3t.rxbuf, (uint16_t)sizeof(subCtx.t3t.rxbuf));

  return ret;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT3TPollerWriteBytes(uint32_t offset, const uint8_t *buf, uint32_t len)
{
  uint16_t        nbRead;
  uint16_t        nbWrite;
  uint16_t        res;
  ReturnCode      result     = ERR_NONE;
  uint32_t        currentLen = len;
  uint32_t        txtLen     = 0U;
  const uint16_t  blockLen   = (uint16_t)NDEF_T3T_BLOCKLEN;
  uint16_t        nbBlocks   = (uint16_t) NDEF_T3T_NBBLOCKSMAX;
  uint16_t        startBlock = (uint16_t)(offset / blockLen);
  uint16_t        startAddr  = (uint16_t)(startBlock * blockLen);
  uint16_t        startOffset = (uint16_t)(offset - (uint32_t) startAddr);
  uint8_t         tmpBuf[NDEF_T3T_BLOCKLEN];

  if (!ndefT3TisT3TDevice(&device) || (len == 0U)) {
    return ERR_PARAM;
  }
  if (state != NDEF_STATE_INVALID) {
    nbBlocks = cc.t3t.nbW;
  }

  if (startOffset != 0U) {
    /* Unaligned write, need to use a tmp buffer */
    res = ndefT3TPollerReadBlocks(startBlock, 1, tmpBuf, blockLen, &nbRead);
    if (res != ERR_NONE) {
      /* Check result */
      result = res;
    } else if (nbRead != blockLen) {
      /* Check len */
      result = ERR_MEM_CORRUPT;
    } else {
      /* Fill the rest of the buffer with user data */
      nbWrite =  NDEF_T3T_BLOCKLEN - startOffset;
      if (nbWrite >  len) {
        nbWrite = (uint16_t) len;
      }
      (void)ST_MEMCPY(&tmpBuf[startOffset], buf, nbWrite);
      res = ndefT3TPollerWriteBlocks(startBlock, 1U /* One block */, tmpBuf);
      if (res == ERR_NONE) {
        txtLen     += (uint32_t) nbWrite;
        currentLen -= (uint32_t) nbWrite;
        startBlock++;
      } else {
        result = res; /* Copy the error code */
      }
    }
  }
  while ((currentLen >= (uint32_t)blockLen) && (result == ERR_NONE)) {
    if (currentLen < ((uint32_t)blockLen * nbBlocks)) {
      /* Reduce the nb of blocks to read */
      nbBlocks = (uint16_t)(currentLen / blockLen);
    }
    nbWrite = blockLen * nbBlocks;
    res     = ndefT3TPollerWriteBlocks(startBlock, (uint8_t) nbBlocks, &buf[txtLen]);
    if (res != ERR_NONE) {
      /* Check result */
      result = res;
    } else {
      txtLen     += nbWrite;
      currentLen -= nbWrite;
      startBlock += nbBlocks;
    }
  }
  if ((currentLen > 0U) && (result == ERR_NONE)) {
    /* Unaligned write, need to use a tmp buffer */
    res = ndefT3TPollerReadBlocks(startBlock, 1U /* One block */, tmpBuf, blockLen, &nbRead);
    if (res != ERR_NONE) {
      /* Check result */
      result = res;
    } else if (nbRead != blockLen) {
      /* Check len */
      result = ERR_MEM_CORRUPT;
    } else {
      /* Fill the beginning of the buffer with user data */
      (void)ST_MEMCPY(tmpBuf, &buf[txtLen], currentLen);
      res = ndefT3TPollerWriteBlocks(startBlock, 1U /* One block */, tmpBuf);
      if (res == ERR_NONE) {
        currentLen = 0U;
      } else {
        result = res; /* Copy the error code */
      }
    }
  }
  if ((currentLen == 0U) && (result == ERR_NONE)) {
    result = ERR_NONE;
  }
  return result;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT3TPollerWriteAttributeInformationBlock()
{
  uint8_t    dataIt;
  uint16_t   checksum;
  uint8_t   *buf;
  ReturnCode ret;

  if (!ndefT3TisT3TDevice(&device)) {
    return ERR_PARAM;
  }
  if (state < NDEF_STATE_INITIALIZED) {
    return ERR_WRONG_STATE;
  }
  dataIt        = 0U;
  buf           = ccBuf;
  checksum      = 0U;
  buf[dataIt]   = ((uint8_t)(cc.t3t.majorVersion << 4U)) |  cc.t3t.minorVersion; /* Byte  0 Ver          */
  checksum     += buf[dataIt];
  dataIt++;
  buf[dataIt]   = cc.t3t.nbR;                                                         /* Byte  1 Nbr          */
  checksum     += buf[dataIt];
  dataIt++;
  buf[dataIt]   = cc.t3t.nbW;                                                         /* Byte  2 Nbw          */
  checksum     += buf[dataIt];
  dataIt++;
  buf[dataIt]   = (uint8_t)(cc.t3t.nMaxB >> 8U);                                      /* Byte  3 NmaxB  (MSB) */
  checksum     += buf[dataIt];
  dataIt++;
  buf[dataIt]   = (uint8_t)(cc.t3t.nMaxB >> 0U);                                      /* Byte  4 NmaxB  (LSB) */
  checksum     += buf[dataIt];
  dataIt++;
  buf[dataIt] = 0U;                                                                        /* Byte  5 RFU          */
  dataIt++;
  buf[dataIt] = 0U;                                                                        /* Byte  6 RFU          */
  dataIt++;
  buf[dataIt] = 0U;                                                                        /* Byte  7 RFU          */
  dataIt++;
  buf[dataIt] = 0U;                                                                        /* Byte  8 RFU          */
  dataIt++;
  buf[dataIt]   = cc.t3t.writeFlag;                                                   /* Byte  9 WriteFlag    */
  checksum     += buf[dataIt];
  dataIt++;
  buf[dataIt]   = cc.t3t.rwFlag;                                                      /* Byte 10 RWFlag       */
  checksum     += buf[dataIt];
  dataIt++;
  buf[dataIt]   = (uint8_t)(cc.t3t.Ln >> 16U);                                        /* Byte 11 Ln (MSB)     */
  checksum     += buf[dataIt];
  dataIt++;
  buf[dataIt]   = (uint8_t)(cc.t3t.Ln >>  8U);                                        /* Byte 12 Ln (middle)  */
  checksum     += buf[dataIt];
  dataIt++;
  buf[dataIt]   = (uint8_t)(cc.t3t.Ln >>  0U);                                        /* Byte 13 Ln (LSB)     */
  checksum     += buf[dataIt];
  dataIt++;
  buf[dataIt] = (uint8_t)(checksum >> 8U);                                                 /* Byte 14 checksum MSB */
  dataIt++;
  buf[dataIt] = (uint8_t)(checksum >> 0U);                                                 /* Byte 15 checksum LSB */
  dataIt++;

  ret = ndefT3TPollerWriteBlocks(NDEF_T3T_ATTRIB_INFO_BLOCK_NB, 1U /* One block */, buf);
  return ret;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT3TPollerWriteRawMessage(const uint8_t *buf, uint32_t bufLen)
{
  ReturnCode           ret;

  if (!ndefT3TisT3TDevice(&device) || ((buf == NULL) && (bufLen != 0U))) {
    return ERR_PARAM;
  }
  /* TS T3T v1.0 7.4.3: This procedure assumes that the Reader/Writer has successfully performed the NDEF detection procedure... */
  /* Warning: current tag content must not be changed between NDEF Detect procedure and NDEF read procedure*/

  /* TS T3T v1.0 7.4.3: ... and that the RWFlag in the Attribute Information Block is set to 01h. */
  if ((state != NDEF_STATE_INITIALIZED) && (state != NDEF_STATE_READWRITE)) {
    /* Conclude procedure */
    return ERR_WRONG_STATE;
  }

  /* TS T3T v1.0 7.4.3.2: verify available space */
  ret = ndefT3TPollerCheckAvailableSpace(bufLen);
  if (ret != ERR_NONE) {
    /* Conclude procedure */
    return ERR_PARAM;
  }

  /* TS T3T v1.0 7.4.3.3: update WriteFlag */
  ret = ndefT3TPollerBeginWriteMessage(bufLen);
  if (ret != ERR_NONE) {
    state = NDEF_STATE_INVALID;
    /* Conclude procedure */
    return ret;
  }

  if (bufLen != 0U) {
    /* TS T3T v1.0 7.4.3.4: write new NDEF message */
    ret = ndefT3TPollerWriteBytes(messageOffset, buf, bufLen);
    if (ret != ERR_NONE) {
      /* Conclude procedure */
      state = NDEF_STATE_INVALID;
      return ret;
    }
  }
  /* TS T3T v1.0 7.4.3.5: update Ln value and set WriteFlag to OFF */
  ret = ndefT3TPollerEndWriteMessage(bufLen);
  if (ret != ERR_NONE) {
    /* Conclude procedure */
    state = NDEF_STATE_INVALID;
    return ret;
  }
  return ret;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT3TPollerTagFormat(const ndefCapabilityContainer *cc_p, uint32_t options)
{
  ReturnCode        res;
  rfalFeliCaPollRes buffOut [NDEF_T3T_MAX_DEVICE];
  uint8_t           devCnt     = NDEF_T3T_MAX_DEVICE;
  uint8_t           collisions = 0U;
  NO_WARNING(options); /* options not used in T3T */

  if (!ndefT3TisT3TDevice(&device)) {
    return ERR_PARAM;
  }
  if (cc_p == NULL) {
    /* No default CC found so have to analyse the tag */
    res = ndefT3TPollerReadAttributeInformationBlock();  /* Read current cc */
    if (res != ERR_NONE) {
      return res;
    }
  } else {
    /* Nothing to do */
    (void)ST_MEMCPY(&cc, cc_p, sizeof(ndefCapabilityContainer));
  }

  /* 4.3.3 System Definition Information for SystemCode = 0x12FC (NDEF) */
  res = rfal_nfc->rfalNfcfPollerPoll(RFAL_FELICA_1_SLOT, NDEF_T3T_SYSTEMCODE, (uint8_t)RFAL_FELICA_POLL_RC_NO_REQUEST, buffOut, &devCnt, &collisions);
  if (res != ERR_NONE) {
    return res;
  }
  res = rfal_nfc->rfalNfcfPollerPoll(RFAL_FELICA_1_SLOT, NDEF_T3T_SYSTEMCODE, (uint8_t)RFAL_FELICA_POLL_RC_SYSTEM_CODE, buffOut, &devCnt, &collisions);
  if (res != ERR_NONE) {
    return res;
  }
  state            = NDEF_STATE_INITIALIZED; /* to be sure that the block will be written */
  cc.t3t.Ln        = 0U; /* Force actual stored NDEF size to 0 */
  cc.t3t.writeFlag = 0U; /* Force WriteFlag to 0 */
  res = ndefT3TPollerWriteAttributeInformationBlock();
  return res;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT3TPollerCheckPresence()
{
  ReturnCode        retcode;
  uint16_t          nbRead;

  if (!ndefT3TisT3TDevice(&device)) {
    return ERR_PARAM;
  }
  /* Perform a simple readblock */
  retcode = ndefT3TPollerReadBlocks(0U /* First block */, 1U /* One Block */, subCtx.t3t.rxbuf, NDEF_T3T_BLOCKLEN, &nbRead);
  return retcode;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT3TPollerCheckAvailableSpace(uint32_t messageLen)
{
  if (!ndefT3TisT3TDevice(&device)) {
    return ERR_PARAM;
  }

  if (state == NDEF_STATE_INVALID) {
    return ERR_WRONG_STATE;
  }
  if (messageLen  > areaLen) {
    return ERR_NOMEM;
  }
  return ERR_NONE;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT3TPollerBeginWriteMessage(uint32_t messageLen)
{
  ReturnCode ret;
  NO_WARNING(messageLen);

  if (!ndefT3TisT3TDevice(&device)) {
    return ERR_PARAM;
  }

  if ((state != NDEF_STATE_INITIALIZED) && (state != NDEF_STATE_READWRITE)) {
    return ERR_WRONG_STATE;
  }
  /* Update WriteFlag */
  cc.t3t.writeFlag = NDEF_T3T_WRITEFLAG_ON;
  ret                   = ndefT3TPollerWriteAttributeInformationBlock();
  if (ret != ERR_NONE) {
    /* Conclude procedure */
    state = NDEF_STATE_INVALID;
    return ret;
  }
  state = NDEF_STATE_INITIALIZED;
  return ERR_NONE;
}

/*******************************************************************************/
ReturnCode NdefClass::ndefT3TPollerEndWriteMessage(uint32_t messageLen)
{
  ReturnCode ret;

  if (!ndefT3TisT3TDevice(&device)) {
    return ERR_PARAM;
  }
  if (state != NDEF_STATE_INITIALIZED) {
    return ERR_WRONG_STATE;
  }
  /* TS T3T v1.0 7.4.3.5 Update Attribute Information Block */
  cc.t3t.writeFlag = NDEF_T3T_WRITEFLAG_OFF;
  cc.t3t.Ln        = messageLen;
  ret                   = ndefT3TPollerWriteAttributeInformationBlock();
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
ReturnCode NdefClass::ndefT3TPollerWriteRawMessageLen(uint32_t rawMessageLen)
{
  if (!ndefT3TisT3TDevice(&device)) {
    return ERR_PARAM;
  }

  if ((state != NDEF_STATE_INITIALIZED) && (state != NDEF_STATE_READWRITE)) {
    return ERR_WRONG_STATE;
  }
  return ndefT3TPollerEndWriteMessage(rawMessageLen);
}
