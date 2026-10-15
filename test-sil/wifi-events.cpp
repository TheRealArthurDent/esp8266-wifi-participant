// WiFi events for the ESP8266 Arduino core's host emulation.
//
// The emulation never fires the SDK's WiFi events, so callbacks like
// WiFi.onStationModeGotIP() never run. This file fires the events listed in
// the WIFI_EVENTS environment variable, at the given number of milliseconds
// after boot (in chronological order), e.g.:
//
//   WIFI_EVENTS="got_ip@1000,disconnected@3000"
//
// Events are delivered while the sketch waits in delay(), like the SDK does
// on the chip. Each fired event is logged to stderr, so it doesn't mix with
// the serial output on stdout.
//
// Each event also sets the connect status that WiFi.status() reports (see
// core-host-emulation.patch): idle until the first event, connected after
// got_ip, connecting again after disconnected, as the SDK reconnects on its
// own.
//
// Refs #143, #152

#include <Arduino.h>
#include <user_interface.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

extern "C" wifi_event_handler_cb_t wifi_event_handler_cb_emu;
extern "C" station_status_t mock_station_connect_status;

namespace
{
struct ScheduledEvent
{
  unsigned long at_ms;
  std::string name;
};

struct Schedule
{
  std::vector<ScheduledEvent> events;
  size_t next = 0;
};

// Parses WIFI_EVENTS on first use.
auto schedule() -> Schedule &
{
  static Schedule instance = []() -> Schedule
  {
    Schedule parsed;
    const char *spec = getenv("WIFI_EVENTS");
    std::string rest = (spec == nullptr) ? "" : spec;
    while (!rest.empty())
    {
      const size_t comma = rest.find(',');
      const std::string item = rest.substr(0, comma);
      rest = (comma == std::string::npos) ? "" : rest.substr(comma + 1);

      const size_t separator = item.find('@');
      try
      {
        parsed.events.push_back({std::stoul(item.substr(separator + 1)), item.substr(0, separator)});
      }
      catch (const std::logic_error &)
      {
        std::cerr << "[wifi-events] ignoring '" << item << "', expected <event>@<ms>\n";
      }
    }
    return parsed;
  }();
  return instance;
}

void fire(const ScheduledEvent &scheduled)
{
  // The addresses stay 0.0.0.0; the firmware reads WiFi.localIP() instead.
  System_Event_t event{};

  if (scheduled.name == "got_ip")
  {
    event.event = EVENT_STAMODE_GOT_IP;
    mock_station_connect_status = STATION_GOT_IP;
  }
  else if (scheduled.name == "disconnected")
  {
    event.event = EVENT_STAMODE_DISCONNECTED;
    event.event_info.disconnected.reason = REASON_BEACON_TIMEOUT;
    mock_station_connect_status = STATION_CONNECTING;
  }
  else
  {
    std::cerr << "[wifi-events] unknown event '" << scheduled.name << "'\n";
    return;
  }

  // The status changes before the callbacks run, as on the chip.
  std::cerr << "[wifi-events] " << scheduled.name << " at " << millis() << " ms\n";
  if (wifi_event_handler_cb_emu != nullptr)
  {
    wifi_event_handler_cb_emu(&event);
  }
}
} // namespace

// Overrides the emulation's empty default, see core-host-emulation.patch,
// which also fixes its name.
extern "C" void mock_sdk_task() // NOLINT(readability-identifier-naming)
{
  Schedule &pending = schedule();
  while (pending.next < pending.events.size() && millis() >= pending.events[pending.next].at_ms)
  {
    fire(pending.events[pending.next++]);
  }
}
