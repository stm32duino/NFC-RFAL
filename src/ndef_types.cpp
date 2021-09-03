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
 *  \brief NDEF RTD and MIME types
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
 * GLOBAL DEFINES
 ******************************************************************************
 */


/*
 ******************************************************************************
 * GLOBAL TYPES
 ******************************************************************************
 */


/*! NDEF type table to associate a ndefTypeId and a string */
typedef struct {
  ndefTypeId              typeId;        /*!< NDEF Type Id       */
  uint8_t                 tnf;           /*!< TNF                */
  const ndefConstBuffer8 *bufTypeString; /*!< Type String buffer */
} ndefTypeTable;


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


/*****************************************************************************/
ReturnCode NdefClass::ndefTypeStringToTypeId(uint8_t tnf, const ndefConstBuffer8 *bufTypeString, ndefTypeId *typeId)
{
  /*! Empty string */
  static const uint8_t    ndefTypeEmpty[] = "";    /*!< Empty string */
  static ndefConstBuffer8 bufTypeEmpty    = { ndefTypeEmpty, sizeof(ndefTypeEmpty) - 1U };

  // TODO Transform the enum (u32) to defines (u8), re-order to u32-u8-u8 to compact buffer !
  static const ndefTypeTable typeTable[] = {
    { NDEF_TYPE_EMPTY,           NDEF_TNF_EMPTY,               &bufTypeEmpty              },
    { NDEF_TYPE_RTD_DEVICE_INFO, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeDeviceInfo      },
    { NDEF_TYPE_RTD_TEXT,        NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeText            },
    { NDEF_TYPE_RTD_URI,         NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeUri             },
    { NDEF_TYPE_RTD_AAR,         NDEF_TNF_RTD_EXTERNAL_TYPE,   &bufRtdTypeAar             },
    { NDEF_TYPE_MEDIA_VCARD,     NDEF_TNF_MEDIA_TYPE,          &bufMediaTypeVCard         },
    { NDEF_TYPE_MEDIA_WIFI,      NDEF_TNF_MEDIA_TYPE,          &bufMediaTypeWifi          },
  };

  uint32_t i;

  if ((bufTypeString == NULL) || (typeId == NULL)) {
    return ERR_PROTO;
  }

  for (i = 0; i < SIZEOF_ARRAY(typeTable); i++) {
    /* Check TNF and length are the same, then compare the content */
    if (typeTable[i].tnf == tnf) {
      if (bufTypeString->length == typeTable[i].bufTypeString->length) {
        if (bufTypeString->length == 0U) {
          /* Empty type */
          *typeId = typeTable[i].typeId;
          return ERR_NONE;
        } else {
          if (ST_BYTECMP(typeTable[i].bufTypeString->buffer, bufTypeString->buffer, bufTypeString->length) == 0) {
            *typeId = typeTable[i].typeId;
            return ERR_NONE;
          }
        }
      }
    }
  }

  return ERR_NOTFOUND;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRecordTypeStringToTypeId(const ndefRecord *record, ndefTypeId *typeId)
{
  ReturnCode err;

  uint8_t          tnf;
  ndefConstBuffer8 bufRecordType;

  if ((record == NULL) || (typeId == NULL)) {
    return ERR_PARAM;
  }

  err = ndefRecordGetType(record, &tnf, &bufRecordType);
  if (err != ERR_NONE) {
    return err;
  }
  if (tnf >= NDEF_TNF_RESERVED) {
    return ERR_INTERNAL;
  }

  switch (tnf) {
    case NDEF_TNF_EMPTY:               /* Fall through */
    case NDEF_TNF_RTD_WELL_KNOWN_TYPE: /* Fall through */
    case NDEF_TNF_RTD_EXTERNAL_TYPE:   /* Fall through */
    case NDEF_TNF_MEDIA_TYPE:          /* Fall through */
      err = ndefTypeStringToTypeId(tnf, &bufRecordType, typeId);
      break;
    default:
      err = ERR_NOT_IMPLEMENTED;
      break;
  }

  return err;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRecordToType(const ndefRecord *record, ndefType *type)
{
  const ndefType *ndeftype;
  ReturnCode err;
  ndefTypeId typeId;

  ndeftype = ndefRecordGetNdefType(record);
  if (ndeftype != NULL) {
    /* Return the well-known type contained in the record */
    (void)ST_MEMCPY(type, ndeftype, sizeof(ndefType));
    return ERR_NONE;
  }

  err = ndefRecordTypeStringToTypeId(record, &typeId);
  if (err != ERR_NONE) {
    return err;
  }

  switch (typeId) {
    case NDEF_TYPE_EMPTY:
      return ndefRecordToEmptyType(record, type);
    case NDEF_TYPE_RTD_DEVICE_INFO:
      return ndefRecordToRtdDeviceInfo(record, type);
    case NDEF_TYPE_RTD_TEXT:
      return ndefRecordToRtdText(record, type);
    case NDEF_TYPE_RTD_URI:
      return ndefRecordToRtdUri(record, type);
    case NDEF_TYPE_RTD_AAR:
      return ndefRecordToRtdAar(record, type);
    case NDEF_TYPE_MEDIA_VCARD:
      return ndefRecordToVCard(record, type);
    case NDEF_TYPE_MEDIA_WIFI:
      return ndefRecordToWifi(record, type);
    default:
      return ERR_NOT_IMPLEMENTED;
  }
}


/*****************************************************************************/
ReturnCode NdefClass::ndefTypeToRecord(const ndefType *type, ndefRecord *record)
{
  if (type == NULL) {
    return ERR_PARAM;
  }

  switch (type->id) {
    case NDEF_TYPE_EMPTY:
      return ndefEmptyTypeToRecord(type, record);
    case NDEF_TYPE_RTD_DEVICE_INFO:
      return ndefRtdDeviceInfoToRecord(type, record);
    case NDEF_TYPE_RTD_TEXT:
      return ndefRtdTextToRecord(type, record);
    case NDEF_TYPE_RTD_URI:
      return ndefRtdUriToRecord(type, record);
    case NDEF_TYPE_RTD_AAR:
      return ndefRtdAarToRecord(type, record);
    case NDEF_TYPE_MEDIA_VCARD:
      return ndefVCardToRecord(type, record);
    case NDEF_TYPE_MEDIA_WIFI:
      return ndefWifiToRecord(type, record);
    default:
      return ERR_NOT_IMPLEMENTED;
  }
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRecordSetNdefType(ndefRecord *record, const ndefType *type)
{
  uint32_t payloadLength;

  if ((record == NULL) ||
      (type                   == NULL)               ||
      (type->id                > NDEF_TYPE_ID_COUNT) ||
      (type->getPayloadLength == NULL)               ||
      (type->getPayloadItem   == NULL)) {
    return ERR_PARAM;
  }

  record->ndeftype = type;

  /* Set Short Record bit accordingly */
  payloadLength = ndefRecordGetPayloadLength(record);
  ndefHeaderSetValueSR(record, (payloadLength <= NDEF_SHORT_RECORD_LENGTH_MAX) ? 1 : 0);

  return ERR_NONE;
}


/*****************************************************************************/
const ndefType *NdefClass::ndefRecordGetNdefType(const ndefRecord *record)
{
  if (record == NULL) {
    return NULL;
  }

  if (record->ndeftype != NULL) {
    /* Check whether it is a valid NDEF type */
    if ((record->ndeftype->id                < NDEF_TYPE_ID_COUNT) &&
        (record->ndeftype->getPayloadItem   != NULL) &&
        (record->ndeftype->getPayloadLength != NULL)) {
      return record->ndeftype;
    }
  }

  return NULL;
}
