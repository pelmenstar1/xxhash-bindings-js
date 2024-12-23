{
"targets": [
  {
    "target_name": "xxhash-allnative",
    "sources": [ 
      "../../native/index.cpp",
      "../../native/fileHash.cpp",
      "../../native/oneshotHash.cpp",
      "../../native/createHashState.cpp",

      "../../native/hashers.cpp",
      "../../native/xxhash.c",
      "../../native/errorMacro.cpp",
      "../../native/v8Utils.cpp",
      "../../native/v8HashAdapter.cpp",
      "../../native/v8HashState.cpp",
     
      "../../native/platform/blockReader.cpp",
      "../../native/platform/memoryMap.cpp",
      "../../native/platform/platformOperationStatus.cpp",
    ],
    "include_dirs": [
      "<!(node -e \"require('nan')\")"
    ],
  }
]
}