# ble-communication

This project aimed to design, implement, and observe the functionality of a Bluetooth Low Energy (BLE) network. The network is constructed using sensors, each of which reports to a central gateway node that manages the results. In addition, the gateway node hosts a CoAP server that allows wireless devices to read the data over Wi-Fi. 

The implementation has genuine use-cases and is hardware that you would expect to use in real-world situations. An example of a real-world use case is an air quality monitor that periodically evaluates air-quality characteristics (such as temperature and humidity) and uses these findings to adjust the air. 

The scope of this investigation was limited to two sensor nodes that were connected to a single gateway. The gateway and sensor nodes remained powered on throughout testing, and power consumption was not a factor in performance measurements. The gateway is running a CoAP server that is visible to devices within the local Wi-Fi network. Communication with devices outside the local Wi-Fi network was not within the scope of this project.
