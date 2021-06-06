array=(var1 var2 var3 var4)
array[0]=9.125.90.1
array[1]=9.125.90.2
array[2]=9.125.90.3
array[3]=9.125.90.4

# the array Number
echo ${#array[@]}
echo There are ${#array[@]} member in this array.
#first method to access the array.

echo method 1:
echo ${array[@] }
echo 

echo method 2:
echo ${array[0]}
echo ${array[1]}
echo ${array[2]}
echo ${array[3]}
echo 

echo method 3 & 4:
i_var=0
for var in ${array[@]} 
do
  echo $var
  echo ${array[$i_var]}
  let $((i_var++))
done

echo 
echo method 5
echo ${var1}
echo ${var2}

