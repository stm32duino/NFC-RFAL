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
 *  \brief NDEF Wifi type header file
 *
 * NDEF Wifi type provides functionalities to handle Wifi records.
 *
 * \addtogroup NDEF
 * @{
 *
 */

#ifndef NDEF_TYPE_WIFI_H
#define NDEF_TYPE_WIFI_H


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


#define NDEF_WIFI_AUTHENTICATION_NONE       0U  /*!< WPS No Authentication (Should be 1, but set to 0 for Android native support) */
#define NDEF_WIFI_AUTHENTICATION_WPAPSK     2U  /*!< WPS Authentication based on WPAPSK  */
#define NDEF_WIFI_AUTHENTICATION_SHARED     3U  /*!< WPS Authentication                  */
#define NDEF_WIFI_AUTHENTICATION_WPA        4U  /*!< WPS Authentication based on WPA     */
#define NDEF_WIFI_AUTHENTICATION_WPA2       5U  /*!< WPS Authentication based on WPA2    */
#define NDEF_WIFI_AUTHENTICATION_WPA2PSK    6U  /*!< WPS Authentication based on WPA2PSK */


#define NDEF_WIFI_ENCRYPTION_NONE    0U  /*!< WPS No Encryption (Should be 1, but set to 0 for Android native support) */
#define NDEF_WIFI_ENCRYPTION_WEP     2U  /*!< WPS Encryption based on WEP  */
#define NDEF_WIFI_ENCRYPTION_TKIP    3U  /*!< WPS Encryption based on TKIP */
#define NDEF_WIFI_ENCRYPTION_AES     4U  /*!< WPS Encryption based on AES  */


/*
 ******************************************************************************
 * GLOBAL TYPES
 ******************************************************************************
 */


/*! Structure to store Network SSID, Authentication Type, Encryption Type and Network Key */
typedef struct {
  ndefConstBuffer bufNetworkSSID;   /*!< Network SSID        */
  ndefConstBuffer bufNetworkKey;    /*!< Network Key         */
  uint8_t         authentication;   /*!< Authentication type */
  uint8_t         encryption;       /*!< Encryption          */
} ndefTypeWifi;


/*! Wifi Record Type buffers */
extern const ndefConstBuffer8 bufMediaTypeWifi;  /*! Wifi Record Type buffer  */


/*
 ******************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 ******************************************************************************
 */


#endif /* NDEF_TYPE_WIFI_H */

/**
  * @}
  *
  */
