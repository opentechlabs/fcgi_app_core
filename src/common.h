/*
 * common.h
 *
 * Copyright (C) 2012		OpenTech Labs
 *				Andrew Clayton <andrew@opentechlabs.co.uk>
 *
 * This software is released under the MIT License (MIT-LICENSE.txt)
 * and the GNU Affero General Public License version 3 (AGPL-3.0.txt)
 */

#ifndef _COMMON_H_
#define _COMMON_H_

/*
 * Lets not pretend we care about running this on anything
 * other than Linux.
 *
 * On Linux, MySQL defines _GNU_SOURCE 1 in my_global.h, but
 * lets do it explicitly here anyway.
 */
#define _GNU_SOURCE 1

#include <fcgi_stdio.h>
#include <ctemplate.h>

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <netdb.h>

#include <glib.h>

#include "app_config.h"
#include "db.h"
#include "utils.h"

#define BUF_SIZE	4096
#define SQL_MAX		8192

#define SHA256		 5
#define SHA512		10

/*
 * These three define the number of nanoseconds in a second,
 * millisecond and microsecond.
 */
#define NS_SEC		1000000000
#define NS_MSEC		1000000
#define NS_USEC		1000

/* Length of time (seconds) an activation key is valid */
#define KEY_EXP		86400

/* Macro to check if the request method is POST */
#define IS_POST()	(strstr(env_vars.request_method, "POST"))
/* Macro to check if the request method is GET */
#define IS_GET()	(strstr(env_vars.request_method, "GET"))
/* Macro to check if a char *variable is set, i.e a len > 0 */
#define IS_SET(var)	((strlen(var) > 0) ? 1 : 0)

/* Unbreak __func__ by my_global.h */
#ifdef __func__
#undef __func__
#endif

/*
 * Wrapper around fprintf(). It will prepend the text passed it with
 * seconds.microseconds pid function:
 *
 * e.g if you call it like: d_fprintf(debug, "This is a test\n");
 * You will get:
 *
 * 1304600723.663486 1843 main: This is a test
 */
#define d_fprintf(stream, fmt, ...) \
	do { \
		if (stream == debug_log && !DEBUG_LEVEL) \
			break; \
		struct timespec tp; \
		char tenant[NI_MAXHOST]; \
		get_tenant(env_vars.host, tenant); \
		clock_gettime(CLOCK_REALTIME, &tp); \
		fprintf(stream, "%ld.%06ld %d %s %s: " fmt, tp.tv_sec, \
				tp.tv_nsec / NS_USEC, getpid(), tenant, \
				__func__, ##__VA_ARGS__); \
		fflush(stream); \
	} while (0)

/*
 * Wrapper around mysql_real_escape_string()
 *
 * Given a string it will return a string, that must be free'd, that is safe
 * to pass to mysql.
 */
static inline char *make_mysql_safe_string(const char *string)
{
	char *safe = malloc(strlen(string) * 2 + 1);
	mysql_real_escape_string(conn, safe, string, strlen(string));
	return safe;
}

/*
 * Structure that defines a users session. The session is stored
 * in a tokyocabinet database table inbetween requests.
 */
struct user_session {
	char *tenant;
	unsigned long long sid;
	unsigned int uid;
	unsigned char capabilities;
	char *username;
	char *name;
	time_t login_at;
	time_t last_seen;
	char *origin_ip;
	char *client_id;
	char *session_id;
	char *csrf_token;
	unsigned int restrict_ip;
	char *user_hdr;
} user_session;
struct user_session user_session;

/*
 * This structure maps to the environment variable list sent
 * by the application. We don't store every item.
 */
struct env_vars {
	char *request_uri;
	char *request_method;
	char *content_type;
	char *http_cookie;
	char *http_user_agent;
	char *remote_addr;
	char *host;
	char *query_string;
} env_vars;
struct env_vars env_vars;

/* Structure to hold information about uploaded files via POST */
struct file_info {
	char *orig_file_name;
	char *temp_file_name;
	char *name;
	char *mime_type;
} file_info;
struct file_info file_info;

extern FILE *access_log;
extern FILE *sql_log;
extern FILE *error_log;
extern FILE *debug_log;

extern GList *u_files;
extern GList *avars;
extern GHashTable *qvars;

#endif /* _COMMON_H_ */