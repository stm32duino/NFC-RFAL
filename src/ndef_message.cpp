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
 *  \brief NDEF message
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
 * LOCAL VARIABLES
 ******************************************************************************
 */


/*
 ******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 ******************************************************************************
 */


/*****************************************************************************/
ndefRecord *NdefClass::ndefAllocRecord(void)
{
  if (ndefRecordPoolIndex >= NDEF_MAX_RECORD) {
    return NULL;
  }

  return &ndefRecordPool[ndefRecordPoolIndex++];
}


/*
 ******************************************************************************
 * GLOBAL FUNCTIONS
 ******************************************************************************
 */
/*****************************************************************************/


ReturnCode NdefClass::ndefMessageInit(ndefMessage *message)
{
  if (message == NULL) {
    return ERR_PARAM;
  }

  message->record           = NULL;
  message->info.length      = 0;
  message->info.recordCount = 0;

  ndefRecordPoolIndex = 0;

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefMessageGetInfo(const ndefMessage *message, ndefMessageInfo *info)
{
  ndefRecord *record;
  uint32_t    length      = 0;
  uint32_t    recordCount = 0;

  if ((message == NULL) || (info == NULL)) {
    return ERR_PARAM;
  }

  record = message->record;

  while (record != NULL) {
    length += ndefRecordGetLength(record);
    recordCount++;

    record = record->next;
  }

  info->length      = length;
  info->recordCount = recordCount;

  return ERR_NONE;
}


/*****************************************************************************/
uint32_t NdefClass::ndefMessageGetRecordCount(const ndefMessage *message)
{
  ndefMessageInfo info;

  if (ndefMessageGetInfo(message, &info) == ERR_NONE) {
    return info.recordCount;
  }

  return 0;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefMessageAppend(ndefMessage *message, ndefRecord *record)
{
  if ((message == NULL) || (record == NULL)) {
    return ERR_PARAM;
  }

  /* Clear the Message Begin bit */
  ndefHeaderClearMB(record);

  /* Record is appended so it is the last in the list, set the Message End bit */
  ndefHeaderSetME(record);

  record->next = NULL;

  if (message->record == NULL) {
    /* Set the Message Begin bit for the first record only */
    ndefHeaderSetMB(record);

    message->record = record;
  } else {
    ndefRecord *current = message->record;

    /* Go through the list of records */
    while (current->next != NULL) {
      current = current->next;
    }

    /* Clear the Message End bit to the record before the one being appended */
    ndefHeaderClearME(current);

    /* Append to the last record */
    current->next = record;
  }

  message->info.length      += ndefRecordGetLength(record);
  message->info.recordCount += 1U;

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefMessageDecode(const ndefConstBuffer *bufPayload, ndefMessage *message)
{
  ReturnCode err;
  uint32_t offset;

  if ((bufPayload == NULL) || (bufPayload->buffer == NULL) || (message == NULL)) {
    return ERR_PARAM;
  }

  err = ndefMessageInit(message);
  if (err != ERR_NONE) {
    return err;
  }

  offset = 0;
  while (offset < bufPayload->length) {
    ndefConstBuffer bufRecord;
    ndefRecord *record = ndefAllocRecord();
    if (record == NULL) {
      return ERR_NOMEM;
    }
    bufRecord.buffer = &bufPayload->buffer[offset];
    bufRecord.length =  bufPayload->length - offset;
    err = ndefRecordDecode(&bufRecord, record);
    if (err != ERR_NONE) {
      return err;
    }
    offset += ndefRecordGetLength(record);

    err = ndefMessageAppend(message, record);
    if (err != ERR_NONE) {
      return err;
    }
  }

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefMessageEncode(const ndefMessage *message, ndefBuffer *bufPayload)
{
  ReturnCode      err;
  ndefMessageInfo info;
  ndefRecord     *record;
  uint32_t        offset;
  uint32_t        remainingLength;

  if ((message == NULL) || (bufPayload == NULL) || (bufPayload->buffer == NULL)) {
    return ERR_PARAM;
  }

  err = ndefMessageGetInfo(message, &info);
  if ((err != ERR_NONE) || (bufPayload->length < info.length)) {
    bufPayload->length = info.length;
    return ERR_NOMEM;
  }

  /* Get the first record */
  record          = ndefMessageGetFirstRecord(message);
  offset          = 0;
  remainingLength = bufPayload->length;

  while (record != NULL) {
    ndefBuffer bufRecord;
    bufRecord.buffer = &bufPayload->buffer[offset];
    bufRecord.length = remainingLength;
    err = ndefRecordEncode(record, &bufRecord);
    if (err != ERR_NONE) {
      bufPayload->length = info.length;
      return err;
    }
    offset          += bufRecord.length;
    remainingLength -= bufRecord.length;

    record = ndefMessageGetNextRecord(record);
  }

  bufPayload->length = offset;
  return ERR_NONE;
}
