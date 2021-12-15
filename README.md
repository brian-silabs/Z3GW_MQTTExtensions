# Z3GW_MQTTExtensions

Edited from GSDK 3.2.2
Adds TC Join handler event access to MQTT broker

## How To Install
0. Make a backup of your SDK (just in case)
1. Replace existing plugins by these ones within the SDK
2. Close any .ISC file opened in Simplicity
3. Refresh SDK within Simplicity Studio (Go to Preferences->SDKs, click on your modified SDK within the list and hit F5)
4. Generate project from your ISC again so the new callback functions are now declared
5. Build
6. Run

## Expected behavior
### Joining Device
Upon device join attempt (see device-table-discovery.c l579) filtered as EMBER_STANDARD_SECURITY_UNSECURED_JOIN
The device table plugin will fire a callback function named emberAfPluginDeviceTableDeviceAttemptingJoinCallback passing the joiner's EUI64
This callback function is then used by the MQTT relay plugin to send this EUI over MQTT as follows :

Topic : `gw/000B57FFFEA7866E/deviceAttemptingJoin`
Payload : `{"eui64":"0x000B57FFFEA8F460"}`


### Network Opening state
Network opening state can be queried asynchronously using 
Query Topic : `gw/000B57FFFEA7866E/isNetworkOpen`

With or without any payload
The Host will answer on the following topic :

Publish Topic : `gw/000B57FFFEA7866E/networkOpeningState`
Payload : `{"open":false,"duration":0}`


But will also automatically be sent upon opening and closing on the same publish topic with the same format

Publish Topic : `gw/000B57FFFEA7866E/networkOpeningState`
Payload : `{"open":false,"duration":0}`
