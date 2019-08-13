import paho.mqtt.client as paho


# MQTT function when something is published
def on_publish(client, userdata, result):
    print("data published!")

def main():
    # MQTT raspberry pi IP address and port
    broker = "$YOUR_BROKER_ADDRESS"
    port = 1883
    # sending conformation that openHAB is ready for more measurements
    client = paho.Client("feedback")
    client.on_publish = on_publish
    client.connect(broker, port)
    ret = client.publish("rasp/feedback", "1")
    print(ret)


if __name__ == '__main__':
    main()
