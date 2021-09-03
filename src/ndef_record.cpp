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
 *  \brief NDEF record
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

/*
 ******************************************************************************
 * GLOBAL MACROS
 ******************************************************************************
 */


/*
 ******************************************************************************
 * GLOBAL FUNCTIONS
 ******************************************************************************
 */


/*****************************************************************************/
ReturnCode NdefClass::ndefRecordReset(ndefRecord *record)
{
  ndefConstBuffer8 bufEmpty        = { NULL, 0 };
  ndefConstBuffer  bufEmptyPayload = { NULL, 0 };

  if (record == NULL) {
    return ERR_PARAM;
  }

  record->header = ndefHeader(0U, 0U, 0U, 0U, 0U, NDEF_TNF_EMPTY);

  (void)ndefRecordSetType(record, NDEF_TNF_EMPTY, &bufEmpty);

  (void)ndefRecordSetId(record, &bufEmpty);

  /* Set the SR bit */
  (void)ndefRecordSetPayload(record, &bufEmptyPayload);

  record->ndeftype = NULL;

  record->next = NULL;

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRecordInit(ndefRecord *record, uint8_t tnf, const ndefConstBuffer8 *bufType, const ndefConstBuffer8 *bufId, const ndefConstBuffer *bufPayload)
{
  if ((record == NULL) || (bufType == NULL) || (bufPayload == NULL)) {
    return ERR_PARAM;
  }

  (void)ndefRecordReset(record);

  (void)ndefRecordSetType(record, tnf, bufType);

  (void)ndefRecordSetId(record, bufId);

  (void)ndefRecordSetPayload(record, bufPayload);

  return ERR_NONE;
}


/*****************************************************************************/
uint32_t NdefClass::ndefRecordGetHeaderLength(const ndefRecord *record)
{
  uint32_t length;

  if (record == NULL) {
    return 0;
  }

  length  = sizeof(uint8_t);      /* header (MB:1 + ME:1 + CF:1 + SR:1 + IL:1 + TNF:3 => 8 bits) */
  length += sizeof(uint8_t);      /* Type length */
  if (ndefHeaderIsSetSR(record)) {
    length += sizeof(uint8_t);  /* Short record */
  } else {
    length += sizeof(uint32_t); /* Standard record */
  }
  if (ndefHeaderIsSetIL(record)) {
    length += sizeof(uint8_t);  /* Id length */
  }
  length += record->typeLength;   /* Type */
  length += record->idLength;     /* Id */

  return length;
}


/*****************************************************************************/
uint32_t NdefClass::ndefRecordGetLength(const ndefRecord *record)
{
  uint32_t length;

  if (record == NULL) {
    return 0;
  }

  length  = ndefRecordGetHeaderLength(record);  /* Header */
  length += ndefRecordGetPayloadLength(record); /* Payload */

  return length;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRecordSetType(ndefRecord *record, uint8_t tnf, const ndefConstBuffer8 *bufType)
{
  if ((record  == NULL) ||
      (bufType == NULL) ||
      ((bufType->buffer == NULL) && (bufType->length != 0U))) {
    return ERR_PARAM;
  }

  ndefHeaderSetTNF(record, tnf);

  record->typeLength = bufType->length;
  record->type       = bufType->buffer;

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRecordGetType(const ndefRecord *record, uint8_t *tnf, ndefConstBuffer8 *bufType)
{
  if ((record == NULL) || (tnf == NULL) || (bufType == NULL)) {
    return ERR_PARAM;
  }

  *tnf            = ndefHeaderTNF(record);
  bufType->buffer = record->type;
  bufType->length = record->typeLength;

  return ERR_NONE;
}


/*****************************************************************************/
bool NdefClass::ndefRecordTypeMatch(const ndefRecord *record, uint8_t tnf, const ndefConstBuffer8 *bufType)
{
  if ((record == NULL) || (bufType == NULL)) {
    return false;
  }

  if ((ndefHeaderTNF(record) == tnf)             &&
      (record->typeLength    == bufType->length) &&
      (ST_BYTECMP(record->type, bufType->buffer, bufType->length) == 0)) {
    return true;
  }

  return false;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRecordSetId(ndefRecord *record, const ndefConstBuffer8 *bufId)
{
  if ((record == NULL) ||
      (bufId  == NULL) ||
      ((bufId->buffer == NULL) && (bufId->length != 0U))) {
    return ERR_PARAM;
  }

  if (bufId->buffer != NULL) {
    ndefHeaderSetIL(record);
  } else {
    ndefHeaderClearIL(record);
  }

  record->id       = bufId->buffer;
  record->idLength = bufId->length;

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRecordGetId(const ndefRecord *record, ndefConstBuffer8 *bufId)
{
  if ((record == NULL) || (bufId == NULL)) {
    return ERR_PARAM;
  }

  bufId->buffer = record->id;
  bufId->length = record->idLength;

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRecordSetPayload(ndefRecord *record, const ndefConstBuffer *bufPayload)
{
  if ((record     == NULL) ||
      (bufPayload == NULL) ||
      ((bufPayload->buffer == NULL) && (bufPayload->length != 0U))) {
    return ERR_PARAM;
  }

  ndefHeaderSetValueSR(record, (bufPayload->length <= NDEF_SHORT_RECORD_LENGTH_MAX) ? 1 : 0);

  record->bufPayload.buffer = bufPayload->buffer;
  record->bufPayload.length = bufPayload->length;

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRecordGetPayload(const ndefRecord *record, ndefConstBuffer *bufPayload)
{
  if ((record == NULL) || (bufPayload == NULL)) {
    return ERR_PARAM;
  }

  bufPayload->buffer = record->bufPayload.buffer;
  bufPayload->length = ndefRecordGetPayloadLength(record);

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRecordDecode(const ndefConstBuffer *bufPayload, ndefRecord *record)
{
  uint32_t offset;

  if ((bufPayload == NULL) || (bufPayload->buffer == NULL) || (record == NULL)) {
    return ERR_PARAM;
  }

  if (ndefRecordReset(record) != ERR_NONE) {
    return ERR_INTERNAL;
  }

  /* Get "header" byte */
  offset = 0;
  if ((offset + sizeof(uint8_t)) > bufPayload->length) {
    return ERR_PROTO;
  }
  record->header = bufPayload->buffer[offset];
  offset++;

  /* Get Type length */
  if ((offset + sizeof(uint8_t)) > bufPayload->length) {
    return ERR_PROTO;
  }
  record->typeLength = bufPayload->buffer[offset];
  offset++;

  /* Decode Payload length */
  if (ndefHeaderIsSetSR(record)) {
    /* Short record */
    if ((offset + sizeof(uint8_t)) > bufPayload->length) {
      return ERR_PROTO;
    }
    record->bufPayload.length = bufPayload->buffer[offset]; /* length stored on a single byte for Short Record */
    offset++;
  } else {
    /* Standard record */
    if ((offset + sizeof(uint32_t)) > bufPayload->length) {
      return ERR_PROTO;
    }
    record->bufPayload.length = GETU32(&bufPayload->buffer[offset]);
    offset += sizeof(uint32_t);
  }

  /* Get Id length */
  if (ndefHeaderIsSetIL(record)) {
    if ((offset + sizeof(uint8_t)) > bufPayload->length) {
      return ERR_PROTO;
    }
    record->idLength = bufPayload->buffer[offset];
    offset++;
  } else {
    record->idLength = 0;
  }

  /* Get Type */
  if (record->typeLength > 0U) {
    if ((offset + record->typeLength) > bufPayload->length) {
      return ERR_PROTO;
    }
    record->type = &bufPayload->buffer[offset];
    offset += record->typeLength;
  } else {
    record->type = NULL;
  }

  /* Get Id */
  if (record->idLength > 0U) {
    if ((offset + record->idLength) > bufPayload->length) {
      return ERR_PROTO;
    }
    record->id = &bufPayload->buffer[offset];
    offset += record->idLength;
  } else {
    record->id = NULL;
  }

  /* Get Payload */
  if (record->bufPayload.length > 0U) {
    if ((offset + record->bufPayload.length) > bufPayload->length) {
      return ERR_PROTO;
    }
    record->bufPayload.buffer = &bufPayload->buffer[offset];
  } else {
    record->bufPayload.buffer = NULL;
  }

  // Doesn't recognize WKT ...

  record->next = NULL;

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRecordEncodeHeader(const ndefRecord *record, ndefBuffer *bufHeader)
{
  uint32_t offset;
  uint32_t payloadLength;

  if ((record == NULL) || (bufHeader == NULL) || (bufHeader->buffer == NULL)) {
    return ERR_PARAM;
  }

  if (bufHeader->length < NDEF_RECORD_HEADER_LEN) {
    bufHeader->length = NDEF_RECORD_HEADER_LEN;
    return ERR_NOMEM;
  }

  /* Start encoding the record */
  offset = 0;
  bufHeader->buffer[offset] = record->header;
  offset++;

  /* Set Type length */
  bufHeader->buffer[offset] = record->typeLength;
  offset++;

  /* Encode Payload length */
  payloadLength = ndefRecordGetPayloadLength(record);

  if (payloadLength <= NDEF_SHORT_RECORD_LENGTH_MAX) {
    /* Short record */
    bufHeader->buffer[offset] = (uint8_t)payloadLength;
    offset++;
  } else {
    /* Standard record */
    bufHeader->buffer[offset] = (uint8_t)(payloadLength >> 24);
    offset++;
    bufHeader->buffer[offset] = (uint8_t)(payloadLength >> 16);
    offset++;
    bufHeader->buffer[offset] = (uint8_t)(payloadLength >> 8);
    offset++;
    bufHeader->buffer[offset] = (uint8_t)(payloadLength);
    offset++;
  }

  /* Encode Id length */
  if (ndefHeaderIsSetIL(record)) {
    bufHeader->buffer[offset] = record->idLength;
    offset++;
  }

  bufHeader->length = offset;

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRecordPayloadEncode(const ndefRecord *record, ndefBuffer *bufPayload)
{
  uint32_t payloadLength;
  uint32_t offset;
  bool     begin;
  ndefConstBuffer bufPayloadItem;

  if ((record == NULL) || (bufPayload == NULL)) {
    return ERR_PROTO;
  }

  payloadLength = ndefRecordGetPayloadLength(record);
  if (payloadLength > bufPayload->length) {
    return ERR_NOMEM;
  }

  begin  = true;
  offset = 0;
  while (ndefRecordGetPayloadItem(record, &bufPayloadItem, begin) != NULL) {
    begin = false;
    if (bufPayloadItem.length > 0U) {
      ST_MEMCPY(&bufPayload->buffer[offset], bufPayloadItem.buffer, bufPayloadItem.length);
    }
    offset += bufPayloadItem.length;
  }

  bufPayload->length = offset;

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRecordEncode(const ndefRecord *record, ndefBuffer *bufRecord)
{
  ReturnCode err;
  ndefBuffer bufHeader;
  ndefBuffer bufPayload;
  uint32_t   offset;

  if ((record == NULL) || (bufRecord == NULL) || (bufRecord->buffer == NULL)) {
    return ERR_PARAM;
  }

  if (bufRecord->length < ndefRecordGetLength(record)) {
    bufRecord->length = ndefRecordGetLength(record);
    return ERR_NOMEM;
  }

  /* Encode header at the beginning of buffer provided */
  bufHeader.buffer = bufRecord->buffer;
  bufHeader.length = bufRecord->length;
  err = ndefRecordEncodeHeader(record, &bufHeader);
  if (err != ERR_NONE) {
    return err;
  }

  offset = bufHeader.length;

  /* Set Type */
  if (record->typeLength > 0U) {
    (void)ST_MEMCPY(&bufRecord->buffer[offset], record->type, record->typeLength);
    offset += record->typeLength;
  }

  /* Set Id */
  if (record->idLength > 0U) {
    (void)ST_MEMCPY(&bufRecord->buffer[offset], record->id, record->idLength);
    offset += record->idLength;
  }

  /* Set Payload */
  bufPayload.buffer = &bufRecord->buffer[offset];
  bufPayload.length =  bufRecord->length - offset;
  err = ndefRecordPayloadEncode(record, &bufPayload);
  if (err != ERR_NONE) {
    return err;
  }

  bufRecord->length = offset + bufPayload.length;

  return ERR_NONE;
}


/*****************************************************************************/
uint32_t NdefClass::ndefRecordGetPayloadLength(const ndefRecord *record)
{
  uint32_t payloadLength;

  if (record == NULL) {
    return 0;
  }

  if ((record->ndeftype != NULL) && (record->ndeftype->getPayloadLength != NULL)) {
    payloadLength = record->ndeftype->getPayloadLength(record->ndeftype);
  } else {
    payloadLength = record->bufPayload.length;
  }

  return payloadLength;
}


/*****************************************************************************/
const uint8_t *NdefClass::ndefRecordGetPayloadItem(const ndefRecord *record, ndefConstBuffer *bufPayloadItem, bool begin)
{
  if ((record == NULL) || (bufPayloadItem == NULL)) {
    return NULL;
  }

  bufPayloadItem->buffer = NULL;
  bufPayloadItem->length = 0;

  if ((record->ndeftype != NULL) && (record->ndeftype->getPayloadItem != NULL)) {
    record->ndeftype->getPayloadItem(record->ndeftype, bufPayloadItem, begin);
  } else {
    if (begin == true) {
      (void)ndefRecordGetPayload(record, bufPayloadItem);
    }
  }

  return bufPayloadItem->buffer;
}
