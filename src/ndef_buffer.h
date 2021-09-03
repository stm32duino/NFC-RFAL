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
 *  \brief NDEF buffer type structures
 *
 */

#ifndef NDEF_BUFFER_H
#define NDEF_BUFFER_H


/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */


/*
 ******************************************************************************
 * GLOBAL TYPES
 ******************************************************************************
 */


/*! NDEF structure to handle const buffers */
typedef struct {
  const uint8_t *buffer; /*!< Pointer to const buffer */
  uint32_t       length; /*!< buffer length           */
} ndefConstBuffer;


/*! NDEF structure to handle buffers */
typedef struct {
  uint8_t *buffer; /*!< Pointer to buffer */
  uint32_t length; /*!< buffer length     */
} ndefBuffer;


/*! NDEF structure to handle const buffers limited to 256 bytes */
typedef struct {
  const uint8_t *buffer; /*!< Pointer to const buffer */
  uint8_t        length; /*!< buffer length           */
} ndefConstBuffer8;


/*! NDEF structure to handle buffers limited to 256 bytes */
typedef struct {
  uint8_t *buffer; /*!< Pointer to buffer */
  uint8_t  length; /*!< buffer length     */
} ndefBuffer8;


#endif /* NDEF_BUFFER_H */
