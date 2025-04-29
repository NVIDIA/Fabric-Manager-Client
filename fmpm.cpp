#include <iostream>
#include <string.h>
#include <json/json.h>
#include "nv_fm_agent.h"

std::string version = "1.0.1";

#define MAX_PATH_LEN 256



typedef enum SharedFabricCmdEnum
{
    // This list must start at zero, and be in the same order
    // as the struct all_args list, below:
    SHARED_FABRIC_CMD_HELP = 0, /* Shows help output */

    SHARED_FABRIC_CMD_LIST_PARTITION,               /* Command-line to list partitions */
    SHARED_FABRIC_CMD_ACTIVATE_PARTITION,           /* Command-line to activate a partition */
    SHARED_FABRIC_CMD_DEACTIVATE_PARTITION,         /* Command-line to deactivate a partition */
    SHARED_FABRIC_CMD_SET_ACTIVATED_PARTITION_LIST, /* Command-line to set activated partition list */
    SHARED_FABRIC_CMD_GET_NVLINK_FAILED_DEVICES,    /* Command-line to get Nvlink failed devices */
    SHARED_FABRIC_CMD_LIST_UNSUPPORTED_PARTITION,   /* Command-line to list unsupported partitions */
    SHARED_FABRIC_CMD_HOSTNAME,                     /* Command-line to set host IP address to connect to */
    SHARED_FABRIC_CMD_UNIX_DOMAIN_SOCKET,           /* Command-line to set Unix domain socket to connect to */
    SHARED_FABRIC_CMD_CONVERT_BIN_TO_TXT,           /* Command-line to convert binary state file to text file */
    SHARED_FABRIC_CMD_CONVERT_TXT_TO_BIN,           /* Command-line to convert text state file to binary file */
    SHARED_FABRIC_CMD_COUNT               /* Always keep this last */
} SharedFabricCmdEnum_t;


fmReturn_t
parsePartitionIdlistString(std::string & partitionListStr, unsigned int * partitionIds, unsigned * numPartitions)
{
    char * token = strtok((char *)partitionListStr.c_str(), ",");
    int haveSeen[FM_MAX_FABRIC_PARTITIONS] = { 0 };

    *numPartitions = 0;

    while ( token != NULL )
    {
        int partId = (int)atoi(token);

        if ( partId < 0 || partId >= FM_MAX_FABRIC_PARTITIONS )
        {
            printf("Invalid partition Id %u was given.\n", partId);
            return FM_ST_BADPARAM;
        }

        if ( haveSeen[partId] )
            continue; /* Just ignore it and move on */
        haveSeen[partId] = 1;

        partitionIds[*numPartitions] = partId;
        *numPartitions++;

        token = strtok(NULL, ",");
    }

    return FM_ST_SUCCESS;
}

fmReturn_t
listPartitions(fmHandle_t fmHandle)
{
    fmReturn_t fmReturn;
    fmFabricPartitionList_t partitionList;

    memset(&partitionList, 0, sizeof(fmFabricPartitionList_t));
    partitionList.version = fmFabricPartitionList_version;

    fmReturn = fmGetSupportedFabricPartitions(fmHandle, &partitionList);
    if (fmReturn != FM_ST_SUCCESS) {
        std::cout << "Failed to get partition list. fmReturn: " << fmReturn << std::endl;
	return fmReturn;
    } else {
        Json::Value jsonPartitionList(Json::objectValue);
        Json::Value jsonPartitionInfoList(Json::arrayValue);
        Json::Value jsonPartitionInfo(Json::objectValue);
        Json::Value jsonPartitionGpuInfoList(Json::arrayValue);
        Json::Value jsonPartitionGpuInfo(Json::objectValue);

        jsonPartitionList["version"] = partitionList.version;
        jsonPartitionList["numPartitions"] = partitionList.numPartitions;
        jsonPartitionList["maxNumPartitions"] = partitionList.maxNumPartitions;
        jsonPartitionInfoList.clear();

        for ( unsigned int partIdx = 0; partIdx < partitionList.numPartitions; ++partIdx )
        {
            fmFabricPartitionInfo_t * partInfo = &partitionList.partitionInfo[partIdx];
            jsonPartitionGpuInfoList.clear();

            jsonPartitionInfo["partitionId"] = partInfo->partitionId;
            jsonPartitionInfo["isActive"] = partInfo->isActive;
            jsonPartitionInfo["numGpus"] = partInfo->numGpus;

            for ( unsigned int gpuIdx = 0; gpuIdx < partInfo->numGpus; ++gpuIdx )
            {
                fmFabricPartitionGpuInfo_t * gpuInfo = &partInfo->gpuInfo[gpuIdx];
                jsonPartitionGpuInfo["physicalId"] = gpuInfo->physicalId;
                jsonPartitionGpuInfo["uuid"] = gpuInfo->uuid;
                jsonPartitionGpuInfo["pciBusId"] = gpuInfo->pciBusId;
                jsonPartitionGpuInfo["numNvLinksAvailable"] = gpuInfo->numNvLinksAvailable;
                jsonPartitionGpuInfo["maxNumNvLinks"] = gpuInfo->maxNumNvLinks;
                jsonPartitionGpuInfo["nvlinkLineRateMBps"] = gpuInfo->nvlinkLineRateMBps;
                jsonPartitionGpuInfoList.append(jsonPartitionGpuInfo);
            }

            jsonPartitionInfo["gpuInfo"] = jsonPartitionGpuInfoList;
            jsonPartitionInfoList.append(jsonPartitionInfo);
        }

        jsonPartitionList["partitionInfo"] = jsonPartitionInfoList;

        Json::StyledWriter styledWriter;
        std::string sStyled = styledWriter.write(jsonPartitionList);
        fprintf(stdout, "%s", sStyled.c_str());
    }
    return fmReturn;
}

fmReturn_t
listUnsupportedPartitions(fmHandle_t fmHandle)
{
    fmReturn_t fmReturn;
    fmUnsupportedFabricPartitionList_t partitionList;

    memset(&partitionList, 0, sizeof(fmUnsupportedFabricPartitionList_t));
    partitionList.version = fmUnsupportedFabricPartitionList_version;

    fmReturn = fmGetUnsupportedFabricPartitions(fmHandle, &partitionList);
    if ( fmReturn != FM_ST_SUCCESS )
    {
        std::cout << "Failed to list unsupported partitions. fmReturn: " << fmReturn << std::endl;
        return fmReturn;
    }

    Json::Value jsonPartitionList(Json::objectValue);
    Json::Value jsonPartitionInfoList(Json::arrayValue);
    Json::Value jsonPartitionInfo(Json::objectValue);
    Json::Value jsonPartitionGpuList(Json::arrayValue);

    jsonPartitionList["version"] = partitionList.version;
    jsonPartitionList["numPartitions"] = partitionList.numPartitions;
    jsonPartitionInfoList.clear();

    for ( unsigned int partIdx = 0; partIdx < partitionList.numPartitions; ++partIdx )
    {
        fmUnsupportedFabricPartitionInfo_t * partInfo = &partitionList.partitionInfo[partIdx];
        jsonPartitionGpuList.clear();

        jsonPartitionInfo["partitionId"] = partInfo->partitionId;
        jsonPartitionInfo["numGpus"] = partInfo->numGpus;

        for ( unsigned int gpuIdx = 0; gpuIdx < partInfo->numGpus; ++gpuIdx )
        {
            jsonPartitionGpuList.append(partInfo->gpuPhysicalIds[gpuIdx]);
        }

        jsonPartitionInfo["gpuPhysicalIds"] = jsonPartitionGpuList;
        jsonPartitionInfoList.append(jsonPartitionInfo);
    }

    jsonPartitionList["partitionInfo"] = jsonPartitionInfoList;

    Json::StyledWriter styledWriter;
    std::string sStyled = styledWriter.write(jsonPartitionList);
    fprintf(stdout, "%s", sStyled.c_str());

    return fmReturn;
}

fmReturn_t
activatePartition(fmHandle_t fmHandle, fmFabricPartitionId_t partitionId)
{
    fmReturn_t fmReturn;

    fmReturn = fmActivateFabricPartition(fmHandle, partitionId);
    if (fmReturn != FM_ST_SUCCESS) {
        std::cout << "Failed to activate partition. fmReturn: " << fmReturn
            << std::endl;
    }
    return fmReturn;
}

fmReturn_t
deactivatePartition(fmHandle_t fmHandle, fmFabricPartitionId_t partitionId)
{
    fmReturn_t fmReturn;

    fmReturn = fmDeactivateFabricPartition(fmHandle, partitionId);
    if (fmReturn != FM_ST_SUCCESS) {
        std::cout << "Failed to deactivate partition. fmReturn: " << fmReturn << std::endl;
    }
    return fmReturn;
}

fmReturn_t
setActivatedPartitionList(fmHandle_t fmHandle, unsigned int * partIds, unsigned numPartitions)
{
    fmReturn_t fmReturn;
    fmActivatedFabricPartitionList_t partitionList;

    memset(&partitionList, 0, sizeof(fmActivatedFabricPartitionList_t));
    partitionList.version = fmActivatedFabricPartitionList_version;
    partitionList.numPartitions = numPartitions;

    for ( unsigned i = 0; i < numPartitions; i++ )
    {
        partitionList.partitionIds[i] = partIds[i];
    }

    fmReturn = fmSetActivatedFabricPartitions(fmHandle, &partitionList);
    if ( fmReturn != FM_ST_SUCCESS )
    {
        std::cout << "Failed to set activated partitions. fmReturn: " << fmReturn << std::endl;
    }

    return fmReturn;
}

fmReturn_t
getNvlinkFailedDevices(fmHandle_t fmHandle)
{
    fmReturn_t fmReturn;
    fmNvlinkFailedDevices_t nvlinkFailedDevice;

    memset(&nvlinkFailedDevice, 0, sizeof(fmNvlinkFailedDevices_t));
    nvlinkFailedDevice.version = fmNvlinkFailedDevices_version;

    fmReturn = fmGetNvlinkFailedDevices(fmHandle, &nvlinkFailedDevice);
    if ( fmReturn != FM_ST_SUCCESS )
    {
        std::cout << "Failed to NVLink failed devices. fmReturn: " << fmReturn << std::endl;
        return fmReturn;
    }

    Json::Value jsonNvlinkFailedDevices(Json::objectValue);
    Json::Value jsonNvlinkFailedGpuInfo(Json::objectValue);
    Json::Value jsonNvlinkFailedSwitchInfo(Json::objectValue);
    Json::Value jsonNvlinkFailedGpuInfoList(Json::arrayValue);
    Json::Value jsonNvlinkFailedSwitchInfoList(Json::arrayValue);
    Json::Value jsonPortList(Json::arrayValue);

    jsonNvlinkFailedDevices["version"] = nvlinkFailedDevice.version;
    jsonNvlinkFailedDevices["numGpus"] = nvlinkFailedDevice.numGpus;
    jsonNvlinkFailedDevices["numSwitches"] = nvlinkFailedDevice.numSwitches;
    jsonNvlinkFailedGpuInfoList.clear();
    jsonNvlinkFailedSwitchInfoList.clear();

    uint32_t i, j;
    for ( i = 0; i < nvlinkFailedDevice.numGpus; i++ )
    {
        fmNvlinkFailedDeviceInfo_t & gpuInfo = nvlinkFailedDevice.gpuInfo[i];
        jsonNvlinkFailedGpuInfo["uuid"] = gpuInfo.uuid;
        jsonNvlinkFailedGpuInfo["pciBusId"] = gpuInfo.pciBusId;
        jsonNvlinkFailedGpuInfo["numPorts"] = gpuInfo.numPorts;

        jsonPortList.clear();
        for ( j = 0; j < gpuInfo.numPorts; j++ )
        {
            jsonPortList.append(gpuInfo.portNum[j]);
        }
        jsonNvlinkFailedGpuInfo["portNum"] = jsonPortList;
        jsonNvlinkFailedGpuInfoList.append(jsonNvlinkFailedGpuInfo);
    }

    jsonNvlinkFailedDevices["gpuInfo"] = jsonNvlinkFailedGpuInfoList;

    for ( i = 0; i < nvlinkFailedDevice.numSwitches; i++ )
    {
        fmNvlinkFailedDeviceInfo_t & switchInfo = nvlinkFailedDevice.switchInfo[i];
        jsonNvlinkFailedSwitchInfo["uuid"] = switchInfo.uuid;
        jsonNvlinkFailedSwitchInfo["pciBusId"] = switchInfo.pciBusId;
        jsonNvlinkFailedSwitchInfo["numPorts"] = switchInfo.numPorts;

        jsonPortList.clear();
        for ( j = 0; j < switchInfo.numPorts; j++ )
        {
            jsonPortList.append(switchInfo.portNum[j]);
        }
        jsonNvlinkFailedSwitchInfo["portNum"] = jsonPortList;
        jsonNvlinkFailedSwitchInfoList.append(jsonNvlinkFailedSwitchInfo);
    }
    jsonNvlinkFailedDevices["switchInfo"] = jsonNvlinkFailedSwitchInfoList;

    Json::StyledWriter styledWriter;
    std::string sStyled = styledWriter.write(jsonNvlinkFailedDevices);
    fprintf(stdout, "%s", sStyled.c_str());

    return fmReturn;
}

int main(int argc, char **argv)
{
    fmReturn_t fmReturn;
    fmHandle_t fmHandle = NULL;
    unsigned int operation = SHARED_FABRIC_CMD_HELP;
    unsigned int partitionId = 0;

    char mHostname[MAX_PATH_LEN];        /* Hostname */
    char mUnixSockPath[MAX_PATH_LEN];    /* Unix domain socket path */
    unsigned mNumPartitions = 0;         /* number of partitions in mPartitionList */
    unsigned int mPartitionIds[FM_MAX_FABRIC_PARTITIONS];  /* List of partitionIds */

    if ((argc == 1) || ((argc == 2) && (std::string(argv[1]) == "-h"))) {
        std::cout << "-l : List partitions" << std::endl;
        std::cout << "-a <Partition ID> : Activate partition" << std::endl;
        std::cout << "-d <Partition ID> : Deactivate partition" << std::endl;
        std::cout << "--hostname : hostname or IP address (TCP socket) of Fabric Manager." << std::endl;
        std::cout << "     If this option is not specified, the default is 127.0.0.1" << std::endl;
        std::cout << "     Requires FM_CMD_BIND_INTERFACE=<IP of Fabric Manager> in Fabric Manager configuration" << std::endl;
        std::cout << "     See details: https://docs.nvidia.com/datacenter/tesla/fabric-manager-user-guide/index.html#the-fabric-manager-api-interface" << std::endl;
        std::cout << "--unix-domain-socket : UNIX domain socket path for Fabric Manager connection" << std::endl;
        std::cout << "     If this option is specified, UNIX domain socket will be used instead of hostname or IP (TCP socket)." << std::endl;
        std::cout << "     Requires FM_CMD_UNIX_SOCKET_PATH=<UNIX domain socket> in Fabric manager configuration." << std::endl;
        std::cout << "     See details: https://docs.nvidia.com/datacenter/tesla/fabric-manager-user-guide/index.html#the-fabric-manager-domain-socket-interface" << std::endl;
        std::cout << "--get-nvlink-failed-devices : Query all NVLink failed devices" << std::endl;
        std::cout << "--list-unsupported-partitions : Query all unsupported fabric partitions" << std::endl;
        std::cout << "--set-activated-list <Partition IDs> : Set a list of currently activated fabric partitions." << std::endl;
        std::cout << "     Comma separated, no space! Used for Fabric Manager Resiliency Mode." << std::endl;
        std::cout << "     Requires FABRIC_MODE_RESTART=1 for Fabric Manager resiliency restart mode." << std::endl;
        std::cout << "     More details:" << std::endl;
        std::cout << "     https://docs.nvidia.com/datacenter/tesla/fabric-manager-user-guide/index.html#the-fabric-manager-restart-mode" << std::endl;
        std::cout << "     https://docs.nvidia.com/datacenter/tesla/fabric-manager-user-guide/index.html#resiliency" << std::endl;
        std::cout << "-v : Show version" << std::endl;
        return 0;
    }

    /* Clear mUnixSockPath */
    memset(mUnixSockPath, 0, sizeof(mUnixSockPath));
    /* Default to connect to localhost */
    strncpy(mHostname, "127.0.0.1", sizeof(mHostname));

    for (int i = 1; i < argc; ++i) {

        if (std::string(argv[i]) == "-l") {
            operation = SHARED_FABRIC_CMD_LIST_PARTITION;
        } else if (std::string(argv[i]) == "-a") {
            operation = SHARED_FABRIC_CMD_ACTIVATE_PARTITION;
            if (i + 1 < argc) {
                partitionId = std::stoul(argv[i + 1]);
                ++i;
            } else {
                std::cerr << "Error: -a option requires partition ID." << std::endl;
                return 1;
            }
            if ( partitionId >= FM_MAX_FABRIC_PARTITIONS ) {
                std::cout << "Invalid Partition ID." << std::endl;
                return FM_ST_BADPARAM;
            }
        } else if (std::string(argv[i]) == "-d") {
            operation = SHARED_FABRIC_CMD_DEACTIVATE_PARTITION;
            if (i + 1 < argc) {
                partitionId = std::stoul(argv[i + 1]);
                ++i;
            } else {
                std::cerr << "Error: -d option requires partition ID." << std::endl;
                return FM_ST_BADPARAM;
            }
        } else if (std::string(argv[i]) == "--set-activated-list") {
            operation = SHARED_FABRIC_CMD_SET_ACTIVATED_PARTITION_LIST;
            if (i + 1 < argc) {
                std::string partitionListStr = std::string(argv[i + 1]);
                fmReturn_t ret = parsePartitionIdlistString(partitionListStr, mPartitionIds, &mNumPartitions);
                if ( ret != FM_ST_SUCCESS )
                {
                   return ret;
                }
                ++i;
            } else {
                std::cerr << "Error: --set-activated-list option requires list of partition IDs." << std::endl;
                return 1;
            }
        } else if (std::string(argv[i]) == "--get-nvlink-failed-devices") {
            operation =  SHARED_FABRIC_CMD_GET_NVLINK_FAILED_DEVICES;
        } else if (std::string(argv[i]) == "--list-unsupported-partitions") {
            operation = SHARED_FABRIC_CMD_LIST_UNSUPPORTED_PARTITION;
        } else if (std::string(argv[i]) == "--hostname") {
            if (i + 1 < argc) {
                std::string str = argv[i + 1];
                snprintf(mHostname, MAX_PATH_LEN, "%s", str.c_str());
                ++i;
            } else {
                std::cerr << "Error: --hostname option requires user specified IP or hostname." << std::endl;
                return 1;
            }
        } else if (std::string(argv[i]) == "--unix-domain-socket") {
            if (i + 1 < argc) {
                std::string str = argv[i + 1];
                snprintf(mUnixSockPath, MAX_PATH_LEN, "%s", str.c_str());
                ++i;
            } else {
                std::cerr << "Error: --unix-domain-socket option requires user specified socket value." << std::endl;
                return 1;
            }
        } else if (std::string(argv[i]) == "-v") {
            std::cout << version << std::endl;
            return 0;
        } else {
            std::cerr << "Invalid argument.\n" << std::endl;
            return FM_ST_BADPARAM;
        }
    }

    /* Initialize Fabric Manager API interface library */
    fmReturn = fmLibInit();
    if (FM_ST_SUCCESS != fmReturn) {
        std::cout << "Failed to initialize Fabric Manager API interface library." << std::endl;
        return fmReturn;
    }

    /* Connect to Fabric Manager instance */
    fmConnectParams_t connectParams;
    connectParams.timeoutMs = 1000; // in milliseconds
    connectParams.version = fmConnectParams_version;

    memset(connectParams.addressInfo, 0, sizeof(connectParams.addressInfo));
    if ( strnlen(mUnixSockPath, MAX_PATH_LEN) > 0 )
    {
        snprintf(connectParams.addressInfo, MAX_PATH_LEN, "%s", mUnixSockPath);
        connectParams.addressIsUnixSocket = 1;
    }
    if ( strnlen(mHostname, MAX_PATH_LEN) > 0 )
    {
        snprintf(connectParams.addressInfo, MAX_PATH_LEN, "%s", mHostname);
        connectParams.addressIsUnixSocket = 0;
    }


    strncpy(connectParams.addressInfo, mHostname, sizeof(mHostname));
    connectParams.addressIsUnixSocket = 0;

    fmReturn = fmConnect(&connectParams, &fmHandle);
    if (fmReturn != FM_ST_SUCCESS){
        std::cout << "Failed to connect to Fabric Manager instance." << std::endl;
        return fmReturn;
    }

    if ( operation == SHARED_FABRIC_CMD_LIST_PARTITION ) {
        /* List supported partitions */
        fmReturn = listPartitions(fmHandle);
    } else if ( operation == SHARED_FABRIC_CMD_ACTIVATE_PARTITION ) {
        /* Activate a partition */
        fmReturn = activatePartition(fmHandle, partitionId);
    } else if ( operation == SHARED_FABRIC_CMD_DEACTIVATE_PARTITION ) {
        /* Deactivate a partition */
        fmReturn = deactivatePartition(fmHandle, partitionId);
    } else if ( operation ==  SHARED_FABRIC_CMD_SET_ACTIVATED_PARTITION_LIST ) {
        /* Set activated partition list */
        fmReturn = setActivatedPartitionList(fmHandle, mPartitionIds, mNumPartitions);
    } else if ( operation ==  SHARED_FABRIC_CMD_GET_NVLINK_FAILED_DEVICES ) {
        /* Get NvLink failed devices */
        fmReturn = getNvlinkFailedDevices(fmHandle);
    } else if ( operation ==  SHARED_FABRIC_CMD_LIST_UNSUPPORTED_PARTITION ) {
        /* List Unsupported partitions */
        fmReturn = listUnsupportedPartitions(fmHandle);
    }


    /* Clean up */
    fmDisconnect(fmHandle);
    fmLibShutdown();
    return fmReturn;
}
