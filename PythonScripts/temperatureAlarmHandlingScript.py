import requests
import json
import paho.mqtt.client as paho

# list of tokens from ThingsBoard thermometers for temperature check
LIST_OF_TEMP_TOKENS = ['homeuroom1utemp', 'homeuroom1utemp_ntc']


# function for MQTT counter-measure activation
def mqttEmergency(data):
    # MQTT raspberry pi IP address and port
    broker = "$YOUR_BROKER_ADDRESS"
    port = 1883
    # publishing whether to activate the counter-measures or not
    # ("1" - activate, "0" - don't activate/deactivate)
    client = paho.Client("emergency")
    client.on_publish = on_publish
    client.connect(broker, port)
    ret = client.publish("emergency/fan", data)
    print(ret)


# MQTT function when something is published
def on_publish(client, userdata, result):
    print("data published!")


# function for cURL response check
def response(r):
    if (int(str(r.status_code)[0]) == 2):
        print("success! status code is: " + str(r.status_code))
    else:
        print("gone wrong! status code is: " + str(r.status_code))


# function for handling cURL POST requests
def postRequest(url, header, data):
    r = requests.post(url, data=data, headers=header)
    response(r)


# function for handling cURL GET requests
def getRequest(url, header):
    r = requests.get(url, headers=header)
    print(r.content)
    response(r)
    return r


def main():
    print("beginning alarm handling...")
    # parts of the request URL witch we will combine back together for further use
    url1 = 'http://ec2-52-59-207-8.eu-central-1.compute.amazonaws.com/api/v1/'
    url2 = '/attributes?clientKeys=alarm'
    url3 = '/attributes'
    # headers for successful get or post cURL request
    headers1 = {'Accept': 'application/json', 'Accept-Charset': 'UTF-8',
                'X-Authorization': 'Bearer eyJhbGciOiJIUzUxMiJ9.eyJzdWIiOiJlZGlucGpldmljQGdtYWlsLmNvbSIsInNjb3BlcyI6WyJURU5BTlRfQURNSU4iXSwidXNlcklkIjoiMWYzZjYwMTAtYTZkNi0xMWU5LWJkMmItY2YyYmY3MjczMWE2IiwiZmlyc3ROYW1lIjoiRWRpbiIsImxhc3ROYW1lIjoiUGpldmljIiwiZW5hYmxlZCI6dHJ1ZSwiaXNQdWJsaWMiOmZhbHNlLCJ0ZW5hbnRJZCI6IjYwMGVlYzQwLTU2YjktMTFlOS1iNDZlLWQ3N2IyODFjNDI1YSIsImN1c3RvbWVySWQiOiIxMzgxNDAwMC0xZGQyLTExYjItODA4MC04MDgwODA4MDgwODAiLCJpc3MiOiJ0aGluZ3Nib2FyZC5pbyIsImlhdCI6MTU2NDU2ODg4OSwiZXhwIjoxNTY0NTY5Nzg5fQ.fUoMg1jSZg5sfxCt1jVNGNe5Ve3jXsUalNU91oKeHM-CjyC86ydEGZZ7gmLsgtVY4Pa2YoP-048WhJaj05lCWA'}
    headers2 = {'content-type': 'application/json', 'Accept-Charset': 'UTF-8'}
    # URLs for fetching alarm status (11,22) and updating alarm status (33,44)
    url11 = url1 + LIST_OF_TEMP_TOKENS[0] + url2
    url22 = url1 + LIST_OF_TEMP_TOKENS[1] + url2
    url33 = url1 + LIST_OF_TEMP_TOKENS[0] + url3
    url44 = url1 + LIST_OF_TEMP_TOKENS[1] + url3
    # current alarm status from temp (r) and temp_ntc (p)
    r = getRequest(url11, headers1)
    p = getRequest(url22, headers1)
    # check weather both thermostats have reported temperature anomaly and go into "emergency mode"
    if json.loads(r.content)["client"]["alarm"] == "pending" and json.loads(p.content)["client"]["alarm"] == "pending":
        print("Setting the alarm status to \"active\"")
        data = "{alarm: active}"
        postRequest(url33, headers2, data)
        postRequest(url44, headers2, data)
        # activate counter-measures through the MQTT protocol
        mqttEmergency("1")
    # check if we're already in emergency mode and do nothing
    elif json.loads(r.content)["client"]["alarm"] == "active" and json.loads(p.content)["client"]["alarm"] == "active":
        print("Leaving the alarm status to \"active\"")
    # any other situation is presented as "normal mode" and counter-measuers are deactivated through MQTT or just
    # remain deactivated
    else:
        print("leavinf the alarm status as is")
        mqttEmergency("0")
    # get end alarm status
    getRequest(url11, headers1)
    getRequest(url22, headers1)
    print("done!")


if __name__ == '__main__':
    main()
