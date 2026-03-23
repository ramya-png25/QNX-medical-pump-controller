#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <time.h>

// Definitions
#define PUMP_PULSE_CODE _PULSE_CODE_MINAVAIL
#define WATCHDOG_TIMEOUT_MS 1000  // 1 Second timeout
#define PUMP_INTERVAL_MS 500      // Pump runs every 500ms

// Global State
volatile int pump_heartbeat = 0; // The "Counter" the watchdog checks
int pump_active = 1;

// --- Watchdog Thread Function ---
void* watchdog_thread(void* arg) {
    printf("[WATCHDOG] Started. Monitoring Pump...\n");
    int last_heartbeat = 0;

    while (pump_active) {
        delay(WATCHDOG_TIMEOUT_MS); // QNX specific delay

        if (pump_heartbeat == last_heartbeat) {
            printf("\n[!!! ALERT !!!] WATCHDOG TRIGGERED: Pump Thread Hang Detected!\n");
            printf("[SAFE STATE] Stopping fluid delivery. System Halted.\n");
            pump_active = 0; // Enter Safe State
            exit(EXIT_FAILURE);
        }

        last_heartbeat = pump_heartbeat;
        printf("[WATCHDOG] Check passed. Pump is healthy.\n");
    }
    return NULL;
}

// --- Main Pump Controller ---
int main() {
    struct sigevent event;
    struct itimerspec itime;
    timer_t timer_id;
    int chid, rcvid;
    struct _pulse pulse;
    pthread_t watchdog_tid;

    // 1. Create a Channel for communication
    chid = ChannelCreate(0);
    if (chid == -1) {
        perror("ChannelCreate failed");
        return EXIT_FAILURE;
    }

    // 2. Start the Watchdog Thread
    pthread_create(&watchdog_tid, NULL, watchdog_thread, NULL);

    // 3. Setup the Pulse Event for the Timer
    event.sigev_notify = SIGEV_PULSE;
    event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, chid, _NTO_SIDE_CHANNEL, 0);
    event.sigev_priority = 10; // High Priority for Medical safety
    event.sigev_code = PUMP_PULSE_CODE;

    // 4. Create and Start the Timer
    timer_create(CLOCK_REALTIME, &event, &timer_id);

    itime.it_value.tv_sec = 0;
    itime.it_value.tv_nsec = PUMP_INTERVAL_MS * 1000000L;
    itime.it_interval.tv_sec = 0;
    itime.it_interval.tv_nsec = PUMP_INTERVAL_MS * 1000000L;
    timer_settime(timer_id, 0, &itime, NULL);

    printf("[PUMP] Controller running at %dms intervals.\n", PUMP_INTERVAL_MS);

    int cycle_count = 0;
    while (pump_active) {
        // Wait for the Timer Pulse
        rcvid = MsgReceive(chid, &pulse, sizeof(pulse), NULL);

        if (rcvid == 0 && pulse.code == PUMP_PULSE_CODE) {
            cycle_count++;

            // SIMULATING A HANG: After 10 cycles, we freeze the pump
            if (cycle_count == 10) {
                printf("\n[DEBUG] Simulating a software freeze...\n");
                while(1); // Infinite loop to trigger watchdog
            }

            // Normal Operation
            printf("[PUMP] Delivered 0.1mg dose. Cycle: %d\n", cycle_count);
            pump_heartbeat++; // "Kick" the watchdog
        }
    }

    return 0;
}
