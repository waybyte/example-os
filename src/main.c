/**
 * @file main.c
 * @brief Example program to demonstrate use of Logicrom OS APIs
 * @version 0.1
 * @date 2022-03-03
 * 
 */

#include <stdio.h>
#include <unistd.h>

#include <lib.h>
#include <ril.h>
#include <os_api.h>

#define MSG_ID_READY 1

static uint32_t task1_id, task2_id;
static uint32_t task_sem;

/**
 * URC Handler
 * @param param1	URC Code
 * @param param2	URC Parameter
 */
static void urc_callback(unsigned int param1, unsigned int param2)
{
	switch (param1) {
	case URC_SYS_INIT_STATE_IND:
		if (param2 == SYS_STATE_SMSOK) {
			/* Ready for SMS */
		}
		break;
	case URC_SIM_CARD_STATE_IND:
		switch (param2) {
		case SIM_STAT_NOT_INSERTED:
			debug(DBG_OFF, "SYSTEM: SIM card not inserted!\n");
			break;
		case SIM_STAT_READY:
			debug(DBG_INFO, "SYSTEM: SIM card Ready!\n");
			break;
		case SIM_STAT_PIN_REQ:
			debug(DBG_OFF, "SYSTEM: SIM PIN required!\n");
			break;
		case SIM_STAT_PUK_REQ:
			debug(DBG_OFF, "SYSTEM: SIM PUK required!\n");
			break;
		case SIM_STAT_NOT_READY:
			debug(DBG_OFF, "SYSTEM: SIM card not recognized!\n");
			break;
		default:
			debug(DBG_OFF, "SYSTEM: SIM ERROR: %d\n", param2);
		}
		break;
	case URC_GSM_NW_STATE_IND:
		debug(DBG_OFF, "SYSTEM: GSM NW State: %d\n", param2);
		break;
	case URC_GPRS_NW_STATE_IND:
		break;
	case URC_CFUN_STATE_IND:
		break;
	case URC_COMING_CALL_IND:
		debug(DBG_OFF, "Incoming voice call from: %s\n", ((struct ril_callinfo_t *)param2)->number);
		/* Take action here, Answer/Hang-up */
		break;
	case URC_CALL_STATE_IND:
		switch (param2) {
		case CALL_STATE_BUSY:
			debug(DBG_OFF, "The number you dialed is busy now\n");
			break;
		case CALL_STATE_NO_ANSWER:
			debug(DBG_OFF, "The number you dialed has no answer\n");
			break;
		case CALL_STATE_NO_CARRIER:
			debug(DBG_OFF, "The number you dialed cannot reach\n");
			break;
		case CALL_STATE_NO_DIALTONE:
			debug(DBG_OFF, "No Dial tone\n");
			break;
		default:
			break;
		}
		break;
	case URC_NEW_SMS_IND:
		debug(DBG_OFF, "SMS: New SMS (%d)\n", param2);
		/* Handle New SMS */
		break;
	case URC_MODULE_VOLTAGE_IND:
		debug(DBG_INFO, "VBatt Voltage: %d\n", param2);
		break;
	case URC_ALARM_RING_IND:
		break;
	case URC_FILE_DOWNLOAD_STATUS:
		break;
	case URC_FOTA_STARTED:
		break;
	case URC_FOTA_FINISHED:
		break;
	case URC_FOTA_FAILED:
		break;
	case URC_STKPCI_RSP_IND:
		break;
	default:
		break;
	}
}

/**
 * Task 1
 * @param arg	Task Argument
 */
static void task1_function(void *arg)
{
	printf("Task 1 Running!\n");
	printf("Notify task 2\n");
	/* sending message to task 2 */
	os_message_send(task2_id, MSG_ID_READY, 0, 0);
	/* Wait for semaphore */
	printf("Waiting for semaphore\n");
	os_semaphore_take(task_sem, OS_WAIT_FOREVER);
	printf("Semaphore taken, exiting\n");
#ifdef SOC_RDA8910
	/* task delete is only available on 4G modules */
	/* exit */
	os_task_delete(task1_id);
#endif
}

/**
 * Task 2
 * @param arg	Task Argument
 */
static void task2_function(void *arg)
{
	struct osmsg_t msg;

	os_task_sleep(100);
	printf("Task 2 delayed start!\n");

	/* wait for message from task */
	os_message_get(&msg);

	if (msg.message == MSG_ID_READY) {
		printf("Received ready message from Task 1\n");
		os_task_sleep(100);
		printf("Task 2 sem give\n");
		os_semaphore_give(task_sem);
	}

	printf("Task 2 exit\n");
}

/**
 * Task 3
 * @param arg	Task Argument
 */
static void task3_function(void *arg)
{
	printf("Task 3 running!");

	for (int i = 0; i < 10; i++) {
		printf("Hello World %d\n", i);
		sleep(1);
	}
}

/**
 * Application main entry point
 */
int main(int argc, char *argv[])
{
	/*
	 * Initialize library and Setup STDIO
	 */
	logicrom_init("/dev/ttyS0", urc_callback);

	printf("System Ready\n");

	task_sem = os_semaphore_create(0);

	/* Create Application tasks */
	task1_id = os_task_create(task1_function, NULL, FALSE);
	task2_id = os_task_create(task2_function, NULL, FALSE);
	os_task_create(task3_function, NULL, FALSE);

	printf("System Initialization finished\n");

	while (1) {
		/* Main task */
		sleep(1);
	}
}

