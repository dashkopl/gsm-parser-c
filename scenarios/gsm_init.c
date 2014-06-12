#include "gsm_init.h"
#include <stdio.h>

static uint8_t SKIP_PIN_ENTER = 0;

// returns 3 when need to skip next command!
uint8_t gsm_init_callback(char* answer, uint8_t action){
    // TODO
    #ifdef __DEBUG__
    printf("Called action %d with %s answer\r\n",action,answer);
    #endif // __DEBUG__
    switch(action) {
    case 0:
        if ( (strcmp(answer, "AT\r\r\n")==0) || (strcmp(answer, "\r\n")==0) )
            return 0;
        else
        if (strcmp(answer, "OK\r\n")==0)
            return 1;
        else {
            printf("Failed: %s \r\n",answer);
            return 2;
        }
        break;

    case 1:
        if ( (strcmp(answer, "ATE0\r\r\n")==0) || (strcmp(answer, "\r\n")==0) )
            return 0;
        else
            if (strcmp(answer, "OK\r\n")==0)
            return 1;
        else {
            printf("Failed: %s \r\n",answer);
            return 2;
        }
        break;

    case 2:
        if ( strcmp(answer, "+CPIN: SIM PUK\r\n") == 0 )
            return 2;
        else
        if ( (strcmp(answer, "\r\n")==0) || (strcmp(answer, "+CPIN: SIM PIN\r\n")==0) )
            return 0;
        else
        if (strcmp(answer, "OK\r\n")==0) {
            if (SKIP_PIN_ENTER != 1)
                return 1;
            else
                return 3;
        }
        else
        if ( strcmp(answer, "+CPIN: READY\r\n") == 0 ){  // need to skip next modem command
            SKIP_PIN_ENTER = 1;
            return 0;
        }
        else
            return 2;
        break;

    case 3:     // PIN CODE WAS INPUT
        if (strcmp(answer, "\r\n")==0)
            return 0;
        else
        if (strcmp(answer, "OK\r\n")==0)
            return 1;
        else
        if (strcmp(answer, "ERROR\r\n")==0) // WRONG PIN!!!
            return 2;
        else
            return 2;
        break;

    case 4:
        if ( (strcmp(answer, "\r\n")==0) || ( strcmp(answer, "+CPIN: READY\r\n") == 0 ) )
            return 0;
        else
        if (strcmp(answer, "OK\r\n")==0)
            return 1;
        else
            return 2;
        break;

    default:
        return 2;
        break;
    }
    return 2;
}
void gsm_init_setup(gsm_modem* modem, char pin[2][8], char puk[2][8]){
    // TODO
    //uint8_t i = 0;
    gsm_scenario scene;

    modem->action_queue.head  = 0;
    modem->action_queue.tail  = 0;

    scene.actions[0] = (GSM_ACTION) { EXEC_CMD,  AC_PRESENSE, ""             };
    scene.actions[1] = (GSM_ACTION) { EXEC_CMD,  AC_ECHOOFF,  ""             };
    scene.actions[2] = (GSM_ACTION) { READ_CMD,  AC_PINCODE,  ""             };

    scene.actions[3] = (GSM_ACTION) { WRITE_CMD, AC_PINCODE,  (pin == NULL)? "0000": pin[0] };
    scene.actions[4] = (GSM_ACTION) { READ_CMD,  AC_PINCODE,  ""             };

    scene.actions[5] = (GSM_ACTION) { SCEN_FINISH, 0, "" };

    /*
        Need to add multiple skip option in modem descriptor!
    */
    /*
    // MUST HAVE part
    scene.actions[0] = (GSM_ACTION) { EXEC_CMD,  AC_PRESENSE, ""             };
    scene.actions[1] = (GSM_ACTION) { EXEC_CMD,  AC_ECHOOFF,  ""             };
    scene.actions[2] = (GSM_ACTION) { READ_CMD,  AC_PINCODE,  ""             };
    // part built on skips
    strcpy(str,puk[0]); strcat(str,","); strcat(str,pin[0]);
    scene.actions[3] = (GSM_ACTION) { WRITE_CMD, AC_PINCODE,  (pin == NULL)? "0000": pin[0] };
    scene.actions[4] = (GSM_ACTION) { WRITE_CMD, AC_PINCODE,  (puk == NULL)? "0000": str };  // must be through strcat

    strcpy(str,puk[0]); strcat(str,","); strcat(str,pin[0]);
    scene.actions[5] = (GSM_ACTION) { WRITE_CMD, AC_PIN2CODE,  (pin == NULL)? "0000": pin[1] };
    scene.actions[6] = (GSM_ACTION) { WRITE_CMD, AC_PIN2CODE,  (puk == NULL)? "0000": str };  // must be through strcat

    // again MUST HAVE part
    scene.actions[7] = (GSM_ACTION) { READ_CMD,  AC_PINCODE,  ""             };

    scene.actions[8] = (GSM_ACTION) { SCEN_FINISH, 0, "" };
    */

    scene.callback   = &gsm_init_callback;

    gsm_add_task(modem,&scene);
    return;
}