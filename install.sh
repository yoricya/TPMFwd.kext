cd /tmp
sudo rm -r /tmp/TPMFwd.kext

cp -R ~/Library/Developer/Xcode/DerivedData/TPMFwd-*/Build/Products/Debug/TPMFwd.kext /tmp/TPMFwd.kext
sudo chown -R root:wheel /tmp/TPMFwd.kext
sudo chmod -R 755 /tmp/TPMFwd.kext

sudo kextutil -v /tmp/TPMFwd.kext
log show --predicate 'senderImagePath contains "TPM"' --info --last 2m   
