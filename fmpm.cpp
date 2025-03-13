#include <iostream>
#include <string.h>
#include "nv_fm_agent.h"
#include <json/json.h>

std::string version = "1.0.0";

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
        std::cout << "Failed to deactivate partition. fmReturn: " << fmReturn
            << std::endl;
    }
    return fmReturn;
}

int main(int argc, char **argv)
{
    fmReturn_t fmReturn;
    fmHandle_t fmHandle = NULL;
    char hostIpAddress[] = "127.0.0.1";
    unsigned int operation = 0;
    fmFabricPartitionId_t partitionId = 0;

    if (std::string(argv[1]) == "-l" && argc == 2) {
        operation = 0;
    } else if (std::string(argv[1]) == "-a" && argc == 3) {
        operation = 1;
        partitionId = std::stoul(argv[2]);
        if ( partitionId >= FM_MAX_FABRIC_PARTITIONS ) {
            std::cout << "Invalid Partition ID." << std::endl;
            return FM_ST_BADPARAM;
        }
    } else if (std::string(argv[1]) == "-d" && argc == 3) {
        operation = 2;
        partitionId = std::stoul(argv[2]);
    } else if (std::string(argv[1]) == "-h" && argc == 2) {
        std::cout << "-l : List partitions" << std::endl;
        std::cout << "-a <Partition ID> : Activate partition" << std::endl;
        std::cout << "-d <Partition ID> : Deactivate partition" << std::endl;
        std::cout << "-v : Show version" <<std::endl;
        return 1;
    } else if (std::string(argv[1]) == "-v" && argc == 2) {
        std::cout << version << std::endl;
        return 1;
    } else {
        std::cout << "Invalid input.\n" << std::endl;
        return FM_ST_BADPARAM;
    }

    /* Initialize Fabric Manager API interface library */
    fmReturn = fmLibInit();
    if (FM_ST_SUCCESS != fmReturn) {
        std::cout << "Failed to initialize Fabric Manager API interface library." << std::endl;
        return fmReturn;
    }

    /* Connect to Fabric Manager instance */
    fmConnectParams_t connectParams;
    strncpy(connectParams.addressInfo, hostIpAddress, sizeof(hostIpAddress));
    connectParams.timeoutMs = 1000; // in milliseconds
    connectParams.version = fmConnectParams_version;
    connectParams.addressIsUnixSocket = 0;
    fmReturn = fmConnect(&connectParams, &fmHandle);
    if (fmReturn != FM_ST_SUCCESS){
        std::cout << "Failed to connect to Fabric Manager instance." << std::endl;
        return fmReturn;
    }

    if ( operation == 0 ) {
        /* List supported partitions */
        fmReturn = listPartitions(fmHandle);
    } else if ( operation == 1 ) {
        /* Activate a partition */
        fmReturn = activatePartition(fmHandle, partitionId);
    } else if ( operation == 2 ) {
        /* Deactivate a partition */
        fmReturn = deactivatePartition(fmHandle, partitionId);
    }

    /* Clean up */
    fmDisconnect(fmHandle);
    fmLibShutdown();
    return fmReturn;
}
