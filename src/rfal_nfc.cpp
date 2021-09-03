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
 *  \brief RFAL NFC device
 *
 *  This module provides the required features to behave as an NFC Poller
 *  or Listener device. It grants an easy to use interface for the following
 *  activities: Technology Detection, Collision Resollution, Activation,
 *  Data Exchange, and Deactivation
 *
 *  This layer is influenced by (but not fully aligned with) the NFC Forum
 *  specifications, in particular: Activity 2.0 and NCI 2.0
 *
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "rfal_nfc.h"
#include "nfc_utils.h"


/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/

/*
******************************************************************************
* GLOBAL MACROS
******************************************************************************
*/
#define rfalNfcIsRemDevPoller( tp )    ( ((tp)>= RFAL_NFC_POLL_TYPE_NFCA) && ((tp)<=RFAL_NFC_POLL_TYPE_AP2P ) )
#define rfalNfcIsRemDevListener( tp )  ( /*((tp)>= RFAL_NFC_LISTEN_TYPE_NFCA) && */ ((tp)<=RFAL_NFC_LISTEN_TYPE_AP2P) )

#define rfalNfcNfcNotify( st )         if( gNfcDev.disc.notifyCb != NULL )  gNfcDev.disc.notifyCb( st )


/** Constructor I2C
 *  @param i2c object
 *  @param address the address of the component's instance
 */
RfalNfcClass::RfalNfcClass(RfalRfClass *rfal_rf) : rfalRfDev(rfal_rf)
{
  memset(&gNfcDev, 0, sizeof(rfalNfc));
  memset(&gIsoDep, 0, sizeof(rfalIsoDep));
  memset(&gRfalNfcb, 0, sizeof(rfalNfcb));
  memset(&gNfcip, 0, sizeof(rfalNfcDep));
  memset(&gRfalNfcfGreedyF, 0, sizeof(rfalNfcfGreedyF));
}


/*******************************************************************************/
ReturnCode RfalNfcClass::rfalNfcInitialize(void)
{
  ReturnCode err;

  gNfcDev.state = RFAL_NFC_STATE_NOTINIT;

  EXIT_ON_ERR(err, rfalRfDev->rfalInitialize());        /* Initialize RFAL */

  gNfcDev.state = RFAL_NFC_STATE_IDLE;         /* Go to initialized */
  return ERR_NONE;
}

/*******************************************************************************/
ReturnCode RfalNfcClass::rfalNfcDiscover(const rfalNfcDiscoverParam *disParams)
{
  /* Check if initialization has been performed */
  if (gNfcDev.state != RFAL_NFC_STATE_IDLE) {
    return ERR_WRONG_STATE;
  }

  /* Check valid parameters */
  if ((disParams == NULL) || (disParams->devLimit > RFAL_NFC_MAX_DEVICES) || (disParams->devLimit == 0U)                                                ||
      (((disParams->techs2Find & RFAL_NFC_POLL_TECH_F) != 0U)     && (disParams->nfcfBR != RFAL_BR_212) && (disParams->nfcfBR != RFAL_BR_424))        ||
      ((((disParams->techs2Find & RFAL_NFC_POLL_TECH_AP2P) != 0U) && (disParams->ap2pBR > RFAL_BR_424)) || (disParams->GBLen > RFAL_NFCDEP_GB_MAX_LEN))) {
    return ERR_PARAM;
  }

  /* Initialize context for discovery */
  gNfcDev.activeDev       = NULL;
  gNfcDev.techsFound      = RFAL_NFC_TECH_NONE;
  gNfcDev.devCnt          = 0;
  gNfcDev.discRestart     = true;
  gNfcDev.disc            = *disParams;


  /* Calculate Listen Mask */
  gNfcDev.lmMask  = 0U;
  gNfcDev.lmMask |= (((gNfcDev.disc.techs2Find & RFAL_NFC_LISTEN_TECH_A) != 0U) ? RFAL_LM_MASK_NFCA : 0U);
  gNfcDev.lmMask |= (((gNfcDev.disc.techs2Find & RFAL_NFC_LISTEN_TECH_B) != 0U) ? RFAL_LM_MASK_NFCB : 0U);
  gNfcDev.lmMask |= (((gNfcDev.disc.techs2Find & RFAL_NFC_LISTEN_TECH_F) != 0U) ? RFAL_LM_MASK_NFCF : 0U);
  gNfcDev.lmMask |= (((gNfcDev.disc.techs2Find & RFAL_NFC_LISTEN_TECH_AP2P) != 0U) ? RFAL_LM_MASK_ACTIVE_P2P : 0U);

  /* Check if Listen Mode is supported/Enabled */
  if (gNfcDev.lmMask != 0U) {
    return ERR_NOTSUPP;
  }

  gNfcDev.state = RFAL_NFC_STATE_START_DISCOVERY;

  return ERR_NONE;
}

/*******************************************************************************/
ReturnCode RfalNfcClass::rfalNfcDeactivate(bool discovery)
{
  /* Check for valid state */
  if (gNfcDev.state <= RFAL_NFC_STATE_IDLE) {
    return ERR_WRONG_STATE;
  }

  /* Check if discovery is to continue afterwards */
  if (discovery == true) {
    /* If so let the state machine continue*/
    gNfcDev.discRestart = discovery;
    gNfcDev.state       = RFAL_NFC_STATE_DEACTIVATION;
  } else {
    /* Otherwise deactivate immediately and go to IDLE */
    rfalNfcDeactivation();
    gNfcDev.state = RFAL_NFC_STATE_IDLE;
  }

  return ERR_NONE;
}

/*******************************************************************************/
ReturnCode RfalNfcClass::rfalNfcSelect(uint8_t devIdx)
{
  /* Check for valid state */
  if (gNfcDev.state != RFAL_NFC_STATE_POLL_SELECT) {
    return ERR_WRONG_STATE;
  }

  gNfcDev.selDevIdx = devIdx;
  gNfcDev.state     = RFAL_NFC_STATE_POLL_ACTIVATION;

  return ERR_NONE;
}

/*******************************************************************************/
rfalNfcState RfalNfcClass::rfalNfcGetState(void)
{
  return gNfcDev.state;
}

/*******************************************************************************/
ReturnCode RfalNfcClass::rfalNfcGetDevicesFound(rfalNfcDevice **devList, uint8_t *devCnt)
{
  /* Check for valid state */
  if (gNfcDev.state < RFAL_NFC_STATE_POLL_SELECT) {
    return ERR_WRONG_STATE;
  }

  /* Check valid parameters */
  if ((devList == NULL) || (devCnt == NULL)) {
    return ERR_PARAM;
  }

  *devCnt  = gNfcDev.devCnt;
  *devList = gNfcDev.devList;

  return ERR_NONE;
}

/*******************************************************************************/
ReturnCode RfalNfcClass::rfalNfcGetActiveDevice(rfalNfcDevice **dev)
{
  /* Check for valid state */
  if (gNfcDev.state < RFAL_NFC_STATE_ACTIVATED) {
    return ERR_WRONG_STATE;
  }

  /* Check valid parameter */
  if (dev == NULL) {
    return ERR_PARAM;
  }

  /* Check for valid state */
  if ((gNfcDev.devCnt == 0U) || (gNfcDev.activeDev == NULL)) {
    return ERR_REQUEST;
  }

  *dev = gNfcDev.activeDev;
  return ERR_NONE;
}

/*******************************************************************************/
void RfalNfcClass::rfalNfcWorker(void)
{
  ReturnCode err;

  rfalRfDev->rfalWorker();                                                                     /* Execute RFAL process  */

  switch (gNfcDev.state) {
    /*******************************************************************************/
    case RFAL_NFC_STATE_NOTINIT:
    case RFAL_NFC_STATE_IDLE:
      break;

    /*******************************************************************************/
    case RFAL_NFC_STATE_START_DISCOVERY:

      /* Initialize context for discovery cycle */
      gNfcDev.devCnt      = 0;
      gNfcDev.selDevIdx   = 0;
      gNfcDev.techsFound  = RFAL_NFC_TECH_NONE;
      gNfcDev.techs2do    = gNfcDev.disc.techs2Find;
      gNfcDev.state       = RFAL_NFC_STATE_POLL_TECHDETECT;

      /* Check if Low power Wake-Up is to be performed */
      if (gNfcDev.disc.wakeupEnabled) {
        /* Initialize Low power Wake-up mode and wait */
        err = rfalRfDev->rfalWakeUpModeStart((gNfcDev.disc.wakeupConfigDefault ? NULL : &gNfcDev.disc.wakeupConfig));
        if (err == ERR_NONE) {
          gNfcDev.state = RFAL_NFC_STATE_WAKEUP_MODE;
          rfalNfcNfcNotify(gNfcDev.state);                                  /* Notify caller that WU was started */
        }
      }

      break;

    /*******************************************************************************/
    case RFAL_NFC_STATE_WAKEUP_MODE:

      /* Check if the Wake-up mode has woke */
      if (rfalRfDev->rfalWakeUpModeHasWoke()) {
        rfalRfDev->rfalWakeUpModeStop();                                                 /* Disable Wake-up mode           */
        gNfcDev.state = RFAL_NFC_STATE_POLL_TECHDETECT;                       /* Go to Technology detection     */

        rfalNfcNfcNotify(gNfcDev.state);                                      /* Notify caller that WU has woke */
      }

      break;

    /*******************************************************************************/
    case RFAL_NFC_STATE_POLL_TECHDETECT:

      /* Start total duration timer */
      gNfcDev.discTmr = (uint32_t)timerCalculateTimer(gNfcDev.disc.totalDuration);

      err = rfalNfcPollTechDetetection();                                       /* Perform Technology Detection                         */
      if (err != ERR_BUSY) {                                                    /* Wait until all technologies are performed            */
        if ((err != ERR_NONE) || (gNfcDev.techsFound == RFAL_NFC_TECH_NONE)) { /* Check if any error occurred or no techs were found   */
          rfalRfDev->rfalFieldOff();
          gNfcDev.state = RFAL_NFC_STATE_LISTEN_TECHDETECT;                 /* Nothing found as poller, go to listener */
          break;
        }

        gNfcDev.techs2do = gNfcDev.techsFound;                                /* Store the found technologies for collision resolution */
        gNfcDev.state    = RFAL_NFC_STATE_POLL_COLAVOIDANCE;                  /* One or more devices found, go to Collision Avoidance  */
      }
      break;


    /*******************************************************************************/
    case RFAL_NFC_STATE_POLL_COLAVOIDANCE:

      err = rfalNfcPollCollResolution();                                        /* Resolve any eventual collision                       */
      if (err != ERR_BUSY) {                                                    /* Wait until all technologies are performed            */
        if ((err != ERR_NONE) || (gNfcDev.devCnt == 0U)) {                    /* Check if any error occurred or no devices were found */
          gNfcDev.state = RFAL_NFC_STATE_DEACTIVATION;
          break;                                                            /* Unable to retrieve any device, restart loop          */
        }

        /* Check if more than one device has been found */
        if (gNfcDev.devCnt > 1U) {
          /* If more than one device was found inform upper layer to choose which one to activate */
          if (gNfcDev.disc.notifyCb != NULL) {
            gNfcDev.state = RFAL_NFC_STATE_POLL_SELECT;
            gNfcDev.disc.notifyCb(gNfcDev.state);
            break;
          }
        }

        /* If only one device or no callback has been set, activate the first device found */
        gNfcDev.selDevIdx = 0U;
        gNfcDev.state = RFAL_NFC_STATE_POLL_ACTIVATION;
      }
      break;


    /*******************************************************************************/
    case RFAL_NFC_STATE_POLL_ACTIVATION:

      if (rfalNfcPollActivation(gNfcDev.selDevIdx) != ERR_NONE) {               /* Activate selected device           */
        gNfcDev.state = RFAL_NFC_STATE_DEACTIVATION;                          /* If Activation failed, restart loop */
        break;
      }

      gNfcDev.state = RFAL_NFC_STATE_ACTIVATED;                                 /* Device has been properly activated */
      rfalNfcNfcNotify(gNfcDev.state);                                          /* Inform upper layer that a device has been activated */
      break;


    /*******************************************************************************/
    case RFAL_NFC_STATE_DATAEXCHANGE:

      rfalNfcDataExchangeGetStatus();                                           /* Run the internal state machine */

      if (gNfcDev.dataExErr != ERR_BUSY) {                                      /* If Dataexchange has terminated */
        gNfcDev.state = RFAL_NFC_STATE_DATAEXCHANGE_DONE;                     /* Go to done state               */
        rfalNfcNfcNotify(gNfcDev.state);                                      /* And notify caller              */
      }
      if (gNfcDev.dataExErr == ERR_SLEEP_REQ) {                                 /* Check if Listen mode has to go to Sleep */
        gNfcDev.state = RFAL_NFC_STATE_LISTEN_SLEEP;                          /* Go to Listen Sleep state       */
        rfalNfcNfcNotify(gNfcDev.state);                                      /* And notify caller              */
      }
      break;


    /*******************************************************************************/
    case RFAL_NFC_STATE_DEACTIVATION:

      rfalNfcDeactivation();                                                    /* Deactivate current device */

      gNfcDev.state = ((gNfcDev.discRestart) ? RFAL_NFC_STATE_START_DISCOVERY : RFAL_NFC_STATE_IDLE);
      rfalNfcNfcNotify(gNfcDev.state);                                          /* Notify caller             */
      break;

    /*******************************************************************************/
    case RFAL_NFC_STATE_LISTEN_TECHDETECT:

      if (timerIsExpired(gNfcDev.discTmr)) {
        rfalRfDev->rfalFieldOff();

        gNfcDev.state = RFAL_NFC_STATE_START_DISCOVERY;                       /* Restart the discovery loop */
        break;
      }

      break;

    /*******************************************************************************/
    case RFAL_NFC_STATE_ACTIVATED:
    case RFAL_NFC_STATE_POLL_SELECT:
    case RFAL_NFC_STATE_DATAEXCHANGE_DONE:
    default:
      return;
  }
}


/*******************************************************************************/
ReturnCode RfalNfcClass::rfalNfcDataExchangeStart(uint8_t *txData, uint16_t txDataLen, uint8_t **rxData, uint16_t **rvdLen, uint32_t fwt)
{
  ReturnCode            err;
  rfalTransceiveContext ctx;

  /*******************************************************************************/
  /* The Data Exchange is divided in two different moments, the trigger/Start of *
   *  the transfer followed by the check until its completion                    */
  if ((gNfcDev.state >= RFAL_NFC_STATE_ACTIVATED) && (gNfcDev.activeDev != NULL)) {

    /*******************************************************************************/
    /* In Listen mode is the Poller that initiates the communicatation             */
    /* Assign output parameters and rfalNfcDataExchangeGetStatus will return       */
    /* incoming data from Poller/Initiator                                         */
    if ((gNfcDev.state == RFAL_NFC_STATE_ACTIVATED) && rfalNfcIsRemDevPoller(gNfcDev.activeDev->type)) {
      if (txDataLen > 0U) {
        return ERR_WRONG_STATE;
      }

      *rvdLen = (uint16_t *)&gNfcDev.rxLen;
      *rxData = (uint8_t *)((gNfcDev.activeDev->rfInterface == RFAL_NFC_INTERFACE_ISODEP) ? gNfcDev.rxBuf.isoDepBuf.inf :
                            ((gNfcDev.activeDev->rfInterface == RFAL_NFC_INTERFACE_NFCDEP) ? gNfcDev.rxBuf.nfcDepBuf.inf : gNfcDev.rxBuf.rfBuf));
      return ERR_NONE;
    }


    /*******************************************************************************/
    switch (gNfcDev.activeDev->rfInterface) {                                     /* Check which RF interface shall be used/has been activated */
      /*******************************************************************************/
      case RFAL_NFC_INTERFACE_RF:

        rfalCreateByteFlagsTxRxContext(ctx, (uint8_t *)txData, txDataLen, gNfcDev.rxBuf.rfBuf, sizeof(gNfcDev.rxBuf.rfBuf), &gNfcDev.rxLen, RFAL_TXRX_FLAGS_DEFAULT, fwt);
        *rxData = (uint8_t *)gNfcDev.rxBuf.rfBuf;
        *rvdLen = (uint16_t *)&gNfcDev.rxLen;
        err = rfalRfDev->rfalStartTransceive(&ctx);
        break;

      /*******************************************************************************/
      case RFAL_NFC_INTERFACE_ISODEP: {
          rfalIsoDepTxRxParam isoDepTxRx;

          if (txDataLen > 0U) {
            ST_MEMCPY((uint8_t *)gNfcDev.txBuf.isoDepBuf.inf, txData, txDataLen);
          }

          isoDepTxRx.DID          = RFAL_ISODEP_NO_DID;
          isoDepTxRx.ourFSx       = RFAL_ISODEP_FSX_KEEP;
          isoDepTxRx.FSx          = gNfcDev.activeDev->proto.isoDep.info.FSx;
          isoDepTxRx.dFWT         = gNfcDev.activeDev->proto.isoDep.info.dFWT;
          isoDepTxRx.FWT          = gNfcDev.activeDev->proto.isoDep.info.FWT;
          isoDepTxRx.txBuf        = &gNfcDev.txBuf.isoDepBuf;
          isoDepTxRx.txBufLen     = txDataLen;
          isoDepTxRx.isTxChaining = false;
          isoDepTxRx.rxBuf        = &gNfcDev.rxBuf.isoDepBuf;
          isoDepTxRx.rxLen        = &gNfcDev.rxLen;
          isoDepTxRx.isRxChaining = &gNfcDev.isRxChaining;
          *rxData                 = (uint8_t *)gNfcDev.rxBuf.isoDepBuf.inf;
          *rvdLen                 = (uint16_t *)&gNfcDev.rxLen;

          /*******************************************************************************/
          /* Trigger a RFAL ISO-DEP Transceive                                           */
          err = rfalIsoDepStartTransceive(isoDepTxRx);
          break;
        }

      /*******************************************************************************/
      case RFAL_NFC_INTERFACE_NFCDEP: {
          rfalNfcDepTxRxParam nfcDepTxRx;

          if (txDataLen > 0U) {
            ST_MEMCPY((uint8_t *)gNfcDev.txBuf.nfcDepBuf.inf, txData, txDataLen);
          }

          nfcDepTxRx.DID          = RFAL_NFCDEP_DID_KEEP;
          nfcDepTxRx.FSx          = rfalNfcDepLR2FS((uint8_t)rfalNfcDepPP2LR(gNfcDev.activeDev->proto.nfcDep.activation.Target.ATR_RES.PPt));
          nfcDepTxRx.dFWT         = gNfcDev.activeDev->proto.nfcDep.info.dFWT;
          nfcDepTxRx.FWT          = gNfcDev.activeDev->proto.nfcDep.info.FWT;
          nfcDepTxRx.txBuf        = &gNfcDev.txBuf.nfcDepBuf;
          nfcDepTxRx.txBufLen     = txDataLen;
          nfcDepTxRx.isTxChaining = false;
          nfcDepTxRx.rxBuf        = &gNfcDev.rxBuf.nfcDepBuf;
          nfcDepTxRx.rxLen        = &gNfcDev.rxLen;
          nfcDepTxRx.isRxChaining = &gNfcDev.isRxChaining;
          *rxData                 = (uint8_t *)gNfcDev.rxBuf.nfcDepBuf.inf;
          *rvdLen                 = (uint16_t *)&gNfcDev.rxLen;

          /*******************************************************************************/
          /* Trigger a RFAL NFC-DEP Transceive                                           */
          err = rfalNfcDepStartTransceive(&nfcDepTxRx);
          break;
        }

      /*******************************************************************************/
      default:
        err = ERR_PARAM;
        break;
    }

    /* If a transceive has succesfully started flag Data Exchange as ongoing */
    if (err == ERR_NONE) {
      gNfcDev.dataExErr = ERR_BUSY;
      gNfcDev.state     = RFAL_NFC_STATE_DATAEXCHANGE;
    }

    return err;
  }

  return ERR_WRONG_STATE;
}


/*******************************************************************************/
ReturnCode RfalNfcClass::rfalNfcDataExchangeGetStatus(void)
{
  /*******************************************************************************/
  /* Check if it's the first frame received in Listen mode */
  if (gNfcDev.state == RFAL_NFC_STATE_ACTIVATED) {
    /* Continue data exchange as normal */
    gNfcDev.dataExErr = ERR_BUSY;
    gNfcDev.state     = RFAL_NFC_STATE_DATAEXCHANGE;

    /* Check if we performing in T3T CE */
    if ((gNfcDev.activeDev->type == RFAL_NFC_POLL_TYPE_NFCF) && (gNfcDev.activeDev->rfInterface == RFAL_NFC_INTERFACE_RF)) {
      /* The first frame has been retrieved by rfalListenMode, flag data immediately                  */
      /* Can only call rfalGetTransceiveStatus() after starting a transceive with rfalStartTransceive */
      gNfcDev.dataExErr = ERR_NONE;
    }
  }


  /*******************************************************************************/
  /* Check if we are in we have been placed to sleep, and return last error     */
  if (gNfcDev.state == RFAL_NFC_STATE_LISTEN_SLEEP) {
    return gNfcDev.dataExErr;                                /* ERR_SLEEP_REQ */
  }


  /*******************************************************************************/
  /* Check if Data exchange has been started */
  if ((gNfcDev.state != RFAL_NFC_STATE_DATAEXCHANGE) && (gNfcDev.state != RFAL_NFC_STATE_DATAEXCHANGE_DONE)) {
    return ERR_WRONG_STATE;
  }

  /* Check if Data exchange is still ongoing */
  if (gNfcDev.dataExErr == ERR_BUSY) {
    switch (gNfcDev.activeDev->rfInterface) {
      /*******************************************************************************/
      case RFAL_NFC_INTERFACE_RF:
        gNfcDev.dataExErr = rfalRfDev->rfalGetTransceiveStatus();
        break;

      /*******************************************************************************/
      case RFAL_NFC_INTERFACE_ISODEP:
        gNfcDev.dataExErr = rfalIsoDepGetTransceiveStatus();
        break;

      /*******************************************************************************/
      case RFAL_NFC_INTERFACE_NFCDEP:
        gNfcDev.dataExErr = rfalNfcDepGetTransceiveStatus();
        break;

      /*******************************************************************************/
      default:
        gNfcDev.dataExErr = ERR_PARAM;
        break;
    }

  }

  return gNfcDev.dataExErr;
}

/*!
 ******************************************************************************
 * \brief Poller Technology Detection
 *
 * This method implements the Technology Detection / Poll for different
 * device technologies.
 *
 * \return  ERR_NONE         : Operation completed with no error
 * \return  ERR_BUSY         : Operation ongoing
 * \return  ERR_XXXX         : Error occurred
 *
 ******************************************************************************
 */
ReturnCode RfalNfcClass::rfalNfcPollTechDetetection(void)
{
  ReturnCode           err;

  err = ERR_NONE;

  /* Supress warning when specific RFAL features have been disabled */
  NO_WARNING(err);


  /*******************************************************************************/
  /* AP2P Technology Detection                                                   */
  /*******************************************************************************/
  if (((gNfcDev.disc.techs2Find & RFAL_NFC_POLL_TECH_AP2P) != 0U) && ((gNfcDev.techs2do & RFAL_NFC_POLL_TECH_AP2P) != 0U)) {
    gNfcDev.techs2do &= ~RFAL_NFC_POLL_TECH_AP2P;

    EXIT_ON_ERR(err, rfalRfDev->rfalSetMode(RFAL_MODE_POLL_ACTIVE_P2P, gNfcDev.disc.ap2pBR, gNfcDev.disc.ap2pBR));
    rfalRfDev->rfalSetErrorHandling(RFAL_ERRORHANDLING_NFC);
    rfalRfDev->rfalSetFDTListen(RFAL_FDT_LISTEN_AP2P_POLLER);
    rfalRfDev->rfalSetFDTPoll(RFAL_TIMING_NONE);
    rfalRfDev->rfalSetGT(RFAL_GT_AP2P_ADJUSTED);
    EXIT_ON_ERR(err, rfalRfDev->rfalFieldOnAndStartGT());                                       /* Turns the Field On and starts GT timer */

    err = rfalNfcNfcDepActivate(gNfcDev.devList, RFAL_NFCDEP_COMM_ACTIVE, NULL, 0);  /* Poll for NFC-A devices */
    if (err == ERR_NONE) {
      gNfcDev.techsFound |= RFAL_NFC_POLL_TECH_AP2P;

      gNfcDev.devList->type        = RFAL_NFC_LISTEN_TYPE_AP2P;
      gNfcDev.devList->rfInterface = RFAL_NFC_INTERFACE_NFCDEP;
      gNfcDev.devCnt++;

      return ERR_NONE;
    }

    rfalRfDev->rfalFieldOff();
    return ERR_BUSY;
  }


  /*******************************************************************************/
  /* Passive NFC-A Technology Detection                                          */
  /*******************************************************************************/
  if (((gNfcDev.disc.techs2Find & RFAL_NFC_POLL_TECH_A) != 0U) && ((gNfcDev.techs2do & RFAL_NFC_POLL_TECH_A) != 0U)) {
    gNfcDev.techs2do &= ~RFAL_NFC_POLL_TECH_A;

    {
      rfalNfcaSensRes sensRes;

      EXIT_ON_ERR(err, rfalNfcaPollerInitialize());                              /* Initialize RFAL for NFC-A */
      EXIT_ON_ERR(err, rfalRfDev->rfalFieldOnAndStartGT());                                 /* Turns the Field On and starts GT timer */

      err = rfalNfcaPollerTechnologyDetection(gNfcDev.disc.compMode, &sensRes);  /* Poll for NFC-A devices */
      if (err == ERR_NONE) {
        gNfcDev.techsFound |= RFAL_NFC_POLL_TECH_A;
      }

      return ERR_BUSY;
    }

  }


  /*******************************************************************************/
  /* Passive NFC-B Technology Detection                                          */
  /*******************************************************************************/
  if (((gNfcDev.disc.techs2Find & RFAL_NFC_POLL_TECH_B) != 0U) && ((gNfcDev.techs2do & RFAL_NFC_POLL_TECH_B) != 0U)) {
    gNfcDev.techs2do &= ~RFAL_NFC_POLL_TECH_B;

    {
      rfalNfcbSensbRes sensbRes;
      uint8_t          sensbResLen;

      EXIT_ON_ERR(err, rfalNfcbPollerInitialize());                             /* Initialize RFAL for NFC-B */
      EXIT_ON_ERR(err, rfalRfDev->rfalFieldOnAndStartGT());                                /* As field is already On only starts GT timer */

      err = rfalNfcbPollerTechnologyDetection(gNfcDev.disc.compMode, &sensbRes, &sensbResLen);   /* Poll for NFC-B devices */
      if (err == ERR_NONE) {
        gNfcDev.techsFound |= RFAL_NFC_POLL_TECH_B;
      }

      return ERR_BUSY;
    }
  }

  /*******************************************************************************/
  /* Passive NFC-F Technology Detection                                          */
  /*******************************************************************************/
  if (((gNfcDev.disc.techs2Find & RFAL_NFC_POLL_TECH_F) != 0U) && ((gNfcDev.techs2do & RFAL_NFC_POLL_TECH_F) != 0U)) {
    gNfcDev.techs2do &= ~RFAL_NFC_POLL_TECH_F;

    EXIT_ON_ERR(err, rfalNfcfPollerInitialize(gNfcDev.disc.nfcfBR));              /* Initialize RFAL for NFC-F */
    EXIT_ON_ERR(err, rfalRfDev->rfalFieldOnAndStartGT());                                    /* As field is already On only starts GT timer */

    err = rfalNfcfPollerCheckPresence();                                          /* Poll for NFC-F devices */
    if (err == ERR_NONE) {
      gNfcDev.techsFound |= RFAL_NFC_POLL_TECH_F;
    }

    return ERR_BUSY;
  }


  /*******************************************************************************/
  /* Passive NFC-V Technology Detection                                          */
  /*******************************************************************************/
  if (((gNfcDev.disc.techs2Find & RFAL_NFC_POLL_TECH_V) != 0U) && ((gNfcDev.techs2do & RFAL_NFC_POLL_TECH_V) != 0U)) {
    gNfcDev.techs2do &= ~RFAL_NFC_POLL_TECH_V;

    {
      rfalNfcvInventoryRes invRes;

      EXIT_ON_ERR(err, rfalNfcvPollerInitialize());                                 /* Initialize RFAL for NFC-V */
      EXIT_ON_ERR(err, rfalRfDev->rfalFieldOnAndStartGT());                                    /* As field is already On only starts GT timer */

      err = rfalNfcvPollerCheckPresence(&invRes);                                   /* Poll for NFC-V devices */
      if (err == ERR_NONE) {
        gNfcDev.techsFound |= RFAL_NFC_POLL_TECH_V;
      }

      return ERR_BUSY;
    }
  }


  /*******************************************************************************/
  /* Passive Proprietary Technology ST25TB                                       */
  /*******************************************************************************/
  if (((gNfcDev.disc.techs2Find & RFAL_NFC_POLL_TECH_ST25TB) != 0U) && ((gNfcDev.techs2do & RFAL_NFC_POLL_TECH_ST25TB) != 0U)) {
    gNfcDev.techs2do &= ~RFAL_NFC_POLL_TECH_ST25TB;

    EXIT_ON_ERR(err, rfalSt25tbPollerInitialize());                               /* Initialize RFAL for NFC-V */
    EXIT_ON_ERR(err, rfalRfDev->rfalFieldOnAndStartGT());                                    /* As field is already On only starts GT timer */

    err = rfalSt25tbPollerCheckPresence(NULL);                                    /* Poll for ST25TB devices */
    if (err == ERR_NONE) {
      gNfcDev.techsFound |= RFAL_NFC_POLL_TECH_ST25TB;
    }
  }

  return ERR_NONE;
}

/*!
 ******************************************************************************
 * \brief Poller Collision Resolution
 *
 * This method implements the Collision Resolution on all technologies that
 * have been detected before.
 *
 * \return  ERR_NONE         : Operation completed with no error
 * \return  ERR_BUSY         : Operation ongoing
 * \return  ERR_XXXX         : Error occurred
 *
 ******************************************************************************
 */
ReturnCode RfalNfcClass::rfalNfcPollCollResolution(void)
{
  uint8_t    i;
  uint8_t    devCnt;
  ReturnCode err;

  err    = ERR_NONE;
  devCnt = 0;
  i      = 0;

  /* Supress warning when specific RFAL features have been disabled */
  NO_WARNING(err);
  NO_WARNING(devCnt);
  NO_WARNING(i);

  /* Check if device limit has been reached */
  if (gNfcDev.devCnt >= gNfcDev.disc.devLimit) {
    return ERR_NONE;
  }

  /*******************************************************************************/
  /* NFC-A Collision Resolution                                                  */
  /*******************************************************************************/
  if (((gNfcDev.techsFound & RFAL_NFC_POLL_TECH_A) != 0U) && ((gNfcDev.techs2do & RFAL_NFC_POLL_TECH_A) != 0U)) {  /* If a NFC-A device was found/detected, perform Collision Resolution */
    rfalNfcaListenDevice nfcaDevList[RFAL_NFC_MAX_DEVICES];

    gNfcDev.techs2do &= ~RFAL_NFC_POLL_TECH_A;

    EXIT_ON_ERR(err, rfalNfcaPollerInitialize());                                 /* Initialize RFAL for NFC-A */
    EXIT_ON_ERR(err, rfalRfDev->rfalFieldOnAndStartGT());                                    /* Ensure GT again as other technologies have also been polled */

    err = rfalNfcaPollerFullCollisionResolution(gNfcDev.disc.compMode, (gNfcDev.disc.devLimit - gNfcDev.devCnt), nfcaDevList, &devCnt);
    if ((err == ERR_NONE) && (devCnt != 0U)) {
      for (i = 0; i < devCnt; i++) {                                            /* Copy devices found form local Nfca list into global device list */
        gNfcDev.devList[gNfcDev.devCnt].type     = RFAL_NFC_LISTEN_TYPE_NFCA;
        gNfcDev.devList[gNfcDev.devCnt].dev.nfca = nfcaDevList[i];
        gNfcDev.devCnt++;
      }
    }

    return ERR_BUSY;
  }

  /*******************************************************************************/
  /* NFC-B Collision Resolution                                                  */
  /*******************************************************************************/
  if (((gNfcDev.techsFound & RFAL_NFC_POLL_TECH_B) != 0U) && ((gNfcDev.techs2do & RFAL_NFC_POLL_TECH_B) != 0U)) {  /* If a NFC-B device was found/detected, perform Collision Resolution */
    rfalNfcbListenDevice nfcbDevList[RFAL_NFC_MAX_DEVICES];

    gNfcDev.techs2do &= ~RFAL_NFC_POLL_TECH_B;

    EXIT_ON_ERR(err, rfalNfcbPollerInitialize());                                 /* Initialize RFAL for NFC-B */
    EXIT_ON_ERR(err, rfalRfDev->rfalFieldOnAndStartGT());                                    /* Ensure GT again as other technologies have also been polled */

    err = rfalNfcbPollerCollisionResolution(gNfcDev.disc.compMode, (gNfcDev.disc.devLimit - gNfcDev.devCnt), nfcbDevList, &devCnt);
    if ((err == ERR_NONE) && (devCnt != 0U)) {
      for (i = 0; i < devCnt; i++) {                                            /* Copy devices found form local Nfcb list into global device list */
        gNfcDev.devList[gNfcDev.devCnt].type     = RFAL_NFC_LISTEN_TYPE_NFCB;
        gNfcDev.devList[gNfcDev.devCnt].dev.nfcb = nfcbDevList[i];
        gNfcDev.devCnt++;
      }
    }

    return ERR_BUSY;
  }

  /*******************************************************************************/
  /* NFC-F Collision Resolution                                                  */
  /*******************************************************************************/
  if (((gNfcDev.techsFound & RFAL_NFC_POLL_TECH_F) != 0U) && ((gNfcDev.techs2do & RFAL_NFC_POLL_TECH_F) != 0U)) { /* If a NFC-F device was found/detected, perform Collision Resolution */
    rfalNfcfListenDevice nfcfDevList[RFAL_NFC_MAX_DEVICES];

    gNfcDev.techs2do &= ~RFAL_NFC_POLL_TECH_F;

    EXIT_ON_ERR(err, rfalNfcfPollerInitialize(gNfcDev.disc.nfcfBR));              /* Initialize RFAL for NFC-F */
    EXIT_ON_ERR(err, rfalRfDev->rfalFieldOnAndStartGT());                                    /* Ensure GT again as other technologies have also been polled */

    err = rfalNfcfPollerCollisionResolution(gNfcDev.disc.compMode, (gNfcDev.disc.devLimit - gNfcDev.devCnt), nfcfDevList, &devCnt);
    if ((err == ERR_NONE) && (devCnt != 0U)) {
      for (i = 0; i < devCnt; i++) {                                            /* Copy devices found form local Nfcf list into global device list */
        gNfcDev.devList[gNfcDev.devCnt].type     = RFAL_NFC_LISTEN_TYPE_NFCF;
        gNfcDev.devList[gNfcDev.devCnt].dev.nfcf = nfcfDevList[i];
        gNfcDev.devCnt++;
      }
    }

    return ERR_BUSY;
  }

  /*******************************************************************************/
  /* NFC-V Collision Resolution                                                  */
  /*******************************************************************************/
  if (((gNfcDev.techsFound & RFAL_NFC_POLL_TECH_V) != 0U) && ((gNfcDev.techs2do & RFAL_NFC_POLL_TECH_V) != 0U)) { /* If a NFC-V device was found/detected, perform Collision Resolution */
    rfalNfcvListenDevice nfcvDevList[RFAL_NFC_MAX_DEVICES];

    gNfcDev.techs2do &= ~RFAL_NFC_POLL_TECH_V;

    EXIT_ON_ERR(err, rfalNfcvPollerInitialize());                                 /* Initialize RFAL for NFC-V */
    EXIT_ON_ERR(err, rfalRfDev->rfalFieldOnAndStartGT());                                    /* Ensure GT again as other technologies have also been polled */

    err = rfalNfcvPollerCollisionResolution(RFAL_COMPLIANCE_MODE_NFC, (gNfcDev.disc.devLimit - gNfcDev.devCnt), nfcvDevList, &devCnt);
    if ((err == ERR_NONE) && (devCnt != 0U)) {
      for (i = 0; i < devCnt; i++) {                                            /* Copy devices found form local Nfcf list into global device list */
        gNfcDev.devList[gNfcDev.devCnt].type     = RFAL_NFC_LISTEN_TYPE_NFCV;
        gNfcDev.devList[gNfcDev.devCnt].dev.nfcv = nfcvDevList[i];
        gNfcDev.devCnt++;
      }
    }

    return ERR_BUSY;
  }

  /*******************************************************************************/
  /* ST25TB Collision Resolution                                                 */
  /*******************************************************************************/
  if (((gNfcDev.techsFound & RFAL_NFC_POLL_TECH_ST25TB) != 0U) && ((gNfcDev.techs2do & RFAL_NFC_POLL_TECH_ST25TB) != 0U)) { /* If a ST25TB device was found/detected, perform Collision Resolution */
    rfalSt25tbListenDevice st25tbDevList[RFAL_NFC_MAX_DEVICES];

    gNfcDev.techs2do &= ~RFAL_NFC_POLL_TECH_ST25TB;

    rfalSt25tbPollerInitialize();                                                 /* Initialize RFAL for ST25TB */
    EXIT_ON_ERR(err, rfalRfDev->rfalFieldOnAndStartGT());                                    /* Ensure GT again as other technologies have also been polled */

    err = rfalSt25tbPollerCollisionResolution((gNfcDev.disc.devLimit - gNfcDev.devCnt), st25tbDevList, &devCnt);
    if ((err == ERR_NONE) && (devCnt != 0U)) {
      for (i = 0; i < devCnt; i++) {                                            /* Copy devices found form local Nfcf list into global device list */
        gNfcDev.devList[gNfcDev.devCnt].type       = RFAL_NFC_LISTEN_TYPE_ST25TB;
        gNfcDev.devList[gNfcDev.devCnt].dev.st25tb = st25tbDevList[i];
        gNfcDev.devCnt++;
      }
    }

    return ERR_BUSY;
  }

  return ERR_NONE;                                                                  /* All technologies have been performed */
}


/*!
 ******************************************************************************
 * \brief Poller Activation
 *
 * This method Activates a given device according to it's type and
 * protocols supported
 *
 * \param[in]  devIt : device's position on the list to be activated
 *
 * \return  ERR_NONE         : Operation completed with no error
 * \return  ERR_BUSY         : Operation ongoing
 * \return  ERR_XXXX         : Error occurred
 *
 ******************************************************************************
 */
ReturnCode RfalNfcClass::rfalNfcPollActivation(uint8_t devIt)
{
  ReturnCode err;

  err = ERR_NONE;

  /* Supress warning when specific RFAL features have been disabled */
  NO_WARNING(err);

  if (devIt > gNfcDev.devCnt) {
    return ERR_WRONG_STATE;
  }

  switch (gNfcDev.devList[devIt].type) {
    /*******************************************************************************/
    /* AP2P Activation                                                             */
    /*******************************************************************************/
    case RFAL_NFC_LISTEN_TYPE_AP2P:
      /* Activation has already been perfomed (ATR_REQ) */

      gNfcDev.devList[devIt].nfcid     = gNfcDev.devList[devIt].proto.nfcDep.activation.Target.ATR_RES.NFCID3;
      gNfcDev.devList[devIt].nfcidLen  = RFAL_NFCDEP_NFCID3_LEN;
      break;


    /*******************************************************************************/
    /* Passive NFC-A Activation                                                    */
    /*******************************************************************************/
    case RFAL_NFC_LISTEN_TYPE_NFCA:

      rfalNfcaPollerInitialize();
      if (gNfcDev.devList[devIt].dev.nfca.isSleep) {                            /* Check if desired device is in Sleep */
        rfalNfcaSensRes sensRes;
        rfalNfcaSelRes  selRes;

        EXIT_ON_ERR(err, rfalNfcaPollerCheckPresence(RFAL_14443A_SHORTFRAME_CMD_WUPA, &sensRes));     /* Wake up all cards  */
        EXIT_ON_ERR(err, rfalNfcaPollerSelect(gNfcDev.devList[devIt].dev.nfca.nfcId1, gNfcDev.devList[devIt].dev.nfca.nfcId1Len, &selRes));     /* Select specific device  */
      }

      /* Set NFCID */
      gNfcDev.devList[devIt].nfcid    = gNfcDev.devList[devIt].dev.nfca.nfcId1;
      gNfcDev.devList[devIt].nfcidLen = gNfcDev.devList[devIt].dev.nfca.nfcId1Len;

      /*******************************************************************************/
      /* Perform protocol specific activation                                        */
      switch (gNfcDev.devList[devIt].dev.nfca.type) {
        /*******************************************************************************/
        case RFAL_NFCA_T1T:

          /* No further activation needed for T1T (RID already performed) */

          gNfcDev.devList[devIt].nfcid    = gNfcDev.devList[devIt].dev.nfca.ridRes.uid;
          gNfcDev.devList[devIt].nfcidLen = RFAL_T1T_UID_LEN;

          gNfcDev.devList[devIt].rfInterface = RFAL_NFC_INTERFACE_RF;
          break;

        case RFAL_NFCA_T2T:

          /* No further activation needed for a T2T */

          gNfcDev.devList[devIt].rfInterface = RFAL_NFC_INTERFACE_RF;
          break;


        /*******************************************************************************/
        case RFAL_NFCA_T4T:                                                   /* Device supports ISO-DEP */

          /* Perform ISO-DEP (ISO14443-4) activation: RATS and PPS if supported */
          rfalIsoDepInitialize();
          EXIT_ON_ERR(err, rfalIsoDepPollAHandleActivation((rfalIsoDepFSxI)RFAL_ISODEP_FSDI_DEFAULT, RFAL_ISODEP_NO_DID, RFAL_BR_424, &gNfcDev.devList[devIt].proto.isoDep));

          gNfcDev.devList[devIt].rfInterface = RFAL_NFC_INTERFACE_ISODEP;   /* NFC-A T4T device activated */
          break;



        /*******************************************************************************/
        case RFAL_NFCA_T4T_NFCDEP:                                            /* Device supports both T4T and NFC-DEP */
        case RFAL_NFCA_NFCDEP:                                                /* Device supports NFC-DEP */

          /* Perform NFC-DEP (P2P) activation: ATR and PSL if supported */
          EXIT_ON_ERR(err, rfalNfcNfcDepActivate(&gNfcDev.devList[devIt], RFAL_NFCDEP_COMM_PASSIVE, NULL, 0));

          gNfcDev.devList[devIt].nfcid    = gNfcDev.devList[devIt].proto.nfcDep.activation.Target.ATR_RES.NFCID3;
          gNfcDev.devList[devIt].nfcidLen = RFAL_NFCDEP_NFCID3_LEN;

          gNfcDev.devList[devIt].rfInterface = RFAL_NFC_INTERFACE_NFCDEP;   /* NFC-A P2P device activated */
          break;

        /*******************************************************************************/
        default:
          return ERR_WRONG_STATE;
      }
      break;


    /*******************************************************************************/
    /* Passive NFC-B Activation                                                    */
    /*******************************************************************************/
    case RFAL_NFC_LISTEN_TYPE_NFCB:

      rfalNfcbPollerInitialize();
      if (gNfcDev.devList[devIt].dev.nfcb.isSleep) {                            /* Check if desired device is in Sleep */
        rfalNfcbSensbRes sensbRes;
        uint8_t          sensbResLen;

        /* Wake up all cards. SENSB_RES may return collision but the NFCID0 is available to explicitly select NFC-B card via ATTRIB; so error will be ignored here */
        rfalNfcbPollerCheckPresence(RFAL_NFCB_SENS_CMD_ALLB_REQ, RFAL_NFCB_SLOT_NUM_1, &sensbRes, &sensbResLen);
      }

      /* Set NFCID */
      gNfcDev.devList[devIt].nfcid    = gNfcDev.devList[devIt].dev.nfcb.sensbRes.nfcid0;
      gNfcDev.devList[devIt].nfcidLen = RFAL_NFCB_NFCID0_LEN;

      /* Check if device supports  ISO-DEP (ISO14443-4) */
      if ((gNfcDev.devList[devIt].dev.nfcb.sensbRes.protInfo.FsciProType & RFAL_NFCB_SENSB_RES_PROTO_ISO_MASK) != 0U) {
        rfalIsoDepInitialize();
        /* Perform ISO-DEP (ISO14443-4) activation: RATS and PPS if supported    */
        EXIT_ON_ERR(err, rfalIsoDepPollBHandleActivation((rfalIsoDepFSxI)RFAL_ISODEP_FSDI_DEFAULT, RFAL_ISODEP_NO_DID, RFAL_BR_424, 0x00, &gNfcDev.devList[devIt].dev.nfcb, NULL, 0, &gNfcDev.devList[devIt].proto.isoDep));

        gNfcDev.devList[devIt].rfInterface = RFAL_NFC_INTERFACE_ISODEP;       /* NFC-B T4T device activated */
        break;
      }

      gNfcDev.devList[devIt].rfInterface =  RFAL_NFC_INTERFACE_RF;              /* NFC-B device activated     */
      break;


    /*******************************************************************************/
    /* Passive NFC-F Activation                                                    */
    /*******************************************************************************/
    case RFAL_NFC_LISTEN_TYPE_NFCF:

      rfalNfcfPollerInitialize(gNfcDev.disc.nfcfBR);

      if (rfalNfcfIsNfcDepSupported(&gNfcDev.devList[devIt].dev.nfcf)) {
        /* Perform NFC-DEP (P2P) activation: ATR and PSL if supported */
        EXIT_ON_ERR(err, rfalNfcNfcDepActivate(&gNfcDev.devList[devIt], RFAL_NFCDEP_COMM_PASSIVE, NULL, 0));

        /* Set NFCID */
        gNfcDev.devList[devIt].nfcid    = gNfcDev.devList[devIt].proto.nfcDep.activation.Target.ATR_RES.NFCID3;
        gNfcDev.devList[devIt].nfcidLen = RFAL_NFCDEP_NFCID3_LEN;

        gNfcDev.devList[devIt].rfInterface = RFAL_NFC_INTERFACE_NFCDEP;       /* NFC-F P2P device activated */
        break;
      }

      /* Set NFCID */
      gNfcDev.devList[devIt].nfcid    = gNfcDev.devList[devIt].dev.nfcf.sensfRes.NFCID2;
      gNfcDev.devList[devIt].nfcidLen = RFAL_NFCF_NFCID2_LEN;

      gNfcDev.devList[devIt].rfInterface = RFAL_NFC_INTERFACE_RF;               /* NFC-F T3T device activated */
      break;

    /*******************************************************************************/
    /* Passive NFC-V Activation                                                    */
    /*******************************************************************************/
    case RFAL_NFC_LISTEN_TYPE_NFCV:

      rfalNfcvPollerInitialize();

      /* No specific activation needed for a T5T */

      /* Set NFCID */
      gNfcDev.devList[devIt].nfcid    = gNfcDev.devList[devIt].dev.nfcv.InvRes.UID;
      gNfcDev.devList[devIt].nfcidLen = RFAL_NFCV_UID_LEN;

      gNfcDev.devList[devIt].rfInterface = RFAL_NFC_INTERFACE_RF;               /* NFC-V T5T device activated */
      break;


    /*******************************************************************************/
    /* Passive ST25TB Activation                                                   */
    /*******************************************************************************/
    case RFAL_NFC_LISTEN_TYPE_ST25TB:

      rfalSt25tbPollerInitialize();

      /* No specific activation needed for a ST25TB */

      /* Set NFCID */
      gNfcDev.devList[devIt].nfcid    = gNfcDev.devList[devIt].dev.st25tb.UID;
      gNfcDev.devList[devIt].nfcidLen = RFAL_ST25TB_UID_LEN;

      gNfcDev.devList[devIt].rfInterface = RFAL_NFC_INTERFACE_RF;               /* ST25TB device activated */
      break;

    /*******************************************************************************/
    default:
      return ERR_WRONG_STATE;
  }

  gNfcDev.activeDev = &gNfcDev.devList[devIt];                                      /* Assign active device to be used further on */
  return ERR_NONE;
}

/*!
 ******************************************************************************
 * \brief Poller NFC DEP Activate
 *
 * This method performs NFC-DEP Activation
 *
 * \param[in]  device    : device info
 * \param[in]  commMode  : communication mode (Passive/Active)
 * \param[in]  atrReq    : received ATR_REQ
 * \param[in]  atrReqLen : received ATR_REQ size
 *
 * \return  ERR_NONE     : Operation completed with no error
 * \return  ERR_BUSY     : Operation ongoing
 * \return  ERR_XXXX     : Error occurred
 *
 ******************************************************************************
 */
ReturnCode RfalNfcClass::rfalNfcNfcDepActivate(rfalNfcDevice *device, rfalNfcDepCommMode commMode, const uint8_t *atrReq, uint16_t atrReqLen)
{
  rfalNfcDepAtrParam          initParam;

  (void)atrReq;
  (void)atrReqLen;

  /* If we are in Poll mode */
  if (rfalNfcIsRemDevListener(device->type)) {
    /*******************************************************************************/
    /* If Passive F use the NFCID2 retrieved from SENSF                            */
    if (device->type == RFAL_NFC_LISTEN_TYPE_NFCF) {
      initParam.nfcid    = device->dev.nfcf.sensfRes.NFCID2;
      initParam.nfcidLen = RFAL_NFCF_NFCID2_LEN;
    } else {
      initParam.nfcid    = gNfcDev.disc.nfcid3;
      initParam.nfcidLen = RFAL_NFCDEP_NFCID3_LEN;
    }

    initParam.BS        = RFAL_NFCDEP_Bx_NO_HIGH_BR;
    initParam.BR        = RFAL_NFCDEP_Bx_NO_HIGH_BR;
    initParam.DID       = RFAL_NFCDEP_DID_NO;
    initParam.NAD       = RFAL_NFCDEP_NAD_NO;
    initParam.LR        = RFAL_NFCDEP_LR_254;
    initParam.GB        = gNfcDev.disc.GB;
    initParam.GBLen     = gNfcDev.disc.GBLen;
    initParam.commMode  = commMode;
    initParam.operParam = (RFAL_NFCDEP_OPER_FULL_MI_EN | RFAL_NFCDEP_OPER_EMPTY_DEP_DIS | RFAL_NFCDEP_OPER_ATN_EN | RFAL_NFCDEP_OPER_RTOX_REQ_EN);

    rfalNfcDepInitialize();
    /* Perform NFC-DEP (P2P) activation: ATR and PSL if supported */
    return rfalNfcDepInitiatorHandleActivation(&initParam, RFAL_BR_424, &device->proto.nfcDep);
  } else {
    return ERR_INTERNAL;
  }
}


/*!
 ******************************************************************************
 * \brief Poller NFC Deactivate
 *
 * This method Deactivates the device if a deactivation procedure exists
 *
 * \return  ERR_NONE  : Operation completed with no error
 * \return  ERR_BUSY  : Operation ongoing
 * \return  ERR_XXXX  : Error occurred
 *
 ******************************************************************************
 */
ReturnCode RfalNfcClass::rfalNfcDeactivation(void)
{
  /* Check if a device has been activated */
  if (gNfcDev.activeDev != NULL) {
    switch (gNfcDev.activeDev->rfInterface) {
      /*******************************************************************************/
      case RFAL_NFC_INTERFACE_RF:
        break;                                                                /* No specific deactivation to be performed */

      /*******************************************************************************/
      case RFAL_NFC_INTERFACE_ISODEP:
        rfalIsoDepDeselect();                                                 /* Send a Deselect to device */
        break;

      /*******************************************************************************/
      case RFAL_NFC_INTERFACE_NFCDEP:
        rfalNfcDepRLS();                                                      /* Send a Release to device */
        break;

      default:
        return ERR_REQUEST;
    }
  }

  rfalRfDev->rfalWakeUpModeStop();

  rfalRfDev->rfalFieldOff();

  gNfcDev.activeDev = NULL;
  return ERR_NONE;
}


/*!
 *****************************************************************************
 * \brief  Calculate Timer
 *
 * This method calculates when the timer will be expired given the amount
 * time in milliseconds /a tOut.
 * Once the timer has been calculated it will then be used to check when
 * it expires.
 *
 * \see timersIsExpired
 *
 * \param[in]  time : time/duration in Milliseconds for the timer
 *
 * \return u32 : The new timer calculated based on the given time
 *****************************************************************************
 */
uint32_t RfalNfcClass::timerCalculateTimer(uint16_t time)
{
  return (millis() + time);
}


/*!
 *****************************************************************************
 * \brief  Checks if a Timer is Expired
 *
 * This method checks if a timer has already expired.
 * Based on the given timer previously calculated it checks if this timer
 * has already elapsed
 *
 * \see timersCalculateTimer
 *
 * \param[in]  timer : the timer to check
 *
 * \return true  : timer has already expired
 * \return false : timer is still running
 *****************************************************************************
 */
bool RfalNfcClass::timerIsExpired(uint32_t timer)
{
  uint32_t uDiff;
  int32_t sDiff;

  uDiff = (timer - millis());   /* Calculate the diff between the timers */
  sDiff = uDiff;                            /* Convert the diff to a signed var      */
  /* Having done this has two side effects:
   * 1) all differences smaller than -(2^31) ms (~25d) will become positive
   *    Signaling not expired: acceptable!
   * 2) Time roll-over case will be handled correctly: super!
   */

  /* Check if the given timer has expired already */
  if (sDiff < 0) {
    return true;
  }

  return false;
}
