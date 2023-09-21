# one time setup
mkinitramfs -o ramdisk.img
echo "add-auto-load-safe-path path/to/linux/scripts/gdb/vmlinux-gdb.py" >> ~/.gdbinit

# one time kernel setup
cd linux
./scripts/config -e DEBUG_INFO -e GDB_SCRIPTS
<make kernel image>

# each debug session run
qemu-system-x86_64 \
  -kernel arch/x86_64/boot/bzImage \
  -nographic \
  -append "console=ttyS0 nokaslr" \
  -initrd ramdisk.img \
  -m 512 \
  --enable-kvm \
  -cpu host \
  -s -S &
gdb vmlinux
