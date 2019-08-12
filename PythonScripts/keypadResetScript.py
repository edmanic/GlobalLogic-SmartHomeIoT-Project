import requests
import json
import paho.mqtt.client as paho


# MQTT function when something is published
def on_publish(client, userdata, result):
    print("data published!")


# function for cURL response check
def response(r):
    if int(str(r.status_code)[0]) == 2:
        print("success! status code is: " + str(r.status_code))
    else:
        print("gone wrong! status code is: " + str(r.status_code))


def main():
    print("Reseting login keypad...")
    # request URL and header for cURL requests
    url = 'http://$THINGSBOARD_HOST/api/v1/homeuroom1ulogin/attributes?sharedKeys=reset(YorN)'
    headers = {'Accept': 'application/json', 'Accept-Charset': 'UTF-8',
               'X-Authorization': 'Bearer $YOUR_AUTH_KEY'}
    # cURL GET request for checking whether wa want to reset the keypad
    r = requests.get(url, headers=headers)
    print(r.content)
    response(r)
    # check if the reset shared attribute for the keypad and whether reset is requested
    if json.loads(r.content.decode('utf-8'))["shared"]["reset(YorN)"] == 'Y':
        # MQTT Raspberry Pi IP address and port
        broker = "$YOUR_BROKER_ADDRESS"
        port = 1883
        # sending the reset request to the keypad through the MQTT protocol
        client = paho.Client("feedback")
        client.on_publish = on_publish
        client.connect(broker, port)
        ret = client.publish("home/room1/login", "reset")
        print(ret)


if __name__ == '__main__':
    main()
