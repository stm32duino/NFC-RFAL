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
 *  This module provides an interface to handle NDEF message
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
 * GLOBAL VARIABLE DEFINITIONS
 ******************************************************************************
 */

/*
 ******************************************************************************
 * GLOBAL FUNCTIONS
 ******************************************************************************
 */


/*******************************************************************************/
ReturnCode NdefClass::ndefPollerContextInitialization(rfalNfcDevice *dev)
{
  if ((dev == NULL)) {
    return ERR_PARAM;
  }

  /* Save NDEF Device type */
  ndefPollerGetDeviceType(dev);

  switch (type) {
    case NDEF_DEV_NONE:
    case NDEF_DEV_T1T:
    default:
      return ERR_NOTSUPP;
    case NDEF_DEV_T2T:
      return ndefT2TPollerContextInitialization(dev);
    case NDEF_DEV_T3T:
      return ndefT3TPollerContextInitialization(dev);
    case NDEF_DEV_T4T:
      return ndefT4TPollerContextInitialization(dev);
    case NDEF_DEV_T5T:
      return ndefT5TPollerContextInitialization(dev);
  }
}

/*******************************************************************************/
ReturnCode NdefClass::ndefPollerNdefDetect(ndefInfo *info)
{
  if (info == NULL) {
    return ERR_PARAM;
  }

  switch (type) {
    case NDEF_DEV_NONE:
    case NDEF_DEV_T1T:
    default:
      return ERR_NOTSUPP;
    case NDEF_DEV_T2T:
      return ndefT2TPollerNdefDetect(info);
    case NDEF_DEV_T3T:
      return ndefT3TPollerNdefDetect(info);
    case NDEF_DEV_T4T:
      return ndefT4TPollerNdefDetect(info);
    case NDEF_DEV_T5T:
      return ndefT5TPollerNdefDetect(info);
  }
}

/*******************************************************************************/
ReturnCode NdefClass::ndefPollerReadRawMessage(uint8_t *buf, uint32_t bufLen, uint32_t *rcvdLen)
{
  if (buf == NULL || rcvdLen == NULL) {
    return ERR_PARAM;
  }

  switch (type) {
    case NDEF_DEV_NONE:
    case NDEF_DEV_T1T:
    default:
      return ERR_NOTSUPP;
    case NDEF_DEV_T2T:
      return ndefT2TPollerReadRawMessage(buf, bufLen, rcvdLen);
    case NDEF_DEV_T3T:
      return ndefT3TPollerReadRawMessage(buf, bufLen, rcvdLen);
    case NDEF_DEV_T4T:
      return ndefT4TPollerReadRawMessage(buf, bufLen, rcvdLen);
    case NDEF_DEV_T5T:
      return ndefT5TPollerReadRawMessage(buf, bufLen, rcvdLen);
  }
}

/*******************************************************************************/
ReturnCode NdefClass::ndefPollerReadBytes(uint32_t offset, uint32_t len, uint8_t *buf, uint32_t *rcvdLen)
{
  if (buf == NULL || rcvdLen == NULL) {
    return ERR_PARAM;
  }

  switch (type) {
    case NDEF_DEV_NONE:
    case NDEF_DEV_T1T:
    default:
      return ERR_NOTSUPP;
    case NDEF_DEV_T2T:
      return ndefT2TPollerReadBytes(offset, len, buf, rcvdLen);
    case NDEF_DEV_T3T:
      return ndefT3TPollerReadBytes(offset, len, buf, rcvdLen);
    case NDEF_DEV_T4T:
      return ndefT4TPollerReadBytes(offset, len, buf, rcvdLen);
    case NDEF_DEV_T5T:
      return ndefT5TPollerReadBytes(offset, len, buf, rcvdLen);
  }
}

/*******************************************************************************/
ReturnCode NdefClass::ndefPollerWriteRawMessage(const uint8_t *buf, uint32_t bufLen)
{
  if (buf == NULL) {
    return ERR_PARAM;
  }

  switch (type) {
    case NDEF_DEV_NONE:
    case NDEF_DEV_T1T:
    default:
      return ERR_NOTSUPP;
    case NDEF_DEV_T2T:
      return ndefT2TPollerWriteRawMessage(buf, bufLen);
    case NDEF_DEV_T3T:
      return ndefT3TPollerWriteRawMessage(buf, bufLen);
    case NDEF_DEV_T4T:
      return ndefT4TPollerWriteRawMessage(buf, bufLen);
    case NDEF_DEV_T5T:
      return ndefT5TPollerWriteRawMessage(buf, bufLen);
  }
}

/*******************************************************************************/
ReturnCode NdefClass::ndefPollerTagFormat(const ndefCapabilityContainer *cc_p, uint32_t options)
{
  switch (type) {
    case NDEF_DEV_NONE:
    case NDEF_DEV_T1T:
    default:
      return ERR_NOTSUPP;
    case NDEF_DEV_T2T:
      return ndefT2TPollerTagFormat(cc_p, options);
    case NDEF_DEV_T3T:
      return ndefT3TPollerTagFormat(cc_p, options);
    case NDEF_DEV_T4T:
      return ndefT4TPollerTagFormat(cc_p, options);
    case NDEF_DEV_T5T:
      return ndefT5TPollerTagFormat(cc_p, options);
  }
}

/*******************************************************************************/
ReturnCode NdefClass::ndefPollerWriteRawMessageLen(uint32_t rawMessageLen)
{
  switch (type) {
    case NDEF_DEV_NONE:
    case NDEF_DEV_T1T:
    default:
      return ERR_NOTSUPP;
    case NDEF_DEV_T2T:
      return ndefT2TPollerWriteRawMessageLen(rawMessageLen);
    case NDEF_DEV_T3T:
      return ndefT3TPollerWriteRawMessageLen(rawMessageLen);
    case NDEF_DEV_T4T:
      return ndefT4TPollerWriteRawMessageLen(rawMessageLen);
    case NDEF_DEV_T5T:
      return ndefT5TPollerWriteRawMessageLen(rawMessageLen);
  }
}

/*******************************************************************************/
ReturnCode NdefClass::ndefPollerWriteBytes(uint32_t offset, const uint8_t *buf, uint32_t len)
{
  if (buf == NULL) {
    return ERR_PARAM;
  }

  switch (type) {
    case NDEF_DEV_NONE:
    case NDEF_DEV_T1T:
    default:
      return ERR_NOTSUPP;
    case NDEF_DEV_T2T:
      return ndefT2TPollerWriteBytes(offset, buf, len);
    case NDEF_DEV_T3T:
      return ndefT3TPollerWriteBytes(offset, buf, len);
    case NDEF_DEV_T4T:
      return ndefT4TPollerWriteBytes(offset, buf, len);
    case NDEF_DEV_T5T:
      return ndefT5TPollerWriteBytes(offset, buf, len);
  }
}

/*******************************************************************************/
ReturnCode NdefClass::ndefPollerCheckPresence()
{
  switch (type) {
    case NDEF_DEV_NONE:
    case NDEF_DEV_T1T:
    default:
      return ERR_NOTSUPP;
    case NDEF_DEV_T2T:
      return ndefT2TPollerCheckPresence();
    case NDEF_DEV_T3T:
      return ndefT3TPollerCheckPresence();
    case NDEF_DEV_T4T:
      return ndefT4TPollerCheckPresence();
    case NDEF_DEV_T5T:
      return ndefT5TPollerCheckPresence();
  }
}

/*******************************************************************************/
ReturnCode NdefClass::ndefPollerCheckAvailableSpace(uint32_t messageLen)
{
  switch (type) {
    case NDEF_DEV_NONE:
    case NDEF_DEV_T1T:
    default:
      return ERR_NOTSUPP;
    case NDEF_DEV_T2T:
      return ndefT2TPollerCheckAvailableSpace(messageLen);
    case NDEF_DEV_T3T:
      return ndefT3TPollerCheckAvailableSpace(messageLen);
    case NDEF_DEV_T4T:
      return ndefT4TPollerCheckAvailableSpace(messageLen);
    case NDEF_DEV_T5T:
      return ndefT5TPollerCheckAvailableSpace(messageLen);
  }
}

/*******************************************************************************/
ReturnCode NdefClass::ndefPollerBeginWriteMessage(uint32_t messageLen)
{
  switch (type) {
    case NDEF_DEV_NONE:
    case NDEF_DEV_T1T:
    default:
      return ERR_NOTSUPP;
    case NDEF_DEV_T2T:
      return ndefT2TPollerBeginWriteMessage(messageLen);
    case NDEF_DEV_T3T:
      return ndefT3TPollerBeginWriteMessage(messageLen);
    case NDEF_DEV_T4T:
      return ndefT4TPollerBeginWriteMessage(messageLen);
    case NDEF_DEV_T5T:
      return ndefT5TPollerBeginWriteMessage(messageLen);
  }
}

/*******************************************************************************/
ReturnCode NdefClass::ndefPollerEndWriteMessage(uint32_t messageLen)
{
  switch (type) {
    case NDEF_DEV_NONE:
    case NDEF_DEV_T1T:
    default:
      return ERR_NOTSUPP;
    case NDEF_DEV_T2T:
      return ndefT2TPollerEndWriteMessage(messageLen);
    case NDEF_DEV_T3T:
      return ndefT3TPollerEndWriteMessage(messageLen);
    case NDEF_DEV_T4T:
      return ndefT4TPollerEndWriteMessage(messageLen);
    case NDEF_DEV_T5T:
      return ndefT5TPollerEndWriteMessage(messageLen);
  }
}

/*******************************************************************************/
ReturnCode NdefClass::ndefPollerWriteMessage(const ndefMessage *message)
{
  ReturnCode      err;
  ndefMessageInfo info;
  ndefRecord     *record;
  uint8_t         recordHeaderBuf[NDEF_RECORD_HEADER_LEN];
  ndefBuffer      bufHeader;
  ndefConstBuffer bufPayloadItem;
  uint32_t        offset;
  bool            firstPayloadItem;

  if ((message == NULL)) {
    return ERR_PARAM;
  }

  if ((state != NDEF_STATE_INITIALIZED) && (state != NDEF_STATE_READWRITE)) {
    return ERR_WRONG_STATE;
  }

  (void)ndefMessageGetInfo(message, &info);

  /* Verify length of the NDEF message */
  err = ndefPollerCheckAvailableSpace(info.length);
  if (err != ERR_NONE) {
    /* Conclude procedure */
    return ERR_PARAM;
  }

  /* Reset L-Field/NLEN field */
  err = ndefPollerBeginWriteMessage(info.length);
  if (err != ERR_NONE) {
    state = NDEF_STATE_INVALID;
    /* Conclude procedure */
    return err;
  }

  if (info.length != 0U) {
    offset = messageOffset;
    record = ndefMessageGetFirstRecord(message);

    while (record != NULL) {
      bufHeader.buffer = recordHeaderBuf;
      bufHeader.length = sizeof(recordHeaderBuf);
      (void)ndefRecordEncodeHeader(record, &bufHeader);
      err = ndefPollerWriteBytes(offset, bufHeader.buffer, bufHeader.length);
      if (err != ERR_NONE) {
        return err;
      }
      offset += bufHeader.length;
      // TODO Use API to access record internal
      if (record->typeLength != 0U) {
        err = ndefPollerWriteBytes(offset, record->type, record->typeLength);
        if (err != ERR_NONE) {
          /* Conclude procedure */
          state = NDEF_STATE_INVALID;
          return err;
        }
        offset += record->typeLength;
      }
      if (record->idLength != 0U) {
        err = ndefPollerWriteBytes(offset, record->id, record->idLength);
        if (err != ERR_NONE) {
          /* Conclude procedure */
          state = NDEF_STATE_INVALID;
          return err;
        }
        offset += record->idLength;
      }
      if (ndefRecordGetPayloadLength(record) != 0U) {
        firstPayloadItem = true;
        while (ndefRecordGetPayloadItem(record, &bufPayloadItem, firstPayloadItem) != NULL) {
          firstPayloadItem = false;
          err = ndefPollerWriteBytes(offset, bufPayloadItem.buffer, bufPayloadItem.length);
          if (err != ERR_NONE) {
            /* Conclude procedure */
            state = NDEF_STATE_INVALID;
            return err;
          }
          offset += bufPayloadItem.length;
        }
      }
      record = ndefMessageGetNextRecord(record);
    }

    err = ndefPollerEndWriteMessage(info.length);
    if (err != ERR_NONE) {
      /* Conclude procedure */
      state = NDEF_STATE_INVALID;
      return err;
    }
  }

  return ERR_NONE;
}

/*******************************************************************************/
ndefDeviceType NdefClass::ndefPollerGetDeviceType(rfalNfcDevice *dev)
{
  if (dev == NULL) {
    type = NDEF_DEV_NONE;
  } else {
    switch (dev->type) {
      case RFAL_NFC_LISTEN_TYPE_NFCA:
        switch (dev->dev.nfca.type) {
          case RFAL_NFCA_T1T:
            type = NDEF_DEV_T1T;
            break;
          case RFAL_NFCA_T2T:
            type = NDEF_DEV_T2T;
            break;
          case RFAL_NFCA_T4T:
            type = NDEF_DEV_T4T;
            break;
          default:
            type = NDEF_DEV_NONE;
            break;
        }
        break;
      case RFAL_NFC_LISTEN_TYPE_NFCB:
        type = NDEF_DEV_T4T;
        break;
      case RFAL_NFC_LISTEN_TYPE_NFCF:
        type = NDEF_DEV_T3T;
        break;
      case RFAL_NFC_LISTEN_TYPE_NFCV:
        type = NDEF_DEV_T5T;
        break;
      default:
        type = NDEF_DEV_NONE;
        break;
    }
  }
  return type;
}


