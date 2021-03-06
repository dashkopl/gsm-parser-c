#include "gsm_init.h"
#include <stdio.h>

static uint8_t SKIP_PIN_ENTER = 0;

// returns 3 when need to skip next command!
uint8_t gsm_init_callback(char* answer, uint8_t action){
    // TODO
//    #ifdef __DEBUG__
//    printf("Called action %d with %s answer\r\n",action,answer);
//    #endif // __DEBUG__
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
                return (8<<2)+0xC3; // skip 8 commands
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
            return 0;
        else
        if (strcmp(answer, "Call Ready\r\n")==0)
            return 1;
        else
        if (strcmp(answer, "ERROR\r\n")==0) // WRONG PIN!!!
            // here must be routine to manage wrong pin
            return 2;
        else
            return 2;
        break;

    case 4:     // CHECK PIN INPUT ANSWER
        if ( strcmp(answer, "+CPIN: SIM PUK\r\n") == 0 )
            return 2;   // temporary
        else
        if ( strcmp(answer, "+CPIN: SIM PIN\r\n")==0 )
            return 2;
        else
        if (strcmp(answer, "\r\n")==0)
            return 0;
        else
        if (strcmp(answer, "OK\r\n")==0) {
            if (SKIP_PIN_ENTER != 1)
                return 1;
            else
                return (6<<2)+0xC3; // skip 6 commands
        }
        else
        if ( strcmp(answer, "+CPIN: READY\r\n") == 0 ){  // need to skip next modem command
            SKIP_PIN_ENTER = 1;
            return 0;
        }
        else
            return 2;
        break;
    case 5:     // NEW PIN CODE WAS INPUTED WITH PUK
        // MUST BE CHECKED ON SIM CARD!
        // SIM answer: \r\nOK\r\n
//        if (strcmp(answer, "\r\n")==0)
//            return 0;
//        else
//        if (strcmp(answer, "OK\r\n")==0)
//            return 1;
//        else
//        if (strcmp(answer, "ERROR\r\n")==0) // WRONG PUK!!!
//            // here must be routine to manage wrong pin
//            return 2;
//        else
//            return 2;
        break;
    case 6:     // CHECK NEW PIN ANSWER
        // must be +CPIN: READY
        break;
    case 7:     // PIN2 CODE WAS INPUT
        if ( (strcmp(answer, "\r\n")==0) || ( strcmp(answer, "+CPIN: READY\r\n") == 0 ) )
            return 0;
        else
        if (strcmp(answer, "OK\r\n")==0)
            return 1;
        else
            return 2;
        break;
    case 8:     // PIN2 CHECK

        break;
    case 9:     // NEW PIN2 CODE WAS INPUTED WITH PUK

        break;
    case 10:    // NEW PIN2 CHECK

        break;
    default:
        return 2;
        break;
    }
    return 2;
}

void gsm_init_setup(gsm_modem* modem, uint32_t pin[2], uint32_t puk[2]){
    gsm_scenario scene;

    modem->action_queue.head  = 0;
    modem->action_queue.tail  = 0;

    // MUST HAVE part
    scene.actions[0] = (GSM_ACTION) { EXEC_CMD,  AC_PRESENSE, "" };
    scene.actions[1] = (GSM_ACTION) { EXEC_CMD,  AC_ECHOOFF,  "" };
    scene.actions[2] = (GSM_ACTION) { READ_CMD,  AC_PINCODE,  "" };
    // part built on skips
    scene.actions[3] = (GSM_ACTION) { WRITE_CMD, AC_PINCODE,  "" };

    if (pin[0] == 0)
        strcpy(scene.actions[3].pParams, "0000");
    else
        sprintf(scene.actions[3].pParams, "%u", pin[0]);

    scene.actions[4] = (GSM_ACTION) { READ_CMD,  AC_PINCODE,  "" };
    scene.actions[5] = (GSM_ACTION) { WRITE_CMD, AC_PINCODE,  "" };  // must be through strcat

    if (puk[0] == 0)
        strcpy(scene.actions[5].pParams, "0000, 0000");
    else
        sprintf(scene.actions[5].pParams, "%u, %u", puk[0], pin[0]);

    scene.actions[6] = (GSM_ACTION) { READ_CMD,  AC_PINCODE,  "" };
    // here enter pin AGAIN!

    scene.actions[7] = (GSM_ACTION) { WRITE_CMD, AC_PIN2CODE,  "" };

    if (pin[1] == 0)
        strcpy(scene.actions[7].pParams, "0000");
    else
        sprintf(scene.actions[7].pParams, "%u", pin[1]);

    scene.actions[8] = (GSM_ACTION) { READ_CMD,  AC_PINCODE,  "" };
    scene.actions[9] = (GSM_ACTION) { WRITE_CMD, AC_PIN2CODE, "" };  // must be through strcat

    if (puk[1] == 0)
        strcpy(scene.actions[9].pParams, "0000");
    else
        sprintf(scene.actions[7].pParams, "%u, %u", puk[1], pin[1]);

    scene.actions[10]= (GSM_ACTION) { READ_CMD,  AC_PINCODE,  "" };
    // enter pin2 AGAIN!

    // SIM900 after \r\nOK\r\n transmits "Call Ready\r\n"

    // MUST HAVE part
    scene.actions[11] = (GSM_ACTION) { SCEN_FINISH, 0, "" };

    scene.callback   = &gsm_init_callback;

    gsm_add_task(modem,&scene);
    return;
}
