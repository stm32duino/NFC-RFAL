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
 *  \brief NDEF record header file
 *
 *
 * NDEF record provides functionalities required to perform record management.
 *
 *
 * \addtogroup NDEF
 * @{
 *
 */

#ifndef NDEF_RECORD_H
#define NDEF_RECORD_H


/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */

#include "ndef_buffer.h"


/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */

#define NDEF_RECORD_HEADER_LEN       7U    /*!< Record header length (sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint8_t) */

#define NDEF_SHORT_RECORD_LENGTH_MAX 255U  /*!< Short record maximum length */


/*! Type Name Format aka TNF types */
#define NDEF_TNF_EMPTY               0U    /*!< TNF Empty             */
#define NDEF_TNF_RTD_WELL_KNOWN_TYPE 1U    /*!< TNF Well-known Type   */
#define NDEF_TNF_MEDIA_TYPE          2U    /*!< TNF Media Type        */
#define NDEF_TNF_URI                 3U    /*!< TNF URI               */
#define NDEF_TNF_RTD_EXTERNAL_TYPE   4U    /*!< TNF External Type     */
#define NDEF_TNF_UNKNOWN             5U    /*!< TNF Unknown           */
#define NDEF_TNF_UNCHANGED           6U    /*!< TNF Unchanged         */
#define NDEF_TNF_RESERVED            7U    /*!< TNF Reserved          */

#define NDEF_TNF_MASK                7U    /*!< Type Name Format mask */


/*
 ******************************************************************************
 * GLOBAL TYPES
 ******************************************************************************
 */

/*! Build the record header byte, made of MB, ME, CF, SR, IL bits and TNF type */
#define ndefHeader(MB, ME, CF, SR, IL, TNF)  ((((MB) & 1U) << 7U) | (((ME) & 1U) << 6U) | (((CF) & 1U) << 5U) | (((SR) & 1U) << 4U) | (((IL) & 1U) << 3U) | ((uint8_t)(TNF) & NDEF_TNF_MASK) )   /*< Build the record header byte, made of MB, ME, CF, SR, IL bits and TNF type */

/*! Read bits in header byte */
#define ndefHeaderMB(record)             ( ((record)->header & 0x80U) >> 7 )      /*!< Return the MB bit from the record header byte */
#define ndefHeaderME(record)             ( ((record)->header & 0x40U) >> 6 )      /*!< Return the ME bit from the record header byte */
#define ndefHeaderCF(record)             ( ((record)->header & 0x20U) >> 5 )      /*!< Return the CF bit from the record header byte */
#define ndefHeaderSR(record)             ( ((record)->header & 0x10U) >> 4 )      /*!< Return the SR bit from the record header byte */
#define ndefHeaderIL(record)             ( ((record)->header & 0x08U) >> 3 )      /*!< Return the IL bit from the record header byte */
#define ndefHeaderTNF(record)            (  (record)->header & NDEF_TNF_MASK )    /*!< Return the TNF type from the record header byte */

/*! Set bits in header byte */
#define ndefHeaderSetMB(record)          ( (record)->header |= (1U << 7) )        /*!< Set the MB bit in the record header byte */
#define ndefHeaderSetME(record)          ( (record)->header |= (1U << 6) )        /*!< Set the ME bit in the record header byte */
#define ndefHeaderSetCF(record)          ( (record)->header |= (1U << 5) )        /*!< Set the CF bit in the record header byte */
#define ndefHeaderSetSR(record)          ( (record)->header |= (1U << 4) )        /*!< Set the SR bit in the record header byte */
#define ndefHeaderSetIL(record)          ( (record)->header |= (1U << 3) )        /*!< Set the IL bit in the record header byte */
#define ndefHeaderSetTNF(record, value)  ( (record)->header |= (uint8_t)(value) & NDEF_TNF_MASK )  /*!< Set the TNF type in the record header byte */

/*! Clear bits in header byte */
#define ndefHeaderClearMB(record)          ( (record)->header &= 0x7FU )        /*!< Clear the MB bit in the record header byte */
#define ndefHeaderClearME(record)          ( (record)->header &= 0xBFU )        /*!< Clear the ME bit in the record header byte */
#define ndefHeaderClearCF(record)          ( (record)->header &= 0xDFU )        /*!< Clear the CF bit in the record header byte */
#define ndefHeaderClearSR(record)          ( (record)->header &= 0xEFU )        /*!< Clear the SR bit in the record header byte */
#define ndefHeaderClearIL(record)          ( (record)->header &= 0xF7U )        /*!< Clear the IL bit in the record header byte */
#define ndefHeaderClearTNF(record, value)  ( (record)->header &= 0xF8U )        /*!< Clear the TNF type in the record header byte */

/*! Set or Clear the MB/ME bit in header byte */
#define ndefHeaderSetValueMB(record, value)    do{ (record)->header &= 0x7FU; (record)->header |= (((uint8_t)(value)) & 1U) << 7; }while(0)   /*!< Write the value to the MB bit in the record header byte */
#define ndefHeaderSetValueME(record, value)    do{ (record)->header &= 0xBFU; (record)->header |= (((uint8_t)(value)) & 1U) << 6; }while(0)   /*!< Write the value to the ME bit in the record header byte */
#define ndefHeaderSetValueSR(record, value)    do{ (record)->header &= 0xEFU; (record)->header |= (((uint8_t)(value)) & 1U) << 4; }while(0)   /*!< Write the value to the SR bit in the record header byte */

/*! Test bit in header byte */
#define ndefHeaderIsSetMB(record)        ( ndefHeaderMB(record) == 1U )           /*!< Return true if the Message Begin bit is set */
#define ndefHeaderIsSetSR(record)        ( ndefHeaderSR(record) == 1U )           /*!< Return true if the Short Record bit is set  */
#define ndefHeaderIsSetIL(record)        ( ndefHeaderIL(record) == 1U )           /*!< Return true if the Id Length bit is set     */


typedef struct ndefTypeStruct ndefType;       /*!< Forward declaration */
typedef struct ndefMessageStruct ndefMessage; /*!< Forward declaration */

/*! Record type */
typedef struct ndefRecordStruct {
  uint8_t  header;               /*!< Header byte made of MB:1 ME:1 CF:1 SR:1 IL:1 TNF:3 => 8 bits */
  uint8_t  typeLength;           /*!< Type length in bytes */
  uint8_t  idLength;             /*!< Id Length, presence depends on the IL bit */
  const uint8_t *type;           /*!< Type follows the structure implied by the value of the TNF field */
  const uint8_t *id;             /*!< Id (middle and terminating record chunks MUST NOT have an ID field) */
  ndefConstBuffer bufPayload;    /*!< Payload buffer. Payload length depends on the SR bit (either coded on 1 or 4 bytes) */

  const ndefType *ndeftype;      /*!< Well-known type data */

  struct ndefRecordStruct *next; /*!< Pointer to the next record, if any */
} ndefRecord;


/*
 ******************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 ******************************************************************************
 */


#endif /* NDEF_RECORD_H */

/**
  * @}
  *
  */
