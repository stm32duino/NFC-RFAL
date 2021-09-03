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
 *  \brief NDEF message header file
 *
 * NDEF Message provides functionalities required to perform message management.
 * A message is a list of records.
 *
 *  The most common interfaces are:
 *    <br>&nbsp; ndefMessageReset()
 *    <br>&nbsp; ndefMessageAppend()
 *    <br>&nbsp; ndefMessageEncode()
 *    <br>&nbsp; ndefMessageDecode()
 *
 * \addtogroup NDEF
 * @{
 *
 */

#ifndef NDEF_MESSAGE_H
#define NDEF_MESSAGE_H


/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */

#include "ndef_record.h"


/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */

/*! Message scanning macros */
#define ndefMessageGetFirstRecord(message)    (((message) == NULL) ? NULL : (message)->record)  /*!< Get first record */
#define ndefMessageGetNextRecord(record)      (((record)  == NULL) ? NULL : (record)->next)     /*!< Get next record  */

/*
 ******************************************************************************
 * GLOBAL TYPES
 ******************************************************************************
 */


/*! Message information */
typedef struct {
  uint32_t length;      /*!< Message length in bytes          */
  uint32_t recordCount; /*!< Number of records in the message */
} ndefMessageInfo;


/*! NDEF message */
struct ndefMessageStruct {
  ndefRecord     *record; /*!< Pointer to a record */
  ndefMessageInfo info;   /*!< Message information, e.g. length in bytes, record count */
};


/*
 ******************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 ******************************************************************************
 */


#endif /* NDEF_MESSAGE_H */

/**
  * @}
  *
  */
