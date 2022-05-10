/*******************************************************************************
*
*  phtrdsMsgLyr.c - Service routines for concurrent access to a circular buffer
*                   modelling a message queue
*
*   Notes:          Error checking omitted...
*
*******************************************************************************/

#include    "phtrdsMsgLyr.h"

/***( Global variables )**************************************************/

msgq_t      queue [NUM_QUEUES];             /* declare queue as an array of
                                                message queues */

/***( Support functions )*************************************************/

/* put message msg into queue (circular buffer) pointed to by queue_ptr */
static void PutMsg ( msgq_t *queue_ptr, msg_t msg )
{
  pthread_mutex_lock ( &(queue_ptr->buffer_lock) );
  queue_ptr->buffer [queue_ptr->bufin] = msg;
  queue_ptr->bufin = ( queue_ptr->bufin + 1 ) % BUFSIZE;
  pthread_mutex_unlock ( &(queue_ptr->buffer_lock) );
}

/* return message msg from queue (circular buffer) pointed to by queue_ptr */
static msg_t GetMsg ( msgq_t *queue_ptr )
{
  msg_t msg;

  pthread_mutex_lock ( &(queue_ptr->buffer_lock) );
  msg = queue_ptr->buffer [queue_ptr->bufout];
  queue_ptr->bufout = ( queue_ptr->bufout + 1 ) % BUFSIZE;
  pthread_mutex_unlock ( &(queue_ptr->buffer_lock) );

  return ( msg );
}

/* initialise array of message queues  */
void initialiseQueues ( void )
{
  int i;

  for ( i = 0; i < NUM_QUEUES; i++ )
  {
    queue [i].bufin = 0;
    queue [i].bufout = 0;
    pthread_mutex_init ( &(queue [i].buffer_lock), NULL );
    /* queue [i].buffer_lock = PTHREAD_MUTEX_INITIALIZER; */
    /* Create semaphores */
    sem_init ( &(queue [i].items), LOCAL, 0 );          /* There are no messages */
    sem_init ( &(queue [i].slots), LOCAL, BUFSIZE );    /* There are BUFSIZE slots */
  }
}

/* destroy array of message queues  */
void destroyQueues ( void )
{
  int i;

  for ( i = 0; i < NUM_QUEUES; i++ )
  {
    /* Destroy mutex */
    pthread_mutex_destroy ( &(queue [i].buffer_lock) );
    /* Destroy semaphores */
    sem_destroy ( &(queue [i].items) );
    sem_destroy ( &(queue [i].slots) );
  }
}

/* emulate SDL Output operation */
void sendMessage ( msgq_t *queue_ptr, msg_t msg )
{
  sem_wait ( &(queue_ptr->slots) );
  PutMsg ( queue_ptr, msg );
  sem_post ( &(queue_ptr->items) );
}

/* emulate SDL Input operation */
msg_t receiveMessage ( msgq_t *queue_ptr )
{
  msg_t msg;

  sem_wait ( &(queue_ptr->items) );
  msg = GetMsg ( queue_ptr );
  sem_post ( &(queue_ptr->slots) );

  return ( msg );
}
