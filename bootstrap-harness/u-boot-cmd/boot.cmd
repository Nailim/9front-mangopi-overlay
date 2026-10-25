echo "Loading plan9.ini ..."
mw.b 0x40800000 0 0x2000
if fatload mmc 0:1 0x40800000 plan9.ini; then
	echo plan9.ini loaded
fi

echo "Loading 9sun20i ..."
fatload mmc 0:1 ${kernel_addr_r} 9sun20i
go ${kernel_addr_r}
