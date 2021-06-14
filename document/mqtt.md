use putty for macbook
@terminal 
$>ssh root@206.189.145.76 


command server to recieve mqtt msg
$> mosquitto_sub -h localhost -t "#" -u "khomkrit" -P "tonkla0709" // # recieve all topic income


command server to publish to device
$> mosquitto_pub -h localhost -t "/cmd/01-00001" -m {/"cmd/":1}  -u "khomkrit" -P "tonkla0709" 
   -h server
   -t topic
   -m message  send as text of json text

$> mosquitto_pub -h localhost -t "/cmd/01-00001" -m "{"cmd":1,"time":1}"  -u "khomkrit" -P "tonkla0709" 

$> osquitto_pub -h localhost -t "/cmd/01-00001" -q 2  -m "{"cmd":9,"time":1}"  -u "khomkrit" -P "tonkla0709  
  -q for Qos 1 2 
  


String to JSON 
https://arduinojson.org/v6/example/string/