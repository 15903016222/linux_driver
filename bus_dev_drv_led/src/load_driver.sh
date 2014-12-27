echo "insmod or rmmod? input [ins | rm]"
read flag
if [ $flag = "ins" ];then
insmod led_drv.ko 
insmod led_dev.ko
elif [ $flag = "rm" ];then
rmmod led_drv
rmmod led_dev
else echo "please input [ins | rm]"
fi 
