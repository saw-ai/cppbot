#!/bin/bash

date > /root/bot/date
ps aux | grep "/root/bot/bot" | grep -v "grep" | grep -v "bot.sh"
a=$?
echo $a >> /root/bot/date
if [ $a == 1 ]
then
	/root/bot/bot
fi
