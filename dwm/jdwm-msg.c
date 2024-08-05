#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <yajl/yajl_gen.h>
#include <argp.h>

#define IPC_MAGIC "JDWM-IPC"
// clang-format off
#define IPC_MAGIC_ARR { 'J', 'D', 'W', 'M', '-', 'I', 'P', 'C' }
// clang-format on
#define IPC_MAGIC_LEN 8 // Not including null char

#define IPC_EVENT_TAG_CHANGE "tag_change_event"
#define IPC_EVENT_CLIENT_FOCUS_CHANGE "client_focus_change_event"
#define IPC_EVENT_LAYOUT_CHANGE "layout_change_event"
#define IPC_EVENT_MONITOR_FOCUS_CHANGE "monitor_focus_change_event"
#define IPC_EVENT_FOCUSED_TITLE_CHANGE "focused_title_change_event"
#define IPC_EVENT_FOCUSED_STATE_CHANGE "focused_state_change_event"

#define YSTR(str) yajl_gen_string(gen, (unsigned char *)str, strlen(str))
#define YINT(num) yajl_gen_integer(gen, num)
#define YDOUBLE(num) yajl_gen_double(gen, num)
#define YBOOL(v) yajl_gen_bool(gen, v)
#define YNULL() yajl_gen_null(gen)
#define YARR(body)                         \
	{                                  \
		yajl_gen_array_open(gen);  \
		body;                      \
		yajl_gen_array_close(gen); \
	}
#define YMAP(body)                       \
	{                                \
		yajl_gen_map_open(gen);  \
		body;                    \
		yajl_gen_map_close(gen); \
	}

typedef unsigned long Window;

const char *DEFAULT_SOCKET_PATH = "/tmp/jdwm.sock";
static int sock_fd = -1;
static unsigned int ignore_reply = 0;

typedef enum IPCMessageType {
	IPC_TYPE_RUN_COMMAND = 0,
	IPC_TYPE_GET_MONITORS = 1,
	IPC_TYPE_GET_TAGS = 2,
	IPC_TYPE_GET_LAYOUTS = 3,
	IPC_TYPE_GET_DWM_CLIENT = 4,
	IPC_TYPE_SUBSCRIBE = 5,
	IPC_TYPE_EVENT = 6
} IPCMessageType;

// Every IPC message must begin with this
typedef struct jdwm_ipc_header {
	uint8_t magic[IPC_MAGIC_LEN];
	uint32_t size;
	uint8_t type;
} __attribute((packed)) jdwm_ipc_header_t;

struct arguments {
	char *args[100];
};

static int recv_message(uint8_t *msg_type, uint32_t *reply_size, uint8_t **reply)
{
	uint32_t read_bytes = 0;
	const int32_t to_read = sizeof(jdwm_ipc_header_t);
	char header[to_read];
	char *walk = header;

	// Try to read header
	while (read_bytes < to_read) {
		ssize_t n = read(sock_fd, header + read_bytes, to_read - read_bytes);

		if (n == 0) {
			if (read_bytes == 0) {
				fprintf(stderr, "Unexpectedly reached EOF while reading header.");
				fprintf(stderr, "Read %" PRIu32 " bytes, expected %" PRIu32 " total bytes.\n",
					read_bytes, to_read);
				return -2;
			} else {
				fprintf(stderr, "Unexpectedly reached EOF while reading header.");
				fprintf(stderr, "Read %" PRIu32 " bytes, expected %" PRIu32 " total bytes.\n",
					read_bytes, to_read);
				return -3;
			}
		} else if (n == -1) {
			return -1;
		}

		read_bytes += n;
	}

	// Check if magic string in header matches
	if (memcmp(walk, IPC_MAGIC, IPC_MAGIC_LEN) != 0) {
		fprintf(stderr, "Invalid magic string. Got '%.*s', expected '%s'\n", IPC_MAGIC_LEN, walk, IPC_MAGIC);
		return -3;
	}

	walk += IPC_MAGIC_LEN;

	// Extract reply size
	memcpy(reply_size, walk, sizeof(uint32_t));
	walk += sizeof(uint32_t);

	// Extract message type
	memcpy(msg_type, walk, sizeof(uint8_t));
	walk += sizeof(uint8_t);

	(*reply) = malloc(*reply_size);

	// Extract payload
	read_bytes = 0;
	while (read_bytes < *reply_size) {
		ssize_t n = read(sock_fd, *reply + read_bytes, *reply_size - read_bytes);

		if (n == 0) {
			fprintf(stderr, "Unexpectedly reached EOF while reading payload.");
			fprintf(stderr, "Read %" PRIu32 " bytes, expected %" PRIu32 " bytes.\n", read_bytes,
				*reply_size);
			free(*reply);
			return -2;
		} else if (n == -1) {
			if (errno == EINTR || errno == EAGAIN) continue;
			free(*reply);
			return -1;
		}

		read_bytes += n;
	}

	return 0;
}

static int read_socket(IPCMessageType *msg_type, uint32_t *msg_size, char **msg)
{
	int ret = -1;

	while (ret != 0) {
		ret = recv_message((uint8_t *)msg_type, msg_size, (uint8_t **)msg);

		if (ret < 0) {
			// Try again (non-fatal error)
			if (ret == -1 && (errno == EINTR || errno == EAGAIN)) continue;

			fprintf(stderr, "Error receiving response from socket. ");
			fprintf(stderr, "The connection might have been lost.\n");
			exit(2);
		}
	}

	return 0;
}

static ssize_t write_socket(const void *buf, size_t count)
{
	size_t written = 0;

	while (written < count) {
		const ssize_t n = write(sock_fd, ((uint8_t *)buf) + written, count - written);

		if (n == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
				continue;
			else
				return n;
		}
		written += n;
	}
	return written;
}

static void connect_to_socket()
{
	struct sockaddr_un addr;

	int sock = socket(AF_UNIX, SOCK_STREAM, 0);

	// Initialize struct to 0
	memset(&addr, 0, sizeof(struct sockaddr_un));

	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, DEFAULT_SOCKET_PATH);

	connect(sock, (const struct sockaddr *)&addr, sizeof(struct sockaddr_un));

	sock_fd = sock;
}

static int send_message(IPCMessageType msg_type, uint32_t msg_size, uint8_t *msg)
{
	jdwm_ipc_header_t header = { .magic = IPC_MAGIC_ARR, .size = msg_size, .type = msg_type };

	size_t header_size = sizeof(jdwm_ipc_header_t);
	size_t total_size = header_size + msg_size;

	uint8_t buffer[total_size];

	// Copy header to buffer
	memcpy(buffer, &header, header_size);
	// Copy message to buffer
	memcpy(buffer + header_size, msg, header.size);

	write_socket(buffer, total_size);

	return 0;
}

static int is_float(const char *s)
{
	size_t len = strlen(s);
	int is_dot_used = 0;
	int is_minus_used = 0;

	// Floats can only have one decimal point in between or digits
	// Optionally, floats can also be below zero (negative)
	for (int i = 0; i < len; i++) {
		if (isdigit(s[i]))
			continue;
		else if (!is_dot_used && s[i] == '.' && i != 0 && i != len - 1) {
			is_dot_used = 1;
			continue;
		} else if (!is_minus_used && s[i] == '-' && i == 0) {
			is_minus_used = 1;
			continue;
		} else
			return 0;
	}

	return 1;
}

static int is_unsigned_int(const char *s)
{
	size_t len = strlen(s);

	// Unsigned int can only have digits
	for (int i = 0; i < len; i++) {
		if (isdigit(s[i]))
			continue;
		else
			return 0;
	}

	return 1;
}

static int is_signed_int(const char *s)
{
	size_t len = strlen(s);

	// Signed int can only have digits and a negative sign at the start
	for (int i = 0; i < len; i++) {
		if (isdigit(s[i]))
			continue;
		else if (i == 0 && s[i] == '-') {
			continue;
		} else
			return 0;
	}

	return 1;
}

static void flush_socket_reply()
{
	IPCMessageType reply_type;
	uint32_t reply_size;
	char *reply;

	read_socket(&reply_type, &reply_size, &reply);

	free(reply);
}

static void print_socket_reply()
{
	IPCMessageType reply_type;
	uint32_t reply_size;
	char *reply;

	read_socket(&reply_type, &reply_size, &reply);

	printf("%.*s\n", reply_size, reply);
	fflush(stdout);
	free(reply);
}

static int run_command(const char *name, char *args[], int argc)
{
	const unsigned char *msg;
	size_t msg_size;

	yajl_gen gen = yajl_gen_alloc(NULL);

	// Message format:
	// {
	//   "command": "<name>",
	//   "args": [ ... ]
	// }
	// clang-format off
  YMAP(
    YSTR("command"); YSTR(name);
    YSTR("args"); YARR(
      for (int i = 0; i < argc; i++) {
        if (is_signed_int(args[i])) {
          long long num = atoll(args[i]);
          YINT(num);
        } else if (is_float(args[i])) {
          float num = atof(args[i]);
          YDOUBLE(num);
        } else {
          YSTR(args[i]);
        }
      }
    )
  )
	// clang-format on

	yajl_gen_get_buf(gen, &msg, &msg_size);

	send_message(IPC_TYPE_RUN_COMMAND, msg_size, (uint8_t *)msg);

	if (!ignore_reply)
		print_socket_reply();
	else
		flush_socket_reply();

	yajl_gen_free(gen);

	return 0;
}

static int get_monitors()
{
	send_message(IPC_TYPE_GET_MONITORS, 1, (uint8_t *)"");
	print_socket_reply();
	return 0;
}

static int get_tags()
{
	send_message(IPC_TYPE_GET_TAGS, 1, (uint8_t *)"");
	print_socket_reply();

	return 0;
}

static int get_layouts()
{
	send_message(IPC_TYPE_GET_LAYOUTS, 1, (uint8_t *)"");
	print_socket_reply();

	return 0;
}

static int get_dwm_client(Window win)
{
	const unsigned char *msg;
	size_t msg_size;

	yajl_gen gen = yajl_gen_alloc(NULL);

	// Message format:
	// {
	//   "client_window_id": "<win>"
	// }
	// clang-format off
  YMAP(
    YSTR("client_window_id"); YINT(win);
  )
	// clang-format on

	yajl_gen_get_buf(gen, &msg, &msg_size);

	send_message(IPC_TYPE_GET_DWM_CLIENT, msg_size, (uint8_t *)msg);

	print_socket_reply();

	yajl_gen_free(gen);

	return 0;
}

static int subscribe(const char *event)
{
	const unsigned char *msg;
	size_t msg_size;

	yajl_gen gen = yajl_gen_alloc(NULL);

	// Message format:
	// {
	//   "event": "<event>",
	//   "action": "subscribe"
	// }
	// clang-format off
  YMAP(
    YSTR("event"); YSTR(event);
    YSTR("action"); YSTR("subscribe");
  )
	// clang-format on

	yajl_gen_get_buf(gen, &msg, &msg_size);

	send_message(IPC_TYPE_SUBSCRIBE, msg_size, (uint8_t *)msg);

	if (!ignore_reply)
		print_socket_reply();
	else
		flush_socket_reply();

	yajl_gen_free(gen);

	return 0;
}

static void usage_error(const char *prog_name, const char *format, ...)
{
	va_list args;
	va_start(args, format);

	fprintf(stderr, "Error: ");
	vfprintf(stderr, format, args);
	fprintf(stderr, "\nusage: %s <command> [...]\n", prog_name);
	fprintf(stderr, "Try '%s help'\n", prog_name);

	va_end(args);
	exit(1);
}

static void print_usage(const char *name)
{
	printf("usage: %s [options] <command> [...]\n", name);
	puts("");
	puts("Commands:");
	puts("  run_command <name> [args...]    Run an IPC command");
	puts("");
	puts("  get_monitors                    Get monitor properties");
	puts("");
	puts("  get_tags                        Get list of tags");
	puts("");
	puts("  get_layouts                     Get list of layouts");
	puts("");
	puts("  get_dwm_client <window_id>      Get dwm client proprties");
	puts("");
	puts("  subscribe [events...]           Subscribe to specified events");
	puts("                                  Options: " IPC_EVENT_TAG_CHANGE ",");
	puts("                                  " IPC_EVENT_LAYOUT_CHANGE ",");
	puts("                                  " IPC_EVENT_CLIENT_FOCUS_CHANGE ",");
	puts("                                  " IPC_EVENT_MONITOR_FOCUS_CHANGE ",");
	puts("                                  " IPC_EVENT_FOCUSED_TITLE_CHANGE ",");
	puts("                                  " IPC_EVENT_FOCUSED_STATE_CHANGE);
	puts("");
	puts("  help                            Display this message");
	puts("");
	puts("Options:");
	puts("  --ignore-reply                  Don't print reply messages from");
	puts("                                  run_command and subscribe.");
	puts("");
}

// GNU argp parser
const char *argp_program_version = "jdwm-msg " VERSION;
const char *argp_program_bug_address = "https://github.com/JeffofBread/jdwm/issues";
static char doc[] =
	"A custom build of dwm-msg (from dwm ipc patch) made by JeffofBread. If you wish to know more, check out jdwm's github page at https://github.com/JeffofBread/jdwm or dwm ipc's github page at https://github.com/mihirlad55/dwm-ipc";
static char args_doc[] = "";
static struct argp_option options[] = {
	{ "simple-version", 'v', 0, 0, "Simplified version output" },
	{ 0 }
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = state->input;
	switch (key) {
	    case 'v':
		    printf("%s", VERSION);
		    exit(EXIT_SUCCESS);
		    break;
	    case ARGP_KEY_ARG: // Case if too many args
		    if (state->arg_num >= 0) argp_usage(state);
		    arguments->args[state->arg_num] = arg;
		    break;
		//case ARGP_KEY_END: // Case if too few args
		    //if (state->arg_num < 2)
		    //argp_usage (state);
		    //break;
	    default:
		    return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

int main(int argc, char *argv[])
{
	const char *prog_name = argv[0];

	connect_to_socket();
	if (sock_fd == -1) {
		fprintf(stderr, "Failed to connect to socket\n");
		return 1;
	}

	// GNU arg parser
	struct arguments arguments;
	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	return 0;
}