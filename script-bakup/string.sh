char="1234567890"
echo length is ${#char}
char1=${char//89/12}
echo $char1
char1=${char1//12/56}
echo $char1
echo
echo $char
