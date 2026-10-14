// WiFi events for the ESP8266 Arduino core's host emulation.
//
// The emulation reports WiFi as always connected and never fires the SDK's
// WiFi events, so callbacks like WiFi.onStationModeGotIP() never run. This
// file fires the events listed in the WIFI_EVENTS environment variable, at
// the given number of milliseconds after boot, for example:
//
//   WIFI_EVENTS="got_ip@1000,disconnected@3000"
//
// Events are delivered while the sketch waits in delay(), like the SDK does
// on the chip. Each fired event is logged to stderr, so it doesn't mix with
// the serial output on stdout.
//
// Refs #143

#include <Arduino.h>
#include <user_interface.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

extern "C" wifi_event_handler_cb_t wifi_event_handler_cb_emu;

namespace
{
struct ScheduledEvent
{
  unsigned long atMs;
  std::string name;
};

std::vector<ScheduledEvent> scheduledEvents;
size_t nextEvent = 0;
bool parsed = false;

void parseWifiEvents()
{
  parsed = true;
  const char *spec = getenv("WIFI_EVENTS");
  if (spec == nullptr)
  {
    return;
  }

  std::string rest(spec);
  while (!rest.empty())
  {
    const size_t comma = rest.find(',');
    const std::string item = rest.substr(0, comma);
    rest = (comma == std::string::npos) ? "" : rest.substr(comma + 1);

    const size_t at = item.find('@');
    if (at == std::string::npos)
    {
      fprintf(stderr, "[wifi-events] ignoring '%s', expected <event>@<ms>\n", item.c_str());
      continue;
    }
    scheduledEvents.push_back({strtoul(item.c_str() + at + 1, nullptr, 10), item.substr(0, at)});
  }
  std::stable_sort(scheduledEvents.begin(), scheduledEvents.end(), [](const ScheduledEvent &a, const ScheduledEvent &b)
                   { return a.atMs < b.atMs; });
}

void fire(const ScheduledEvent &scheduled)
{
  System_Event_t event;
  memset(&event, 0, sizeof(event));

  if (scheduled.name == "got_ip")
  {
    event.event = EVENT_STAMODE_GOT_IP;
    event.event_info.got_ip.ip.addr = IPAddress(192, 168, 4, 2);
    event.event_info.got_ip.mask.addr = IPAddress(255, 255, 255, 0);
    event.event_info.got_ip.gw.addr = IPAddress(192, 168, 4, 1);
  }
  else if (scheduled.name == "disconnected")
  {
    event.event = EVENT_STAMODE_DISCONNECTED;
    event.event_info.disconnected.reason = REASON_BEACON_TIMEOUT;
  }
  else
  {
    fprintf(stderr, "[wifi-events] unknown event '%s'\n", scheduled.name.c_str());
    return;
  }

  fprintf(stderr, "[wifi-events] %s at %lu ms\n", scheduled.name.c_str(), millis());
  if (wifi_event_handler_cb_emu != nullptr)
  {
    wifi_event_handler_cb_emu(&event);
  }
}
} // namespace

// Overrides the emulation's empty default, see core-host-emulation.patch.
extern "C" void mock_sdk_task()
{
  if (!parsed)
  {
    parseWifiEvents();
  }
  while (nextEvent < scheduledEvents.size() && millis() >= scheduledEvents[nextEvent].atMs)
  {
    fire(scheduledEvents[nextEvent++]);
  }
}
