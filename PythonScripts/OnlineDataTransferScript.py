from pymongo import MongoClient
from datetime import datetime, timedelta
import requests
import time
import datetime
import re
import sys
import paho.mqtt.client as paho


def main():
    print("Uploading data to cloud...")
    # parts of the request URL witch we will combine back together for further use
    # Full Request url: "http://$THINGSBOARD_HOST/api/v1/$ACCESS_TOKEN/telemetry"
    url1 = '$THINGSBOARD_HOST'
    url2 = '/api/v1/'
    url4 = '/telemetry'
    # header for the cURL post request
    headers = {'content-type': 'application/json', 'Accept-Charset': 'UTF-8'}
    # the data sent from our sensor containing measurement and topic
    data = sys.argv[1]
    # extracting the sensor data from document
    splitter = str(data).replace(" ", "")
    splitter = re.split("[,{}]", splitter)
    payload = "{" + splitter[1] + "}"
    # extracting the topic from document and converting
    # it into access key for the appropriate device in thingsBoard
    topic = splitter[2].split(":")[1].split("/")
    url3 = ''
    for i in range(len(topic)):
        if i == 0:
            url3 = url3 + topic[i]
        else:
            url3 = url3 + 'u' + topic[i]
    # sending sensor data (telemetry) to the thingsBoard device
    url = url1 + url2 + url3 + url4
    r = requests.post(url, data=payload, headers=headers)
    # response function to check if request succeeded or not
    if int(str(r.status_code)[0]) == 2:
        print("success! status code is: " + str(r.status_code))
    else:
        print("gone wrong! status code is: " + str(r.status_code))
    print("done!")


if __name__ == '__main__':
    main()
