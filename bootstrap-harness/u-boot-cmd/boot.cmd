echo "Loading 9sun20i ..."
fatload mmc 0:1 ${kernel_addr_r} 9sun20i
go ${kernel_addr_r}
