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
 *  \brief NDEF MIME types
 *
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */


#include "ndef_class.h"
#include "ndef_record.h"
#include "ndef_types.h"
#include "ndef_types_mime.h"


/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */


/*
 ******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************
 */


/*! Media Type strings */
static const uint8_t ndefMediaTypeVCard[]     = "text/x-vCard";              /*!< vCard Type */

const ndefConstBuffer8 bufMediaTypeVCard     = { ndefMediaTypeVCard,     sizeof(ndefMediaTypeVCard) - 1U     };  /*!< vCard Type buffer    */


/*! vCard delimiters */
static const uint8_t COLON[]     = ":";
static const uint8_t SEMICOLON[] = ";";
static const uint8_t NEWLINE[]   = "\r\n";

static const ndefConstBuffer bufDelimiterColon     = { COLON,     sizeof(COLON) - 1U };     /*!< ":"    */
static const ndefConstBuffer bufDelimiterSemicolon = { SEMICOLON, sizeof(SEMICOLON) - 1U }; /*!< ";"    */
static const ndefConstBuffer bufDelimiterNewLine   = { NEWLINE, sizeof(NEWLINE) - 1U};      /*!< "\r\n" */

/*! vCard keyword types */
static const uint8_t TYPE_BEGIN[]   = "BEGIN";
static const uint8_t TYPE_END[]     = "END";
static const uint8_t TYPE_VERSION[] = "VERSION";
static const uint8_t VALUE_VCARD[]  = "VCARD";
static const uint8_t VALUE_2_1[]    = "2.1";

static const ndefConstBuffer bufTypeBegin   = { TYPE_BEGIN,   sizeof(TYPE_BEGIN) - 1U };   /*!< "BEGIN"   */
static const ndefConstBuffer bufTypeEnd     = { TYPE_END,     sizeof(TYPE_END) - 1U };     /*!< "END"     */
static const ndefConstBuffer bufTypeVersion = { TYPE_VERSION, sizeof(TYPE_VERSION) - 1U }; /*!< "VERSION" */
static const ndefConstBuffer bufValueVCard  = { VALUE_VCARD,  sizeof(VALUE_VCARD) - 1U };  /*!< "VCARD"   */
static const ndefConstBuffer bufValue_2_1   = { VALUE_2_1,    sizeof(VALUE_2_1) - 1U };    /*!< "2.1"     */


/*
 ******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 ******************************************************************************
 */

#ifdef __cplusplus
extern "C" {
#endif
static uint32_t ndefVCardPayloadGetLength(const ndefType *vCard);
static const uint8_t *ndefVCardToPayloadItem(const ndefType *vCard, ndefConstBuffer *bufItem, bool begin);
#ifdef __cplusplus
}
#endif


/*
 ******************************************************************************
 * GLOBAL FUNCTIONS
 ******************************************************************************
 */


/*
 * Media
 */


/*****************************************************************************/
ReturnCode NdefClass::ndefMedia(ndefType *media, const ndefConstBuffer8 *bufType, const ndefConstBuffer *bufPayload)
{
  ndefTypeMedia *typeMedia;

  if ((media      == NULL) ||
      (bufType    == NULL) || (bufType->buffer    == NULL) || (bufType->length    == 0U) ||
      (bufPayload == NULL) || (bufPayload->buffer == NULL) || (bufPayload->length == 0U)) {
    return ERR_PARAM;
  }

  media->id               = NDEF_TYPE_MEDIA;
  media->getPayloadLength = NULL;
  media->getPayloadItem   = NULL;
  typeMedia               = &media->data.media;

  typeMedia->bufType.buffer    = bufType->buffer;
  typeMedia->bufType.length    = bufType->length;
  typeMedia->bufPayload.buffer = bufPayload->buffer;
  typeMedia->bufPayload.length = bufPayload->length;

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefGetMedia(const ndefType *media, ndefConstBuffer8 *bufType, ndefConstBuffer *bufPayload)
{
  const ndefTypeMedia *typeMedia;

  if ((media   == NULL) || (media->id != NDEF_TYPE_MEDIA) ||
      (bufType == NULL) || (bufPayload == NULL)) {
    return ERR_PARAM;
  }

  typeMedia = &media->data.media;

  bufType->buffer    = typeMedia->bufType.buffer;
  bufType->length    = typeMedia->bufType.length;

  bufPayload->buffer = typeMedia->bufPayload.buffer;
  bufPayload->length = typeMedia->bufPayload.length;

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRecordToMedia(const ndefRecord *record, ndefType *media)
{
  const ndefType *ndeftype;
  ndefConstBuffer8 bufType;

  if ((record == NULL) || (media == NULL)) {
    return ERR_PARAM;
  }

  if (ndefHeaderTNF(record) != NDEF_TNF_MEDIA_TYPE) {
    return ERR_PROTO;
  }

  ndeftype = ndefRecordGetNdefType(record);
  if (ndeftype != NULL) {
    (void)ST_MEMCPY(media, ndeftype, sizeof(ndefType));
    return ERR_NONE;
  }

  bufType.buffer = record->type;
  bufType.length = record->typeLength;

  return ndefMedia(media, &bufType, &record->bufPayload);
}


/*****************************************************************************/
ReturnCode NdefClass::ndefMediaToRecord(const ndefType *media, ndefRecord *record)
{
  const ndefTypeMedia *typeMedia;

  if ((media  == NULL) || (media->id != NDEF_TYPE_MEDIA) ||
      (record == NULL)) {
    return ERR_PARAM;
  }

  typeMedia = &media->data.media;

  (void)ndefRecordReset(record);

  (void)ndefRecordSetType(record, NDEF_TNF_MEDIA_TYPE, &typeMedia->bufType);

  (void)ndefRecordSetPayload(record, &typeMedia->bufPayload);

  return ERR_NONE;
}


/*
 * VCard
 */

#ifdef __cplusplus
extern "C" {
#endif
/*****************************************************************************/
static uint32_t ndefVCardPayloadGetLength(const ndefType *vCard)
{
  const ndefTypeVCard *vCardData;
  uint32_t payloadLength = 0;
  uint32_t i;

  if ((vCard == NULL) || (vCard->id != NDEF_TYPE_MEDIA_VCARD)) {
    return 0;
  }

  vCardData = &vCard->data.vCard;

  payloadLength += bufTypeBegin.length;
  payloadLength += bufDelimiterColon.length;
  payloadLength += bufValueVCard.length;
  payloadLength += bufDelimiterNewLine.length;

  payloadLength += bufTypeVersion.length;
  payloadLength += bufDelimiterColon.length;
  payloadLength += bufValue_2_1.length;
  payloadLength += bufDelimiterNewLine.length;

  i = 0;
  while (vCardData->entry[i].bufTypeBuffer != NULL) {
    payloadLength += vCardData->entry[i].bufTypeLength;

    if (vCardData->entry[i].bufSubTypeBuffer != NULL) {
      payloadLength += bufDelimiterSemicolon.length;
      payloadLength += vCardData->entry[i].bufSubTypeLength;
    }

    payloadLength += bufDelimiterColon.length;

    payloadLength += vCardData->entry[i].bufValueLength;
    payloadLength += bufDelimiterNewLine.length;

    i++;
  }

  payloadLength += bufTypeEnd.length;
  payloadLength += bufDelimiterColon.length;
  payloadLength += bufValueVCard.length;
  /* No New line after "END:VCARD" */

  return payloadLength;
}


/*****************************************************************************/
static const uint8_t *ndefVCardToPayloadItem(const ndefType *vCard, ndefConstBuffer *bufItem, bool begin)
{
  static uint32_t item     = 0;
  static uint32_t userItem = 0;
  static uint32_t index    = 0;
  const ndefTypeVCard *vCardData;

  if ((vCard   == NULL) || (vCard->id != NDEF_TYPE_MEDIA_VCARD) ||
      (bufItem == NULL)) {
    return NULL;
  }

  vCardData = &vCard->data.vCard;

  if (begin == true) {
    item     = 0;
    userItem = 0;
    index    = 0;
  }

  bufItem->buffer = NULL;
  bufItem->length = 0;

  /* Check for for empty vCard */
  if (vCardData->entry[0].bufTypeBuffer == NULL) {
    return NULL;
  }

  switch (item) {
    /* BEGIN:VCARD\r\n */
    case 0:
      bufItem->buffer = bufTypeBegin.buffer;
      bufItem->length = bufTypeBegin.length;
      item++; /* Move to next item for next call */
      break;
    case 1:
      bufItem->buffer = bufDelimiterColon.buffer;
      bufItem->length = bufDelimiterColon.length;
      item++; /* Move to next item for next call */
      break;
    case 2:
      bufItem->buffer = bufValueVCard.buffer;
      bufItem->length = bufValueVCard.length;
      item++; /* Move to next item for next call */
      break;
    case 3:
      bufItem->buffer = bufDelimiterNewLine.buffer;
      bufItem->length = bufDelimiterNewLine.length;
      item++; /* Move to next item for next call */
      break;

    /* VERSION:2.1\r\n */
    case 4:
      bufItem->buffer = bufTypeVersion.buffer;
      bufItem->length = bufTypeVersion.length;
      item++; /* Move to next item for next call */
      break;
    case 5:
      bufItem->buffer = bufDelimiterColon.buffer;
      bufItem->length = bufDelimiterColon.length;
      item++; /* Move to next item for next call */
      break;
    case 6:
      bufItem->buffer = bufValue_2_1.buffer;
      bufItem->length = bufValue_2_1.length;
      item++; /* Move to next item for next call */
      break;
    case 7:
      bufItem->buffer = bufDelimiterNewLine.buffer;
      bufItem->length = bufDelimiterNewLine.length;
      item++; /* Move to next item for next call */
      userItem = 0;
      index    = 0;
      break;
    case 8:
      /* User data */
      switch (userItem) {
        case 0:
          /* Type */
          if (vCardData->entry[index].bufTypeBuffer == NULL) {
            bufItem->buffer = bufTypeEnd.buffer;
            bufItem->length = bufTypeEnd.length;
            item++; /* Exit the user item loop */
            break;
          }
          bufItem->buffer = vCardData->entry[index].bufTypeBuffer;
          bufItem->length = vCardData->entry[index].bufTypeLength;

          if (vCardData->entry[index].bufSubTypeBuffer != NULL) {
            userItem++;
          } else {
            userItem += 3U; /* Skip sending semicolon and subtype, move to colon */
          }
          break;
        case 1:
          bufItem->buffer = bufDelimiterSemicolon.buffer;
          bufItem->length = bufDelimiterSemicolon.length;
          userItem++;
          break;
        case 2:
          /* Subtype */
          bufItem->buffer = vCardData->entry[index].bufSubTypeBuffer;
          bufItem->length = vCardData->entry[index].bufSubTypeLength;
          userItem++;
          break;
        case 3:
          bufItem->buffer = bufDelimiterColon.buffer;
          bufItem->length = bufDelimiterColon.length;
          userItem++;
          break;
        case 4:
          /* Value */
          bufItem->buffer = vCardData->entry[index].bufValueBuffer;
          bufItem->length = vCardData->entry[index].bufValueLength;
          userItem++;
          break;
        case 5:
          bufItem->buffer = bufDelimiterNewLine.buffer;
          bufItem->length = bufDelimiterNewLine.length;
          index++;
          userItem = 0; /* Looping to case 0 */
          break;
        default:
          /* MISRA 16.4: mandatory default statement */
          break;
      }
      break;
    case 9:
      bufItem->buffer = bufDelimiterColon.buffer;
      bufItem->length = bufDelimiterColon.length;
      item++; /* Move to next item for next call */
      break;
    case 10:
      bufItem->buffer = bufValueVCard.buffer;
      bufItem->length = bufValueVCard.length;
      item++; /* Move to next item for next call */
      break;
    default:
      bufItem->buffer = NULL;
      bufItem->length = 0;
      break;
  }

  return bufItem->buffer;
}
#ifdef __cplusplus
}
#endif


/*****************************************************************************/
ReturnCode NdefClass::ndefVCard(ndefType *vCard, const ndefVCardInput *bufVCardInput, uint32_t bufVCardInputCount)
{
  ndefTypeVCard *vCardData;
  uint32_t i;

  if ((vCard == NULL)            || (bufVCardInput == NULL) ||
      (bufVCardInputCount == 0U) || (bufVCardInputCount > NDEF_VCARD_ENTRY_MAX)) {
    return ERR_PARAM;
  }

  vCard->id               = NDEF_TYPE_MEDIA_VCARD;
  vCard->getPayloadLength = ndefVCardPayloadGetLength;
  vCard->getPayloadItem   = ndefVCardToPayloadItem;
  vCardData               = &vCard->data.vCard;

  /* Initialize array */
  for (i = 0; i < NDEF_VCARD_ENTRY_MAX; i++) {
    vCardData->entry[i].bufTypeBuffer    = NULL;
    vCardData->entry[i].bufTypeLength    = 0;
    vCardData->entry[i].bufSubTypeBuffer = NULL;
    vCardData->entry[i].bufSubTypeLength = 0;
    vCardData->entry[i].bufValueBuffer   = NULL;
    vCardData->entry[i].bufValueLength   = 0;
  }

  /* Pointer to user input */
  for (i = 0; i < bufVCardInputCount; i++) {
    vCardData->entry[i].bufTypeBuffer    = bufVCardInput[i].bufType->buffer;
    vCardData->entry[i].bufSubTypeBuffer = bufVCardInput[i].bufSubType->buffer;
    vCardData->entry[i].bufValueBuffer   = bufVCardInput[i].bufValue->buffer;

    vCardData->entry[i].bufTypeLength    = (uint8_t)bufVCardInput[i].bufType->length;
    vCardData->entry[i].bufSubTypeLength = (uint8_t)bufVCardInput[i].bufSubType->length;
    vCardData->entry[i].bufValueLength   = (uint16_t)bufVCardInput[i].bufValue->length;
  }

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefGetVCard(const ndefType *vCard, const ndefConstBuffer *bufType, ndefConstBuffer *bufSubType, ndefConstBuffer *bufValue)
{
  const ndefTypeVCard *vCardData;
  uint32_t i;

  if ((vCard   == NULL) || (vCard->id != NDEF_TYPE_MEDIA_VCARD) ||
      (bufType == NULL) || (bufSubType == NULL) || (bufValue == NULL)) {
    return ERR_PARAM;
  }

  vCardData = &vCard->data.vCard;

  bufSubType->buffer = NULL;
  bufSubType->length = 0;

  bufValue->buffer = NULL;
  bufValue->length = 0;

  for (i = 0; i < NDEF_VCARD_ENTRY_MAX; i++) {
    if ((vCardData->entry[i].bufTypeLength != 0U) &&
        (vCardData->entry[i].bufTypeLength == bufType->length)) {
      if (bufType->length > 0U) {
        if (ST_BYTECMP(vCardData->entry[i].bufTypeBuffer, bufType->buffer, bufType->length) == 0) {
          bufSubType->buffer = vCardData->entry[i].bufSubTypeBuffer;
          bufSubType->length = vCardData->entry[i].bufSubTypeLength;

          bufValue->buffer = vCardData->entry[i].bufValueBuffer;
          bufValue->length = vCardData->entry[i].bufValueLength;

          return ERR_NONE;
        }
      }
    }
  }

  return ERR_NOTFOUND;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefVCardFindMarker(const ndefConstBuffer *bufPayload, const ndefConstBuffer *bufMarker, uint32_t *offset)
{
  uint32_t tempOffset;

  if ((bufPayload == NULL) || (bufPayload->buffer == NULL) ||
      (bufMarker  == NULL) || (bufMarker->buffer  == NULL) ||
      (offset     == NULL)) {
    return ERR_PROTO;
  }

  tempOffset = 0;
  while (tempOffset < (bufPayload->length - bufMarker->length)) {
    if (bufMarker->length > 0U) {
      // TODO Convert To Upper to be case insensitive
      if (ST_BYTECMP(&bufPayload->buffer[tempOffset], bufMarker->buffer, bufMarker->length) == 0) {
        *offset = tempOffset;
        return ERR_NONE;
      }
    }
    tempOffset++;
  }

  return ERR_NOTFOUND;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefVCardExtractLine(const ndefConstBuffer *bufPayload, const ndefConstBuffer *bufMarker, ndefConstBuffer *bufLine)
{
  ReturnCode err;
  uint32_t markerOffset;

  if ((bufPayload == NULL) || (bufPayload->buffer == NULL) ||
      (bufMarker  == NULL) || (bufMarker->buffer  == NULL) ||
      (bufLine    == NULL) || (bufLine->buffer    == NULL)) {
    return ERR_PROTO;
  }

  err = ndefVCardFindMarker(bufPayload, bufMarker, &markerOffset);
  if (err == ERR_NONE) {
    /* Return up to the marker */
    bufLine->buffer = bufPayload->buffer;
    bufLine->length = markerOffset;
  } else {
    /* Return up to the end of the payload */
    bufLine->buffer = bufPayload->buffer;
    bufLine->length = bufPayload->length;
  }

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefVCardParseLine(const ndefConstBuffer *bufLine, ndefVCardEntry *vCardEntry)
{
  ReturnCode err;
  ndefConstBuffer bufKeyword;
  uint32_t semicolonOffset;
  uint32_t colonOffset;

  if ((bufLine == NULL) || (bufLine->buffer == NULL) || (vCardEntry == NULL)) {
    return ERR_PROTO;
  }

  /* Look for the type delimiter colon ":" */
  err = ndefVCardFindMarker(bufLine, &bufDelimiterColon, &colonOffset);
  if (err != ERR_NONE) {
    /* Invalid line */
    return err;
  }

  /* Keyword is located at the beginning of the line up to the delimiter colon */
  bufKeyword.buffer = bufLine->buffer;
  bufKeyword.length = colonOffset;

  vCardEntry->bufTypeBuffer = bufLine->buffer;
  vCardEntry->bufTypeLength = (uint8_t)colonOffset;

  /* Look for any subtype delimiter semicolon ";" */
  err = ndefVCardFindMarker(&bufKeyword, &bufDelimiterSemicolon, &semicolonOffset);
  if (err == ERR_NONE) {
    /* Subtype found (remove the leading ";") */
    vCardEntry->bufSubTypeBuffer = &bufLine->buffer[semicolonOffset + 1U];
    vCardEntry->bufSubTypeLength = (uint8_t)(colonOffset - semicolonOffset - 1U);

    /* Adjust the Type length */
    vCardEntry->bufTypeLength -= vCardEntry->bufSubTypeLength + 1U;
  }

  /* The value follows ":" until the end of the line */
  vCardEntry->bufValueBuffer = &bufLine->buffer[colonOffset + 1U];
  vCardEntry->bufValueLength = (uint16_t)(bufLine->length - colonOffset);

  return ERR_NONE;
}


/*****************************************************************************/
bool NdefClass::ndefIsVCardTypeFound(const ndefTypeVCard *vCard, const uint8_t *typeString)
{
  uint32_t i;
  for (i = 0; i < NDEF_VCARD_ENTRY_MAX; i++) {
    if (vCard->entry[i].bufTypeLength > 0U) {
      if (ST_BYTECMP(typeString, vCard->entry[i].bufTypeBuffer, vCard->entry[i].bufTypeLength) == 0) {
        return true;
      }
    }
  }

  return false;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefPayloadToVcard(const ndefConstBuffer *bufPayload, ndefType *vCard)
{
  ReturnCode err;
  ndefTypeVCard *vCardData;

  uint32_t i;
  uint32_t offset;
  uint32_t entry_count;
  ndefConstBuffer bufLinePayload;
  ndefConstBuffer bufLine;

  if ((bufPayload == NULL) || (bufPayload->buffer == NULL) || (bufPayload->length == 0U) ||
      (vCard      == NULL)) {
    return ERR_PROTO;
  }

  vCard->id               = NDEF_TYPE_MEDIA_VCARD;
  vCard->getPayloadLength = ndefVCardPayloadGetLength;
  vCard->getPayloadItem   = ndefVCardToPayloadItem;
  vCardData               = &vCard->data.vCard;

  /* Clear the vCard before parsing */
  for (i = 0; i < NDEF_VCARD_ENTRY_MAX; i++) {
    vCardData->entry[i].bufTypeBuffer    = NULL;
    vCardData->entry[i].bufTypeLength    = 0;
    vCardData->entry[i].bufSubTypeBuffer = NULL;
    vCardData->entry[i].bufSubTypeLength = 0;
    vCardData->entry[i].bufValueBuffer   = NULL;
    vCardData->entry[i].bufValueLength   = 0;
  }

  offset = 0;
  entry_count = 0;
  while ((offset < bufPayload->length) && (entry_count < NDEF_VCARD_ENTRY_MAX)) {
    /* Search an "end of line" or end of payload among the remaining to parse */
    bufLinePayload.buffer = &bufPayload->buffer[offset];
    bufLinePayload.length = bufPayload->length - offset;

    (void)ndefVCardExtractLine(&bufLinePayload, &bufDelimiterNewLine, &bufLine);

    /* Parse line and fill vCard entry */
    err = ndefVCardParseLine(&bufLine, &vCardData->entry[entry_count]);
    if (err == ERR_NONE) {
      entry_count++;
    }

    /* Move to the next line */
    offset += bufLine.length + bufDelimiterNewLine.length;
  }

  /* Check BEGIN, VERSION and END types exist */
  if (ndefIsVCardTypeFound(vCardData, bufTypeBegin.buffer) == false) {
    return ERR_SYNTAX;
  } else if (ndefIsVCardTypeFound(vCardData, bufTypeVersion.buffer) == false) {
    return ERR_SYNTAX;
  } else if (ndefIsVCardTypeFound(vCardData, bufTypeEnd.buffer) == false) {
    return ERR_SYNTAX;
  } else {
    /* MISRA 15.7 - Empty else */
  }

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRecordToVCard(const ndefRecord *record, ndefType *vCard)
{
  const ndefType *ndeftype;

  if ((record == NULL) || (vCard == NULL)) {
    return ERR_PARAM;
  }

  if (! ndefRecordTypeMatch(record, NDEF_TNF_MEDIA_TYPE, &bufMediaTypeVCard)) { /* "text/x-vCard" */
    return ERR_PROTO;
  }

  ndeftype = ndefRecordGetNdefType(record);
  if (ndeftype != NULL) {
    (void)ST_MEMCPY(vCard, ndeftype, sizeof(ndefType));
    return ERR_NONE;
  }

  return ndefPayloadToVcard(&record->bufPayload, vCard);
}


/*****************************************************************************/
ReturnCode NdefClass::ndefVCardToRecord(const ndefType *vCard, ndefRecord *record)
{
  if ((vCard  == NULL) || (vCard->id != NDEF_TYPE_MEDIA_VCARD) ||
      (record == NULL)) {
    return ERR_PARAM;
  }

  (void)ndefRecordReset(record);

  (void)ndefRecordSetType(record, NDEF_TNF_MEDIA_TYPE, &bufMediaTypeVCard);

  (void)ndefRecordSetNdefType(record, vCard);

  return ERR_NONE;
}
