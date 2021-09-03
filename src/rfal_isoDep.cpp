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
 *  \brief Implementation of ISO-DEP protocol
 *
 *  This implementation was based on the following specs:
 *    - ISO/IEC 14443-4  2nd Edition 2008-07-15
 *    - NFC Forum Digital Protocol  1.1 2014-01-14
 *
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "rfal_nfc.h"
#include "rfal_isoDep.h"
#include "rfal_rf.h"
#include "nfc_utils.h"

/*
 ******************************************************************************
 * ENABLE SWITCH
 ******************************************************************************
 */


/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#define ISODEP_CRC_LEN                  RFAL_CRC_LEN   /*!< ISO1443 CRC Length */


#define ISODEP_PCB_POS                  (0U)         /*!< PCB position on message header*/
#define ISODEP_SWTX_INF_POS             (1U)         /*!< INF position in a S-WTX       */

#define ISODEP_DID_POS                  (1U)         /*!< DID position on message header*/

#define ISODEP_DSL_MAX_LEN              ( RFAL_ISODEP_PCB_LEN + RFAL_ISODEP_DID_LEN ) /*!< Deselect Req/Res length */

#define ISODEP_PCB_xBLOCK_MASK          (0xC0U)      /*!< Bit mask for Block type       */
#define ISODEP_PCB_IBLOCK               (0x00U)      /*!< Bit mask indicating a I-Block */
#define ISODEP_PCB_RBLOCK               (0x80U)      /*!< Bit mask indicating a R-Block */
#define ISODEP_PCB_SBLOCK               (0xC0U)      /*!< Bit mask indicating a S-Block */
#define ISODEP_PCB_INVALID              (0x40U)      /*!< Bit mask of an Invalid PCB    */

#define ISODEP_HDR_MAX_LEN              (RFAL_ISODEP_PCB_LEN + RFAL_ISODEP_DID_LEN + RFAL_ISODEP_NAD_LEN)          /*!< Max header length (PCB + DID + NAD)      */

#define ISODEP_PCB_IB_VALID_MASK        (ISODEP_PCB_B6_BIT | ISODEP_PCB_B2_BIT)                     /*!< Bit mask for the MUST bits on I-Block    */
#define ISODEP_PCB_IB_VALID_VAL         (ISODEP_PCB_B2_BIT)                                         /*!< Value for the MUST bits on I-Block       */
#define ISODEP_PCB_RB_VALID_MASK        (ISODEP_PCB_B6_BIT | ISODEP_PCB_B3_BIT | ISODEP_PCB_B2_BIT) /*!< Bit mask for the MUST bits on R-Block    */
#define ISODEP_PCB_RB_VALID_VAL         (ISODEP_PCB_B6_BIT | ISODEP_PCB_B2_BIT)                     /*!< Value for the MUST bits on R-Block       */
#define ISODEP_PCB_SB_VALID_MASK        (ISODEP_PCB_B3_BIT | ISODEP_PCB_B2_BIT | ISODEP_PCB_B1_BIT) /*!< Bit mask for the MUST bits on I-Block    */
#define ISODEP_PCB_SB_VALID_VAL         (ISODEP_PCB_B2_BIT)                                         /*!< Value for the MUST bits on I-Block       */


#define ISODEP_PCB_B1_BIT               (0x01U)      /*!< Bit mask for the RFU S Blocks                                        */
#define ISODEP_PCB_B2_BIT               (0x02U)      /*!< Bit mask for the RFU bit2 in I,S,R Blocks                            */
#define ISODEP_PCB_B3_BIT               (0x04U)      /*!< Bit mask for the RFU bit3 in R Blocks                                */
#define ISODEP_PCB_B6_BIT               (0x20U)      /*!< Bit mask for the RFU bit2 in R Blocks                                */
#define ISODEP_PCB_CHAINING_BIT         (0x10U)      /*!< Bit mask for the chaining bit of an ISO DEP I-Block in PCB.          */
#define ISODEP_PCB_DID_BIT              (0x08U)      /*!< Bit mask for the DID presence bit of an ISO DEP I,S,R Blocks PCB.    */
#define ISODEP_PCB_NAD_BIT              (0x04U)      /*!< Bit mask for the NAD presence bit of an ISO DEP I,S,R Blocks in PCB  */
#define ISODEP_PCB_BN_MASK              (0x01U)      /*!< Bit mask for the block number of an ISO DEP I,R Block in PCB         */

#define ISODEP_SWTX_PL_MASK             (0xC0U)      /*!< Bit mask for the Power Level bits of the inf byte of an WTX request or response */
#define ISODEP_SWTX_WTXM_MASK           (0x3FU)      /*!< Bit mask for the WTXM bits of the inf byte of an WTX request or response        */


#define ISODEP_RBLOCK_INF_LEN           (0U)         /*!< INF length of R-Block               Digital 1.1 15.1.3 */
#define ISODEP_SDSL_INF_LEN             (0U)         /*!< INF length of S(DSL)                Digital 1.1 15.1.3 */
#define ISODEP_SWTX_INF_LEN             (1U)         /*!< INF length of S(WTX)                Digital 1.1 15.2.2 */

#define ISODEP_WTXM_MIN                 (1U)         /*!< Minimum allowed value for the WTXM, Digital 1.0 13.2.2 */
#define ISODEP_WTXM_MAX                 (59U)        /*!< Maximum allowed value for the WTXM, Digital 1.0 13.2.2 */

#define ISODEP_PCB_Sxx_MASK             (0x30U)      /*!< Bit mask for the S-Block type                          */
#define ISODEP_PCB_DESELECT             (0x00U)      /*!< Bit mask for S-Block indicating Deselect               */
#define ISODEP_PCB_WTX                  (0x30U)      /*!< Bit mask for S-Block indicating Waiting Time eXtension */

#define ISODEP_PCB_Rx_MASK              (0x10U)      /*!< Bit mask for the R-Block type       */
#define ISODEP_PCB_ACK                  (0x00U)      /*!< Bit mask for R-Block indicating ACK */
#define ISODEP_PCB_NAK                  (0x10U)      /*!< Bit mask for R-Block indicating NAK */

#define ISODEP_FWT_DEACTIVATION         (71680U)     /*!< FWT to be used after DESELECT, Digital 1.0 A9   */
#define ISODEP_MAX_RERUNS               (0x0FFFFFFFU)/*!< Maximum rerun retrys for a blocking protocol run*/


#define ISODEP_PCBSBLOCK                ( 0x00U | ISODEP_PCB_SBLOCK | ISODEP_PCB_B2_BIT ) /*!< PCB Value of a S-Block                 */
#define ISODEP_PCB_SDSL                 ( ISODEP_PCBSBLOCK | ISODEP_PCB_DESELECT )        /*!< PCB Value of a S-Block with DESELECT   */
#define ISODEP_PCB_SWTX                 ( ISODEP_PCBSBLOCK | ISODEP_PCB_WTX )             /*!< PCB Value of a S-Block with WTX        */
#define ISODEP_PCB_SPARAMETERS          ( ISODEP_PCB_SBLOCK | ISODEP_PCB_WTX )            /*!< PCB Value of a S-Block with PARAMETERS */

#define ISODEP_FWI_LIS_MAX_NFC          8U                            /*!< FWT Listener Max FWIT4ATmax FWIBmax  Digital 1.1  A6 & A3  */
#define ISODEP_FWI_LIS_MAX_EMVCO        7U                            /*!< FWT Listener Max FWIMAX       EMVCo 2.6 A.5                */
#define ISODEP_FWI_LIS_MAX              (uint8_t)((gIsoDep.compMode == RFAL_COMPLIANCE_MODE_EMV) ? ISODEP_FWI_LIS_MAX_EMVCO : ISODEP_FWI_LIS_MAX_NFC)  /*!< FWI Listener Max as NFC / EMVCo */
#define ISODEP_FWT_LIS_MAX              rfalIsoDepFWI2FWT(ISODEP_FWI_LIS_MAX)             /*!< FWT Listener Max                       */

#define ISODEP_FWI_MIN_10               (1U)      /*!< Minimum value for FWI Digital 1.0 11.6.2.17 */
#define ISODEP_FWI_MIN_11               (0U)      /*!< Default value for FWI Digital 1.1 13.6.2    */
#define ISODEP_FWI_MAX                  (14U)     /*!< Maximum value for FWI Digital 1.0 11.6.2.17 */
#define ISODEP_SFGI_MIN                 (0U)      /*!< Default value for FWI Digital 1.1 13.6.2.22 */
#define ISODEP_SFGI_MAX                 (14U)     /*!< Maximum value for FWI Digital 1.1 13.6.2.22 */


#define RFAL_ISODEP_SPARAM_TVL_HDR_LEN  (2U)                                                   /*!< S(PARAMETERS) TVL header length: Tag + Len */
#define RFAL_ISODEP_SPARAM_HDR_LEN      (RFAL_ISODEP_PCB_LEN + RFAL_ISODEP_SPARAM_TVL_HDR_LEN) /*!< S(PARAMETERS) header length: PCB + Tag + Len */


/**********************************************************************************************************************/
/**********************************************************************************************************************/
#define RFAL_ISODEP_NO_PARAM                   (0U)     /*!< No parameter flag for isoDepHandleControlMsg()     */

#define RFAL_ISODEP_CMD_RATS                   (0xE0U)  /*!< RATS command   Digital 1.1  13.6.1                 */

#define RFAL_ISODEP_ATS_MIN_LEN                (1U)                                                   /*!< Minimum ATS length   Digital 1.1  13.6.2 */
#define RFAL_ISODEP_ATS_HDR_LEN                (5U)                                                   /*!< ATS headerlength     Digital 1.1  13.6.2 */
#define RFAL_ISODEP_ATS_MAX_LEN                (RFAL_ISODEP_ATS_HDR_LEN + RFAL_ISODEP_ATS_HB_MAX_LEN) /*!< Maximum ATS length   Digital 1.1  13.6.2 */
#define RFAL_ISODEP_ATS_T0_FSCI_MASK           (0x0FU)                                                /*!< ATS T0's FSCI mask   Digital 1.1  13.6.2 */
#define RFAL_ISODEP_ATS_TB_FWI_SHIFT           (4U)                                                   /*!< ATS TB's FWI shift   Digital 1.1  13.6.2 */
#define RFAL_ISODEP_ATS_FWI_MASK               (0x0FU)                                                /*!< ATS TB's FWI shift   Digital 1.1  13.6.2 */


#define RFAL_ISODEP_PPS_SB                     (0xD0U)  /*!< PPS REQ PPSS's SB value (no CID)   ISO14443-4  5.3 */
#define RFAL_ISODEP_PPS_MASK                   (0xF0U)  /*!< PPS REQ PPSS's SB mask             ISO14443-4  5.3 */
#define RFAL_ISODEP_PPS_SB_DID_MASK            (0x0FU)  /*!< PPS REQ PPSS's DID|CID mask        ISO14443-4  5.3 */
#define RFAL_ISODEP_PPS_PPS0_PPS1_PRESENT      (0x11U)  /*!< PPS REQ PPS0 indicating that PPS1 is present       */
#define RFAL_ISODEP_PPS_PPS1                   (0x00U)  /*!< PPS REQ PPS1 fixed value           ISO14443-4  5.3 */
#define RFAL_ISODEP_PPS_PPS1_DSI_SHIFT         (2U)     /*!< PPS REQ PPS1 fixed value           ISO14443-4  5.3 */
#define RFAL_ISODEP_PPS_PPS1_DXI_MASK          (0x0FU)  /*!< PPS REQ PPS1 fixed value           ISO14443-4  5.3 */
#define RFAL_ISODEP_PPS_RES_LEN                (1U)     /*!< PPS Response length                ISO14443-4  5.4 */
#define RFAL_ISODEP_PPS_STARTBYTE_POS          (0U)     /*!< PPS REQ PPSS's byte position       ISO14443-4  5.4 */
#define RFAL_ISODEP_PPS_PPS0_POS               (1U)     /*!< PPS REQ PPS0's byte position       ISO14443-4  5.4 */
#define RFAL_ISODEP_PPS_PPS1_POS               (2U)     /*!< PPS REQ PPS1's byte position       ISO14443-4  5.4 */
#define RFAL_ISODEP_PPS0_VALID_MASK            (0xEFU)  /*!< PPS REQ PPS0 valid coding mask     ISO14443-4  5.4 */

#define RFAL_ISODEP_CMD_ATTRIB                 (0x1DU)  /*!< ATTRIB command                 Digital 1.1  14.6.1 */
#define RFAL_ISODEP_ATTRIB_PARAM2_DSI_SHIFT    (6U)     /*!< ATTRIB PARAM2 DSI shift        Digital 1.1  14.6.1 */
#define RFAL_ISODEP_ATTRIB_PARAM2_DRI_SHIFT    (4U)     /*!< ATTRIB PARAM2 DRI shift        Digital 1.1  14.6.1 */
#define RFAL_ISODEP_ATTRIB_PARAM2_DXI_MASK     (0xF0U)  /*!< ATTRIB PARAM2 DxI mask         Digital 1.1  14.6.1 */
#define RFAL_ISODEP_ATTRIB_PARAM2_FSDI_MASK    (0x0FU)  /*!< ATTRIB PARAM2 FSDI mask        Digital 1.1  14.6.1 */
#define RFAL_ISODEP_ATTRIB_PARAM4_DID_MASK     (0x0FU)  /*!< ATTRIB PARAM4 DID mask         Digital 1.1  14.6.1 */
#define RFAL_ISODEP_ATTRIB_HDR_LEN             (9U)     /*!< ATTRIB REQ header length       Digital 1.1  14.6.1 */

#define RFAL_ISODEP_ATTRIB_RES_HDR_LEN         (1U)     /*!< ATTRIB RES header length       Digital 1.1  14.6.2 */
#define RFAL_ISODEP_ATTRIB_RES_DID_MASK        (0x0FU)  /*!< ATTRIB RES DID mask            Digital 1.1  14.6.2 */
#define RFAL_ISODEP_ATTRIB_RES_MBLI_MASK       (0x0FU)  /*!< ATTRIB RES MBLI mask           Digital 1.1  14.6.2 */
#define RFAL_ISODEP_ATTRIB_RES_MBLI_SHIFT      (4U)     /*!< ATTRIB RES MBLI shift          Digital 1.1  14.6.2 */

#define RFAL_ISODEP_DID_MASK                   (0x0FU)  /*!< ISODEP's DID mask                                  */
#define RFAL_ISODEP_DID_00                     (0U)     /*!< ISODEP's DID value 0                               */

#define RFAL_ISODEP_FSDI_MAX_NFC               (8U)     /*!< Max FSDI value   Digital 2.0  14.6.1.9 & B7 & B8   */
#define RFAL_ISODEP_FSDI_MAX_EMV               (0x0CU)  /*!< Max FSDI value   EMVCo 3.0  5.7.2.5                */

#define RFAL_ISODEP_RATS_PARAM_FSDI_MASK       (0xF0U)  /*!< Mask bits for FSDI in RATS                         */
#define RFAL_ISODEP_RATS_PARAM_FSDI_SHIFT      (4U)     /*!< Shift for FSDI in RATS                             */
#define RFAL_ISODEP_RATS_PARAM_DID_MASK        (0x0FU)  /*!< Mask bits for DID in RATS                          */

#define RFAL_ISODEP_ATS_TL_OFFSET              (0x00U)  /*!< Offset of TL on ATS                                */
#define RFAL_ISODEP_ATS_TA_OFFSET              (0x02U)  /*!< Offset of TA if it is present on ATS               */
#define RFAL_ISODEP_ATS_TB_OFFSET              (0x03U)  /*!< Offset of TB if both TA and TB is present on ATS   */
#define RFAL_ISODEP_ATS_TC_OFFSET              (0x04U)  /*!< Offset of TC if both TA,TB & TC are present on ATS */
#define RFAL_ISODEP_ATS_HIST_OFFSET            (0x05U)  /*!< Offset of Historical Bytes if TA, TB & TC are present on ATS          */
#define RFAL_ISODEP_ATS_TC_ADV_FEAT            (0x10U)  /*!< Bit mask indicating support for Advanced protocol features: DID & NAD */
#define RFAL_ISODEP_ATS_TC_DID                 (0x02U)  /*!< Bit mask indicating support for DID                 */
#define RFAL_ISODEP_ATS_TC_NAD                 (0x01U)  /*!< Bit mask indicating support for NAD                 */

#define RFAL_ISODEP_PPS0_PPS1_PRESENT          (0x11U) /*!< PPS0 byte indicating that PPS1 is present            */
#define RFAL_ISODEP_PPS0_PPS1_NOT_PRESENT      (0x01U) /*!< PPS0 byte indicating that PPS1 is NOT present        */
#define RFAL_ISODEP_PPS1_DRI_MASK              (0x03U) /*!< PPS1 byte DRI mask bits                              */
#define RFAL_ISODEP_PPS1_DSI_MASK              (0x0CU) /*!< PPS1 byte DSI mask bits                              */
#define RFAL_ISODEP_PPS1_DSI_SHIFT             (2U)    /*!< PPS1 byte DSI shift                                  */
#define RFAL_ISODEP_PPS1_DxI_MASK              (0x03U) /*!< PPS1 byte DSI/DRS mask bits                          */


/*! Delta Time for polling during Activation (ATS) : 20ms    Digital 1.0 11.7.1.1 & A.7    */
#define RFAL_ISODEP_T4T_DTIME_POLL_10          rfalConvMsTo1fc(20)

/*! Delta Time for polling during Activation (ATS) : 16.4ms  Digital 1.1 13.8.1.1 & A.6
 *  Use 16 ms as testcase T4AT_BI_10_03 sends a frame exactly at the border */
#define RFAL_ISODEP_T4T_DTIME_POLL_11          216960U

/*! Activation frame waiting time FWT(act) = 71680/fc (~5286us) Digital 1.1 13.8.1.1 & A.6 */
#define RFAL_ISODEP_T4T_FWT_ACTIVATION         (71680U + RFAL_ISODEP_T4T_DTIME_POLL_11)


/*! Delta frame waiting time = 16/fc  Digital 1.0  11.7.1.3 & A.7*/
#define RFAL_ISODEP_DFWT_10                      16U

/*! Delta frame waiting time = 16/fc  Digital 2.0  14.8.1.3 & B.7*/
#define RFAL_ISODEP_DFWT_20                      49152U

/*
 ******************************************************************************
 * MACROS
 ******************************************************************************
 */

#define isoDep_PCBisIBlock( pcb )       ( ((pcb) & (ISODEP_PCB_xBLOCK_MASK | ISODEP_PCB_IB_VALID_MASK)) == (ISODEP_PCB_IBLOCK | ISODEP_PCB_IB_VALID_VAL)) /*!< Checks if pcb is a I-Block */
#define isoDep_PCBisRBlock( pcb )       ( ((pcb) & (ISODEP_PCB_xBLOCK_MASK | ISODEP_PCB_RB_VALID_MASK)) == (ISODEP_PCB_RBLOCK | ISODEP_PCB_RB_VALID_VAL)) /*!< Checks if pcb is a R-Block */
#define isoDep_PCBisSBlock( pcb )       ( ((pcb) & (ISODEP_PCB_xBLOCK_MASK | ISODEP_PCB_SB_VALID_MASK)) == (ISODEP_PCB_SBLOCK | ISODEP_PCB_SB_VALID_VAL)) /*!< Checks if pcb is a S-Block */

#define isoDep_PCBisChaining( pcb )     ( ((pcb) & ISODEP_PCB_CHAINING_BIT) == ISODEP_PCB_CHAINING_BIT) /*!< Checks if pcb is indicating chaining */

#define isoDep_PCBisDeselect( pcb )     ( ((pcb) & ISODEP_PCB_Sxx_MASK) == ISODEP_PCB_DESELECT)         /*!< Checks if pcb is indicating DESELECT */
#define isoDep_PCBisWTX( pcb )          ( ((pcb) & ISODEP_PCB_Sxx_MASK) == ISODEP_PCB_WTX)              /*!< Checks if pcb is indicating WTX      */

#define isoDep_PCBisACK( pcb )          ( ((pcb) & ISODEP_PCB_Rx_MASK) == ISODEP_PCB_ACK)               /*!< Checks if pcb is indicating ACK      */
#define isoDep_PCBisNAK( pcb )          ( ((pcb) & ISODEP_PCB_Rx_MASK) == ISODEP_PCB_NAK)               /*!< Checks if pcb is indicating ACK      */

#define isoDep_PCBhasDID( pcb )         ( ((pcb) & ISODEP_PCB_DID_BIT) == ISODEP_PCB_DID_BIT)           /*!< Checks if pcb is indicating DID      */
#define isoDep_PCBhasNAD( pcb )         ( ((pcb) & ISODEP_PCB_NAD_BIT) == ISODEP_PCB_NAD_BIT)           /*!< Checks if pcb is indicating NAD      */


#define isoDep_PCBisIChaining( pcb )    ( isoDep_PCBisIBlock(pcb) && isoDep_PCBisChaining(pcb) )       /*!< Checks if pcb is I-Block indicating chaining*/

#define isoDep_PCBisSDeselect( pcb )    ( isoDep_PCBisSBlock(pcb) && isoDep_PCBisDeselect(pcb) )       /*!< Checks if pcb is S-Block indicating DESELECT*/
#define isoDep_PCBisSWTX( pcb )         ( isoDep_PCBisSBlock(pcb) && isoDep_PCBisWTX(pcb) )            /*!< Checks if pcb is S-Block indicating WTX     */

#define isoDep_PCBisRACK( pcb )         ( isoDep_PCBisRBlock(pcb) && isoDep_PCBisACK(pcb) )            /*!< Checks if pcb is R-Block indicating ACK     */
#define isoDep_PCBisRNAK( pcb )         ( isoDep_PCBisRBlock(pcb) && isoDep_PCBisNAK(pcb) )            /*!< Checks if pcb is R-Block indicating NAK     */


#define isoDep_PCBIBlock( bn )          ( (uint8_t)( 0x00U | ISODEP_PCB_IBLOCK | ISODEP_PCB_B2_BIT | ((bn) & ISODEP_PCB_BN_MASK) ))  /*!< Returns an I-Block with the given block number (bn)                     */
#define isoDep_PCBIBlockChaining( bn )  ( (uint8_t)(isoDep_PCBIBlock(bn) | ISODEP_PCB_CHAINING_BIT))                              /*!< Returns an I-Block with the given block number (bn) indicating chaining */

#define isoDep_PCBRBlock( bn )          ( (uint8_t)( 0x00U | ISODEP_PCB_RBLOCK | ISODEP_PCB_B6_BIT | ISODEP_PCB_B2_BIT | ((bn) & ISODEP_PCB_BN_MASK) ) ) /*!< Returns an R-Block with the given block number (bn)                */
#define isoDep_PCBRACK( bn )            ( (uint8_t)( isoDep_PCBRBlock( bn ) | ISODEP_PCB_ACK ) )                                                      /*!< Returns an R-Block with the given block number (bn) indicating ACK */
#define isoDep_PCBRNAK( bn )            ( (uint8_t)( isoDep_PCBRBlock( bn ) | ISODEP_PCB_NAK ) )                                                      /*!< Returns an R-Block with the given block number (bn) indicating NAK */


#define isoDep_GetBN( pcb )             ( (uint8_t) ((pcb) & ISODEP_PCB_BN_MASK   ) )                    /*!< Returns the block number (bn) from the given pcb */
#define isoDep_GetWTXM( inf )           ( (uint8_t) ((inf) & ISODEP_SWTX_WTXM_MASK) )                    /*!< Returns the WTX value from the given inf byte    */
#define isoDep_isWTXMValid( wtxm )      (((wtxm) >= ISODEP_WTXM_MIN) && ((wtxm) <= ISODEP_WTXM_MAX))     /*!< Checks if the given wtxm is valid                */

#define isoDep_WTXMListenerMax( fwt )   ( MIN( (uint8_t)(ISODEP_FWT_LIS_MAX / (fwt)), ISODEP_WTXM_MAX) ) /*!< Calculates the Max WTXM value for the given fwt as a Listener    */

#define isoDepCalcdSGFT( s )            (384U  * ((uint32_t)1U << (s)))                                  /*!< Calculates the dSFGT with given SFGI  Digital 1.1  13.8.2.1 & A.6*/
#define isoDepCalcSGFT( s )             (4096U * ((uint32_t)1U << (s)))                                  /*!< Calculates the SFGT with given SFGI  Digital 1.1  13.8.2         */

#define isoDep_PCBNextBN( bn )          (((uint8_t)(bn)^0x01U) & ISODEP_PCB_BN_MASK)                     /*!< Returns the value of the next block number based on bn     */
#define isoDep_PCBPrevBN( bn )          isoDep_PCBNextBN(bn)                                             /*!< Returns the value of the previous block number based on bn */
#define isoDep_ToggleBN( bn )           ((bn) = (((bn)^0x01U) & ISODEP_PCB_BN_MASK) )                    /*!< Toggles the block number value of the given bn             */

#define isoDep_WTXAdjust( v )           ((v) - ((v)>>3))                                                 /*!< Adjust WTX timer value to a percentage of the total, current 88% */


/*! ISO 14443-4 7.5.6.2 & Digital 1.1 - 15.2.6.2  The CE SHALL NOT attempt error recovery and remains in Rx mode upon Transmission or a Protocol Error */
#define isoDepReEnableRx( rxB, rxBL, rxL )              rfalRfDev->rfalTransceiveBlockingTx( NULL, 0, rxB, rxBL, rxL, RFAL_TXRX_FLAGS_DEFAULT, RFAL_FWT_NONE )

#define isoDepTimerStart( timer, time_ms ) (timer) = timerCalculateTimer((uint16_t)(time_ms))            /*!< Configures and starts the WTX timer  */
#define isoDepTimerisExpired( timer )      timerIsExpired( timer )                               /*!< Checks WTX timer has expired         */

/*
 ******************************************************************************
 * LOCAL DATA TYPES
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


/*
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 */

/*******************************************************************************/
void RfalNfcClass::isoDepClearCounters(void)
{
  gIsoDep.cntIRetrys   = 0;
  gIsoDep.cntRRetrys   = 0;
  gIsoDep.cntSRetrys   = 0;
}

/*******************************************************************************/
ReturnCode RfalNfcClass::isoDepTx(uint8_t pcb, const uint8_t *txBuf, uint8_t *infBuf, uint16_t infLen, uint32_t fwt)
{
  uint8_t    *txBlock;
  uint16_t   txBufLen;
  uint8_t    computedPcb;


  txBlock         = infBuf;                      /* Point to beginning of the INF, and go backwards     */
  gIsoDep.lastPCB = pcb;                         /* Store the last PCB sent                             */


  if (infLen > 0U) {
    if (((uint32_t)infBuf - (uint32_t)txBuf) < gIsoDep.hdrLen) { /* Check that we can fit the header in the given space */
      return ERR_NOMEM;
    }
  }


  /*******************************************************************************/
  /* Compute optional PCB bits */
  computedPcb = pcb;
  if ((gIsoDep.did != RFAL_ISODEP_NO_DID) || ((gIsoDep.did == RFAL_ISODEP_DID_00) && gIsoDep.lastDID00)) {
    computedPcb |= ISODEP_PCB_DID_BIT;
  }
  if (gIsoDep.nad != RFAL_ISODEP_NO_NAD)                                                                  {
    computedPcb |= ISODEP_PCB_NAD_BIT;
  }
  if ((gIsoDep.isTxChaining) && (isoDep_PCBisIBlock(computedPcb)))                                       {
    computedPcb |= ISODEP_PCB_CHAINING_BIT;
  }


  /*******************************************************************************/
  /* Compute Payload on the given txBuf, start by the PCB | DID | NAD | before INF */

  if (gIsoDep.nad != RFAL_ISODEP_NO_NAD) {
    *(--txBlock) = gIsoDep.nad;                /* NAD is optional */
  }

  if ((gIsoDep.did != RFAL_ISODEP_NO_DID) || ((gIsoDep.did == RFAL_ISODEP_DID_00) && gIsoDep.lastDID00)) {
    *(--txBlock)  = gIsoDep.did;               /* DID is optional */
  }

  *(--txBlock)      = computedPcb;               /* PCB always present */

  txBufLen = (infLen + (uint16_t)((uint32_t)infBuf - (uint32_t)txBlock)); /* Calculate overall buffer size */

  if (txBufLen > (gIsoDep.fsx - ISODEP_CRC_LEN)) {                        /* Check if msg length violates the maximum frame size FSC */
    return ERR_NOTSUPP;
  }

  return rfalRfDev->rfalTransceiveBlockingTx(txBlock, txBufLen, gIsoDep.rxBuf, gIsoDep.rxBufLen, gIsoDep.rxLen, RFAL_TXRX_FLAGS_DEFAULT, ((gIsoDep.role == ISODEP_ROLE_PICC) ? RFAL_FWT_NONE : fwt));
}

/*******************************************************************************/
ReturnCode RfalNfcClass::isoDepHandleControlMsg(rfalIsoDepControlMsg controlMsg, uint8_t param)
{
  uint8_t  pcb;
  uint8_t  ctrlMsgBuf[ISODEP_CONTROLMSG_BUF_LEN];
  uint8_t  infLen;
  uint32_t fwtTemp;

  infLen  = 0;
  fwtTemp = (gIsoDep.fwt + gIsoDep.dFwt);
  ST_MEMSET(ctrlMsgBuf, 0x00, ISODEP_CONTROLMSG_BUF_LEN);

  switch (controlMsg) {
    /*******************************************************************************/
    case ISODEP_R_ACK:

      if (gIsoDep.cntRRetrys++ > gIsoDep.maxRetriesR) {
        return ERR_PROTO;
      }

      pcb = isoDep_PCBRACK(gIsoDep.blockNumber);
      break;

    /*******************************************************************************/
    case ISODEP_R_NAK:
      if (gIsoDep.cntRRetrys++ > gIsoDep.maxRetriesR) {
        return ERR_TIMEOUT;
      }

      pcb = isoDep_PCBRNAK(gIsoDep.blockNumber);
      break;

    /*******************************************************************************/
    case ISODEP_S_WTX:
      if (gIsoDep.cntSRetrys++ > gIsoDep.maxRetriesS) {
        return ERR_PROTO;
      }

      /* Check if WTXM is valid */
      if (! isoDep_isWTXMValid(param)) {
        return ERR_PROTO;
      }

      if (gIsoDep.role == ISODEP_ROLE_PCD) {
        /* Calculate temp Wait Time eXtension */
        fwtTemp = (gIsoDep.fwt * param);
        fwtTemp = MIN(RFAL_ISODEP_MAX_FWT, fwtTemp);
        fwtTemp += gIsoDep.dFwt;
      }

      pcb = ISODEP_PCB_SWTX;
      ctrlMsgBuf[ RFAL_ISODEP_PCB_LEN + RFAL_ISODEP_DID_LEN + infLen++] = param;
      break;

    /*******************************************************************************/
    case ISODEP_S_DSL:
      if (gIsoDep.cntSRetrys++ > gIsoDep.maxRetriesS) {
        return ERR_PROTO;
      }

      if (gIsoDep.role == ISODEP_ROLE_PCD) {
        /* Digital 1.0 - 13.2.7.3 Poller must wait fwtDEACTIVATION */
        fwtTemp = ISODEP_FWT_DEACTIVATION;
        gIsoDep.state = ISODEP_ST_PCD_WAIT_DSL;
      }
      pcb = ISODEP_PCB_SDSL;
      break;

    /*******************************************************************************/
    default:
      return ERR_INTERNAL;
  }

  return isoDepTx(pcb, ctrlMsgBuf, &ctrlMsgBuf[RFAL_ISODEP_PCB_LEN + RFAL_ISODEP_DID_LEN], infLen, fwtTemp);
}

/*
 ******************************************************************************
 * GLOBAL FUNCTIONS
 ******************************************************************************
 */


/*******************************************************************************/
void RfalNfcClass::rfalIsoDepInitialize(void)
{
  gIsoDep.state        = ISODEP_ST_IDLE;
  gIsoDep.role         = ISODEP_ROLE_PCD;
  gIsoDep.did          = RFAL_ISODEP_NO_DID;
  gIsoDep.nad          = RFAL_ISODEP_NO_NAD;
  gIsoDep.blockNumber  = 0;
  gIsoDep.isTxChaining = false;
  gIsoDep.isRxChaining = false;
  gIsoDep.lastDID00    = false;
  gIsoDep.lastPCB      = ISODEP_PCB_INVALID;
  gIsoDep.fsx          = (uint16_t)RFAL_ISODEP_FSX_16;
  gIsoDep.ourFsx       = (uint16_t)RFAL_ISODEP_FSX_16;
  gIsoDep.hdrLen       = RFAL_ISODEP_PCB_LEN;

  gIsoDep.rxLen        = NULL;
  gIsoDep.rxBuf        = NULL;

  gIsoDep.isTxPending  = false;
  gIsoDep.isWait4WTX   = false;

  gIsoDep.compMode       = RFAL_COMPLIANCE_MODE_NFC;
  gIsoDep.maxRetriesR    = RFAL_ISODEP_MAX_R_RETRYS;
  gIsoDep.maxRetriesS    = RFAL_ISODEP_MAX_S_RETRYS;
  gIsoDep.maxRetriesI    = RFAL_ISODEP_MAX_I_RETRYS;
  gIsoDep.maxRetriesRATS = RFAL_ISODEP_RATS_RETRIES;

  isoDepClearCounters();
}


/*******************************************************************************/
void RfalNfcClass::rfalIsoDepInitializeWithParams(rfalComplianceMode compMode, uint8_t maxRetriesR, uint8_t maxRetriesS, uint8_t maxRetriesI, uint8_t maxRetriesRATS)
{
  rfalIsoDepInitialize();

  gIsoDep.compMode       = compMode;
  gIsoDep.maxRetriesR    = maxRetriesR;
  gIsoDep.maxRetriesS    = maxRetriesS;
  gIsoDep.maxRetriesI    = maxRetriesI;
  gIsoDep.maxRetriesRATS = maxRetriesRATS;
}


/*******************************************************************************/
ReturnCode RfalNfcClass::isoDepDataExchangePCD(uint16_t *outActRxLen, bool *outIsChaining)
{
  ReturnCode ret;
  uint8_t    rxPCB;

  /* Check out parameters */
  if ((outActRxLen == NULL) || (outIsChaining == NULL)) {
    return ERR_PARAM;
  }

  *outIsChaining = false;

  /* Calculate header required and check if the buffers InfPositions are suitable */
  gIsoDep.hdrLen = RFAL_ISODEP_PCB_LEN;
  if (gIsoDep.did != RFAL_ISODEP_NO_DID)  {
    gIsoDep.hdrLen  += RFAL_ISODEP_DID_LEN;
  }
  if (gIsoDep.nad != RFAL_ISODEP_NO_NAD)  {
    gIsoDep.hdrLen  += RFAL_ISODEP_NAD_LEN;
  }

  /* check if there is enough space before the infPos to append ISO-DEP headers on rx and tx */
  if ((gIsoDep.rxBufInfPos < gIsoDep.hdrLen) || (gIsoDep.txBufInfPos < gIsoDep.hdrLen)) {
    return ERR_PARAM;
  }

  /*******************************************************************************/
  /* Wait until SFGT has been fulfilled (as a PCD) */
  if (gIsoDep.SFGTTimer != 0U) {
    if (!isoDepTimerisExpired(gIsoDep.SFGTTimer)) {
      return ERR_BUSY;
    }
  }
  /* Once done, clear SFGT timer */
  gIsoDep.SFGTTimer = 0;


  /*******************************************************************************/
  switch (gIsoDep.state) {
    /*******************************************************************************/
    case ISODEP_ST_IDLE:
      return ERR_NONE;

    /*******************************************************************************/
    case ISODEP_ST_PCD_TX:
      ret = isoDepTx(isoDep_PCBIBlock(gIsoDep.blockNumber), gIsoDep.txBuf, &gIsoDep.txBuf[gIsoDep.txBufInfPos], gIsoDep.txBufLen, (gIsoDep.fwt + gIsoDep.dFwt));
      switch (ret) {
        case ERR_NONE:
          gIsoDep.state = ISODEP_ST_PCD_RX;
          break;

        default:
          return ret;
      }
    /* fall through */

    /*******************************************************************************/
    case ISODEP_ST_PCD_WAIT_DSL:   /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */
    case ISODEP_ST_PCD_RX:

      ret = rfalRfDev->rfalGetTransceiveStatus();
      switch (ret) {
        /* Data rcvd with error or timeout -> Send R-NAK */
        case ERR_TIMEOUT:
        case ERR_CRC:
        case ERR_PAR:
        case ERR_FRAMING:          /* added to handle test cases scenario TC_POL_NFCB_T4AT_BI_82_x_y & TC_POL_NFCB_T4BT_BI_82_x_y */
        case ERR_INCOMPLETE_BYTE:  /* added to handle test cases scenario TC_POL_NFCB_T4AT_BI_82_x_y & TC_POL_NFCB_T4BT_BI_82_x_y  */

          if (gIsoDep.isRxChaining) {
            /* Rule 5 - In PICC chaining when a invalid/timeout occurs -> R-ACK */
            EXIT_ON_ERR(ret, isoDepHandleControlMsg(ISODEP_R_ACK, RFAL_ISODEP_NO_PARAM));
          } else if (gIsoDep.state == ISODEP_ST_PCD_WAIT_DSL) {
            /* Rule 8 - If s-Deselect response fails MAY retransmit */
            EXIT_ON_ERR(ret, isoDepHandleControlMsg(ISODEP_S_DSL, RFAL_ISODEP_NO_PARAM));
          } else {
            /* Rule 4 - When a invalid block or timeout occurs -> R-NACK */
            EXIT_ON_ERR(ret, isoDepHandleControlMsg(ISODEP_R_NAK, RFAL_ISODEP_NO_PARAM));
          }
          return ERR_BUSY;

        case ERR_NONE:
          break;

        case ERR_BUSY:
          return ERR_BUSY;  /* Debug purposes */

        default:
          return ret;
      }

      /*******************************************************************************/
      /* No error, process incoming msg                                              */
      /*******************************************************************************/

      (*outActRxLen) = rfalConvBitsToBytes(*outActRxLen);


      /* Check rcvd msg length, cannot be less then the expected header */
      if (((*outActRxLen) < gIsoDep.hdrLen) || ((*outActRxLen) >= gIsoDep.ourFsx)) {
        return ERR_PROTO;
      }

      /* Grab rcvd PCB */
      rxPCB = gIsoDep.rxBuf[ ISODEP_PCB_POS ];


      /* EMVCo doesn't allow usage of for CID or NAD   EMVCo 2.6 TAble 10.2 */
      if ((gIsoDep.compMode == RFAL_COMPLIANCE_MODE_EMV) && (isoDep_PCBhasDID(rxPCB) || isoDep_PCBhasNAD(rxPCB))) {
        return ERR_PROTO;
      }

      /* If we are expecting DID, check if PCB signals its presence and if device ID match*/
      if ((gIsoDep.did != RFAL_ISODEP_NO_DID) && (!isoDep_PCBhasDID(rxPCB) || (gIsoDep.did != gIsoDep.rxBuf[ ISODEP_DID_POS ]))) {
        return ERR_PROTO;
      }


      /*******************************************************************************/
      /* Process S-Block                                                             */
      /*******************************************************************************/
      if (isoDep_PCBisSBlock(rxPCB)) {
        /* Check if is a Wait Time eXtension */
        if (isoDep_PCBisSWTX(rxPCB)) {
          /* Rule 3 - respond to S-block: get 1st INF byte S(STW): Power + WTXM */
          EXIT_ON_ERR(ret, isoDepHandleControlMsg(ISODEP_S_WTX, isoDep_GetWTXM(gIsoDep.rxBuf[gIsoDep.hdrLen])));
          return ERR_BUSY;
        }

        /* Check if is a deselect response */
        if (isoDep_PCBisSDeselect(rxPCB)) {
          if (gIsoDep.state == ISODEP_ST_PCD_WAIT_DSL) {
            rfalIsoDepInitialize();         /* Session finished reInit vars */
            return ERR_NONE;
          }

          /* Deselect response not expected  */
          /* fall through to PROTO error */
        }
        /* Unexpected S-Block */
        return ERR_PROTO;
      }

      /*******************************************************************************/
      /* Process R-Block                                                             */
      /*******************************************************************************/
      else if (isoDep_PCBisRBlock(rxPCB)) {
        if (isoDep_PCBisRACK(rxPCB)) {                           /* Check if is a R-ACK */
          if (isoDep_GetBN(rxPCB) == gIsoDep.blockNumber) {    /* Expected block number  */
            /* Rule B - ACK with expected bn -> Increment block number */
            gIsoDep.blockNumber = isoDep_PCBNextBN(gIsoDep.blockNumber);

            /* R-ACK only allowed when PCD chaining */
            if (!gIsoDep.isTxChaining) {
              return ERR_PROTO;
            }

            /* Rule 7 - Chaining transaction done, continue chaining */
            isoDepClearCounters();
            return ERR_NONE;  /* This block has been transmitted */
          } else {
            /* Rule 6 - R-ACK with wrong block number retransmit */
            if (gIsoDep.cntIRetrys++ < gIsoDep.maxRetriesI) {
              gIsoDep.cntRRetrys = 0;            /* Clear R counter only */
              gIsoDep.state = ISODEP_ST_PCD_TX;
              return ERR_BUSY;
            }
            return ERR_PROTO;
          }
        } else { /* Unexcpected R-Block */
          return ERR_PROTO;
        }
      }

      /*******************************************************************************/
      /* Process I-Block                                                             */
      /*******************************************************************************/
      else if (isoDep_PCBisIBlock(rxPCB)) {
        /*******************************************************************************/
        /* is PICC performing chaining                                                 */
        if (isoDep_PCBisChaining(rxPCB)) {
          gIsoDep.isRxChaining = true;
          *outIsChaining       = true;

          if (isoDep_GetBN(rxPCB) == gIsoDep.blockNumber) {
            /* Rule B - ACK with correct block number -> Increase Block number */
            isoDep_ToggleBN(gIsoDep.blockNumber);

            isoDepClearCounters();  /* Clear counters in case R counter is already at max */

            /* Rule 2 - Send ACK */
            EXIT_ON_ERR(ret, isoDepHandleControlMsg(ISODEP_R_ACK, RFAL_ISODEP_NO_PARAM));

            /* Received I-Block with chaining, send current data to DH */

            /* remove ISO DEP header, check is necessary to move the INF data on the buffer */
            *outActRxLen -= gIsoDep.hdrLen;
            if ((gIsoDep.hdrLen != gIsoDep.rxBufInfPos) && (*outActRxLen > 0U)) {
              ST_MEMMOVE(&gIsoDep.rxBuf[gIsoDep.rxBufInfPos], &gIsoDep.rxBuf[gIsoDep.hdrLen], *outActRxLen);
            }

            isoDepClearCounters();
            return ERR_AGAIN;       /* Send Again signalling to run again, but some chaining data has arrived */
          } else {
            /* Rule 5 - PICC chaining invalid I-Block -> R-ACK */
            EXIT_ON_ERR(ret, isoDepHandleControlMsg(ISODEP_R_ACK, RFAL_ISODEP_NO_PARAM));
          }
          return ERR_BUSY;
        }

        gIsoDep.isRxChaining = false; /* clear PICC chaining flag */

        if (isoDep_GetBN(rxPCB) == gIsoDep.blockNumber) {
          /* Rule B - I-Block with correct block number -> Increase Block number */
          isoDep_ToggleBN(gIsoDep.blockNumber);

          /* I-Block transaction done successfully */

          /* remove ISO DEP header, check is necessary to move the INF data on the buffer */
          *outActRxLen -= gIsoDep.hdrLen;
          if ((gIsoDep.hdrLen != gIsoDep.rxBufInfPos) && (*outActRxLen > 0U)) {
            ST_MEMMOVE(&gIsoDep.rxBuf[gIsoDep.rxBufInfPos], &gIsoDep.rxBuf[gIsoDep.hdrLen], *outActRxLen);
          }

          gIsoDep.state = ISODEP_ST_IDLE;
          isoDepClearCounters();
          return ERR_NONE;
        } else {
          if ((gIsoDep.compMode != RFAL_COMPLIANCE_MODE_ISO)) {
            /* Invalid Block (not chaining) -> Raise error   Digital 1.1  15.2.6.4   EMVCo 2.6  10.3.5.4 */
            return ERR_PROTO;
          }

          /* Rule 4 - Invalid Block -> R-NAK */
          EXIT_ON_ERR(ret, isoDepHandleControlMsg(ISODEP_R_NAK, RFAL_ISODEP_NO_PARAM));
          return ERR_BUSY;
        }
      } else { /* not S/R/I - Block */
        return ERR_PROTO;
      }
    /* fall through */

    /*******************************************************************************/
    default:               /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */
      /* MISRA 16.4: no empty default (comment will suffice) */
      break;
  }

  return ERR_INTERNAL;
}

/*******************************************************************************/
ReturnCode RfalNfcClass::rfalIsoDepDeselect(void)
{
  ReturnCode ret;
  uint32_t   cntRerun;
  bool       dummyB;

  /*******************************************************************************/
  /* Check if  rx parameters have been set before, otherwise use global variable *
   * To cope with a Deselect after RATS\ATTRIB without any I-Block exchanged     */
  if ((gIsoDep.rxLen == NULL) || (gIsoDep.rxBuf == NULL)) {
    /* Using local vars would be safe as rfalIsoDepInitialize will clear the   *
     * reference to local vars before exiting (no EXIT_ON_ERR),                *
     * but MISRA 18.6 3217 would be still be flagged. Using static variables   */
    gIsoDep.rxLen       = &gIsoDep.ctrlRxLen;
    gIsoDep.rxBuf       = gIsoDep.ctrlRxBuf;

    gIsoDep.rxBufLen    = ISODEP_CONTROLMSG_BUF_LEN;
    gIsoDep.rxBufInfPos = (RFAL_ISODEP_PCB_LEN + RFAL_ISODEP_DID_LEN);
    gIsoDep.txBufInfPos = (RFAL_ISODEP_PCB_LEN + RFAL_ISODEP_DID_LEN);
  }


  /*******************************************************************************/
  /* The Deselect process is being done blocking, Digital 1.0 - 13.2.7.1 MUST wait response and retry*/
  /* Set the maximum reruns while we will wait for a response */
  cntRerun = ISODEP_MAX_RERUNS;

  /* Send DSL request and run protocol until get a response, error or "timeout" */
  EXIT_ON_ERR(ret, isoDepHandleControlMsg(ISODEP_S_DSL, RFAL_ISODEP_NO_PARAM));
  do {
    ret = isoDepDataExchangePCD(gIsoDep.rxLen, &dummyB);
    rfalRfDev->rfalWorker();
  } while (((cntRerun--) != 0U) && (ret == ERR_BUSY));

  rfalIsoDepInitialize();
  return ((cntRerun == 0U) ? ERR_TIMEOUT : ret);
}


/*******************************************************************************/
uint32_t RfalNfcClass::rfalIsoDepFWI2FWT(uint8_t fwi)
{
  uint32_t result;
  uint8_t  tmpFWI;

  tmpFWI = fwi;

  /* RFU values -> take the default value
   * Digital 1.0  11.6.2.17  FWI[1,14]
   * Digital 1.1  7.6.2.22   FWI[0,14]
   * EMVCo 2.6    Table A.5  FWI[0,14] */
  if (tmpFWI > ISODEP_FWI_MAX) {
    tmpFWI = RFAL_ISODEP_FWI_DEFAULT;
  }

  /* FWT = (256 x 16/fC) x 2^FWI => 2^(FWI+12)  Digital 1.1  13.8.1 & 7.9.1 */

  result = ((uint32_t)1U << (tmpFWI + 12U));
  result = MIN(RFAL_ISODEP_MAX_FWT, result);   /* Maximum Frame Waiting Time must be fulfilled */

  return result;
}


/*******************************************************************************/
uint16_t RfalNfcClass::rfalIsoDepFSxI2FSx(uint8_t FSxI)
{
  uint16_t fsx;
  uint8_t  fsi;

  /* Enforce maximum FSxI/FSx allowed - NFC Forum and EMVCo differ */
  fsi = ((gIsoDep.compMode == RFAL_COMPLIANCE_MODE_EMV) ? MIN(FSxI, RFAL_ISODEP_FSDI_MAX_EMV) : MIN(FSxI, RFAL_ISODEP_FSDI_MAX_NFC));

  switch (fsi) {
    case (uint8_t)RFAL_ISODEP_FSXI_16:           fsx = (uint16_t)RFAL_ISODEP_FSX_16;   break;
    case (uint8_t)RFAL_ISODEP_FSXI_24:           fsx = (uint16_t)RFAL_ISODEP_FSX_24;   break;
    case (uint8_t)RFAL_ISODEP_FSXI_32:           fsx = (uint16_t)RFAL_ISODEP_FSX_32;   break;
    case (uint8_t)RFAL_ISODEP_FSXI_40:           fsx = (uint16_t)RFAL_ISODEP_FSX_40;   break;
    case (uint8_t)RFAL_ISODEP_FSXI_48:           fsx = (uint16_t)RFAL_ISODEP_FSX_48;   break;
    case (uint8_t)RFAL_ISODEP_FSXI_64:           fsx = (uint16_t)RFAL_ISODEP_FSX_64;   break;
    case (uint8_t)RFAL_ISODEP_FSXI_96:           fsx = (uint16_t)RFAL_ISODEP_FSX_96;   break;
    case (uint8_t)RFAL_ISODEP_FSXI_128:          fsx = (uint16_t)RFAL_ISODEP_FSX_128;  break;
    case (uint8_t)RFAL_ISODEP_FSXI_256:          fsx = (uint16_t)RFAL_ISODEP_FSX_256;  break;
    case (uint8_t)RFAL_ISODEP_FSXI_512:          fsx = (uint16_t)RFAL_ISODEP_FSX_512;  break;
    case (uint8_t)RFAL_ISODEP_FSXI_1024:         fsx = (uint16_t)RFAL_ISODEP_FSX_1024; break;
    case (uint8_t)RFAL_ISODEP_FSXI_2048:         fsx = (uint16_t)RFAL_ISODEP_FSX_2048; break;
    case (uint8_t)RFAL_ISODEP_FSXI_4096:         fsx = (uint16_t)RFAL_ISODEP_FSX_4096; break;
    default:                                     fsx = (uint16_t)RFAL_ISODEP_FSX_256;  break;
  }
  return fsx;
}


/*******************************************************************************/
uint16_t RfalNfcClass::rfalIsoDepGetMaxInfLen(void)
{
  /* Check whether all parameters are valid, otherwise return minimum default value */
  if ((gIsoDep.fsx < (uint16_t)RFAL_ISODEP_FSX_16) || (gIsoDep.fsx > (uint16_t)RFAL_ISODEP_FSX_4096) || (gIsoDep.hdrLen > ISODEP_HDR_MAX_LEN)) {
    uint16_t isodepFsx16 = (uint16_t)RFAL_ISODEP_FSX_16;  /* MISRA 10.1 */
    return (isodepFsx16 - RFAL_ISODEP_PCB_LEN - ISODEP_CRC_LEN);
  }

  return (gIsoDep.fsx - gIsoDep.hdrLen - ISODEP_CRC_LEN);
}


/*******************************************************************************/
ReturnCode RfalNfcClass::rfalIsoDepStartTransceive(rfalIsoDepTxRxParam param)
{
  gIsoDep.txBuf        = param.txBuf->prologue;
  gIsoDep.txBufInfPos  = (uint8_t)((uint32_t)param.txBuf->inf - (uint32_t)param.txBuf->prologue);
  gIsoDep.txBufLen     = param.txBufLen;
  gIsoDep.isTxChaining = param.isTxChaining;

  gIsoDep.rxBuf        = param.rxBuf->prologue;
  gIsoDep.rxBufInfPos  = (uint8_t)((uint32_t)param.rxBuf->inf - (uint32_t)param.rxBuf->prologue);
  gIsoDep.rxBufLen     = sizeof(rfalIsoDepBufFormat);

  gIsoDep.rxLen        = param.rxLen;
  gIsoDep.rxChaining   = param.isRxChaining;


  gIsoDep.fwt          = param.FWT;
  gIsoDep.dFwt         = param.dFWT;
  gIsoDep.fsx          = param.FSx;
  gIsoDep.did          = param.DID;

  /* Only change the FSx from activation if no to Keep */
  gIsoDep.ourFsx = ((param.ourFSx != RFAL_ISODEP_FSX_KEEP) ? param.ourFSx : gIsoDep.ourFsx);

  /* Clear inner control params for next dataExchange */
  gIsoDep.isRxChaining  = false;
  isoDepClearCounters();

  if (gIsoDep.role == ISODEP_ROLE_PICC) {
    if (gIsoDep.txBufLen > 0U) {
      /* Ensure that an RTOX Ack is not being expected at moment */
      if (!gIsoDep.isWait4WTX) {
        gIsoDep.state = ISODEP_ST_PICC_TX;
        return ERR_NONE;
      } else {
        /* If RTOX Ack is expected, signal a pending Tx to be transmitted right after */
        gIsoDep.isTxPending = true;
      }
    }

    /* Digital 1.1  15.2.5.1 The first block SHALL be sent by the Reader/Writer */
    gIsoDep.state = ISODEP_ST_PICC_RX;
    return ERR_NONE;
  }

  gIsoDep.state = ISODEP_ST_PCD_TX;
  return ERR_NONE;
}


/*******************************************************************************/
ReturnCode RfalNfcClass::rfalIsoDepGetTransceiveStatus(void)
{
  if (gIsoDep.role == ISODEP_ROLE_PICC) {
    return ERR_NOTSUPP;
  } else {
    return isoDepDataExchangePCD(gIsoDep.rxLen, gIsoDep.rxChaining);
  }
}


/*******************************************************************************/
ReturnCode RfalNfcClass::rfalIsoDepRATS(rfalIsoDepFSxI FSDI, uint8_t DID, rfalIsoDepAts *ats, uint8_t *atsLen)
{
  uint16_t       rcvLen;
  ReturnCode     ret;
  rfalIsoDepRats ratsReq;

  if (ats == NULL) {
    return ERR_PARAM;
  }

  /*******************************************************************************/
  /* Compose RATS */
  ratsReq.CMD   = RFAL_ISODEP_CMD_RATS;
  ratsReq.PARAM = (((uint8_t)FSDI << RFAL_ISODEP_RATS_PARAM_FSDI_SHIFT) & RFAL_ISODEP_RATS_PARAM_FSDI_MASK) | (DID & RFAL_ISODEP_RATS_PARAM_DID_MASK);

  ret = rfalRfDev->rfalTransceiveBlockingTxRx((uint8_t *)&ratsReq, sizeof(rfalIsoDepRats), (uint8_t *)ats, sizeof(rfalIsoDepAts), &rcvLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_ISODEP_T4T_FWT_ACTIVATION);

  if (ret == ERR_NONE) {
    /* Check for valid ATS length  Digital 1.1  13.6.2.1 & 13.6.2.3 */
    if ((rcvLen < RFAL_ISODEP_ATS_MIN_LEN) || (rcvLen > RFAL_ISODEP_ATS_MAX_LEN) || (ats->TL != rcvLen)) {
      return ERR_PROTO;
    }

    /* Assign our FSx, in case the a Deselect is send without Transceive */
    gIsoDep.ourFsx = rfalIsoDepFSxI2FSx((uint8_t)FSDI);
  }

  /* Check and assign if ATS length was requested (length also available on TL) */
  if (atsLen != NULL) {
    *atsLen = (uint8_t)rcvLen;
  }

  return ret;
}


/*******************************************************************************/
ReturnCode RfalNfcClass::rfalIsoDepPPS(uint8_t DID, rfalBitRate DSI, rfalBitRate DRI, rfalIsoDepPpsRes *ppsRes)
{
  uint16_t         rcvLen;
  ReturnCode       ret;
  rfalIsoDepPpsReq ppsReq;

  if ((ppsRes == NULL) || (DSI > RFAL_BR_848) || (DRI > RFAL_BR_848) || (DID > RFAL_ISODEP_DID_MAX)) {
    return ERR_PARAM;
  }

  /*******************************************************************************/
  /* Compose PPS Request */
  ppsReq.PPSS = (RFAL_ISODEP_PPS_SB | (DID & RFAL_ISODEP_PPS_SB_DID_MASK));
  ppsReq.PPS0 = RFAL_ISODEP_PPS_PPS0_PPS1_PRESENT;
  ppsReq.PPS1 = (RFAL_ISODEP_PPS_PPS1 | ((((uint8_t)DSI << RFAL_ISODEP_PPS_PPS1_DSI_SHIFT) | (uint8_t)DRI) & RFAL_ISODEP_PPS_PPS1_DXI_MASK));

  ret = rfalRfDev->rfalTransceiveBlockingTxRx((uint8_t *)&ppsReq, sizeof(rfalIsoDepPpsReq), (uint8_t *)ppsRes, sizeof(rfalIsoDepPpsRes), &rcvLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_ISODEP_T4T_FWT_ACTIVATION);

  if (ret == ERR_NONE) {
    /* Check for valid PPS Response   */
    if ((rcvLen != RFAL_ISODEP_PPS_RES_LEN) || (ppsRes->PPSS != ppsReq.PPSS)) {
      return ERR_PROTO;
    }
  }
  return ret;
}


/*******************************************************************************/
ReturnCode RfalNfcClass::rfalIsoDepATTRIB(const uint8_t *nfcid0, uint8_t PARAM1, rfalBitRate DSI, rfalBitRate DRI, rfalIsoDepFSxI FSDI, uint8_t PARAM3, uint8_t DID, const uint8_t *HLInfo, uint8_t HLInfoLen, uint32_t fwt, rfalIsoDepAttribRes *attribRes, uint8_t *attribResLen)
{
  uint16_t            rcvLen;
  ReturnCode          ret;
  rfalIsoDepAttribCmd attribCmd;


  if ((attribRes == NULL) || (attribResLen == NULL) || (DSI > RFAL_BR_848) || (DRI > RFAL_BR_848) || (DID > RFAL_ISODEP_DID_MAX)) {
    return ERR_NONE;
  }

  /*******************************************************************************/
  /* Compose ATTRIB command */
  attribCmd.cmd          = RFAL_ISODEP_CMD_ATTRIB;
  attribCmd.Param.PARAM1 = PARAM1;
  attribCmd.Param.PARAM2 = (((((uint8_t)DSI << RFAL_ISODEP_ATTRIB_PARAM2_DSI_SHIFT) | ((uint8_t)DRI << RFAL_ISODEP_ATTRIB_PARAM2_DRI_SHIFT)) & RFAL_ISODEP_ATTRIB_PARAM2_DXI_MASK) | ((uint8_t)FSDI & RFAL_ISODEP_ATTRIB_PARAM2_FSDI_MASK));
  attribCmd.Param.PARAM3 = PARAM3;
  attribCmd.Param.PARAM4 = (DID & RFAL_ISODEP_ATTRIB_PARAM4_DID_MASK);
  ST_MEMCPY(attribCmd.nfcid0, nfcid0, RFAL_NFCB_NFCID0_LEN);

  /* Append the Higher layer Info if provided */
  if ((HLInfo != NULL) && (HLInfoLen > 0U)) {
    ST_MEMCPY(attribCmd.HLInfo, HLInfo, MIN(HLInfoLen, RFAL_ISODEP_ATTRIB_HLINFO_LEN));
  }

  ret = rfalRfDev->rfalTransceiveBlockingTxRx((uint8_t *)&attribCmd, (RFAL_ISODEP_ATTRIB_HDR_LEN + MIN((uint16_t)HLInfoLen, RFAL_ISODEP_ATTRIB_HLINFO_LEN)), (uint8_t *)attribRes, sizeof(rfalIsoDepAttribRes), &rcvLen, RFAL_TXRX_FLAGS_DEFAULT, fwt);

  *attribResLen = (uint8_t)rcvLen;

  if (ret == ERR_NONE) {
    /* Check a for valid ATTRIB Response   Digital 1.1  15.6.2.1 */
    if ((rcvLen < RFAL_ISODEP_ATTRIB_RES_HDR_LEN) || ((attribRes->mbliDid & RFAL_ISODEP_ATTRIB_RES_DID_MASK) != DID)) {
      return ERR_PROTO;
    }
  }

  return ret;
}


/*******************************************************************************/
ReturnCode RfalNfcClass::rfalIsoDepPollAHandleActivation(rfalIsoDepFSxI FSDI, uint8_t DID, rfalBitRate maxBR, rfalIsoDepDevice *isoDepDev)
{
  uint8_t          RATSretries;
  uint8_t          msgIt;
  ReturnCode       ret;
  rfalIsoDepPpsRes ppsRes;

  if (isoDepDev == NULL) {
    return ERR_PARAM;
  }

  /* Enable EMD handling according   Digital 1.1  4.1.1.1 ; EMVCo 2.6  4.9.2 */
  rfalRfDev->rfalSetErrorHandling(RFAL_ERRORHANDLING_EMVCO);

  RATSretries = gIsoDep.maxRetriesRATS;

  /***************************************************************************/
  /* Send RATS                                                               */
  do {
    /* Digital 1.1 13.7.1.1 and ISO 14443-4 5.6.1.1 - Upon a failed RATS it may be retransmited [0,1] */
    ret = rfalIsoDepRATS(FSDI, DID, &isoDepDev->activation.A.Listener.ATS, &isoDepDev->activation.A.Listener.ATSLen);

    /* EMVCo 2.6  9.6.1.1 & 9.6.1.2  If a timeout error is detected retransmit, on transmission error abort */
    if ((gIsoDep.compMode == RFAL_COMPLIANCE_MODE_EMV) && (ret != ERR_NONE) && (ret != ERR_TIMEOUT)) {
      break;
    }

    delay(1);
  } while (((RATSretries--) != 0U) && (ret != ERR_NONE));



  /* Switch between NFC Forum and ISO14443-4 behaviour #595
   *   ISO14443-4  5.6.1  If RATS fails, a Deactivation sequence should be performed as defined on clause 8
   *   Activity 1.1  9.6  Device Deactivation Activity is to be only performed when there's an active device */
  if (ret != ERR_NONE) {
    if (gIsoDep.compMode == RFAL_COMPLIANCE_MODE_ISO) {
      rfalIsoDepDeselect();
    }
    return ret;
  }

  /*******************************************************************************/
  /* Process ATS Response                                                        */
  isoDepDev->info.FWI  = RFAL_ISODEP_FWI_DEFAULT; /* Default value   EMVCo 2.6  5.7.2.6  */
  isoDepDev->info.SFGI = 0;
  isoDepDev->info.MBL  = 0;
  isoDepDev->info.DSI  = RFAL_BR_106;
  isoDepDev->info.DRI  = RFAL_BR_106;
  isoDepDev->info.FSxI = (uint8_t)RFAL_ISODEP_FSXI_32;     /* FSC default value is 32 bytes  ISO14443-A  5.2.3 */


  /*******************************************************************************/
  /* Check for ATS optional fields                                               */
  if (isoDepDev->activation.A.Listener.ATS.TL > RFAL_ISODEP_ATS_MIN_LEN) {
    msgIt = RFAL_ISODEP_ATS_MIN_LEN;

    /* Format byte T0 is optional, if present assign FSDI */
    isoDepDev->info.FSxI = (isoDepDev->activation.A.Listener.ATS.T0 & RFAL_ISODEP_ATS_T0_FSCI_MASK);

    /* T0 has already been processed, always the same position */
    msgIt++;

    /* Check if TA is present */
    if ((isoDepDev->activation.A.Listener.ATS.T0 & RFAL_ISODEP_ATS_T0_TA_PRESENCE_MASK) != 0U) {
      rfalIsoDepCalcBitRate(maxBR, ((uint8_t *)&isoDepDev->activation.A.Listener.ATS)[msgIt++], &isoDepDev->info.DSI, &isoDepDev->info.DRI);
    }

    /* Check if TB is present */
    if ((isoDepDev->activation.A.Listener.ATS.T0 & RFAL_ISODEP_ATS_T0_TB_PRESENCE_MASK) != 0U) {
      isoDepDev->info.SFGI  = ((uint8_t *)&isoDepDev->activation.A.Listener.ATS)[msgIt++];
      isoDepDev->info.FWI   = (uint8_t)((isoDepDev->info.SFGI >> RFAL_ISODEP_ATS_TB_FWI_SHIFT) & RFAL_ISODEP_ATS_FWI_MASK);
      isoDepDev->info.SFGI &= RFAL_ISODEP_ATS_TB_SFGI_MASK;
    }

    /* Check if TC is present */
    if ((isoDepDev->activation.A.Listener.ATS.T0 & RFAL_ISODEP_ATS_T0_TC_PRESENCE_MASK) != 0U) {
      /* Check for Protocol features support */
      /* Advanced protocol features defined on Digital 1.0 Table 69, removed after */
      isoDepDev->info.supAdFt = (((((uint8_t *)&isoDepDev->activation.A.Listener.ATS)[msgIt]   & RFAL_ISODEP_ATS_TC_ADV_FEAT) != 0U)  ? true : false);
      isoDepDev->info.supDID  = (((((uint8_t *)&isoDepDev->activation.A.Listener.ATS)[msgIt]   & RFAL_ISODEP_ATS_TC_DID)      != 0U)  ? true : false);
      isoDepDev->info.supNAD  = (((((uint8_t *)&isoDepDev->activation.A.Listener.ATS)[msgIt++] & RFAL_ISODEP_ATS_TC_NAD)      != 0U)  ? true : false);
    }
  }

  isoDepDev->info.FSx  = rfalIsoDepFSxI2FSx(isoDepDev->info.FSxI);

  isoDepDev->info.SFGT = rfalIsoDepSFGI2SFGT((uint8_t)isoDepDev->info.SFGI);
  isoDepTimerStart(gIsoDep.SFGTTimer, isoDepDev->info.SFGT);

  isoDepDev->info.FWT  = rfalIsoDepFWI2FWT(isoDepDev->info.FWI);
  isoDepDev->info.dFWT = RFAL_ISODEP_DFWT_20;

  isoDepDev->info.DID = ((isoDepDev->info.supDID) ? DID : RFAL_ISODEP_NO_DID);
  isoDepDev->info.NAD = RFAL_ISODEP_NO_NAD;


  /*******************************************************************************/
  /* If higher bit rates are supported by both devices, send PPS                 */
  if ((isoDepDev->info.DSI != RFAL_BR_106) || (isoDepDev->info.DRI != RFAL_BR_106)) {
    /* Wait until SFGT has been fulfilled */
    while (!isoDepTimerisExpired(gIsoDep.SFGTTimer)) { /* MISRA 15.6: mandatory brackets */ };

    ret = rfalIsoDepPPS(isoDepDev->info.DID, isoDepDev->info.DSI, isoDepDev->info.DRI, &ppsRes);

    if (ret == ERR_NONE) {
      /* DSI code the divisor from PICC to PCD */
      /* DRI code the divisor from PCD to PICC */
      rfalRfDev->rfalSetBitRate(isoDepDev->info.DRI, isoDepDev->info.DSI);
    } else {
      isoDepDev->info.DSI = RFAL_BR_106;
      isoDepDev->info.DRI = RFAL_BR_106;
    }
  }

  /*******************************************************************************/
  /* Store already FS info,  rfalIsoDepGetMaxInfLen() may be called before setting TxRx params */
  gIsoDep.fsx    = isoDepDev->info.FSx;
  gIsoDep.ourFsx = rfalIsoDepFSxI2FSx((uint8_t)FSDI);

  return ERR_NONE;
}


/*******************************************************************************/
ReturnCode RfalNfcClass::rfalIsoDepPollBHandleActivation(rfalIsoDepFSxI FSDI, uint8_t DID, rfalBitRate maxBR, uint8_t PARAM1, const rfalNfcbListenDevice *nfcbDev, const uint8_t *HLInfo, uint8_t HLInfoLen, rfalIsoDepDevice *isoDepDev)
{
  ReturnCode ret;
  uint8_t    mbli;

  /***************************************************************************/
  /* Initialize ISO-DEP Device with info from SENSB_RES                      */
  isoDepDev->info.FWI     = ((nfcbDev->sensbRes.protInfo.FwiAdcFo >> RFAL_NFCB_SENSB_RES_FWI_SHIFT) & RFAL_NFCB_SENSB_RES_FWI_MASK);
  isoDepDev->info.FWT     = rfalIsoDepFWI2FWT(isoDepDev->info.FWI);
  isoDepDev->info.dFWT    = RFAL_NFCB_DFWT;
  isoDepDev->info.SFGI    = (((uint32_t)nfcbDev->sensbRes.protInfo.SFGI >> RFAL_NFCB_SENSB_RES_SFGI_SHIFT) & RFAL_NFCB_SENSB_RES_SFGI_MASK);
  isoDepDev->info.SFGT    = rfalIsoDepSFGI2SFGT((uint8_t)isoDepDev->info.SFGI);
  isoDepDev->info.FSxI    = ((nfcbDev->sensbRes.protInfo.FsciProType >> RFAL_NFCB_SENSB_RES_FSCI_SHIFT) & RFAL_NFCB_SENSB_RES_FSCI_MASK);
  isoDepDev->info.FSx     = rfalIsoDepFSxI2FSx(isoDepDev->info.FSxI);
  isoDepDev->info.DID     = DID;
  isoDepDev->info.supDID  = (((nfcbDev->sensbRes.protInfo.FwiAdcFo & RFAL_NFCB_SENSB_RES_FO_DID_MASK) != 0U) ? true : false);
  isoDepDev->info.supNAD  = (((nfcbDev->sensbRes.protInfo.FwiAdcFo & RFAL_NFCB_SENSB_RES_FO_NAD_MASK) != 0U) ? true : false);


  /* Check if DID requested is supported by PICC */
  if ((DID != RFAL_ISODEP_NO_DID) && (!isoDepDev->info.supDID)) {
    return ERR_PARAM;
  }

  /* Enable EMD handling according   Digital 2.1  4.1.1.1 ; EMVCo 3.0  4.9.2 */
  rfalRfDev->rfalSetErrorHandling(RFAL_ERRORHANDLING_EMVCO);

  /***************************************************************************/
  /* Set FDT Poll to be used on upcoming communications                      */
  if (gIsoDep.compMode == RFAL_COMPLIANCE_MODE_EMV) {
    /* Disregard Minimum TR2 returned by PICC, always use FDTb MIN   EMVCo 3.0  6.3.2.10  */
    rfalRfDev->rfalSetFDTPoll(RFAL_FDT_POLL_NFCB_POLLER);
  } else {
    /* Apply minimum TR2 from SENSB_RES   Digital 2.1  7.6.2.23 */
    rfalRfDev->rfalSetFDTPoll(rfalNfcbTR2ToFDT(((nfcbDev->sensbRes.protInfo.FsciProType >> RFAL_NFCB_SENSB_RES_PROTO_TR2_SHIFT) & RFAL_NFCB_SENSB_RES_PROTO_TR2_MASK)));
  }


  /* Calculate max Bit Rate */
  rfalIsoDepCalcBitRate(maxBR, nfcbDev->sensbRes.protInfo.BRC, &isoDepDev->info.DSI, &isoDepDev->info.DRI);

  /***************************************************************************/
  /* Send ATTRIB Command                                                     */
  ret = rfalIsoDepATTRIB((const uint8_t *)&nfcbDev->sensbRes.nfcid0,
                         (((nfcbDev->sensbRes.protInfo.FwiAdcFo & RFAL_NFCB_SENSB_RES_ADC_ADV_FEATURE_MASK) != 0U) ? PARAM1 : RFAL_ISODEP_ATTRIB_REQ_PARAM1_DEFAULT),
                         isoDepDev->info.DSI,
                         isoDepDev->info.DRI,
                         FSDI,
                         (gIsoDep.compMode == RFAL_COMPLIANCE_MODE_EMV) ? RFAL_NFCB_SENSB_RES_PROTO_ISO_MASK : (nfcbDev->sensbRes.protInfo.FsciProType & ((RFAL_NFCB_SENSB_RES_PROTO_TR2_MASK << RFAL_NFCB_SENSB_RES_PROTO_TR2_SHIFT) | RFAL_NFCB_SENSB_RES_PROTO_ISO_MASK)), /* EMVCo 2.6 6.4.1.9 */
                         DID,
                         HLInfo,
                         HLInfoLen,
                         (isoDepDev->info.FWT + isoDepDev->info.dFWT),
                         &isoDepDev->activation.B.Listener.ATTRIB_RES,
                         &isoDepDev->activation.B.Listener.ATTRIB_RESLen
                        );

  /***************************************************************************/
  /* Process ATTRIB Response                                                 */
  if (ret == ERR_NONE) {
    /* Digital 1.1 14.6.2.3 - Check if received DID match */
    if ((isoDepDev->activation.B.Listener.ATTRIB_RES.mbliDid & RFAL_ISODEP_ATTRIB_RES_DID_MASK) != DID) {
      return ERR_PROTO;
    }

    /* Retrieve MBLI and calculate new FDS/MBL (Maximum Buffer Length) */
    mbli = ((isoDepDev->activation.B.Listener.ATTRIB_RES.mbliDid >> RFAL_ISODEP_ATTRIB_RES_MBLI_SHIFT) & RFAL_ISODEP_ATTRIB_RES_MBLI_MASK);
    if (mbli > 0U) {
      /* Digital 1.1  14.6.2  Calculate Maximum Buffer Length MBL = FSC x 2^(MBLI-1) */
      isoDepDev->info.MBL = (isoDepDev->info.FSx * ((uint32_t)1U << (mbli - 1U)));
    }

    /* DSI code the divisor from PICC to PCD */
    /* DRI code the divisor from PCD to PICC */
    rfalRfDev->rfalSetBitRate(isoDepDev->info.DRI, isoDepDev->info.DSI);


    if ((nfcbDev->sensbRes.protInfo.FwiAdcFo & RFAL_NFCB_SENSB_RES_ADC_ADV_FEATURE_MASK) != 0U) {
      /* REMARK: SoF EoF TR0 and TR1 are not passed on to RF layer */
    }

    /* Start the SFGT timer */
    isoDepTimerStart(gIsoDep.SFGTTimer, isoDepDev->info.SFGT);
  } else {
    isoDepDev->info.DSI = RFAL_BR_106;
    isoDepDev->info.DRI = RFAL_BR_106;
  }

  /*******************************************************************************/
  /* Store already FS info,  rfalIsoDepGetMaxInfLen() may be called before setting TxRx params */
  gIsoDep.fsx    = isoDepDev->info.FSx;
  gIsoDep.ourFsx = rfalIsoDepFSxI2FSx((uint8_t)FSDI);

  return ret;
}


/*******************************************************************************/
ReturnCode RfalNfcClass::rfalIsoDepPollHandleSParameters(rfalIsoDepDevice *isoDepDev, rfalBitRate maxTxBR, rfalBitRate maxRxBR)
{
  uint8_t                    it;
  uint8_t                    supPCD2PICC;
  uint8_t                    supPICC2PCD;
  uint8_t                    currenttxBR;
  uint8_t                    currentrxBR;
  rfalBitRate                txBR;
  rfalBitRate                rxBR;
  uint16_t                   rcvLen;
  ReturnCode                 ret;
  rfalIsoDepControlMsgSParam sParam;


  if ((isoDepDev == NULL) || (maxTxBR > RFAL_BR_13560) || (maxRxBR > RFAL_BR_13560)) {
    return ERR_PARAM;
  }

  it          = 0;
  supPICC2PCD = 0x00;
  supPCD2PICC = 0x00;
  txBR        = RFAL_BR_106;
  rxBR        = RFAL_BR_106;
  sParam.pcb  = ISODEP_PCB_SPARAMETERS;

  /*******************************************************************************/
  /* Send S(PARAMETERS) - Block Info */
  sParam.sParam.tag         = RFAL_ISODEP_SPARAM_TAG_BLOCKINFO;
  sParam.sParam.value[it++] = RFAL_ISODEP_SPARAM_TAG_BRREQ;
  sParam.sParam.value[it++] = RFAL_ISODEP_SPARAM_TAG_BRREQ_LEN;
  sParam.sParam.length      = it;

  EXIT_ON_ERR(ret, rfalRfDev->rfalTransceiveBlockingTxRx((uint8_t *)&sParam, (RFAL_ISODEP_SPARAM_HDR_LEN + (uint16_t)it), (uint8_t *)&sParam, sizeof(rfalIsoDepControlMsgSParam), &rcvLen, RFAL_TXRX_FLAGS_DEFAULT, (isoDepDev->info.FWT + isoDepDev->info.dFWT)));

  it = 0;

  /*******************************************************************************/
  /* Check S(PARAMETERS) response */
  if ((sParam.pcb != ISODEP_PCB_SPARAMETERS) || (sParam.sParam.tag != RFAL_ISODEP_SPARAM_TAG_BLOCKINFO)  ||
      (sParam.sParam.value[it] != RFAL_ISODEP_SPARAM_TAG_BRIND) || (rcvLen < RFAL_ISODEP_SPARAM_HDR_LEN) ||
      (rcvLen != ((uint16_t)sParam.sParam.length + RFAL_ISODEP_SPARAM_HDR_LEN))) {
    return ERR_PROTO;
  }

  /* Retrieve PICC's bit rate PICC capabilities */
  for (it = 0; it < (rcvLen - (uint16_t)RFAL_ISODEP_SPARAM_TAG_LEN); it++) {
    if ((sParam.sParam.value[it] == RFAL_ISODEP_SPARAM_TAG_SUP_PCD2PICC) && (sParam.sParam.value[it + (uint16_t)RFAL_ISODEP_SPARAM_TAG_LEN] == RFAL_ISODEP_SPARAM_TAG_PCD2PICC_LEN)) {
      supPCD2PICC = sParam.sParam.value[it + RFAL_ISODEP_SPARAM_TAG_PCD2PICC_LEN];
    }

    if ((sParam.sParam.value[it] == RFAL_ISODEP_SPARAM_TAG_SUP_PICC2PCD) && (sParam.sParam.value[it + (uint16_t)RFAL_ISODEP_SPARAM_TAG_LEN] == RFAL_ISODEP_SPARAM_TAG_PICC2PCD_LEN)) {
      supPICC2PCD = sParam.sParam.value[it + RFAL_ISODEP_SPARAM_TAG_PICC2PCD_LEN];
    }
  }

  /*******************************************************************************/
  /* Check if requested bit rates are supported by PICC */
  if ((supPICC2PCD == 0x00U) || (supPCD2PICC == 0x00U)) {
    return ERR_PROTO;
  }

  for (it = 0; it <= (uint8_t)maxTxBR; it++) {
    if ((supPCD2PICC & (0x01U << it)) != 0U) {
      txBR = (rfalBitRate)it; /* PRQA S 4342 # MISRA 10.5 - Layout of enum rfalBitRate and above clamping of maxTxBR guarantee no invalid enum values to be created */
    }
  }
  for (it = 0; it <= (uint8_t)maxRxBR; it++) {
    if ((supPICC2PCD & (0x01U << it)) != 0U) {
      rxBR = (rfalBitRate)it; /* PRQA S 4342 # MISRA 10.5 - Layout of enum rfalBitRate and above clamping of maxTxBR guarantee no invalid enum values to be created */
    }
  }

  it = 0;
  currenttxBR = (uint8_t)txBR;
  currentrxBR = (uint8_t)rxBR;

  /*******************************************************************************/
  /* Send S(PARAMETERS) - Bit rates Activation */
  sParam.sParam.tag         = RFAL_ISODEP_SPARAM_TAG_BLOCKINFO;
  sParam.sParam.value[it++] = RFAL_ISODEP_SPARAM_TAG_BRACT;
  sParam.sParam.value[it++] = (RFAL_ISODEP_SPARAM_TVL_HDR_LEN + RFAL_ISODEP_SPARAM_TAG_PCD2PICC_LEN + RFAL_ISODEP_SPARAM_TVL_HDR_LEN + RFAL_ISODEP_SPARAM_TAG_PICC2PCD_LEN);
  sParam.sParam.value[it++] = RFAL_ISODEP_SPARAM_TAG_SEL_PCD2PICC;
  sParam.sParam.value[it++] = RFAL_ISODEP_SPARAM_TAG_PCD2PICC_LEN;
  sParam.sParam.value[it++] = ((uint8_t)0x01U << currenttxBR);
  sParam.sParam.value[it++] = 0x00U;
  sParam.sParam.value[it++] = RFAL_ISODEP_SPARAM_TAG_SEL_PICC2PCD;
  sParam.sParam.value[it++] = RFAL_ISODEP_SPARAM_TAG_PICC2PCD_LEN;
  sParam.sParam.value[it++] = ((uint8_t)0x01U << currentrxBR);
  sParam.sParam.value[it++] = 0x00U;
  sParam.sParam.length      = it;

  EXIT_ON_ERR(ret, rfalRfDev->rfalTransceiveBlockingTxRx((uint8_t *)&sParam, (RFAL_ISODEP_SPARAM_HDR_LEN + (uint16_t)it), (uint8_t *)&sParam, sizeof(rfalIsoDepControlMsgSParam), &rcvLen, RFAL_TXRX_FLAGS_DEFAULT, (isoDepDev->info.FWT + isoDepDev->info.dFWT)));

  it = 0;

  /*******************************************************************************/
  /* Check S(PARAMETERS) Acknowledge  */
  if ((sParam.pcb != ISODEP_PCB_SPARAMETERS) || (sParam.sParam.tag != RFAL_ISODEP_SPARAM_TAG_BLOCKINFO)  ||
      (sParam.sParam.value[it] != RFAL_ISODEP_SPARAM_TAG_BRACK) || (rcvLen < RFAL_ISODEP_SPARAM_HDR_LEN)) {
    return ERR_PROTO;
  }

  EXIT_ON_ERR(ret, rfalRfDev->rfalSetBitRate(txBR, rxBR));

  isoDepDev->info.DRI = txBR;
  isoDepDev->info.DSI = rxBR;

  return ERR_NONE;
}


/*******************************************************************************/
void RfalNfcClass::rfalIsoDepCalcBitRate(rfalBitRate maxAllowedBR, uint8_t piccBRCapability, rfalBitRate *dsi, rfalBitRate *dri)
{
  uint8_t     driMask;
  uint8_t     dsiMask;
  int8_t      i;
  bool        bitrateFound;
  rfalBitRate curMaxBR;

  curMaxBR = maxAllowedBR;

  do {
    bitrateFound = true;

    (*dsi) = RFAL_BR_106;
    (*dri) = RFAL_BR_106;

    /* Digital 1.0  5.6.2.5 & 11.6.2.14: A received RFU value of b4 = 1b MUST be interpreted as if b7 to b1 ? 0000000b (only 106 kbits/s in both direction) */
    if (((RFAL_ISODEP_BITRATE_RFU_MASK & piccBRCapability) != 0U) || (curMaxBR > RFAL_BR_848)) {
      return;
    }

    /***************************************************************************/
    /* Determine Listen->Poll bit rate */
    dsiMask = (piccBRCapability & RFAL_ISODEP_BSI_MASK);
    for (i = 2; i >= 0; i--) { // Check supported bit rate from the highest
      if (((dsiMask & (0x10U << (uint8_t)i)) != 0U) && (((uint8_t)i + 1U) <= (uint8_t)curMaxBR)) {
        uint8_t newdsi = ((uint8_t) i) + 1U;
        (*dsi) = (rfalBitRate)newdsi; /* PRQA S 4342 # MISRA 10.5 - Layout of enum rfalBitRate and range of loop variable guarantee no invalid enum values to be created */
        break;
      }
    }

    /***************************************************************************/
    /* Determine Poll->Listen bit rate */
    driMask = (piccBRCapability & RFAL_ISODEP_BRI_MASK);
    for (i = 2; i >= 0; i--) { /* Check supported bit rate from the highest */
      if (((driMask & (0x01U << (uint8_t)i)) != 0U) && (((uint8_t)i + 1U) <= (uint8_t)curMaxBR)) {
        uint8_t newdri = ((uint8_t) i) + 1U;
        (*dri) = (rfalBitRate)newdri; /* PRQA S 4342 # MISRA 10.5 - Layout of enum rfalBitRate and range of loop variable guarantee no invalid enum values to be created */
        break;
      }
    }

    /***************************************************************************/
    /* Check if different bit rate is supported */

    /* Digital 1.0 Table 67: if b8=1b, then only the same bit rate divisor for both directions is supported */
    if ((piccBRCapability & RFAL_ISODEP_SAME_BITRATE_MASK) != 0U) {
      (*dsi) = MIN((*dsi), (*dri));
      (*dri) = (*dsi);
      /* Check that the baudrate is supported */
      if ((RFAL_BR_106 != (*dsi)) && (!(((dsiMask & (0x10U << ((uint8_t)(*dsi) - 1U))) != 0U) && ((driMask & (0x01U << ((uint8_t)(*dri) - 1U))) != 0U)))) {
        bitrateFound = false;
        curMaxBR     = (*dsi); /* set allowed bitrate to be lowest and determine bit rate again */
      }
    }
  } while (!(bitrateFound));

}

/*******************************************************************************/
uint32_t RfalNfcClass::rfalIsoDepSFGI2SFGT(uint8_t sfgi)
{
  uint32_t sfgt;
  uint8_t tmpSFGI;

  tmpSFGI = sfgi;

  if (tmpSFGI > ISODEP_SFGI_MAX) {
    tmpSFGI = ISODEP_SFGI_MIN;
  }

  if (tmpSFGI != ISODEP_SFGI_MIN) {
    /* If sfgi != 0 wait SFGT + dSFGT   Digital 1.1  13.8.2.1 */
    sfgt  = isoDepCalcSGFT(sfgi) + isoDepCalcdSGFT(sfgi);
  }
  /* Otherwise use FDTPoll min Digital  1.1  13.8.2.3*/
  else {
    sfgt = RFAL_FDT_POLL_NFCA_POLLER;
  }

  /* Convert carrier cycles to milli seconds */
  return (rfalConv1fcToMs(sfgt) + 1U);
}


/*******************************************************************************/
void RfalNfcClass::rfalIsoDepApdu2IBLockParam(rfalIsoDepApduTxRxParam apduParam, rfalIsoDepTxRxParam *iBlockParam, uint16_t txPos, uint16_t rxPos)
{
  NO_WARNING(rxPos); /* Keep this param for future use */

  iBlockParam->DID    = apduParam.DID;
  iBlockParam->FSx    = apduParam.FSx;
  iBlockParam->ourFSx = apduParam.ourFSx;
  iBlockParam->FWT    = apduParam.FWT;
  iBlockParam->dFWT   = apduParam.dFWT;

  if ((apduParam.txBufLen - txPos) > rfalIsoDepGetMaxInfLen()) {
    iBlockParam->isTxChaining = true;
    iBlockParam->txBufLen     = rfalIsoDepGetMaxInfLen();
  } else {
    iBlockParam->isTxChaining = false;
    iBlockParam->txBufLen     = (apduParam.txBufLen - txPos);
  }

  /* TxBuf is moved to the beginning for every I-Block */
  iBlockParam->txBuf        = (rfalIsoDepBufFormat *)apduParam.txBuf;  /*  PRQA S 0310 # MISRA 11.3 - Intentional safe cast to avoiding large buffer duplication */
  iBlockParam->rxBuf        = apduParam.tmpBuf;                        /* Simply using the apdu buffer is not possible because of current ACK handling */
  iBlockParam->isRxChaining = &gIsoDep.isAPDURxChaining;
  iBlockParam->rxLen        = apduParam.rxLen;
}


/*******************************************************************************/
ReturnCode RfalNfcClass::rfalIsoDepStartApduTransceive(rfalIsoDepApduTxRxParam param)
{
  rfalIsoDepTxRxParam txRxParam;

  /* Initialize and store APDU context */
  gIsoDep.APDUParam = param;
  gIsoDep.APDUTxPos = 0;
  gIsoDep.APDURxPos = 0;

  /* Assign current FSx to calculate INF length */
  gIsoDep.ourFsx = param.ourFSx;
  gIsoDep.fsx    = param.FSx;

  /* Convert APDU TxRxParams to I-Block TxRxParams */
  rfalIsoDepApdu2IBLockParam(gIsoDep.APDUParam, &txRxParam, gIsoDep.APDUTxPos, gIsoDep.APDURxPos);

  return rfalIsoDepStartTransceive(txRxParam);
}


/*******************************************************************************/
ReturnCode RfalNfcClass::rfalIsoDepGetApduTransceiveStatus(void)
{
  ReturnCode          ret;
  rfalIsoDepTxRxParam txRxParam;

  ret = rfalIsoDepGetTransceiveStatus();
  switch (ret) {
    /*******************************************************************************/
    case ERR_NONE:

      /* Check if we are still doing chaining on Tx */
      if (gIsoDep.isTxChaining) {
        /* Add already Tx bytes */
        gIsoDep.APDUTxPos += gIsoDep.txBufLen;

        /* Convert APDU TxRxParams to I-Block TxRxParams */
        rfalIsoDepApdu2IBLockParam(gIsoDep.APDUParam, &txRxParam, gIsoDep.APDUTxPos, gIsoDep.APDURxPos);

        if (txRxParam.txBufLen > 0U) {     /* MISRA 21.18 */
          /* Move next I-Block to beginning of APDU Tx buffer */
          ST_MEMCPY(gIsoDep.APDUParam.txBuf->apdu, &gIsoDep.APDUParam.txBuf->apdu[gIsoDep.APDUTxPos], txRxParam.txBufLen);
        }

        rfalIsoDepStartTransceive(txRxParam);
        return ERR_BUSY;
      }

      if (*gIsoDep.APDUParam.rxLen > 0U) {   /* MISRA 21.18 */
        /* Copy packet from tmp buffer to APDU buffer */
        ST_MEMCPY(&gIsoDep.APDUParam.rxBuf->apdu[gIsoDep.APDURxPos], gIsoDep.APDUParam.tmpBuf->inf, *gIsoDep.APDUParam.rxLen);
        gIsoDep.APDURxPos += *gIsoDep.APDUParam.rxLen;
      }

      /* APDU TxRx is done */
      break;

    /*******************************************************************************/
    case ERR_AGAIN:

      if (*gIsoDep.APDUParam.rxLen > 0U) {   /* MISRA 21.18 */
        /* Copy chained packet from tmp buffer to APDU buffer */
        ST_MEMCPY(&gIsoDep.APDUParam.rxBuf->apdu[gIsoDep.APDURxPos], gIsoDep.APDUParam.tmpBuf->inf, *gIsoDep.APDUParam.rxLen);
        gIsoDep.APDURxPos += *gIsoDep.APDUParam.rxLen;
      }

      /* Wait for next I-Block */
      return ERR_BUSY;

    /*******************************************************************************/
    default:
      return ret;
  }

  *gIsoDep.APDUParam.rxLen = gIsoDep.APDURxPos;

  return ERR_NONE;
}
