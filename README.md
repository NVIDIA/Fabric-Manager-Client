# FM Partition Manager

This is a tool for managing [GPU partitions](https://docs.nvidia.com/datacenter/tesla/fabric-manager-user-guide/index.html#gpu-partitions) for NVIDIA Fabric Manager's Shared NVSwitch feature.

Management operations include listing, activating, deactivating partitions, etc.

```
$ ./fmpm -h
-l : List partitions
-a <Partition ID> : Activate partition
-d <Partition ID> : Deactivate partition
--hostname : hostname or IP address (TCP socket) of Fabric Manager.
     If this option is not specified, the default is 127.0.0.1
     Requires FM_CMD_BIND_INTERFACE=<IP of Fabric Manager> in Fabric Manager configuration
     See details: https://docs.nvidia.com/datacenter/tesla/fabric-manager-user-guide/index.html#the-fabric-manager-api-interface
--unix-domain-socket : UNIX domain socket path for Fabric Manager connection
     If this option is specified, UNIX domain socket will be used instead of hostname or IP (TCP socket).
     Requires FM_CMD_UNIX_SOCKET_PATH=<UNIX domain socket> in Fabric manager configuration.
     See details: https://docs.nvidia.com/datacenter/tesla/fabric-manager-user-guide/index.html#the-fabric-manager-domain-socket-interface
--get-nvlink-failed-devices : Query all NVLink failed devices
--list-unsupported-partitions : Query all unsupported fabric partitions
--set-activated-list <Partition IDs> : Set a list of currently activated fabric partitions.
     Comma separated, no space! Used for Fabric Manager Resiliency Mode.
     Requires FABRIC_MODE_RESTART=1 for Fabric Manager resiliency restart mode.
     More details:
     https://docs.nvidia.com/datacenter/tesla/fabric-manager-user-guide/index.html#the-fabric-manager-restart-mode
     https://docs.nvidia.com/datacenter/tesla/fabric-manager-user-guide/index.html#resiliency
-v : Show version
```

To get started, install the NVIDIA Fabric Manager Development package on your build environment.  
On RHEL, this package is named "nvidia-fabricmanager-devel-\<version\>"  
On Ubuntu, this package is named "nvidia-fabricmanager-dev-\<version\>"

> [!IMPORTANT]
> Some builds of the Ubuntu package install the linker symlink
> `/usr/lib/x86_64-linux-gnu/libnvfm.so` without the `libnvfm.so.1` library it
> points at. The symlink alone does not satisfy `-lnvfm`, so the build fails
> with `cannot find -lnvfm`.
>
> You can check a package without installing it:
>
> ```
> apt-get download nvidia-fabricmanager-dev=<version>
> dpkg -c nvidia-fabricmanager-dev_<version>_amd64.deb | grep libnvfm
> ```
>
> A usable package lists both `libnvfm.so` and `libnvfm.so.1`. If only the
> symlink is listed, unpack a version-suffixed package and build against it
> where it lies, without installing it:
>
> ```
> apt-get download nvidia-fabricmanager-dev-575
> dpkg -x nvidia-fabricmanager-dev-575_575.57.08-1_amd64.deb lib575/
> make NVFM_LIB_DIRS=$PWD/lib575/usr/lib/x86_64-linux-gnu \
>      LDFLAGS="-L$PWD/lib575/usr/lib/x86_64-linux-gnu -lnvfm -ljsoncpp -Wl,-rpath,$PWD/lib575/usr/lib/x86_64-linux-gnu"
> ```
>
> Do not copy `libnvfm.so.1` into a system library directory instead. That puts
> a library from one release ahead of the Fabric Manager the machine is running,
> for every program on it. Behind `-rpath` it reaches only this binary.
>
> `NVFM_LIB_DIRS` is the list of directories checked for `libnvfm.so` before
> linking, and is also how to point the build at a development package installed
> anywhere outside the standard paths.

Install the JSON CPP development package on your build environment.  
On Ubuntu, this package is named "libjsoncpp-dev"  
On RHEL, this package is named "jsoncpp-devel".  
The [EPEL repository](https://www.redhat.com/en/blog/install-epel-linux) must be set up on your system to access this package.
