# FM Partition Manager

This is a tool for managing [GPU partitions](https://docs.nvidia.com/datacenter/tesla/fabric-manager-user-guide/index.html#gpu-partitions) for NVIDIA Fabric Manager's Shared NVSwitch feature.

Management operations include listing, activating, and deactivating partitions.

```
$ ./fmpm -h
-l : List partitions
-a <partition ID> : Activate partition
-d <partition ID> : Deactivate partition
-v : Show version
```

To get started, install the NVIDIA Fabric Manager Development package on your build environment.  
On RHEL, this package is named "nvidia-fabricmanager-devel-\<version\>"  
On Ubuntu, this package is named "nvidia-fabricmanager-dev-\<version\>"

Install the JSON CPP development package on your build environment.  
On Ubuntu, this package is named "libjsoncpp-dev"  
On RHEL, this package is named "jsoncpp-devel".  
The [EPEL repository](https://www.redhat.com/en/blog/install-epel-linux) must be set up on your system to access this package.


Make to build the binary fmpm

## License

By downloading or using this software, I agree to the terms of the [LICENSE](LICENSE)
