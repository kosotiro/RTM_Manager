static volatile sig_atomic_t keeprunning = 1; /* controlling server process state (running / terminated) */

int client_handles[MAX_CLIENTS_SUPPORTED+1];
