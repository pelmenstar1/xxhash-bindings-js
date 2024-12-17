{
"targets": [
  {
    "target_name": "xxhash",
    "sources": [ 
      "native/index.cpp",
      "native/fileHashing.cpp",
      "native/hashers.cpp",
      "native/oneshots.cpp",
      "native/xxhash.c",
      "native/errorMacro.cpp",
      "native/v8Utils.cpp",
      "native/v8HashAdapter.cpp",
      "native/v8HashState.cpp",
      "native/stateHashing.cpp",
      "native/platform/blockReader.cpp",
      "native/platform/memoryMap.cpp",
      "native/platform/platformOperationStatus.cpp",
    ],
    "include_dirs": [
      "<!(node -e \"require('nan')\")"
    ],
  }
]
}