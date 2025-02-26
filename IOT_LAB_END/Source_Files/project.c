#include "contiki.h"
#include "dev/leds.h"
#include "dev/sht11/sht11-sensor.h"
#include "jsontree.h"
#include "json-ws.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/node-id.h>

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


PROCESS(websense_process, "Websense (sky)");
AUTOSTART_PROCESSES(&websense_process);

/*---------------------------------------------------------------------------*/

static CC_INLINE int
get_sensor_id(void)
{
  static int c=0;
  int max_cycle = 25;
  c = (c + 5) % max_cycle;
  return c;
}


/*---------------------------------------------------------------------------*/
static int
output_sensor_id(struct jsontree_context *path)
{
  char buf[8];
  snprintf(buf, sizeof(buf), "%3d", get_sensor_id());
  jsontree_write_atom(path, buf);
  return 0;
}


static struct jsontree_callback sensor_id_sensor_callback =
    JSONTREE_CALLBACK(output_sensor_id, NULL);

static struct jsontree_string desc = JSONTREE_STRING("Tmote Sky");
static struct jsontree_string sensor_id_unit = JSONTREE_STRING("Sensor ID");

   JSONTREE_OBJECT(node_tree,
                    JSONTREE_PAIR("node-type", &desc),
                    JSONTREE_PAIR("time", &json_time_callback));

JSONTREE_OBJECT(sensor_id_sensor_tree,
                JSONTREE_PAIR("unit", &sensor_id_unit),
                JSONTREE_PAIR("value", &sensor_id_sensor_callback));


JSONTREE_OBJECT(rsc_tree,
                JSONTREE_PAIR("Sensor", &sensor_id_sensor_tree)
);

/* complete node tree */
JSONTREE_OBJECT(tree,
                JSONTREE_PAIR("node", &node_tree),
                JSONTREE_PAIR("rsc", &rsc_tree),
                JSONTREE_PAIR("cfg", &json_subscribe_callback));

/*---------------------------------------------------------------------------*/
/* for cosm plugin */
#if WITH_COSM
/* set COSM value callback to be the temp sensor */
struct jsontree_callback cosm_value_callback =
    JSONTREE_CALLBACK(output_sensor_id, NULL);
#endif

PROCESS_THREAD(websense_process, ev, data)
{
  static struct etimer timer;

  PROCESS_BEGIN();

  json_ws_init(&tree);

  SENSORS_ACTIVATE(sht11_sensor);

  json_ws_set_callback("rsc");

  while (1)
  {
    /* Alive indication with the LED */
    etimer_set(&timer, CLOCK_SECOND * 2);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    leds_on(LEDS_RED);
    /*etimer_set(&timer, CLOCK_SECOND / 8);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    leds_off(LEDS_RED);
    */
  }

  PROCESS_END();
}
