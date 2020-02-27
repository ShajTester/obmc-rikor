#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include "rikfan-manager.h"

static gboolean
on_handle_hello_world (Rikfan *interface, GDBusMethodInvocation *invocation,
                       const gchar *greeting, gpointer user_data)
{
	gchar *response;
	response = g_strdup_printf ("Hello world %s!!.", greeting);
	rikfan_complete_hello_world (interface, invocation, response);
	g_print("%s\n", response);
	g_free (response);
	return TRUE;
}


static gboolean
on_handle_fan_mode(Rikfan *interface, GDBusMethodInvocation *invocation,
                   const gchar *greeting, gpointer user_data)
{
	gchar *response;
	response = g_strdup_printf ("The method of the 'Fan Mode' object is called %s", greeting);
	rikfan_complete_fan_mode (interface, invocation, response);
	g_print("%s\n", response);
	g_free (response);
	return TRUE;
}


void 
OnFanModeChanged(GObject * gobject, GParamSpec * pspec, gpointer user_data)
{
	// g_print("Fan mode now set to %s\n", rikfan_get_mode_test((Rikfan)* gobject));
	g_print("Fan mode now set\n");
}


static void
on_bus_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
	Rikfan *interface;
	GError *error;

	/* This is where we'd export some objects on the bus */
	g_print ("on_bus_acquired %s on the session bus\n", name);


	gchar *conn_name;
	g_object_get(connection, "unique-name", &conn_name, NULL);
	printf("%s\n", conn_name);
	g_free(conn_name);

	interface = rikfan_skeleton_new();
	g_signal_connect (interface, "handle-hello-world", G_CALLBACK (on_handle_hello_world), NULL);
	g_signal_connect (interface, "handle-fan-mode", G_CALLBACK (on_handle_fan_mode), NULL);
	g_signal_connect (interface, "notify::ModeTest", G_CALLBACK (OnFanModeChanged), NULL);
	error = NULL;
	if (!g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (interface), connection, "/com/rikor/ares/rikfan", &error))
	{
		printf("ERROR %s\n", error->message);
	}
}

static void
on_name_lost (GDBusConnection *connection,
              const gchar     *name,
              gpointer         user_data)
{
	g_print ("on_name_lost %s on the session bus\n", name);
}


static void
on_name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
	Rikfan *interface;
	GError *error;

	g_print ("on_name_acquired %s on the session bus\n", name);

	// gchar *conn_name;
	// g_object_get(connection, "unique-name", &conn_name, NULL);
	// printf("%s\n", conn_name);
	// g_free(conn_name);

	// interface = rikfan_skeleton_new();
	// g_signal_connect (interface, "handle-hello-world", G_CALLBACK (on_handle_hello_world), NULL);
	// // g_signal_connect (interface, "handle-fan-mode", G_CALLBACK (on_handle_fan_mode), NULL);
	// error = NULL;
	// if (!g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (interface), connection, "/com/rikor/ares", &error))
	// {
	// 	printf("ERROR %s\n", error->message);
	// }
}

static 	GMainLoop *loop;

/*
 * On SIGINT, exit the main loop
 */
static void sig_handler(int signo)
{
	if (signo == SIGINT)
	{
		g_main_loop_quit(loop);
	}
}

int main()
{
	// set up the SIGINT signal handler
	if (signal(SIGINT, &sig_handler) == SIG_ERR)
	{
		g_print("Failed to register SIGINT handler, quitting...\n");
		exit(EXIT_FAILURE);
	}

	loop = g_main_loop_new (NULL, FALSE);

	// guint bus_id = g_bus_own_name(G_BUS_TYPE_SESSION, "com.rikor", G_BUS_NAME_OWNER_FLAGS_NONE, on_bus_acquired,
	//                on_name_acquired, on_name_lost, NULL, NULL);
	guint bus_id = g_bus_own_name(G_BUS_TYPE_SYSTEM, "com.rikor.ares.rikfan", G_BUS_NAME_OWNER_FLAGS_NONE, on_bus_acquired,
	                              on_name_acquired, on_name_lost, NULL, NULL);

	g_print("Initial PID: %d\n", getpid());

	if (bus_id == 0)
	{
		g_print("\nError: can not asquire bus name\n");
	}
	else
	{
		g_print ("bus_id %u\n", bus_id);
		g_main_loop_run (loop);
		g_print("\n\nServer well done\n");
		g_bus_unown_name(bus_id);
	}

	g_main_loop_unref(loop);

	return 0;
}
