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
 *  \brief NDEF RTD (well-known and external) types
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


/*! Device Information defines */
#define NDEF_RTD_DEVICE_INFO_PAYLOAD_MIN     (2U * (sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint8_t))) /*!< Device Information minimum length (2 required TLV structures) */
#define NDEF_RTD_DEVICE_INFO_PAYLOAD_MAX     ((4U * (sizeof(uint8_t) + sizeof(uint8_t) + 255U)) + (sizeof(uint8_t) + sizeof(uint8_t) + 16U)) /*!< Device Information maximum length */
#define NDEF_RTD_DEVICE_INFO_TLV_LENGTH_MIN  (sizeof(uint8_t) + sizeof(uint8_t))  /*!< Device Information minimum TLV length */

/*! Text defines */
#define NDEF_RTD_TEXT_STATUS_OFFSET              0U    /*!< Text status offset */
#define NDEF_RTD_TEXT_LANGUAGE_OFFSET            1U    /*!< Text language offset */

#define NDEF_RTD_TEXT_LANGUAGE_CODE_LEN_MASK  0x3FU    /*!< IANA language code mask (length coded on 6 bits) */

#define NDEF_RTD_TEXT_PAYLOAD_LENGTH_MIN         (sizeof(uint8_t) + sizeof(uint8_t))   /*!< Minimum Text Payload length */

/*! URI defines */
#define NDEF_RTD_URI_PROTOCOL_LEN        1U                        /*!< URI protocol length */
#define NDEF_RTD_URI_PAYLOAD_LENGTH_MIN  (NDEF_RTD_URI_PROTOCOL_LEN + sizeof(uint8_t)) /*!< URI minimum payload length */

/*! URI defines */
#define NDEF_RTD_URI_ID_CODE_OFFSET      0U    /*!< URI Id code offset */
#define NDEF_RTD_URI_FIELD_OFFSET        1U    /*!< URI field offset */


/*
 ******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************
 */


/*! RTD Type strings */
static const uint8_t ndefRtdTypeDeviceInfo[]     = "Di";              /*!< Device Information Record Type {0x44, 0x69} */
static const uint8_t ndefRtdTypeText[]           = "T";               /*!< Text Record Type               {0x54}       */
static const uint8_t ndefRtdTypeUri[]            = "U";               /*!< URI Record Type                {0x55}       */
static const uint8_t ndefRtdTypeAar[]            = "android.com:pkg"; /*!< External Type (Android Application Record)  */

const ndefConstBuffer8 bufRtdTypeDeviceInfo      = { ndefRtdTypeDeviceInfo, sizeof(ndefRtdTypeDeviceInfo) - 1U }; /*!< Device Information Record Type buffer */
const ndefConstBuffer8 bufRtdTypeText            = { ndefRtdTypeText,       sizeof(ndefRtdTypeText) - 1U };       /*!< Text Record Type buffer               */
const ndefConstBuffer8 bufRtdTypeUri             = { ndefRtdTypeUri,        sizeof(ndefRtdTypeUri) - 1U };        /*!< URI Record Type buffer                */
const ndefConstBuffer8 bufRtdTypeAar             = { ndefRtdTypeAar,        sizeof(ndefRtdTypeAar) - 1U };        /*!< AAR External Type Record buffer       */


/*! URI Type strings */
static const uint8_t ndefUriPrefixNone[]         = "";
static const uint8_t ndefUriPrefixHttpWww[]      = "http://www.";
static const uint8_t ndefUriPrefixHttpsWww[]     = "https://www.";
static const uint8_t ndefUriPrefixHttp[]         = "http://";
static const uint8_t ndefUriPrefixHttps[]        = "https://";
static const uint8_t ndefUriPrefixTel[]          = "tel:";
static const uint8_t ndefUriPrefixMailto[]       = "mailto:";
static const uint8_t ndefUriPrefixFtpAnonymous[] = "ftp://anonymous:anonymous@";
static const uint8_t ndefUriPrefixFtpFtp[]       = "ftp://ftp.";
static const uint8_t ndefUriPrefixFtps[]         = "ftps://";
static const uint8_t ndefUriPrefixSftp[]         = "sftp://";
static const uint8_t ndefUriPrefixSmb[]          = "smb://";
static const uint8_t ndefUriPrefixNfs[]          = "nfs://";
static const uint8_t ndefUriPrefixFtp[]          = "ftp://";
static const uint8_t ndefUriPrefixDav[]          = "dav://";
static const uint8_t ndefUriPrefixNews[]         = "news:";
static const uint8_t ndefUriPrefixTelnet[]       = "telnet://";
static const uint8_t ndefUriPrefixImap[]         = "imap:";
static const uint8_t ndefUriPrefixRtsp[]         = "rtsp://";
static const uint8_t ndefUriPrefixUrn[]          = "urn:";
static const uint8_t ndefUriPrefixPop[]          = "pop:";
static const uint8_t ndefUriPrefixSip[]          = "sip:";
static const uint8_t ndefUriPrefixSips[]         = "sips:";
static const uint8_t ndefUriPrefixTftp[]         = "tftp:";
static const uint8_t ndefUriPrefixBtspp[]        = "btspp://";
static const uint8_t ndefUriPrefixBtl2cap[]      = "btl2cap://";
static const uint8_t ndefUriPrefixBtgoep[]       = "btgoep://";
static const uint8_t ndefUriPrefixTcpobex[]      = "tcpobex://";
static const uint8_t ndefUriPrefixIrdaobex[]     = "irdaobex://";
static const uint8_t ndefUriPrefixFile[]         = "file://";
static const uint8_t ndefUriPrefixUrnEpcId[]     = "urn:epc:id:";
static const uint8_t ndefUriPrefixUrnEpcTag[]    = "urn:epc:tag";
static const uint8_t ndefUriPrefixUrnEpcPat[]    = "urn:epc:pat:";
static const uint8_t ndefUriPrefixUrnEpcRaw[]    = "urn:epc:raw:";
static const uint8_t ndefUriPrefixUrnEpe[]       = "urn:epc:";
static const uint8_t ndefUriPrefixUrnNfc[]       = "urn:nfc:";
static const uint8_t ndefUriPrefixEmpty[]        = ""; /* Autodetect filler */

static const ndefConstBuffer ndefUriPrefix[NDEF_URI_PREFIX_COUNT] = {
  { ndefUriPrefixNone, sizeof(ndefUriPrefixNone) - 1U },
  { ndefUriPrefixHttpWww, sizeof(ndefUriPrefixHttpWww) - 1U },
  { ndefUriPrefixHttpsWww, sizeof(ndefUriPrefixHttpsWww) - 1U },
  { ndefUriPrefixHttp, sizeof(ndefUriPrefixHttp) - 1U },
  { ndefUriPrefixHttps, sizeof(ndefUriPrefixHttps) - 1U },
  { ndefUriPrefixTel, sizeof(ndefUriPrefixTel) - 1U },
  { ndefUriPrefixMailto, sizeof(ndefUriPrefixMailto) - 1U },
  { ndefUriPrefixFtpAnonymous, sizeof(ndefUriPrefixFtpAnonymous) - 1U },
  { ndefUriPrefixFtpFtp, sizeof(ndefUriPrefixFtpFtp) - 1U },
  { ndefUriPrefixFtps, sizeof(ndefUriPrefixFtps) - 1U },
  { ndefUriPrefixSftp, sizeof(ndefUriPrefixSftp) - 1U },
  { ndefUriPrefixSmb, sizeof(ndefUriPrefixSmb) - 1U },
  { ndefUriPrefixNfs, sizeof(ndefUriPrefixNfs) - 1U },
  { ndefUriPrefixFtp, sizeof(ndefUriPrefixFtp) - 1U },
  { ndefUriPrefixDav, sizeof(ndefUriPrefixDav) - 1U },
  { ndefUriPrefixNews, sizeof(ndefUriPrefixNews) - 1U },
  { ndefUriPrefixTelnet, sizeof(ndefUriPrefixTelnet) - 1U },
  { ndefUriPrefixImap, sizeof(ndefUriPrefixImap) - 1U },
  { ndefUriPrefixRtsp, sizeof(ndefUriPrefixRtsp) - 1U },
  { ndefUriPrefixUrn, sizeof(ndefUriPrefixUrn) - 1U },
  { ndefUriPrefixPop, sizeof(ndefUriPrefixPop) - 1U },
  { ndefUriPrefixSip, sizeof(ndefUriPrefixSip) - 1U },
  { ndefUriPrefixSips, sizeof(ndefUriPrefixSips) - 1U },
  { ndefUriPrefixTftp, sizeof(ndefUriPrefixTftp) - 1U },
  { ndefUriPrefixBtspp, sizeof(ndefUriPrefixBtspp) - 1U },
  { ndefUriPrefixBtl2cap, sizeof(ndefUriPrefixBtl2cap) - 1U },
  { ndefUriPrefixBtgoep, sizeof(ndefUriPrefixBtgoep) - 1U },
  { ndefUriPrefixTcpobex, sizeof(ndefUriPrefixTcpobex) - 1U },
  { ndefUriPrefixIrdaobex, sizeof(ndefUriPrefixIrdaobex) - 1U },
  { ndefUriPrefixFile, sizeof(ndefUriPrefixFile) - 1U },
  { ndefUriPrefixUrnEpcId, sizeof(ndefUriPrefixUrnEpcId) - 1U },
  { ndefUriPrefixUrnEpcTag, sizeof(ndefUriPrefixUrnEpcTag) - 1U },
  { ndefUriPrefixUrnEpcPat, sizeof(ndefUriPrefixUrnEpcPat) - 1U },
  { ndefUriPrefixUrnEpcRaw, sizeof(ndefUriPrefixUrnEpcRaw) - 1U },
  { ndefUriPrefixUrnEpe, sizeof(ndefUriPrefixUrnEpe) - 1U },
  { ndefUriPrefixUrnNfc, sizeof(ndefUriPrefixUrnNfc) - 1U },
  { ndefUriPrefixEmpty, sizeof(ndefUriPrefixEmpty) - 1U }
};


/*
 ******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 ******************************************************************************
 */

#ifdef __cplusplus
extern "C" {
#endif
static uint32_t ndefEmptyTypePayloadGetLength(const ndefType *empty);
static const uint8_t *ndefEmptyTypePayloadItem(const ndefType *empty, ndefConstBuffer *bufItem, bool begin);
static uint32_t ndefRtdDeviceInfoPayloadGetLength(const ndefType *devInfo);
static const uint8_t *ndefRtdDeviceInfoToPayloadItem(const ndefType *devInfo, ndefConstBuffer *bufItem, bool begin);
static uint32_t ndefRtdTextPayloadGetLength(const ndefType *text);
static const uint8_t *ndefRtdTextToPayloadItem(const ndefType *text, ndefConstBuffer *bufItem, bool begin);
static uint32_t ndefRtdUriPayloadGetLength(const ndefType *uri);
static const uint8_t *ndefRtdUriToPayloadItem(const ndefType *uri, ndefConstBuffer *bufItem, bool begin);
#ifdef __cplusplus
}
#endif

/*
 ******************************************************************************
 * GLOBAL FUNCTIONS
 ******************************************************************************
 */


/*
 * Empty record
 */

#ifdef __cplusplus
extern "C" {
#endif
/*****************************************************************************/
static uint32_t ndefEmptyTypePayloadGetLength(const ndefType *empty)
{
  if ((empty == NULL) || (empty->id != NDEF_TYPE_EMPTY)) {
    return 0;
  }

  return 0;
}


/*****************************************************************************/
static const uint8_t *ndefEmptyTypePayloadItem(const ndefType *empty, ndefConstBuffer *bufItem, bool begin)
{
  if ((empty == NULL) || (empty->id != NDEF_TYPE_EMPTY)) {
    return NULL;
  }

  NO_WARNING(begin);

  if (bufItem != NULL) {
    bufItem->buffer = NULL;
    bufItem->length = 0;
  }

  return NULL;
}
#ifdef __cplusplus
}
#endif


/*****************************************************************************/
ReturnCode NdefClass::ndefEmptyType(ndefType *empty)
{
  if (empty == NULL) {
    return ERR_PARAM;
  }

  empty->id               = NDEF_TYPE_EMPTY;
  empty->getPayloadLength = ndefEmptyTypePayloadGetLength;
  empty->getPayloadItem   = ndefEmptyTypePayloadItem;

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRecordToEmptyType(const ndefRecord *record, ndefType *empty)
{
  ndefConstBuffer8 bufEmpty = { NULL, 0 };

  if ((record == NULL) || (empty == NULL)) {
    return ERR_PARAM;
  }

  if (! ndefRecordTypeMatch(record, NDEF_TNF_EMPTY, &bufEmpty)) {
    return ERR_PARAM;
  }

  if ((record->idLength          != 0U) || (record->id                != NULL) ||
      (record->bufPayload.length != 0U) || (record->bufPayload.buffer != NULL)) {
    return ERR_PARAM;
  }

  return ndefEmptyType(empty);
}


/*****************************************************************************/
ReturnCode NdefClass::ndefEmptyTypeToRecord(const ndefType *empty, ndefRecord *record)
{
  if ((empty  == NULL) || (empty->id != NDEF_TYPE_EMPTY) ||
      (record == NULL)) {
    return ERR_PARAM;
  }

  (void)ndefRecordReset(record);

  (void)ndefRecordSetNdefType(record, empty);

  return ERR_NONE;
}


/*
 * Device Information
 */

#ifdef __cplusplus
extern "C" {
#endif
/*****************************************************************************/
static uint32_t ndefRtdDeviceInfoPayloadGetLength(const ndefType *devInfo)
{
  const ndefTypeRtdDeviceInfo *rtdDevInfo;
  uint32_t payloadLength = 0;
  uint32_t i;

  if ((devInfo == NULL) || (devInfo->id != NDEF_TYPE_RTD_DEVICE_INFO)) {
    return 0;
  }

  rtdDevInfo = &devInfo->data.deviceInfo;

  for (i = 0; i < NDEF_DEVICE_INFO_TYPE_COUNT; i++) {
    if (rtdDevInfo->devInfo[i].length != 0U) {
      payloadLength += sizeof(rtdDevInfo->devInfo[i].type) + sizeof(rtdDevInfo->devInfo[i].length) + (uint32_t)rtdDevInfo->devInfo[i].length;
    }
  }

  return payloadLength;
}


/*****************************************************************************/
static const uint8_t *ndefRtdDeviceInfoToPayloadItem(const ndefType *devInfo, ndefConstBuffer *bufItem, bool begin)
{
  static uint32_t item = 0;
  const ndefTypeRtdDeviceInfo *rtdDevInfo;
  uint32_t index;

  if ((devInfo == NULL) || (devInfo->id != NDEF_TYPE_RTD_DEVICE_INFO) ||
      (bufItem == NULL)) {
    return NULL;
  }

  rtdDevInfo = &devInfo->data.deviceInfo;

  if (begin == true) {
    item = 0;
  }

  bufItem->buffer = NULL;
  bufItem->length = 0;

  index = item / 3U;

  /* Stop streaming on first empty entry */
  if (rtdDevInfo->devInfo[index].length > 0U) {
    switch (item % 3U) {
      case 0:
        bufItem->buffer = &rtdDevInfo->devInfo[index].type;
        bufItem->length = sizeof(rtdDevInfo->devInfo[index].type);
        break;
      case 1:
        bufItem->buffer = &rtdDevInfo->devInfo[index].length;
        bufItem->length = sizeof(rtdDevInfo->devInfo[index].length);
        break;
      case 2:
        bufItem->buffer = rtdDevInfo->devInfo[index].buffer;
        bufItem->length = rtdDevInfo->devInfo[index].length;
        break;
      default:
        bufItem->buffer = NULL;
        bufItem->length = 0;
        break;
    }
  }

  /* Move to next item for next call */
  item++;

  return bufItem->buffer;
}
#ifdef __cplusplus
}
#endif


/*****************************************************************************/
ReturnCode NdefClass::ndefRtdDeviceInfo(ndefType *devInfo, const ndefDeviceInfoEntry *devInfoData, uint8_t devInfoDataCount)
{
  ndefTypeRtdDeviceInfo *rtdDevInfo;
  uint32_t i;
  uint8_t  count;
  uint8_t  manufacturerNameIndex;
  uint8_t  modelNameIndex;

  if ((devInfo     == NULL)    ||
      (devInfoData == NULL)    || (devInfoData->length == 0U) ||
      (devInfoDataCount == 0U) || (devInfoDataCount > NDEF_DEVICE_INFO_TYPE_COUNT)) {
    return ERR_PARAM;
  }

  devInfo->id               = NDEF_TYPE_RTD_DEVICE_INFO;
  devInfo->getPayloadLength = ndefRtdDeviceInfoPayloadGetLength;
  devInfo->getPayloadItem   = ndefRtdDeviceInfoToPayloadItem;
  rtdDevInfo                = &devInfo->data.deviceInfo;

  /* Clear the Device Information structure before parsing */
  for (i = 0; i < NDEF_DEVICE_INFO_TYPE_COUNT; i++) {
    rtdDevInfo->devInfo[i].type   = 0;
    rtdDevInfo->devInfo[i].length = 0;
    rtdDevInfo->devInfo[i].buffer = NULL;
  }

  /* Read Type, Length and Value fields */
  /* Not checking multiple occurrences of a given field, use the last one */
  count = 0;
  manufacturerNameIndex = 0;
  modelNameIndex = 0;

  while (count < devInfoDataCount) {
    uint8_t type   = devInfoData[count].type;
    uint8_t length = devInfoData[count].length;
    if ((type == NDEF_DEVICE_INFO_UUID) && (length != NDEF_UUID_LENGTH)) {
      return ERR_PROTO;
    }
    if ((type > NDEF_DEVICE_INFO_TYPE_COUNT) || (length == 0U)) {
      return ERR_PROTO;
    }
    if (type == NDEF_DEVICE_INFO_MANUFACTURER_NAME) {
      manufacturerNameIndex = count;
    } else {
      if (type == NDEF_DEVICE_INFO_MODEL_NAME) {
        modelNameIndex = count;
      }
    }

    rtdDevInfo->devInfo[count].type   = type;
    rtdDevInfo->devInfo[count].length = length;
    rtdDevInfo->devInfo[count].buffer = devInfoData[count].buffer;
    count++;
  }

  /* Check that both required fields are there */
  if ((manufacturerNameIndex != modelNameIndex) &&
      (rtdDevInfo->devInfo[manufacturerNameIndex].buffer != NULL) &&
      (rtdDevInfo->devInfo[modelNameIndex].buffer        != NULL)) {
    return ERR_NONE;
  } else {
    return ERR_PARAM;
  }
}


/*****************************************************************************/
ReturnCode NdefClass::ndefGetRtdDeviceInfo(const ndefType *devInfo, ndefTypeRtdDeviceInfo *devInfoData)
{
  const ndefTypeRtdDeviceInfo *rtdDevInfo;
  uint32_t i;

  if ((devInfo     == NULL) || (devInfo->id != NDEF_TYPE_RTD_DEVICE_INFO) ||
      (devInfoData == NULL)) {
    return ERR_PARAM;
  }

  rtdDevInfo = &devInfo->data.deviceInfo;

  for (i = 0; i < NDEF_DEVICE_INFO_TYPE_COUNT; i++) {
    devInfoData->devInfo[i].type   = rtdDevInfo->devInfo[i].type;
    devInfoData->devInfo[i].length = rtdDevInfo->devInfo[i].length;
    devInfoData->devInfo[i].buffer = rtdDevInfo->devInfo[i].buffer;
  }

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefPayloadToRtdDeviceInfo(const ndefConstBuffer *bufDevInfo, ndefType *devInfo)
{
  ndefTypeRtdDeviceInfo *rtdDevInfo;
  uint32_t i;
  uint32_t offset;
  uint8_t  count;
  uint8_t  manufacturerNameIndex;
  uint8_t  modelNameIndex;

  if ((bufDevInfo == NULL) || (bufDevInfo->buffer == NULL) || (bufDevInfo->length == 0U) ||
      (devInfo    == NULL)) {
    return ERR_PARAM;
  }

  devInfo->id               = NDEF_TYPE_RTD_DEVICE_INFO;
  devInfo->getPayloadLength = ndefRtdDeviceInfoPayloadGetLength;
  devInfo->getPayloadItem   = ndefRtdDeviceInfoToPayloadItem;
  rtdDevInfo                = &devInfo->data.deviceInfo;

  if ((bufDevInfo->length < NDEF_RTD_DEVICE_INFO_PAYLOAD_MIN) ||
      (bufDevInfo->length > NDEF_RTD_DEVICE_INFO_PAYLOAD_MAX)) {
    return ERR_PROTO;
  }

  /* Extract device information from the buffer */

  /* Clear the Device Information structure before parsing */
  for (i = 0; i < NDEF_DEVICE_INFO_TYPE_COUNT; i++) {
    rtdDevInfo->devInfo[i].type   = 0;
    rtdDevInfo->devInfo[i].length = 0;
    rtdDevInfo->devInfo[i].buffer = NULL;
  }

  /* Read Type, Length and Value fields */
  /* Not checking multiple occurrences of a given field, use the last one */
  offset = 0;
  count = 0;
  manufacturerNameIndex = 0;
  modelNameIndex = 0;

  while (((offset + NDEF_RTD_DEVICE_INFO_TLV_LENGTH_MIN) < bufDevInfo->length)
         && (count < NDEF_DEVICE_INFO_TYPE_COUNT)) {
    uint8_t type   =  bufDevInfo->buffer[offset];
    uint8_t length =  bufDevInfo->buffer[offset + 1U];
    if ((type == NDEF_DEVICE_INFO_UUID) && (length != NDEF_UUID_LENGTH)) {
      return ERR_PROTO;
    }
    if ((type > NDEF_DEVICE_INFO_TYPE_COUNT) || (length == 0U)) {
      return ERR_PROTO;
    }
    if (type == NDEF_DEVICE_INFO_MANUFACTURER_NAME) {
      manufacturerNameIndex = count;
    } else {
      if (type == NDEF_DEVICE_INFO_MODEL_NAME) {
        modelNameIndex = count;
      }
    }

    rtdDevInfo->devInfo[count].type   = type;
    rtdDevInfo->devInfo[count].length = length;
    rtdDevInfo->devInfo[count].buffer = &bufDevInfo->buffer[offset + 2U];
    count++;

    /* Next entry */
    offset += sizeof(uint8_t) + sizeof(uint8_t) + (uint32_t)length;
  }

  /* Check both required fields are there */
  if ((manufacturerNameIndex != modelNameIndex) &&
      (rtdDevInfo->devInfo[manufacturerNameIndex].buffer != NULL) &&
      (rtdDevInfo->devInfo[modelNameIndex].buffer        != NULL)) {
    return ERR_NONE;
  } else {
    return ERR_PARAM;
  }
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRecordToRtdDeviceInfo(const ndefRecord *record, ndefType *devInfo)
{
  const ndefType *ndeftype;

  if ((record == NULL) || (devInfo == NULL)) {
    return ERR_PARAM;
  }

  if (! ndefRecordTypeMatch(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeDeviceInfo)) { /* "Di" */
    return ERR_PROTO;
  }

  ndeftype = ndefRecordGetNdefType(record);
  if (ndeftype != NULL) {
    (void)ST_MEMCPY(devInfo, ndeftype, sizeof(ndefType));
    return ERR_NONE;
  }

  if (record->bufPayload.length < NDEF_RTD_DEVICE_INFO_PAYLOAD_MIN) { /* Device Information Payload Min */
    return ERR_PROTO;
  }

  return ndefPayloadToRtdDeviceInfo(&record->bufPayload, devInfo);
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRtdDeviceInfoToRecord(const ndefType *devInfo, ndefRecord *record)
{
  if ((devInfo == NULL) || (devInfo->id != NDEF_TYPE_RTD_DEVICE_INFO) ||
      (record  == NULL)) {
    return ERR_PARAM;
  }

  (void)ndefRecordReset(record);

  /* "Di" */
  (void)ndefRecordSetType(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeDeviceInfo);

  (void)ndefRecordSetNdefType(record, devInfo);

  return ERR_NONE;
}


/*
 * Text
 */

#ifdef __cplusplus
extern "C" {
#endif
/*****************************************************************************/
static uint32_t ndefRtdTextPayloadGetLength(const ndefType *text)
{
  const ndefTypeRtdText *rtdText;

  if ((text == NULL) || (text->id != NDEF_TYPE_RTD_TEXT)) {
    return 0;
  }

  rtdText = &text->data.text;

  return sizeof(rtdText->status) + rtdText->bufLanguageCode.length + rtdText->bufSentence.length;
}


/*****************************************************************************/
static const uint8_t *ndefRtdTextToPayloadItem(const ndefType *text, ndefConstBuffer *bufItem, bool begin)
{
  static uint32_t item = 0;
  const ndefTypeRtdText *rtdText;

  if ((text    == NULL) || (text->id != NDEF_TYPE_RTD_TEXT) ||
      (bufItem == NULL)) {
    return NULL;
  }

  rtdText = &text->data.text;

  if (begin == true) {
    item = 0;
  }

  switch (item) {
    case 0:
      /* Status byte */
      bufItem->buffer = &rtdText->status;
      bufItem->length = sizeof(rtdText->status);
      break;

    case 1:
      /* Language Code */
      bufItem->buffer = rtdText->bufLanguageCode.buffer;
      bufItem->length = rtdText->bufLanguageCode.length;
      break;

    case 2:
      /* Actual text */
      bufItem->buffer = rtdText->bufSentence.buffer;
      bufItem->length = rtdText->bufSentence.length;
      break;

    default:
      bufItem->buffer = NULL;
      bufItem->length = 0;
      break;
  }

  /* Move to next item for next call */
  item++;

  return bufItem->buffer;
}
#ifdef __cplusplus
}
#endif


/*****************************************************************************/
ReturnCode NdefClass::ndefRtdText(ndefType *text, uint8_t utfEncoding, const ndefConstBuffer8 *bufLanguageCode, const ndefConstBuffer *bufSentence)
{
  ndefTypeRtdText *rtdText;

  if ((text            == NULL) ||
      (bufLanguageCode == NULL) || (bufLanguageCode->buffer == NULL) || (bufLanguageCode->length == 0U) ||
      (bufSentence     == NULL) || (bufSentence->buffer     == NULL) || (bufSentence->length     == 0U)) {
    return ERR_PARAM;
  }

  if (bufLanguageCode->length > NDEF_RTD_TEXT_LANGUAGE_CODE_LEN_MASK) {
    return ERR_PROTO;
  }

  if ((utfEncoding != TEXT_ENCODING_UTF8) && (utfEncoding != TEXT_ENCODING_UTF16)) {
    return ERR_PARAM;
  }

  text->id               = NDEF_TYPE_RTD_TEXT;
  text->getPayloadLength = ndefRtdTextPayloadGetLength;
  text->getPayloadItem   = ndefRtdTextToPayloadItem;
  rtdText                = &text->data.text;

  rtdText->status = (utfEncoding << NDEF_RTD_TEXT_ENCODING_SHIFT) | (bufLanguageCode->length & NDEF_RTD_TEXT_LANGUAGE_CODE_LEN_MASK);

  rtdText->bufLanguageCode.buffer = bufLanguageCode->buffer;
  rtdText->bufLanguageCode.length = bufLanguageCode->length;

  rtdText->bufSentence.buffer = bufSentence->buffer;
  rtdText->bufSentence.length = bufSentence->length;

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefGetRtdText(const ndefType *text, uint8_t *utfEncoding, ndefConstBuffer8 *bufLanguageCode, ndefConstBuffer *bufSentence)
{
  const ndefTypeRtdText *rtdText;

  if ((text        == NULL) || (text->id != NDEF_TYPE_RTD_TEXT) ||
      (utfEncoding == NULL) || (bufLanguageCode == NULL) || (bufSentence == NULL)) {
    return ERR_PARAM;
  }

  rtdText = &text->data.text;

  *utfEncoding            = (rtdText->status >> NDEF_RTD_TEXT_ENCODING_SHIFT) & 1U;

  bufLanguageCode->buffer = rtdText->bufLanguageCode.buffer;
  bufLanguageCode->length = rtdText->bufLanguageCode.length;

  bufSentence->buffer     = rtdText->bufSentence.buffer;
  bufSentence->length     = rtdText->bufSentence.length;

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefPayloadToRtdText(const ndefConstBuffer *bufText, ndefType *text)
{
  ndefTypeRtdText *rtdText;
  uint8_t status;
  uint8_t languageCodeLength;

  if ((bufText == NULL) || (bufText->buffer == NULL) || (bufText->length == 0U) ||
      (text    == NULL)) {
    return ERR_PARAM;
  }

  text->id               = NDEF_TYPE_RTD_TEXT;
  text->getPayloadLength = ndefRtdTextPayloadGetLength;
  text->getPayloadItem   = ndefRtdTextToPayloadItem;
  rtdText                = &text->data.text;

  /* Extract info from the payload */
  status = bufText->buffer[NDEF_RTD_TEXT_STATUS_OFFSET];

  rtdText->status = status;

  /* Extract info from the status byte */
  //uint8_t textUtfEncoding          = (status & NDEF_RTD_TEXT_ENCODING_MASK) >> NDEF_RTD_TEXT_ENCODING_SHIFT;
  languageCodeLength = (status & NDEF_RTD_TEXT_LANGUAGE_CODE_LEN_MASK);

  rtdText->bufLanguageCode.buffer = &(bufText->buffer[NDEF_RTD_TEXT_LANGUAGE_OFFSET]);
  rtdText->bufLanguageCode.length = languageCodeLength;

  rtdText->bufSentence.buffer = &(bufText->buffer[NDEF_RTD_TEXT_LANGUAGE_OFFSET + languageCodeLength]);
  rtdText->bufSentence.length = bufText->length - sizeof(status) - languageCodeLength;

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRecordToRtdText(const ndefRecord *record, ndefType *text)
{
  const ndefType *ndeftype;

  if ((record == NULL) || (text == NULL)) {
    return ERR_PARAM;
  }

  if (! ndefRecordTypeMatch(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeText)) { /* "T" */
    return ERR_PROTO;
  }

  ndeftype = ndefRecordGetNdefType(record);
  if (ndeftype != NULL) {
    (void)ST_MEMCPY(text, ndeftype, sizeof(ndefType));
    return ERR_NONE;
  }

  if (record->bufPayload.length < NDEF_RTD_TEXT_PAYLOAD_LENGTH_MIN) { /* Text Payload Min */
    return ERR_PROTO;
  }

  return ndefPayloadToRtdText(&record->bufPayload, text);
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRtdTextToRecord(const ndefType *text, ndefRecord *record)
{
  if ((text   == NULL) || (text->id != NDEF_TYPE_RTD_TEXT) ||
      (record == NULL)) {
    return ERR_PARAM;
  }

  (void)ndefRecordReset(record);

  /* "T" */
  (void)ndefRecordSetType(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeText);

  (void)ndefRecordSetNdefType(record, text);

  return ERR_NONE;
}


/*
 * URI
 */

#ifdef __cplusplus
extern "C" {
#endif
/*****************************************************************************/
static uint32_t ndefRtdUriPayloadGetLength(const ndefType *uri)
{
  const ndefTypeRtdUri *rtdUri;

  if ((uri == NULL) || (uri->id != NDEF_TYPE_RTD_URI)) {
    return 0;
  }

  rtdUri = &uri->data.uri;

  return sizeof(rtdUri->protocol) + rtdUri->bufUriString.length;
}


/*****************************************************************************/
static const uint8_t *ndefRtdUriToPayloadItem(const ndefType *uri, ndefConstBuffer *bufItem, bool begin)
{
  static uint32_t item = 0;
  const ndefTypeRtdUri *rtdUri;

  if ((uri     == NULL) || (uri->id != NDEF_TYPE_RTD_URI) ||
      (bufItem == NULL)) {
    return NULL;
  }

  rtdUri = &uri->data.uri;

  if (begin == true) {
    item = 0;
  }

  switch (item) {
    case 0:
      /* Protocol byte */
      bufItem->buffer = &rtdUri->protocol;
      bufItem->length = sizeof(rtdUri->protocol);
      break;

    case 1:
      /* URI string */
      bufItem->buffer = rtdUri->bufUriString.buffer;
      bufItem->length = rtdUri->bufUriString.length;
      break;

    default:
      bufItem->buffer = NULL;
      bufItem->length = 0;
      break;
  }

  /* Move to next item for next call */
  item++;

  return bufItem->buffer;
}
#ifdef __cplusplus
}
#endif

/*****************************************************************************/
ReturnCode NdefClass::ndefRtdUriProtocolAutodetect(uint8_t *protocol, ndefConstBuffer *bufUriString)
{
  uint8_t i; /* Protocol fits in 1 byte */

  if ((protocol  == NULL)                       ||
      (*protocol != NDEF_URI_PREFIX_AUTODETECT) ||
      (bufUriString == NULL)) {
    return ERR_PARAM;
  }

  for (i = 0; i < NDEF_URI_PREFIX_COUNT; i++) {
    if (ndefUriPrefix[i].length > 0U) {
      if (ST_BYTECMP(bufUriString->buffer, ndefUriPrefix[i].buffer, ndefUriPrefix[i].length) == 0) {
        *protocol = i;
        /* Move after the protocol string */
        bufUriString->buffer  = &bufUriString->buffer[ndefUriPrefix[i].length];
        bufUriString->length -= ndefUriPrefix[i].length;
        return ERR_NONE;
      }
    }
  }

  *protocol = NDEF_URI_PREFIX_NONE;

  return ERR_NOTFOUND;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRtdUri(ndefType *uri, uint8_t protocol, const ndefConstBuffer *bufUriString)
{
  ndefTypeRtdUri *rtdUri;
  ndefConstBuffer bufUri;
  uint8_t protocolDetect;

  if ((uri == NULL) || (protocol >= NDEF_URI_PREFIX_COUNT) ||
      (bufUriString == NULL) || (bufUriString->buffer == NULL) || (bufUriString->length == 0U)) {
    return ERR_PARAM;
  }

  uri->id               = NDEF_TYPE_RTD_URI;
  uri->getPayloadLength = ndefRtdUriPayloadGetLength;
  uri->getPayloadItem   = ndefRtdUriToPayloadItem;
  rtdUri                = &uri->data.uri;

  bufUri.buffer = bufUriString->buffer;
  bufUri.length = bufUriString->length;
  protocolDetect = protocol;
  if (protocol == NDEF_URI_PREFIX_AUTODETECT) {
    /* Update protocol and URI buffer */
    (void)ndefRtdUriProtocolAutodetect(&protocolDetect, &bufUri);
  }
  rtdUri->protocol = protocolDetect;

  rtdUri->bufUriString.buffer = bufUri.buffer;
  rtdUri->bufUriString.length = bufUri.length;

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefGetRtdUri(const ndefType *uri, ndefConstBuffer *bufProtocol, ndefConstBuffer *bufUriString)
{
  const ndefTypeRtdUri *rtdUri;

  if ((uri         == NULL) || (uri->id != NDEF_TYPE_RTD_URI) ||
      (bufProtocol == NULL) || (bufUriString == NULL)) {
    return ERR_PARAM;
  }

  rtdUri = &uri->data.uri;

  bufProtocol->buffer   = ndefUriPrefix[rtdUri->protocol].buffer;
  bufProtocol->length   = ndefUriPrefix[rtdUri->protocol].length;

  bufUriString->buffer = rtdUri->bufUriString.buffer;
  bufUriString->length = rtdUri->bufUriString.length;

  return ERR_NONE;
}

/*****************************************************************************/
ReturnCode NdefClass::ndefPayloadToRtdUri(const ndefConstBuffer *bufUri, ndefType *uri)
{
  uint8_t protocol;

  if ((bufUri == NULL) || (bufUri->buffer == NULL) || (bufUri->length == 0U) ||
      (uri    == NULL)) {
    return ERR_PARAM;
  }

  /* Extract info from the payload */
  protocol = bufUri->buffer[NDEF_RTD_URI_ID_CODE_OFFSET];

  ndefConstBuffer bufStringUri;
  bufStringUri.buffer = &bufUri->buffer[NDEF_RTD_URI_FIELD_OFFSET];
  bufStringUri.length =  bufUri->length - sizeof(protocol);

  return ndefRtdUri(uri, protocol, &bufStringUri);
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRecordToRtdUri(const ndefRecord *record, ndefType *uri)
{
  const ndefType *ndeftype;

  if ((record == NULL) || (uri == NULL)) {
    return ERR_PARAM;
  }

  if (! ndefRecordTypeMatch(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeUri)) { /* "U" */
    return ERR_PROTO;
  }

  ndeftype = ndefRecordGetNdefType(record);
  if (ndeftype != NULL) {
    (void)ST_MEMCPY(uri, ndeftype, sizeof(ndefType));
    return ERR_NONE;
  }

  if (record->bufPayload.length < NDEF_RTD_URI_PAYLOAD_LENGTH_MIN) {
    return ERR_PROTO;
  }

  return ndefPayloadToRtdUri(&record->bufPayload, uri);
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRtdUriToRecord(const ndefType *uri, ndefRecord *record)
{
  if ((uri    == NULL) || (uri->id != NDEF_TYPE_RTD_URI) ||
      (record == NULL)) {
    return ERR_PARAM;
  }

  (void)ndefRecordReset(record);

  /* "U" */
  (void)ndefRecordSetType(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeUri);

  (void)ndefRecordSetNdefType(record, uri);

  return ERR_NONE;
}


/*
 * NFC Forum External Type (Android Application Record)
 */


/*****************************************************************************/
ReturnCode NdefClass::ndefRtdAar(ndefType *aar, const ndefConstBuffer *bufPayload)
{
  ndefTypeRtdAar *rtdAar;

  if ((aar == NULL) ||
      (bufPayload == NULL) || (bufPayload->buffer == NULL) || (bufPayload->length == 0U)) {
    return ERR_PARAM;
  }

  aar->id               = NDEF_TYPE_RTD_AAR;
  aar->getPayloadLength = NULL;
  aar->getPayloadItem   = NULL;
  rtdAar                = &aar->data.aar;

  rtdAar->bufType.buffer    = bufRtdTypeAar.buffer;
  rtdAar->bufType.length    = bufRtdTypeAar.length;
  rtdAar->bufPayload.buffer = bufPayload->buffer;
  rtdAar->bufPayload.length = bufPayload->length;

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefGetRtdAar(const ndefType *aar, ndefConstBuffer *bufAarString)
{
  const ndefTypeRtdAar *rtdAar;

  if ((aar          == NULL) || (aar->id != NDEF_TYPE_RTD_AAR) ||
      (bufAarString == NULL)) {
    return ERR_PARAM;
  }

  rtdAar = &aar->data.aar;

  bufAarString->buffer = rtdAar->bufPayload.buffer;
  bufAarString->length = rtdAar->bufPayload.length;

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRecordToRtdAar(const ndefRecord *record, ndefType *aar)
{
  if ((record == NULL) || (aar == NULL)) {
    return ERR_PARAM;
  }

  if (! ndefRecordTypeMatch(record, NDEF_TNF_RTD_EXTERNAL_TYPE, &bufRtdTypeAar)) { /* "android.com:pkg" */
    return ERR_PROTO;
  }

  /* No constraint on payload length */

  return ndefRtdAar(aar, &record->bufPayload);
}


/*****************************************************************************/
ReturnCode NdefClass::ndefRtdAarToRecord(const ndefType *aar, ndefRecord *record)
{
  const ndefTypeRtdAar *rtdAar;

  if ((aar    == NULL) || (aar->id != NDEF_TYPE_RTD_AAR) ||
      (record == NULL)) {
    return ERR_PARAM;
  }

  rtdAar = &aar->data.aar;

  (void)ndefRecordReset(record);

  /* "android.com:pkg" */
  (void)ndefRecordSetType(record, NDEF_TNF_RTD_EXTERNAL_TYPE, &bufRtdTypeAar);

  (void)ndefRecordSetPayload(record, &rtdAar->bufPayload);

  return ERR_NONE;
}
