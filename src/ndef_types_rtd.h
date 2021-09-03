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
 *  \brief NDEF RTD (well-known and external) types header file
 *
 * NDEF RTD provides functionalities to handle RTD records, such as Text or URI.
 *
 * \addtogroup NDEF
 * @{
 *
 */

#ifndef NDEF_TYPES_RTD_H
#define NDEF_TYPES_RTD_H


/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */

#include "ndef_types.h"
#include "ndef_record.h"


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

/*! RTD Record Type buffers */
extern const ndefConstBuffer8 bufRtdTypeDeviceInfo; /*! Device Information Record Type buffer               */
extern const ndefConstBuffer8 bufRtdTypeText;       /*! Text Record Type buffer                             */
extern const ndefConstBuffer8 bufRtdTypeUri;        /*! URI Record Type buffer                              */
extern const ndefConstBuffer8 bufRtdTypeAar;        /*! AAR (Android Application Record) Record Type buffer */


/*
 ******************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 ******************************************************************************
 */


#endif /* NDEF_TYPES_RTD_H */

/**
  * @}
  *
  */
