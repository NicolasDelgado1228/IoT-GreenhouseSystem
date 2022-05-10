/*******************************************************************************
*
*  CoffeeMachineTrc.c -   A trace-enabled program implementing the use case
*                         MakeCoffee for the Coffee Machine model v1
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

static void *pCustomer ( void *arg );
static void *pController ( void *arg );
static void *pHardware ( void *arg );

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
  pthread_join ( hw_tid, NULL );
  pthread_join ( cntrllr_tid, NULL );

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
    printf ( "Enter coin value (5 or 10)...\n" );         /* ask for coin value */
    fflush ( stdout );
    fflush ( stdin );
    fgets ( line, sizeof (line), stdin );
    sscanf ( line, "%d", &coinValue );
    OutMsg.signal = (int) sCoin;                          /* signal name = SCoin */
    OutMsg.value = coinValue;                             /* signal attribute = coinValue */
    sendMessage ( &(queue [CONTROLLER_Q]), OutMsg );      /* send message to Controller queue */

    printf ( "Select (C)offee or (T)ea...\n" );           /* ask for beverage */
    fflush ( stdout );
    fflush ( stdin );
    fgets ( line, sizeof (line), stdin );
    sscanf ( line, "%c", &cupType );
    if ( cupType == 'C' )                                 /* signal name = sCoffee */
      OutMsg.signal = (int) sCoffee;
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
  CONTROLLER_STATE_ENUM state,                            /* current Controller EFSM state */
                        state_next;                       /* Controller EFSM next state */
  msg_t                 InMsg,                            /* input message */
                        OutMsg;                           /* output message */
  unsigned int          NbrOfCoffeeCups;                  /* number of coffee cups */

  NbrOfCoffeeCups = 0;
  state_next = IdleC;
  for ( ; ; )
  {
    state = state_next;
    InMsg = receiveMessage ( &(queue [CONTROLLER_Q]) );

    /* show which message (signal name and value) was received in current state */
    printf ( "\tController received signal [%s], value [%d] in state [%s]\n", TO_CONTROLLER_STRING[InMsg.signal], InMsg.value, CONTROLLER_STATE_STRING[state] );
    fflush ( stdout );

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
            sendMessage ( &(queue [HARDWARE_Q]), OutMsg ); /* send message to Hardware */
            state_next = PouringWater;
            break;
          default:
            break;
        }
        break;
      case PouringWater:
        switch ( InMsg.signal )
        {
          case sWaterOk:
            OutMsg.signal = (int) sFillCoffee;
            OutMsg.value = 0;
            sendMessage ( &(queue [HARDWARE_Q]), OutMsg ); /* send message to Hardware */
            state_next = DispensingCoffee;
            break;
          default:
            break;
        }
        break;
      case DispensingCoffee:
        switch ( InMsg.signal )
        {
          case sCoffeeOk:
            OutMsg.signal = (int) sHeatWater;
            OutMsg.value = 0;
            sendMessage ( &(queue [HARDWARE_Q]), OutMsg ); /* send message to Hardware */
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
            printf ( "\n\t\t\t\t\t\t%d Cup(s) of Coffee served!\n", NbrOfCoffeeCups );
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
    /* show next state */
    printf ( "\tController next state is [%s]\n", CONTROLLER_STATE_STRING[state_next] );
    fflush ( stdout );
  }

  return ( NULL );
}

/* Hardware thread */
static void *pHardware ( void *arg )
{
  HARDWARE_STATE_ENUM state,
                      state_next;
  msg_t               InMsg,
                      OutMsg;

  state_next = IdleH;
  for ( ; ; )
  {
    state = state_next;
    InMsg = receiveMessage ( &(queue [HARDWARE_Q]) );

    /* show which message (signal name and value) was received in current state */
    printf ( "\t\t\t\t\t\t\t\tHardware received signal [%s], value [%d] in state [%s]\n", TO_HARDWARE_STRING[InMsg.signal], InMsg.value, HARDWARE_STATE_STRING[state] );
    fflush ( stdout );

    switch ( state )
    {
      case IdleH:
        switch ( InMsg.signal )
        {
          case sFillWater:
            OutMsg.signal = (int) sWaterOk;
            break;
          case sFillCoffee:
            OutMsg.signal = (int) sCoffeeOk;
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
