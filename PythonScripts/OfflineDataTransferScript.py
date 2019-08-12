from pymongo import MongoClient
from datetime import datetime, timedelta
import requests
import time
import datetime
import pymongo
import re
import sys
import paho.mqtt.client as paho


# response function to check if request succeeded or not
def response(r):
    if int(str(r.status_code)[0]) == 2:
        print("Success! Status code is: " + str(r.status_code))
    else:
        print("Failure! Status code is: " + str(r.status_code))


# MQTT function when something is published
def on_publish(client, userdata, result):
    print("Data published!")


def main():
    # MQTT raspberry pi IP address and port
    broker = "$YOUR_BROKER_ADDRESS"
    port = 1883
    # time when connection with the server failed given as system argument
    print(sys.argv[1])
    print("Beginning data transfer sequence...")
    # parts of the server URL witch we will combine back together for further use
    url1 = 'http://ec2-52-59-207-8.eu-central-1.compute.amazonaws.com'
    url2 = '/api/v1/'
    url4 = '/telemetry'
    url5 = '/attributes'
    # header for cURL POST request
    headers = {'content-type': 'application/json'}
    # accessing MongoDB database
    # data preview:
    # (db = house, collection = room, document_example =
    # { "_id" : ObjectId("5d4042c044ae4d7c4539b97c"), "item" : "LessCoolESP8266_RawRoom1",
    # "realName" : "LessCoolESP8266_RawRoom1", "timestamp" : ISODate("2019-07-30T13:14:40.173Z"),
    # "value" : "{hum: nan, topic: home/room1/hum}" })
    clientdb = MongoClient('localhost', 27017)
    db = clientdb.house
    posts = db.room
    # getting current time
    now = (datetime.datetime.now())
    # getting the time when connection lost with server and adding 2 hours
    # because raspberry pi runs in GMT+00:00 timezone
    offset = timedelta(hours=2)
    then = datetime.datetime.strptime((sys.argv[1].split('+')[0] + 'Z'), "%Y-%m-%dT%H:%M:%S.%fZ") - offset
    # querying all of the data stored in the database since we went offline
    cursor = posts.find(
        {'timestamp': {'$lt': now, '$gte': then}}).sort('timestamp', pymongo.DESCENDING)
    counter = 0
    for docs in cursor:
        counter = counter + 1
        # extracting the sensor data from document
        splitter = str(docs['value']).replace(" ", "")
        splitter = re.split("[,{}]", splitter)
        payload = "{" + splitter[1] + "}"
        # extracting the topic from document and converting
        # it into the access key for the appropriate device in ThingsBoard
        topic = splitter[2].split(":")[1].split("/")
        url3 = ''
        for i in range(len(topic)):
            if i == 0:
                url3 = url3 + topic[i]
            else:
                url3 = url3 + 'u' + topic[i]
        # sending sensor data (telemetry) to the ThingsBoard device
        url = url1 + url2 + url3 + url4
        r = requests.post(url, data=payload, headers=headers)
        response(r)
        # sending time when we went offline
        url = url1 + url2 + url3 + url5
        payload = "{wentOffline: \"" + str(sys.argv[1]) + "\"}"
        print(payload)
        r = requests.post(url, data=payload, headers=headers)
        response(r)
        # sending the duration of lost connection
        payload = "{offlineDuration: \"" + str((now - then) - timedelta(hours=2)) + "\"}"
        r = requests.post(url, data=payload, headers=headers)
        response(r)
    print("number of data transferred: " + counter)
    print("Data transfer finished!")
    # sending conformation that openHAB is ready for more measurements
    client = paho.Client("feedback")
    client.on_publish = on_publish
    client.connect(broker, port)
    ret = client.publish("rasp/feedback", "1")
    print(ret)


if __name__ == '__main__':
    main()
