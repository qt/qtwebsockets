#cd test
#/usr/local/bin/wstest -m echoserver -w ws://localhost:9000 &
#/usr/bin/python echoserver.py &

./unittests -xunitxml -o ./unittest_result.xml

#stop server
#pid=$(ps -eo pid,command,lstart | grep '/usr/bin/python' | tail -1 | grep -e '^ (\d+)' -E -o | grep -e '(\d+)' -E -o)
#kill -9 $pid

