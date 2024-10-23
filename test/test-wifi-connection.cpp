#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "wifi-connection.hpp"
#include "wifi-connection-test-wrapper.hpp"

// Mock class definition for WifiDependent
class MockWiFiDependent : public WifiDependent
{
public:
  MOCK_METHOD(void, onWifiConnectionEstablished, (), (override));
  MOCK_METHOD(void, onWifiConnectionLost, (), (override));
};

// Test for successful WiFi connection and notifying dependents
TEST(WifiConnectionTest, NotifyDependentsOnConnect)
{
  // Use the WifiConnectionTestWrapper instead of the original class
  WifiConnectionTestWrapper &connection = static_cast<WifiConnectionTestWrapper &>(WifiConnection::getInstance());

  // Create a mock dependent
  MockWiFiDependent mockDependent;

  // Create a list of dependents and add the mock to it
  std::list<WifiDependent *> dependents;
  dependents.push_back(&mockDependent);

  // Expect the onWifiConnectionEstablished() method to be called once
  EXPECT_CALL(mockDependent, onWifiConnectionEstablished()).Times(1);

  // Initialize the WifiConnection with the list of dependents
  connection.init(dependents);

  // Simulate a WiFi connection event using the test wrapper's exposed method
  connection.onWifiConnect(WiFiEventStationModeGotIP{});

  // Verify that the connection is established
  EXPECT_TRUE(connection.isConnected());
}

// Test for WiFi disconnection and notifying dependents
TEST(WifiConnectionTest, NotifyDependentsOnDisconnect)
{
  // Use the WifiConnectionTestWrapper instead of the original class
  WifiConnectionTestWrapper &connection = static_cast<WifiConnectionTestWrapper &>(WifiConnection::getInstance());

  // Create a mock dependent
  MockWiFiDependent mockDependent;

  // Create a list of dependents and add the mock to it
  std::list<WifiDependent *> dependents;
  dependents.push_back(&mockDependent);

  // Expect the onWifiConnectionLost() method to be called once
  EXPECT_CALL(mockDependent, onWifiConnectionLost()).Times(1);

  // Initialize the WifiConnection with the list of dependents
  connection.init(dependents);

  // Simulate a WiFi disconnection event using the test wrapper's exposed method
  connection.onWifiDisconnect(WiFiEventStationModeDisconnected{});

  // Verify that the connection is lost
  EXPECT_FALSE(connection.isConnected());
}
