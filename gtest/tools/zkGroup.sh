#!/bin/sh

if [ "$1" == "start" ]
then
    zkServer.sh start /etc/zookeeper1/conf/zoo.cfg
    zkServer.sh start /etc/zookeeper2/conf/zoo.cfg
    zkServer.sh start /etc/zookeeper3/conf/zoo.cfg
    ps aux|grep zookeeper
elif [ "$1" == "stop" ]
then
    zkServer.sh stop /etc/zookeeper1/conf/zoo.cfg
    zkServer.sh stop /etc/zookeeper2/conf/zoo.cfg
    zkServer.sh stop /etc/zookeeper3/conf/zoo.cfg
fi

