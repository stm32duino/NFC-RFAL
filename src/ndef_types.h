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
 *  \brief Common NDEF RTD (well-known and external) and Media types header file
 *
 * NDEF types provides an abstraction layer gathering both the RTD and MIME types
 * in a single generic ndefType interface.
 * See ndefTypeStruct and ndefType
 *
 * \addtogroup NDEF
 * @{
 *
 */

#ifndef NDEF_TYPES_H
#define NDEF_TYPES_H


/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */

#include "ndef_record.h"
#include "ndef_buffer.h"
#include "ndef_type_wifi.h"


/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */


/*! Device Information defines */
#define NDEF_UUID_LENGTH                        16U    /*!< Device Information UUID length */


/*! Text defines */
/*! RTD Type Text Encoding */
#define TEXT_ENCODING_UTF8                       0U    /*!< UTF8  text encoding  */
#define TEXT_ENCODING_UTF16                      1U    /*!< UTF16 text encoding  */

#define NDEF_RTD_TEXT_ENCODING_MASK           0x80U    /*!< Text encoding mask             */
#define NDEF_RTD_TEXT_ENCODING_SHIFT             7U    /*!< Text encoding bit shift        */


/*! RTD Type URI Protocols */
#define NDEF_URI_PREFIX_NONE          0x00U    /*!< No URI Protocol               */
#define NDEF_URI_PREFIX_HTTP_WWW      0x01U    /*!< URI Protocol http://www.      */
#define NDEF_URI_PREFIX_HTTPS_WWW     0x02U    /*!< URI Protocol https://www.     */
#define NDEF_URI_PREFIX_HTTP          0x03U    /*!< URI Protocol http://          */
#define NDEF_URI_PREFIX_HTTPS         0x04U    /*!< URI Protocol https://         */
#define NDEF_URI_PREFIX_TEL           0x05U    /*!< URI Protocol tel:             */
#define NDEF_URI_PREFIX_MAILTO        0x06U    /*!< URI Protocol mailto:          */
#define NDEF_URI_PREFIX_FTP_ANONYMOUS 0x07U    /*!< URI Protocol ftp://anonymous@ */
#define NDEF_URI_PREFIX_FTP_FTP       0x08U    /*!< URI Protocol ftp://ftp.       */
#define NDEF_URI_PREFIX_FTPS          0x09U    /*!< URI Protocol ftps://          */
#define NDEF_URI_PREFIX_SFTP          0x0AU    /*!< URI Protocol sftp://          */
#define NDEF_URI_PREFIX_SMB           0x0BU    /*!< URI Protocol smb://           */
#define NDEF_URI_PREFIX_NFS           0x0CU    /*!< URI Protocol nfs://           */
#define NDEF_URI_PREFIX_FTP           0x0DU    /*!< URI Protocol ftp://           */
#define NDEF_URI_PREFIX_DAV           0x0EU    /*!< URI Protocol dav://           */
#define NDEF_URI_PREFIX_NEWS          0x0FU    /*!< URI Protocol news:            */
#define NDEF_URI_PREFIX_TELNET        0x10U    /*!< URI Protocol telnet://        */
#define NDEF_URI_PREFIX_IMAP          0x11U    /*!< URI Protocol imap:            */
#define NDEF_URI_PREFIX_RTSP          0x12U    /*!< URI Protocol rtsp://          */
#define NDEF_URI_PREFIX_URN           0x13U    /*!< URI Protocol urn:             */
#define NDEF_URI_PREFIX_POP           0x14U    /*!< URI Protocol pop:             */
#define NDEF_URI_PREFIX_SIP           0x15U    /*!< URI Protocol sip:             */
#define NDEF_URI_PREFIX_SIPS          0x16U    /*!< URI Protocol sips:            */
#define NDEF_URI_PREFIX_TFTP          0x17U    /*!< URI Protocol tftp:            */
#define NDEF_URI_PREFIX_BTSPP         0x18U    /*!< URI Protocol btspp://         */
#define NDEF_URI_PREFIX_BTL2CAP       0x19U    /*!< URI Protocol btl2cap://       */
#define NDEF_URI_PREFIX_BTGOEP        0x1AU    /*!< URI Protocol btgoep://        */
#define NDEF_URI_PREFIX_TCPOBEX       0x1BU    /*!< URI Protocol tcpobex://       */
#define NDEF_URI_PREFIX_IRDAOBEX      0x1CU    /*!< URI Protocol irdaobex://      */
#define NDEF_URI_PREFIX_FILE          0x1DU    /*!< URI Protocol file://          */
#define NDEF_URI_PREFIX_URN_EPC_ID    0x1EU    /*!< URI Protocol urn:epc:id:      */
#define NDEF_URI_PREFIX_URN_EPC_TAG   0x1FU    /*!< URI Protocol urn:epc:tag      */
#define NDEF_URI_PREFIX_URN_EPC_PAT   0x20U    /*!< URI Protocol urn:epc:pat:     */
#define NDEF_URI_PREFIX_URN_EPC_RAW   0x21U    /*!< URI Protocol urn:epc:raw:     */
#define NDEF_URI_PREFIX_URN_EPC       0x22U    /*!< URI Protocol urn:epc:         */
#define NDEF_URI_PREFIX_URN_NFC       0x23U    /*!< URI Protocol urn:nfc:         */
#define NDEF_URI_PREFIX_AUTODETECT    0x24U    /*!< ST Protocol Autodetect        */
#define NDEF_URI_PREFIX_COUNT         0x25U    /*!< Number of URI protocols       */


/*! vCard defines */
#define NDEF_VCARD_ENTRY_MAX                    16U    /*!< vCard maximum entries */


/*
 ******************************************************************************
 * GLOBAL TYPES
 ******************************************************************************
 */


/*! RTD Device Information types */
#define NDEF_DEVICE_INFO_MANUFACTURER_NAME     0U /*!< Manufacturer name                      */
#define NDEF_DEVICE_INFO_MODEL_NAME            1U /*!< Model name                             */
#define NDEF_DEVICE_INFO_DEVICE_UNIQUE_NAME    2U /*!< Device Unique Name aka "Friendly Name" */
#define NDEF_DEVICE_INFO_UUID                  3U /*!< UUID                                   */
#define NDEF_DEVICE_INFO_FIRMWARE_VERSION      4U /*!< Firmware Version                       */
#define NDEF_DEVICE_INFO_TYPE_COUNT    5U /*!< Maximum Device Information types */


/*! RTD Device Information Entry */
typedef struct {
  uint8_t        type;      /*!< Device Information Type              */
  uint8_t        length;    /*!< Device Information length            */
  const uint8_t *buffer;    /*!< Device Information pointer to buffer */
} ndefDeviceInfoEntry;


/*! RTD Type Device Information */
typedef struct {
  ndefDeviceInfoEntry devInfo[NDEF_DEVICE_INFO_TYPE_COUNT]; /*!< Device Information entries */
} ndefTypeRtdDeviceInfo;


/*! RTD Type Text */
typedef struct {
  uint8_t          status;          /*!< Status byte                   */
  ndefConstBuffer8 bufLanguageCode; /*!< ISO/IANA language code buffer */
  ndefConstBuffer  bufSentence;     /*!< Sentence buffer               */
} ndefTypeRtdText;


/*! RTD Type URI */
typedef struct {
  uint8_t         protocol;     /*!< Protocol Identifier */
  ndefConstBuffer bufUriString; /*!< URI string buffer   */
} ndefTypeRtdUri;


/*! RTD Android Application Record External Type */
typedef struct {
  ndefConstBuffer8 bufType;    /*!< AAR type    */
  ndefConstBuffer  bufPayload; /*!< AAR payload */
} ndefTypeRtdAar;


/*! Media Type */
typedef struct {
  ndefConstBuffer8 bufType;    /*!< Media type    */
  ndefConstBuffer  bufPayload; /*!< Media payload */
} ndefTypeMedia;


/*! vCard input */
typedef struct {
  ndefConstBuffer *bufType;    /*!< Type buffer    */
  ndefConstBuffer *bufSubType; /*!< SubType buffer */
  ndefConstBuffer *bufValue;   /*!< Value buffer   */
} ndefVCardInput;


/*! vCard Entry */
typedef struct {
  const uint8_t *bufTypeBuffer;    /*!< Type buffer           */
  const uint8_t *bufSubTypeBuffer; /*!< Subtype buffer        */
  const uint8_t *bufValueBuffer;   /*!< Value buffer          */
  /* All lengths below fit in a word */
  uint8_t  bufTypeLength;          /*!< Type buffer length    */
  uint8_t  bufSubTypeLength;       /*!< Subtype buffer length */
  uint16_t bufValueLength;         /*!< Value buffer length   */
} ndefVCardEntry;


/*! NDEF Type vCard */
typedef struct {
  ndefVCardEntry entry[NDEF_VCARD_ENTRY_MAX]; /*!< vCard entries */
} ndefTypeVCard;


/*****************************************************************************/

/*! NDEF Type Id enum */
typedef enum {
  NDEF_TYPE_EMPTY = 0,
  NDEF_TYPE_RTD_DEVICE_INFO,
  NDEF_TYPE_RTD_TEXT,
  NDEF_TYPE_RTD_URI,
  NDEF_TYPE_RTD_AAR,
  NDEF_TYPE_MEDIA,
  NDEF_TYPE_MEDIA_VCARD,
  NDEF_TYPE_MEDIA_WIFI,
  NDEF_TYPE_ID_COUNT /* Keep this one last */
} ndefTypeId;


/*! NDEF abstraction Struct */
struct ndefTypeStruct {
  ndefTypeId      id;                                       /*!< Type Id           */
  uint32_t (*getPayloadLength)(const ndefType *type);       /*!< Return payload length, specific to each type */
  const uint8_t *(*getPayloadItem)(const ndefType *type, ndefConstBuffer *item, bool begin); /*!< Payload Encoder, specific to each type */
  union {
    ndefTypeRtdDeviceInfo  deviceInfo;    /*!< Device Information */
    ndefTypeRtdText        text;          /*!< Text               */
    ndefTypeRtdUri         uri;           /*!< URI                */
    ndefTypeRtdAar         aar;           /*!< AAR                */
    ndefTypeMedia          media;         /*!< Media              */
    ndefTypeVCard          vCard;         /*!< vCard              */
    ndefTypeWifi           wifi;          /*!< Wifi               */
  } data;                               /*!< Type data union        */
};


/*
 ******************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 ******************************************************************************
 */


#endif /* NDEF_TYPES_H */

/**
  * @}
  *
  */
