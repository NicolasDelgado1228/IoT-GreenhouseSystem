/*******************************************************************************
*
*  CoffeeMachine.c -      A program implementing the use case MakeCoffee
*                         for the Coffee Machine model v1
*
*   Notes:                Error checking omitted...
*
*******************************************************************************/

#include    <stdio.h>
#include    <string.h>
#include    <errno.h>
#include    "phtrdsMsgLyr.h"              /* pthreads message layer function
                                              prototypes, constants, structs */

/***( Function prototypes )***********************************************/

static void *pCustomer ( void *arg );     /* Customer code */
static void *pController ( void *arg );   /* Controller code */
static void *pHardware ( void *arg );     /* Hardware code */

/***( SDL system creation )***********************************************/
int main ( void )
{
  pthread_t   customr_tid;                /* Customer tid */
  pthread_t   cntrllr_tid;                /* Controller tid */
  pthread_t   hw_tid;                     /* Hardware tid */

  /* Create queues */
  initialiseQueues ();

  /* Create threads */
  pthread_create ( &customr_tid, NULL, pCustomer, NULL );
  pthread_create ( &hw_tid, NULL, pHardware, NULL );
  pthread_create ( &cntrllr_tid, NULL, pController, NULL );

  /* Wait for threads to finish */
  pthread_join ( customr_tid, NULL );
  pthread_join ( cntrllr_tid, NULL );
  pthread_join ( hw_tid, NULL );

  /* Destroy queues */
  destroyQueues ();

  return ( 0 );
}

/***( SDL system processes )**********************************************/

/* Customer thread */
static void *pCustomer ( void *arg )
{
  char  line [100];                   /* temporary keyboard buffer */
  int   coinValue;                    /* coin value */
  char  cupType;                      /* beverage selection */
  msg_t OutMsg;                       /* output message */

  for ( ; ; )
  {
    printf ( "Enter coin value (5 or 10) " );             /* ask for coin value */
    fflush ( stdout );
    fflush ( stdin );
    fgets ( line, sizeof (line), stdin );
    sscanf ( line, "%d", &coinValue );
    OutMsg.signal = (int) sCoin;                          /* signal name = SCoin */
    OutMsg.value = coinValue;                             /* signal attribute = coinValue */
    sendMessage ( &(queue [CONTROLLER_Q]), OutMsg );      /* send message to Controller queue */

    printf ( "Select (C)offee or (T)ea " );               /* ask for beverage */
    fflush ( stdout );
    fflush ( stdin );
    fgets ( line, sizeof (line), stdin );
    sscanf ( line, "%c", &cupType );
    if ( cupType == 'C' )
      OutMsg.signal = (int) sCoffee;                      /* signal name = sCoffee */
    else
      OutMsg.signal = (int) sTea;                         /* signal name = sTea */
    OutMsg.value = 0;                                     /* signal attribute = don't care */
    sendMessage ( &(queue [CONTROLLER_Q]), OutMsg );      /* send message to Controller queue */
  }

  return ( NULL );
}

/* Controller thread */
static void *pController ( void *arg )
{
  CONTROLLER_STATES state,            /* current Controller EFSM state */
                    state_next;       /* Controller EFSM next state */
  msg_t             InMsg,            /* input message */
                    OutMsg;           /* output message */
  unsigned int      NbrOfCoffeeCups;  /* number of coffee cups */

  NbrOfCoffeeCups = 0;
  state_next = IdleC;

  for ( ; ; )
  {
    state = state_next;
    InMsg = receiveMessage ( &(queue [CONTROLLER_Q]) );
    switch ( state )
    {
      case IdleC:
        switch ( InMsg.signal )
        {
          case sCoin:
            if ( InMsg.value == 10 )
              state_next = PaidTen;
            else
              state_next = IdleC;
            break;
          default:
            break;
        }
        break;
      case PaidTen:
        switch ( InMsg.signal )
        {
          case sCoffee:
            OutMsg.signal = (int) sFillWater;
            OutMsg.value = 0;
            sendMessage ( &(queue [HARDWARE_Q]), OutMsg );  /* send message to Hardware */
            state_next = PouringWater;
            break;
          default:
            break;
        }
        break;
      case PouringWater:
        switch ( InMsg.signal )
        {
          case sWaterOK:
            OutMsg.signal = (int) sFillCoffee;
            OutMsg.value = 0;
            sendMessage ( &(queue [HARDWARE_Q]), OutMsg );  /* send message to Hardware */
            state_next = DispensingCoffee;
            break;
          default:
            break;
        }
        break;
      case DispensingCoffee:
        switch ( InMsg.signal )
        {
          case sCoffeeOK:
            OutMsg.signal = (int) sHeatWater;
            OutMsg.value = 0;
            sendMessage ( &(queue [HARDWARE_Q]), OutMsg );  /* send message to Hardware */
            state_next = BrewingCoffee;
            break;
          default:
            break;
        }
        break;
      case BrewingCoffee:
        switch ( InMsg.signal )
        {
          case sWarm:
            NbrOfCoffeeCups++;
            printf ( "\n\t\t\t\t%d Cup(s) of Coffee served!\n", NbrOfCoffeeCups );
            fflush ( stdout );
            state_next = IdleC;
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }
  }

  return ( NULL );
}

/* Hardware thread */
static void *pHardware ( void *arg )
{
  HARDWARE_STATES state,
                  state_next;
  msg_t           InMsg,
                  OutMsg;

  state_next = IdleH;

  for ( ; ; )
  {
    state = state_next;
    InMsg = receiveMessage ( &(queue [HARDWARE_Q]) );
    switch ( state )
    {
      case IdleH:
        switch ( InMsg.signal )
        {
          case sFillWater:
            OutMsg.signal = (int) sWaterOK;
            break;
          case sFillCoffee:
            OutMsg.signal = (int) sCoffeeOK;
            break;
          case sHeatWater:
            OutMsg.signal = (int) sWarm;
            break;
          default:
            break;
        }
        state_next = IdleH;
        OutMsg.value = 0;
        sendMessage ( &(queue [CONTROLLER_Q]), OutMsg );     /* send message to Controller */
        break;
      default:
        break;
    }
  }

  return ( NULL );
}
