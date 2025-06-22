#!/bin/bash

if [ "$QUERY_STRING" == "" ]; then
  export QUERY_STRING=$1
fi

export CLOCKID=`echo $QUERY_STRING | cut -d'&' -f1 | cut -d '=' -f2`
if [ ! -f ./${CLOCKID}.id ]
then
   send_error.sh
fi


# this will read and set all values
. ./real_params $QUERY_STRING


echo '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">'
echo '<html>'
echo '<head>'
echo '<meta http-equiv="refresh" content="120" />'
echo '<title>axvloxk maintenance page</title>'
echo '</head>'

echo '<body>'
echo '<div style="background-color:black;">'
echo '<p><div style="color=light-green"><font color="green">'
echo '<br> Axclock maintenance page '
date +%H:%M
echo '<br>'
echo '<br>'

echo '<table>'

echo '<tr><tb>ClockID</tb><tb>'$CLOCKID'</tb></tr>'
echo '<tr><tb>Timezone</tb><tb>'$TIMEZONE'</tb></tr>'
echo '<tr><tb>Clock color</tb><tb>'$CLOCK_COLOR'</tb></tr>'
echo '<tr><tb>Message color</tb><tb>'$MSG_COLOR'</tb></tr>'
echo '<tr><tb>Brightness</tb><tb>'$BRIGHTNESS'</tb></tr>'
echo '<tr><tb>Message loops</tb><tb>'$MSG_LOOPS'</tb></tr>'
echo '<tr><tb>Message time</tb><tb>'$MSG_TIME'</tb></tr>'
echo '<tr><tb>Alert</tb><tb>'$ALERT'</tb></tr>'
echo '<tr><tb>Alert time</tb><tb>'$ALERT_TIME'</tb></tr>'
echo '<tr><tb>Read time</tb><tb>'$READ_TIME'</tb></tr>'
echo '<tr><tb>Message</tb><tb>'$MESSAGE'</tb></tr>'

echo '</table>'



echo '<br>'
echo '</font></div></p>'
#echo '<br>'
echo '<hr>'
echo '<br>'
echo '<div style="color=light-green">'
echo 'Aktuell:<br>'
echo '<p style="font-size:550%;text-align:center"><font color="purple">'
echo $ACTUALTEMP 
echo '</font></p>'
echo '</div>'

echo '<br>'
#echo '<br><br>
echo '</div>'
echo '<br><br>'
echo '<form action="./set_values.sh">'
echo '  <label for="Clock settings:</label>'
echo '  <input type="text" id="clock_id" name="ClockID"><br><br>'
echo '  <input type="text" id="timezone" name="Timezone"><br><br>'
echo '  <input type="text" id="clock_color" name="Clock color"><br><br>'
echo '  <input type="text" id="msg_color" name="Message color"><br><br>'
echo '  <input type="text" id="brightness" name="Brightness"><br><br>'
echo '  <input type="text" id="mag_loops" name="Message loops"><br><br>'
echo '  <input type="text" id="alert" name="Alert"><br><br>'
echo '  <input type="text" id="message" name="message"><br><br>'
echo '  <input type="submit" value="Send">'
echo '</form>'
echo '</body>'
echo '</html>'
