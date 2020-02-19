#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include "rikfan-manager.h"

int main()
{
	Rikfan *proxy;
	GError *error;
	gchar **buf;

	error = NULL;
	proxy = rikfan_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE,
			"com.rikor.ares.rikfan", "/com/rikor/ares/rikfan", NULL, &error);

	rikfan_call_hello_world_sync(proxy, "fatminmin", buf, NULL, &error);
	g_print("resp: %s\n", *buf);

	rikfan_call_fan_mode_sync(proxy, "testimonials", buf, NULL, &error);
	g_print("resp: %s\n", *buf);

	g_object_unref(proxy);
	return 0;
}
