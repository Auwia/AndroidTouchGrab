/*
 * AndroidTouchGrab
 *
 * Temporarily blocks physical input events from an Android/Linux input
 * device by acquiring an exclusive EVIOCGRAB lock.
 *
 * ADB-generated input remains available because it does not depend on
 * physical touchscreen events from the grabbed input device.
 *
 * License: MIT
 */

#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define PROGRAM_NAME    "AndroidTouchGrab"
#define PROGRAM_VERSION "1.0.0"
#define DEFAULT_DEVICE  "/dev/input/event2"

static int input_fd = -1;
static volatile sig_atomic_t stop_requested = 0;

/*
 * Signal handler.
 *
 * Only set a flag here. Cleanup and output are handled safely
 * in the normal program flow.
 */
static void handle_signal(int signal_number)
{
    (void)signal_number;
    stop_requested = 1;
}

/*
 * Release the EVIOCGRAB lock and close the input device.
 */
static void release_device(void)
{
    if (input_fd < 0)
        return;

    if (ioctl(input_fd, EVIOCGRAB, 0) < 0)
        fprintf(stderr, "Warning: failed to release EVIOCGRAB: %s\n",
                strerror(errno));

    close(input_fd);
    input_fd = -1;
}

/*
 * Print command-line usage information.
 */
static void print_help(const char *program)
{
    printf(
        "%s %s\n"
        "\n"
        "Temporarily block physical input events from an Android/Linux\n"
        "input device while keeping ADB input control available.\n"
        "\n"
        "Usage:\n"
        "  %s [DEVICE]\n"
        "  %s --help\n"
        "  %s --version\n"
        "\n"
        "Arguments:\n"
        "  DEVICE       Input device to grab.\n"
        "               Default: %s\n"
        "\n"
        "Options:\n"
        "  -h, --help       Show this help message.\n"
        "  -v, --version    Show program version.\n"
        "\n"
        "Examples:\n"
        "  %s\n"
        "  %s /dev/input/event2\n"
        "\n"
        "Press Ctrl+C to release the device and exit.\n",
        PROGRAM_NAME,
        PROGRAM_VERSION,
        program,
        program,
        program,
        DEFAULT_DEVICE,
        program,
        program
    );
}

int main(int argc, char **argv)
{
    const char *device = DEFAULT_DEVICE;
    struct sigaction action;

    if (argc > 2) {
        fprintf(stderr, "Error: too many arguments.\n\n");
        print_help(argv[0]);
        return EXIT_FAILURE;
    }

    if (argc == 2) {
        if (strcmp(argv[1], "-h") == 0 ||
            strcmp(argv[1], "--help") == 0) {
            print_help(argv[0]);
            return EXIT_SUCCESS;
        }

        if (strcmp(argv[1], "-v") == 0 ||
            strcmp(argv[1], "--version") == 0) {
            printf("%s %s\n", PROGRAM_NAME, PROGRAM_VERSION);
            return EXIT_SUCCESS;
        }

        device = argv[1];
    }

    memset(&action, 0, sizeof(action));
    action.sa_handler = handle_signal;
    sigemptyset(&action.sa_mask);

    if (sigaction(SIGINT, &action, NULL) < 0 ||
        sigaction(SIGTERM, &action, NULL) < 0) {
        fprintf(stderr, "Error: failed to install signal handlers: %s\n",
                strerror(errno));
        return EXIT_FAILURE;
    }

    input_fd = open(device, O_RDONLY);

    if (input_fd < 0) {
        fprintf(stderr,
                "Error: cannot open input device '%s': %s\n",
                device,
                strerror(errno));
        return EXIT_FAILURE;
    }

    if (ioctl(input_fd, EVIOCGRAB, 1) < 0) {
        fprintf(stderr,
                "Error: cannot grab input device '%s': %s\n",
                device,
                strerror(errno));
        close(input_fd);
        input_fd = -1;
        return EXIT_FAILURE;
    }

    printf("%s %s\n", PROGRAM_NAME, PROGRAM_VERSION);
    printf("Input device grabbed: %s\n", device);
    printf("Physical input events from this device are now blocked.\n");
    printf("ADB input control remains available.\n");
    printf("Press Ctrl+C to release the device.\n");
    fflush(stdout);

    while (!stop_requested)
        pause();

    printf("\nReleasing input device...\n");

    release_device();

    printf("Input device released.\n");

    return EXIT_SUCCESS;
}
