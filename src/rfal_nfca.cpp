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
 *  \brief Provides several NFC-A convenience methods and definitions
 *
 *  It provides a Poller (ISO14443A PCD) interface and as well as
 *  some NFC-A Listener (ISO14443A PICC) helpers.
 *
 *  The definitions and helpers methods provided by this module are only
 *  up to ISO14443-3 layer
 *
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "rfal_nfc.h"
#include "rfal_nfca.h"
#include "nfc_utils.h"

/*
 ******************************************************************************
 * ENABLE SWITCH
 ******************************************************************************
 */

/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */

#define RFAL_NFCA_SLP_FWT           rfalConvMsTo1fc(1)    /*!< Check 1ms for any modulation  ISO14443-3 6.4.3   */
#define RFAL_NFCA_SLP_CMD           0x50U                 /*!< SLP cmd (byte1)    Digital 1.1  6.9.1 & Table 20 */
#define RFAL_NFCA_SLP_BYTE2         0x00U                 /*!< SLP byte2          Digital 1.1  6.9.1 & Table 20 */
#define RFAL_NFCA_SLP_CMD_POS       0U                    /*!< SLP cmd position   Digital 1.1  6.9.1 & Table 20 */
#define RFAL_NFCA_SLP_BYTE2_POS     1U                    /*!< SLP byte2 position Digital 1.1  6.9.1 & Table 20 */

#define RFAL_NFCA_SDD_CT            0x88U                 /*!< Cascade Tag value Digital 1.1 6.7.2              */
#define RFAL_NFCA_SDD_CT_LEN        1U                    /*!< Cascade Tag length                               */

#define RFAL_NFCA_SLP_REQ_LEN       2U                    /*!< SLP_REQ length                                   */

#define RFAL_NFCA_SEL_CMD_LEN       1U                    /*!< SEL_CMD length                                   */
#define RFAL_NFCA_SEL_PAR_LEN       1U                    /*!< SEL_PAR length                                   */
#define RFAL_NFCA_SEL_SELPAR        rfalNfcaSelPar(7U, 0U)/*!< SEL_PAR on Select is always with 4 data/nfcid    */
#define RFAL_NFCA_BCC_LEN           1U                    /*!< BCC length                                       */

#define RFAL_NFCA_SDD_REQ_LEN       (RFAL_NFCA_SEL_CMD_LEN + RFAL_NFCA_SEL_PAR_LEN)   /*!< SDD_REQ length       */
#define RFAL_NFCA_SDD_RES_LEN       (RFAL_NFCA_CASCADE_1_UID_LEN + RFAL_NFCA_BCC_LEN) /*!< SDD_RES length       */

#define RFAL_NFCA_T_RETRANS         5U                    /*!< t RETRANSMISSION [3, 33]ms   EMVCo 2.6  A.5      */
#define RFAL_NFCA_N_RETRANS         2U                    /*!< Number of retries            EMVCo 2.6  9.6.1.3  */


/*! SDD_REQ (Select) Cascade Levels  */
enum {
  RFAL_NFCA_SEL_CASCADE_L1 = 0,  /*!< SDD_REQ Cascade Level 1 */
  RFAL_NFCA_SEL_CASCADE_L2 = 1,  /*!< SDD_REQ Cascade Level 2 */
  RFAL_NFCA_SEL_CASCADE_L3 = 2   /*!< SDD_REQ Cascade Level 3 */
};

/*! SDD_REQ (Select) request Cascade Level command   Digital 1.1 Table 15 */
enum {
  RFAL_NFCA_CMD_SEL_CL1 = 0x93, /*!< SDD_REQ command Cascade Level 1 */
  RFAL_NFCA_CMD_SEL_CL2 = 0x95, /*!< SDD_REQ command Cascade Level 2 */
  RFAL_NFCA_CMD_SEL_CL3 = 0x97, /*!< SDD_REQ command Cascade Level 3 */
};

/*
******************************************************************************
* GLOBAL MACROS
******************************************************************************
*/
#define rfalNfcaSelPar( nBy, nbi )         (uint8_t)((((nBy)<<4U) & 0xF0U) | ((nbi)&0x0FU) )         /*!< Calculates SEL_PAR with the bytes/bits to be sent */
#define rfalNfcaCLn2SELCMD( cl )           (uint8_t)((uint8_t)(RFAL_NFCA_CMD_SEL_CL1) + (2U*(cl)))   /*!< Calculates SEL_CMD with the given cascade level   */
#define rfalNfcaNfcidLen2CL( len )         ((len) / 5U)                                              /*!< Calculates cascade level by the NFCID length      */

/*! Executes the given Tx method (f) and if a Timeout error is detected it retries (rt) times performing a delay of (dl) in between  */
#define rfalNfcaTxRetry( r, f, rt, dl )                            \
      {                                                      \
        uint8_t rts = (uint8_t)(rt);                       \
        do {                                   \
          (r)=(f);                                       \
          if (((rt)!=0U) && ((dl)!=0U)) {                \
            delay(dl);                         \
          }                                              \
        } while( ((rts--) != 0U) && ((r)==ERR_TIMEOUT) );  \
      }

/*
******************************************************************************
* GLOBAL TYPES
******************************************************************************
*/

/*! SLP_REQ (HLTA) format   Digital 1.1  6.9.1 & Table 20 */
typedef struct {
  uint8_t      frame[RFAL_NFCA_SLP_REQ_LEN];  /*!< SLP:  0x50 0x00  */
} rfalNfcaSlpReq;

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

uint8_t RfalNfcClass::rfalNfcaCalculateBcc(const uint8_t *buf, uint8_t bufLen)
{
  uint8_t i;
  uint8_t BCC;

  BCC = 0;

  /* BCC is XOR over first 4 bytes of the SDD_RES  Digital 1.1 6.7.2 */
  for (i = 0; i < bufLen; i++) {
    BCC ^= buf[i];
  }

  return BCC;
}

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/

/*******************************************************************************/
ReturnCode RfalNfcClass::rfalNfcaPollerInitialize(void)
{
  ReturnCode ret;

  EXIT_ON_ERR(ret, rfalRfDev->rfalSetMode(RFAL_MODE_POLL_NFCA, RFAL_BR_106, RFAL_BR_106));
  rfalRfDev->rfalSetErrorHandling(RFAL_ERRORHANDLING_NFC);

  rfalRfDev->rfalSetGT(RFAL_GT_NFCA);
  rfalRfDev->rfalSetFDTListen(RFAL_FDT_LISTEN_NFCA_POLLER);
  rfalRfDev->rfalSetFDTPoll(RFAL_FDT_POLL_NFCA_POLLER);

  return ERR_NONE;
}


/*******************************************************************************/
ReturnCode RfalNfcClass::rfalNfcaPollerCheckPresence(rfal14443AShortFrameCmd cmd, rfalNfcaSensRes *sensRes)
{
  ReturnCode ret;
  uint16_t   rcvLen;

  /* Digital 1.1 6.10.1.3  For Commands ALL_REQ, SENS_REQ, SDD_REQ, and SEL_REQ, the NFC Forum Device      *
   *              MUST treat receipt of a Listen Frame at a time after FDT(Listen, min) as a Timeour Error */

  ret = rfalRfDev->rfalISO14443ATransceiveShortFrame(cmd, (uint8_t *)sensRes, (uint8_t)rfalConvBytesToBits(sizeof(rfalNfcaSensRes)), &rcvLen, RFAL_NFCA_FDTMIN);
  if ((ret == ERR_RF_COLLISION) || (ret == ERR_CRC)  || (ret == ERR_NOMEM) || (ret == ERR_FRAMING) || (ret == ERR_PAR)) {
    ret = ERR_NONE;
  }

  return ret;
}


/*******************************************************************************/
ReturnCode RfalNfcClass::rfalNfcaPollerTechnologyDetection(rfalComplianceMode compMode, rfalNfcaSensRes *sensRes)
{
  ReturnCode ret;

  EXIT_ON_ERR(ret, rfalNfcaPollerCheckPresence(((compMode == RFAL_COMPLIANCE_MODE_EMV) ? RFAL_14443A_SHORTFRAME_CMD_WUPA : RFAL_14443A_SHORTFRAME_CMD_REQA), sensRes));

  /* Send SLP_REQ as  Activity 1.1  9.2.3.6 and EMVCo 2.6  9.2.1.3 */
  if (compMode != RFAL_COMPLIANCE_MODE_ISO) {
    rfalNfcaPollerSleep();
  }
  return ERR_NONE;
}

/*******************************************************************************/
ReturnCode RfalNfcClass::rfalNfcaPollerSingleCollisionResolution(uint8_t devLimit, bool *collPending, rfalNfcaSelRes *selRes, uint8_t *nfcId1, uint8_t *nfcId1Len)
{
  uint8_t         i;
  ReturnCode      ret;
  rfalNfcaSelReq  selReq;
  uint16_t        bytesRx;
  uint8_t         bytesTxRx;
  uint8_t         bitsTxRx;
  bool            doBacktrack = false;
  uint8_t         backtrackCnt = 3;

  /* Check parameters */
  if ((collPending == NULL) || (selRes == NULL) || (nfcId1 == NULL) || (nfcId1Len == NULL)) {
    return ERR_PARAM;
  }

  /* Initialize output parameters */
  *collPending = false;  /* Activity 1.1  9.3.4.6 */
  *nfcId1Len   = 0;
  ST_MEMSET(nfcId1, 0x00, RFAL_NFCA_CASCADE_3_UID_LEN);

  /*******************************************************************************/
  /* Go through all Cascade Levels     Activity 1.1  9.3.4 */
  for (i = (uint8_t)RFAL_NFCA_SEL_CASCADE_L1; i <= (uint8_t)RFAL_NFCA_SEL_CASCADE_L3; i++) {
    /* Initialize the SDD_REQ to send for the new cascade level */
    ST_MEMSET((uint8_t *)&selReq, 0x00, sizeof(rfalNfcaSelReq));
    selReq.selCmd = rfalNfcaCLn2SELCMD(i);

    bytesTxRx    = RFAL_NFCA_SDD_REQ_LEN;
    bitsTxRx     = 0;

    /*******************************************************************************/
    /* Go through Collision loop */
    do {
      uint8_t         collBit = 1; /* standards mandate or recommend collision bit to be set to One. */
      /* Calculate SEL_PAR with the bytes/bits to be sent */
      selReq.selPar = rfalNfcaSelPar(bytesTxRx, bitsTxRx);

      /* Send SDD_REQ (Anticollision frame) - Retry upon timeout  EMVCo 2.6  9.6.1.3 */
      rfalNfcaTxRetry(ret, rfalRfDev->rfalISO14443ATransceiveAnticollisionFrame((uint8_t *)&selReq, &bytesTxRx, &bitsTxRx, &bytesRx, RFAL_NFCA_FDTMIN), ((devLimit == 0U) ? RFAL_NFCA_N_RETRANS : 0U), RFAL_NFCA_T_RETRANS);

      bytesRx = rfalConvBitsToBytes(bytesRx);

      if ((ret == ERR_TIMEOUT)
          && (backtrackCnt != 0U) && !doBacktrack
          && !((RFAL_NFCA_SDD_REQ_LEN == bytesTxRx) && (0U == bitsTxRx))) {
        /* In multiple card scenarios it may always happen that some
         * collisions of a weaker tag go unnoticed. If then a later
         * collision is recognized and the strong tag has a 0 at the
         * collision position then no tag will respond. Catch this
         * corner case and then try with the bit being sent as zero. */
        rfalNfcaSensRes sensRes;
        ret = ERR_RF_COLLISION;
        rfalNfcaPollerCheckPresence(RFAL_14443A_SHORTFRAME_CMD_REQA, &sensRes);
        /* Algorithm below does a post-increment, decrement to go back to current position */
        if (0U == bitsTxRx) {
          bitsTxRx = 7;
          bytesTxRx--;
        } else {
          bitsTxRx--;
        }
        collBit = (uint8_t)(((uint8_t *)&selReq)[bytesTxRx] & (1U << bitsTxRx));
        collBit = (uint8_t)((0U == collBit) ? 1U : 0U); // invert the collision bit
        doBacktrack = true;
        backtrackCnt--;
      } else {
        doBacktrack = false;
      }

      if (ret == ERR_RF_COLLISION) {
        /* Check received length */
        if ((bytesTxRx + ((bitsTxRx != 0U) ? 1U : 0U)) > (RFAL_NFCA_SDD_RES_LEN + RFAL_NFCA_SDD_REQ_LEN)) {
          return ERR_PROTO;
        }

        if (((bytesTxRx + ((bitsTxRx != 0U) ? 1U : 0U)) > (RFAL_NFCA_CASCADE_1_UID_LEN + RFAL_NFCA_SDD_REQ_LEN)) && (backtrackCnt != 0U)) {
          /* Collision in BCC: Anticollide only UID part */
          backtrackCnt--;
          bytesTxRx = RFAL_NFCA_CASCADE_1_UID_LEN + RFAL_NFCA_SDD_REQ_LEN - 1U;
          bitsTxRx = 7;
          collBit = (uint8_t)(((uint8_t *)&selReq)[bytesTxRx] & (1U << bitsTxRx));  /* Not a real collision, extract the actual bit for the subsequent code */
        }

        if ((devLimit == 0U) && !(*collPending)) {
          /* Activity 1.0 & 1.1  9.3.4.12: If CON_DEVICES_LIMIT has a value of 0, then
           * NFC Forum Device is configured to perform collision detection only       */
          *collPending = true;
          return ERR_IGNORE;
        }

        *collPending = true;

        /* Set and select the collision bit, with the number of bytes/bits successfully TxRx */
        if (collBit != 0U) {
          ((uint8_t *)&selReq)[bytesTxRx] = (uint8_t)(((uint8_t *)&selReq)[bytesTxRx] | (1U << bitsTxRx)); /* MISRA 10.3 */
        } else {
          ((uint8_t *)&selReq)[bytesTxRx] = (uint8_t)(((uint8_t *)&selReq)[bytesTxRx] & ~(1U << bitsTxRx)); /* MISRA 10.3 */
        }

        bitsTxRx++;

        /* Check if number of bits form a byte */
        if (bitsTxRx == RFAL_BITS_IN_BYTE) {
          bitsTxRx = 0;
          bytesTxRx++;
        }
      }
    } while (ret == ERR_RF_COLLISION);


    /*******************************************************************************/
    /* Check if Collision loop has failed */
    if (ret != ERR_NONE) {
      return ret;
    }


    /* If collisions are to be reported check whether the response is complete */
    if ((devLimit == 0U) && (bytesRx != sizeof(rfalNfcaSddRes))) {
      return ERR_PROTO;
    }

    /* Check if the received BCC match */
    if (selReq.bcc != rfalNfcaCalculateBcc(selReq.nfcid1, RFAL_NFCA_CASCADE_1_UID_LEN)) {
      return ERR_PROTO;
    }

    /*******************************************************************************/
    /* Anticollision OK, Select this Cascade Level */
    selReq.selPar = RFAL_NFCA_SEL_SELPAR;

    /* Send SEL_REQ (Select command) - Retry upon timeout  EMVCo 2.6  9.6.1.3 */
    rfalNfcaTxRetry(ret, rfalRfDev->rfalTransceiveBlockingTxRx((uint8_t *)&selReq, sizeof(rfalNfcaSelReq), (uint8_t *)selRes, sizeof(rfalNfcaSelRes), &bytesRx, RFAL_TXRX_FLAGS_DEFAULT, RFAL_NFCA_FDTMIN), ((devLimit == 0U) ? RFAL_NFCA_N_RETRANS : 0U), RFAL_NFCA_T_RETRANS);

    if (ret != ERR_NONE) {
      return ret;
    }


    /* Ensure proper response length */
    if (bytesRx != sizeof(rfalNfcaSelRes)) {
      return ERR_PROTO;
    }

    /*******************************************************************************/
    /* Check cascade byte, if cascade tag then go next cascade level */
    if ((ret == ERR_NONE) && (*selReq.nfcid1 == RFAL_NFCA_SDD_CT)) {
      /* Cascade Tag present, store nfcid1 bytes (excluding cascade tag) and continue for next CL */
      ST_MEMCPY(&nfcId1[*nfcId1Len], &((uint8_t *)&selReq.nfcid1)[RFAL_NFCA_SDD_CT_LEN], (RFAL_NFCA_CASCADE_1_UID_LEN - RFAL_NFCA_SDD_CT_LEN));
      *nfcId1Len += (RFAL_NFCA_CASCADE_1_UID_LEN - RFAL_NFCA_SDD_CT_LEN);
    } else {
      /* UID Selection complete, Stop Cascade Level loop */
      ST_MEMCPY(&nfcId1[*nfcId1Len], (uint8_t *)&selReq.nfcid1, RFAL_NFCA_CASCADE_1_UID_LEN);
      *nfcId1Len += RFAL_NFCA_CASCADE_1_UID_LEN;
      return ERR_NONE;
    }
  }
  return ERR_INTERNAL;
}


/*******************************************************************************/
ReturnCode RfalNfcClass::rfalNfcaPollerFullCollisionResolution(rfalComplianceMode compMode, uint8_t devLimit, rfalNfcaListenDevice *nfcaDevList, uint8_t *devCnt)
{
  ReturnCode      ret;
  bool            collPending;
  rfalNfcaSensRes sensRes;
  uint16_t        rcvLen;

  if ((nfcaDevList == NULL) || (devCnt == NULL)) {
    return ERR_PARAM;
  }

  *devCnt = 0;
  ret     = ERR_NONE;

  /*******************************************************************************/
  /* Send ALL_REQ before Anticollision if a Sleep was sent before  Activity 1.1  9.3.4.1 and EMVco 2.6  9.3.2.1 */
  if (compMode != RFAL_COMPLIANCE_MODE_ISO) {
    ret = rfalRfDev->rfalISO14443ATransceiveShortFrame(RFAL_14443A_SHORTFRAME_CMD_WUPA, (uint8_t *)&nfcaDevList->sensRes, (uint8_t)rfalConvBytesToBits(sizeof(rfalNfcaSensRes)), &rcvLen, RFAL_NFCA_FDTMIN);
    if (ret != ERR_NONE) {
      if ((compMode == RFAL_COMPLIANCE_MODE_EMV) || ((ret != ERR_RF_COLLISION) && (ret != ERR_CRC) && (ret != ERR_FRAMING) && (ret != ERR_PAR))) {
        return ret;
      }
    }

    /* Check proper SENS_RES/ATQA size */
    if ((ret == ERR_NONE) && (rfalConvBytesToBits(sizeof(rfalNfcaSensRes)) != rcvLen)) {
      return ERR_PROTO;
    }
  }

  /*******************************************************************************/
  /* Only check for T1T if previous SENS_RES was received without a transmission  *
   * error. When collisions occur bits in the SENS_RES may look like a T1T        */
  /* If T1T Anticollision is not supported  Activity 1.1  9.3.4.3 */
  if (rfalNfcaIsSensResT1T(&nfcaDevList->sensRes) && (devLimit != 0U) && (ret == ERR_NONE) && (compMode != RFAL_COMPLIANCE_MODE_EMV)) {
    /* RID_REQ shall be performed with rfalT1TPollerRid()    Activity 1.1  9.3.4.24 */
    rfalT1TPollerInitialize();
    EXIT_ON_ERR(ret, rfalT1TPollerRid(&nfcaDevList->ridRes));

    /* T1T doesn't support Anticollision */
    *devCnt = 1;
    nfcaDevList->isSleep   = false;
    nfcaDevList->type      = RFAL_NFCA_T1T;
    nfcaDevList->nfcId1Len = RFAL_NFCA_CASCADE_1_UID_LEN;
    ST_MEMCPY(&nfcaDevList->nfcId1, &nfcaDevList->ridRes.uid, RFAL_NFCA_CASCADE_1_UID_LEN);

    return ERR_NONE;
  }

  /*******************************************************************************/
  /* Store the SENS_RES from Technology Detection or from WUPA */
  sensRes = nfcaDevList->sensRes;

  if (devLimit > 0U) { /* MISRA 21.18 */
    ST_MEMSET(nfcaDevList, 0x00, (sizeof(rfalNfcaListenDevice) * devLimit));
  }

  /* Restore the prev SENS_RES, assuming that the SENS_RES received is from first device
   * When only one device is detected it's not woken up then we'll have no SENS_RES (ATQA) */
  nfcaDevList->sensRes = sensRes;


  /*******************************************************************************/
  do {
    uint8_t newDeviceType;

    EXIT_ON_ERR(ret, rfalNfcaPollerSingleCollisionResolution(devLimit, &collPending, &nfcaDevList[*devCnt].selRes, (uint8_t *)&nfcaDevList[*devCnt].nfcId1, (uint8_t *)&nfcaDevList[*devCnt].nfcId1Len));

    /* Assign Listen Device */
    newDeviceType = ((uint8_t)nfcaDevList[*devCnt].selRes.sak) & RFAL_NFCA_SEL_RES_CONF_MASK;  /* MISRA 10.8 */
    /* PRQA S 4342 1 # MISRA 10.5 - Guaranteed that no invalid enum values are created: see guard_eq_RFAL_NFCA_T2T, .... */
    nfcaDevList[*devCnt].type    = (rfalNfcaListenDeviceType)(newDeviceType);
    nfcaDevList[*devCnt].isSleep = false;
    (*devCnt)++;


    /* If a collision was detected and device counter is lower than limit  Activity 1.1  9.3.4.21 */
    if ((*devCnt < devLimit) && ((collPending) || (compMode != RFAL_COMPLIANCE_MODE_ISO))) {
      /* Put this device to Sleep  Activity 1.1  9.3.4.22 */
      rfalNfcaPollerSleep();
      nfcaDevList[(*devCnt - 1U)].isSleep = true;


      /* Send a new SENS_REQ to check for other cards  Activity 1.1  9.3.4.23 */
      ret = rfalNfcaPollerCheckPresence(RFAL_14443A_SHORTFRAME_CMD_REQA, &nfcaDevList[*devCnt].sensRes);
      if (ret == ERR_TIMEOUT) {
        /* No more devices found, exit */
        collPending = false;
      } else {
        /* Another device found, continue loop */
        collPending = true;
      }
    } else {
      /* Exit loop */
      collPending = false;
    }
  } while ((*devCnt < devLimit) && (collPending));

  return ERR_NONE;
}


/*******************************************************************************/
ReturnCode RfalNfcClass::rfalNfcaPollerSelect(const uint8_t *nfcid1, uint8_t nfcidLen, rfalNfcaSelRes *selRes)
{
  uint8_t        i;
  uint8_t        cl;
  uint8_t        nfcidOffset;
  uint16_t       rxLen;
  ReturnCode     ret;
  rfalNfcaSelReq selReq;

  if ((nfcid1 == NULL) || (nfcidLen > RFAL_NFCA_CASCADE_3_UID_LEN) || (selRes == NULL)) {
    return ERR_PARAM;
  }


  /* Calculate Cascate Level */
  cl          = rfalNfcaNfcidLen2CL(nfcidLen);
  nfcidOffset = 0;

  /*******************************************************************************/
  /* Go through all Cascade Levels     Activity 1.1  9.4.4 */
  for (i = RFAL_NFCA_SEL_CASCADE_L1; i <= cl; i++) {
    /* Assign SEL_CMD according to the CLn and SEL_PAR*/
    selReq.selCmd = rfalNfcaCLn2SELCMD(i);
    selReq.selPar = RFAL_NFCA_SEL_SELPAR;

    /* Compute NFCID/Data on the SEL_REQ command   Digital 1.1  Table 18 */
    if (cl != i) {
      *selReq.nfcid1 = RFAL_NFCA_SDD_CT;
      ST_MEMCPY(&selReq.nfcid1[RFAL_NFCA_SDD_CT_LEN], &nfcid1[nfcidOffset], (RFAL_NFCA_CASCADE_1_UID_LEN - RFAL_NFCA_SDD_CT_LEN));
      nfcidOffset += (RFAL_NFCA_CASCADE_1_UID_LEN - RFAL_NFCA_SDD_CT_LEN);
    } else {
      ST_MEMCPY(selReq.nfcid1, &nfcid1[nfcidOffset], RFAL_NFCA_CASCADE_1_UID_LEN);
    }

    /* Calculate nfcid's BCC */
    selReq.bcc = rfalNfcaCalculateBcc((uint8_t *)&selReq.nfcid1, sizeof(selReq.nfcid1));

    /*******************************************************************************/
    /* Send SEL_REQ  */
    EXIT_ON_ERR(ret, rfalRfDev->rfalTransceiveBlockingTxRx((uint8_t *)&selReq, sizeof(rfalNfcaSelReq), (uint8_t *)selRes, sizeof(rfalNfcaSelRes), &rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_NFCA_FDTMIN));

    /* Ensure proper response length */
    if (rxLen != sizeof(rfalNfcaSelRes)) {
      return ERR_PROTO;
    }
  }

  /* REMARK: Could check if NFCID1 is complete */

  return ERR_NONE;
}


/*******************************************************************************/
ReturnCode RfalNfcClass::rfalNfcaPollerSleep(void)
{
  rfalNfcaSlpReq slpReq;
  uint8_t        rxBuf;    /* dummy buffer, just to perform Rx */

  slpReq.frame[RFAL_NFCA_SLP_CMD_POS]   = RFAL_NFCA_SLP_CMD;
  slpReq.frame[RFAL_NFCA_SLP_BYTE2_POS] = RFAL_NFCA_SLP_BYTE2;

  rfalRfDev->rfalTransceiveBlockingTxRx((uint8_t *)&slpReq, sizeof(rfalNfcaSlpReq), &rxBuf, sizeof(rxBuf), NULL, RFAL_TXRX_FLAGS_DEFAULT, RFAL_NFCA_SLP_FWT);

  /* ISO14443-3 6.4.3  HLTA - If PICC responds with any modulation during 1 ms this response shall be interpreted as not acknowledge
     Digital 2.0  6.9.2.1 & EMVCo 3.0  5.6.2.1 - consider the HLTA command always acknowledged
     No check to be compliant with NFC and EMVCo, and to improve interoprability (Kovio RFID Tag)
  */

  return ERR_NONE;
}


/*******************************************************************************/
bool RfalNfcClass::rfalNfcaListenerIsSleepReq(const uint8_t *buf, uint16_t bufLen)
{
  /* Check if length and payload match */
  if ((bufLen != sizeof(rfalNfcaSlpReq)) || (buf[RFAL_NFCA_SLP_CMD_POS] != RFAL_NFCA_SLP_CMD) || (buf[RFAL_NFCA_SLP_BYTE2_POS] != RFAL_NFCA_SLP_BYTE2)) {
    return false;
  }

  return true;
}

/* If the guards here don't compile then the code above cannot work anymore. */
extern uint8_t guard_eq_RFAL_NFCA_T2T[((RFAL_NFCA_SEL_RES_CONF_MASK & (uint8_t)RFAL_NFCA_T2T) == (uint8_t)RFAL_NFCA_T2T) ? 1 : (-1)];
extern uint8_t guard_eq_RFAL_NFCA_T4T[((RFAL_NFCA_SEL_RES_CONF_MASK & (uint8_t)RFAL_NFCA_T4T) == (uint8_t)RFAL_NFCA_T4T) ? 1 : (-1)];
extern uint8_t guard_eq_RFAL_NFCA_NFCDEP[((RFAL_NFCA_SEL_RES_CONF_MASK & (uint8_t)RFAL_NFCA_NFCDEP) == (uint8_t)RFAL_NFCA_NFCDEP) ? 1 : (-1)];
extern uint8_t guard_eq_RFAL_NFCA_T4T_NFCDEP[((RFAL_NFCA_SEL_RES_CONF_MASK & (uint8_t)RFAL_NFCA_T4T_NFCDEP) == (uint8_t)RFAL_NFCA_T4T_NFCDEP) ? 1 : (-1)];
