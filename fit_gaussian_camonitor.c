#include <cadef.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <epicsStdlib.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <tool_lib.h>
#include <epicsTime.h>
#include "fit_tools.h"

// TODO: put in header
#define TIMETEXTLEN 28 /* copied from tool_lib.c wasn't in header */
#define VALID_DOUBLE_DIGITS 6
#define TIME_TO_WAIT 0.5
#define BUFFER_SIZE 128
#define CHANNEL "BL31:image1:ArrayData"

static unsigned long reqElems = 0;
static unsigned long eventMask = DBE_VALUE;
static int nConn;
// TODO: put in header
char TimeFormatStr[30] = "%Y-%m-%d %H:%M:%S.%06f"; /* Time format string   */

gsl_vector *data_sq[DIM];
gsl_vector *mux;
gsl_vector *muy;
FinalPos *fpx;
FinalPos *fpy;
Data *fit_data_x;
Data *fit_data_y;

static void event_handler (evargs args)
{
    if (args.status == ECA_NORMAL)
    {
        printf("testing\n");
        /* ptr to value given a pointer to the structure and the DBR type */
        //dbr_char_t *v = dbr_value_ptr(args.dbr, args.type);
        dbr_ushort_t *v = (void*)args.dbr;

        // create gsl vector from waveform
        for (int i=0; i < DIM; i++) {
            for (int j=0; j < DIM; j++) {
                 gsl_vector_set(data_sq[i], j, (double)v[j]);
            }
        }

        average_dim(data_sq, mux, 0);
        fit(mux, fpx, fit_data_x, 0);

        average_dim(data_sq, muy, 1);
        fit(muy, fpy, fit_data_y, 1);

        char timeText[2 * TIMETEXTLEN + 2];
        epicsTimeStamp ts = ((struct dbr_time_short*)v)->stamp;
        epicsTimeToStrftime(timeText, TIMETEXTLEN, "%Y-%m-%d %H:%M:%S.%06f", &ts);
        printf("%s\n", timeText);
    }
}

static void connection_handler ( struct connection_handler_args args )
{
    pv *ppv = ( pv * ) ca_puser ( args.chid );
    if ( args.op == CA_OP_CONN_UP ) {
        nConn++;
        if (!ppv->onceConnected) {
            ppv->onceConnected = 1;
            ppv->dbfType = ca_field_type(ppv->chid);
            ppv->dbrType = dbf_type_to_DBR_TIME(ppv->dbfType); /* Use native type */
            ppv->status = ca_create_subscription(ppv->dbrType,
                                                ppv->reqElems,
                                                ppv->chid,
                                                eventMask,
                                                event_handler,
                                                (void*)ppv,
                                                NULL);
        }
    }
    else if ( args.op == CA_OP_CONN_DOWN ) {
        nConn--;
        ppv->status = ECA_DISCONN;
    }
}

void call_camonitor() {
    caTimeout = DEFAULT_TIMEOUT;
    caPriority = DEFAULT_CA_PRIORITY;
    int result, returncode = 0;

    /* Allocate PV structure array */
    pv* pvs = malloc(sizeof(pv));
    if (!pvs)
    {
        fprintf(stderr, "Memory allocation for channel structures failed.\n");
        exit(-1);
    }

    pvs->name = CHANNEL;

    /* Start up Channel Access */
    result = ca_context_create(ca_enable_preemptive_callback);
    if (result != ECA_NORMAL) {
        fprintf(stderr, "CA error %s occurred while trying to start"
               " channel access.\n", ca_message(result));
        exit(-1);
    }

    /* create CA virtual circuits */
    returncode = create_pvs(pvs, 1, connection_handler);
    if ( returncode ) {
        fprintf(stderr, "create pvs failed.");
        exit(-1);
    }
    /* Check if the channel didn't connect */
    ca_pend_event(caTimeout);
    if (!pvs[0].onceConnected) {
        print_time_val_sts(&pvs[0], reqElems);
        fprintf(stderr, "Camera data channel not found.");
        exit(-1);
    }

    /* Read and print data forever */
    ca_pend_event(0);
    free(pvs);
    return;
}

int main () {
    for (int i=0; i <= DIM; i++) {
        data_sq[i] = gsl_vector_alloc(DIM);
    }

    mux = gsl_vector_alloc(DIM);
    muy = gsl_vector_alloc(DIM);

    fpx = malloc(sizeof(FinalPos));
    fit_data_x = malloc(sizeof(Data));
    fit_data_x->t = malloc(DIM * sizeof(double));
    fit_data_x->y = malloc(DIM * sizeof(double));
    fit_data_x->n = DIM;

    fpy = malloc(sizeof(FinalPos));
    fit_data_y = malloc(sizeof(Data));
    fit_data_y->t = malloc(DIM * sizeof(double));
    fit_data_y->y = malloc(DIM * sizeof(double));
    fit_data_y->n = DIM;

    call_camonitor();

    // clean up
    for (int i=0; i < DIM; i++) {
        gsl_vector_free(data_sq[i]);
    }

    gsl_vector_free(mux);
    gsl_vector_free(muy);
    free(fpx);
    free(fpy);
    free(fit_data_x);
    free(fit_data_y);
    return 0;
}
