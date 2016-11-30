// Refer to: controller

struct event_handler {
    struct list event_list; // to list event handlers
    struct user_event user_event_lists[MAX_USER_TYPE];
    pthread_cond_t wake_cond; // thread conditions used in loop

};

// This introduce basic cs mode server using event mechanisim.
// The event could be support by myself or by libevent.
int
basic_cs_main()
{
    // Start event task
    start_event_task_thread();

    // Add all event you want into events
    register_modules();
}

static int
start_event_task_thread()
{
    pthread_create(&event->event_thread, event_handler);
}

static void
ctrl_event_handler()
{
}

static int
register_modules()
{
    struct event_data e;
    init_and_add_event(&e);
    register_event_callback(&e);
}
